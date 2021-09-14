// 02562 Rendering Framework
// Inspired by BSP tree in GEL (http://www.imm.dtu.dk/GEL/)
// BSP tree in GEL originally written by Bent Dalgaard Larsen.
// This file written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <vector>
#include <optix_world.h>
#include "AccObj.h"
#include "Object3D.h"
#include "HitInfo.h"
#include "BspTree.h"

using namespace std;
using namespace optix;

namespace
{
  const float f_eps = 1.0e-6f;  
  const float d_eps = 1.0e-12f;
}

BspTree::~BspTree()
{
  delete_node(root);
}

void BspTree::init(const vector<Object3D*>& geometry, const std::vector<const Plane*>& scene_planes)
{
  root = new BspNode;
  Accelerator::init(geometry, scene_planes);
  for(unsigned int i = 0; i < geometry.size(); ++i)
    bbox.include(geometry[i]->compute_bbox());
  vector<AccObj*> objects = primitives;
  subdivide_node(*root, bbox, 0, objects);
}

bool BspTree::closest_hit(Ray& r, HitInfo& hit) const
{
  // Call closest_plane(...) and intersect_node(...) instead of
  // Accelerator::closest_hit(...) to use the BSP tree.
  // Using intersect_min_max(...) before intersect_node(...) gives
  // a good speed-up in many scenes.

  return Accelerator::closest_hit(r, hit);
}

bool BspTree::any_hit(Ray& r, HitInfo& hit) const
{
  // Call any_plane(...) and intersect_node(...) instead of
  // Accelerator::any_hit(...) to use the BSP tree.
  // Using intersect_min_max(...) before intersect_node(...) gives
  // a good speed-up in many scenes.
  
  return Accelerator::any_hit(r, hit);
}

bool BspTree::intersect_min_max(Ray& r) const
{
  float3 p1 = (bbox.m_min - r.origin)/r.direction;
  float3 p2 = (bbox.m_max - r.origin)/r.direction;
  float3 pmin = fminf(p1, p2);
  float3 pmax = fmaxf(p1, p2);
  float tmin = fmaxf(pmin);
  float tmax = fminf(pmax);
  if(tmin > tmax || tmin > r.tmax || tmax < r.tmin)
    return false;
  r.tmin = optix::fmaxf(tmin - 1.0e-4f, r.tmin);
  r.tmax = optix::fminf(tmax + 1.0e-4f, r.tmax);
  return true;
}

void BspTree::subdivide_node(BspNode& node, Aabb& bbox, unsigned int level, vector<AccObj*>& objects) 
{
  const int TESTS = 4;

  // This is a recursive function building the BSP tree.
  //
  // Input:  node            (the node to be subdivided if it does not fulfil a stop criterion)
  //         bbox            (bounding box of the geometry to be stored in the node)
  //         level           (subdivision level of the node)
  //         objects         (array of pointers to primitive objects)
  //
  // Output: node.axis_leaf  (flag signalling if the node is a leaf or which axis it was split in, always set)
  //         node.plane      (displacement along the axis of the splitting plane, set if not leaf)
  //         node.left       (pointer to the left node in the next level of the tree, set if not leaf)
  //         node.right      (pointer to the right node in the next level of the tree, set if not leaf)
  //         node.id         (index pointing to the primitive objects associated with the node, set if leaf)
  //         node.count      (number of primitive objects associated with the node, set if leaf)
  //
  // Relevant data fields that are available (see BspTree.h)
  // max_objects             (maximum number of primitive objects in a leaf, stop criterion)
  // max_level               (maximum subdivision level, stop criterion)
  // tree_objects            (array for storing primitive objects associated with leaves)
  // 
  //
  // Hint: Finding a good way of positioning the splitting planes is hard.
  //       When positioning a plane, try a couple of places inside the bounding
  //       box along each axis and compute a cost for each tested placement.
  //       You can use the number of primitives times the bounding box area
  //       as the cost of a node and take the sum of the left and right nodes
  //       to estimate the cost of a particular plane position. After all the
  //       tests, use the plane position with minimum cost.

  if(objects.size() <= max_objects || level == max_level)
  {
    node.axis_leaf = bsp_leaf; // Means that this is a leaf
    node.id = tree_objects.size();
    node.count = objects.size();

    tree_objects.resize(tree_objects.size() + objects.size());
    for(unsigned int i = 0; i < objects.size(); ++i)
      tree_objects[node.id + i] = objects[i];
  }
  else
  {
    Aabb left_bbox = bbox;
    Aabb right_bbox = bbox;
    vector<AccObj*> left_objects;
    vector<AccObj*> right_objects;
    node.left = new BspNode;
    node.right = new BspNode;

    double min_cost = 1.0e27;
    for(unsigned int i = 0; i < 3; ++i)
    {
      for(unsigned int k = 1; k < TESTS; ++k)
      {
        left_bbox = bbox;
        right_bbox = bbox;

        float max_corner = *(&bbox.m_max.x + i);
        float min_corner = *(&bbox.m_min.x + i);
        float center = (max_corner - min_corner)*k/static_cast<float>(TESTS)+min_corner;

        *(&left_bbox.m_max.x + i) = center;
        *(&right_bbox.m_min.x + i) = center;

        // Try putting the triangles in the left and right boxes
        unsigned int left_count = 0;
        unsigned int right_count = 0;
        for(unsigned int j = 0; j < objects.size(); ++j)
        {
          AccObj* obj = objects[j];
          left_count += left_bbox.intersects(obj->bbox);
          right_count += right_bbox.intersects(obj->bbox);
        }

        double cost = left_count*left_bbox.area() + right_count*right_bbox.area();
        if(cost < min_cost)
        {
          min_cost = cost;
          node.axis_leaf = static_cast<BspNodeType>(i);
          node.plane = center;
          node.left->count = left_count;
          node.right->count = right_count;
        }
      }
    }

    // Now chose the right splitting plane
    float max_corner = *(&bbox.m_max.x + node.axis_leaf);
    float min_corner = *(&bbox.m_min.x + node.axis_leaf);
    float size = max_corner - min_corner;
    float center = node.plane;
    float diff = f_eps < size/8.0f ? size/8.0f : f_eps;

    if(node.left->count == 0)
    {
      // Find min position of all triangle vertices and place the center there
      center = max_corner;
      for(unsigned int j = 0; j < objects.size(); ++j)
      {
        AccObj* obj = objects[j];
        float obj_min_corner = *(&obj->bbox.m_min.x + node.axis_leaf);
        if(obj_min_corner < center)
          center = obj_min_corner;
      }
      center -= diff;
    }
    if(node.right->count == 0)
    {
      // Find max position of all triangle vertices and place the center there
      center = min_corner;
      for(unsigned int j = 0; j<objects.size(); ++j)
      {
        AccObj* obj = objects[j];
        float obj_max_corner = *(&obj->bbox.m_max.x + node.axis_leaf);
        if(obj_max_corner > center)
          center = obj_max_corner;
      }
      center += diff;
    }

    node.plane = center;
    left_bbox = bbox;
    right_bbox = bbox;
    *(&left_bbox.m_max.x + node.axis_leaf) = center;
    *(&right_bbox.m_min.x + node.axis_leaf) = center;

    // Now put the triangles in the right and left node
    for(unsigned int i = 0; i < objects.size(); ++i)
    {
      AccObj* obj = objects[i];
      if(left_bbox.intersects(obj->bbox))
        left_objects.push_back(obj);
      if(right_bbox.intersects(obj->bbox))
        right_objects.push_back(obj);
    }

    objects.clear();
    subdivide_node(*node.left, left_bbox, level + 1, left_objects);
    subdivide_node(*node.right, right_bbox, level + 1, right_objects);
  }
}

bool BspTree::intersect_node(Ray& ray, HitInfo& hit, const BspNode& node) const 
{
  // This is a recursive function computing ray-scene intersection
  // using the BSP tree.
  //
  // Input:  ray       (ray to find the first intersection for)
  //         node      (node of the BSP tree to intersect with)
  //
  // Output: ray.tmin  (minimum distance to intersection after considering the node)
  //         ray.tmax  (maximum distance to intersection after considering the node)
  //         hit       (hit info retrieved from primitive intersection function)
  //
  // Relevant data fields that are available (see BspTree.h)
  // tree_objects      (array of primitive objects associated with leaves)
  // 
  //
  // Hint: Stop the recursion once a leaf node has been found and get
  //       access to the intersect function of a primitive object through
  //       the geometry field.

  if(node.axis_leaf == bsp_leaf)
  {
    bool found = false;
    for(unsigned int i = 0; i < node.count; ++i)
    {
      const AccObj* obj = tree_objects[node.id + i];
      if(obj->geometry->intersect(ray, hit, obj->prim_idx))
      {
        ray.tmax = hit.dist;
        found = true;
      }
    }
    return found;
  }
  else
  {
    BspNode *near_node;
    BspNode *far_node;
    float axis_direction = *(&ray.direction.x + node.axis_leaf);
    float axis_origin = *(&ray.origin.x + node.axis_leaf);
    if(axis_direction >= 0.0f)
    {
      near_node = node.left;
      far_node = node.right;
    }
    else
    {
      near_node = node.right;
      far_node = node.left;
    }

    // In order to avoid instability
    float t;
    if(fabs(axis_direction) < d_eps)
      t = (node.plane - axis_origin)/d_eps; // intersect node plane;
    else
      t = (node.plane - axis_origin)/axis_direction; // intersect node plane;

    if(t > ray.tmax)
      return intersect_node(ray, hit, *near_node);
    else if(t < ray.tmin)
      return intersect_node(ray, hit, *far_node);
    else
    {
      float t_max = ray.tmax;
      ray.tmax = t;
      if(intersect_node(ray, hit, *near_node))
        return true;
      else
      {
        ray.tmin = t;
        ray.tmax = t_max;
        return intersect_node(ray, hit, *far_node);
      }
    }
  }
}

void BspTree::delete_node(BspNode *node) 
{
  if(node)
  {
    if(node->left)
      delete_node(node->left);
    if(node->right)
      delete_node(node->right);
    delete node;
  }
}
