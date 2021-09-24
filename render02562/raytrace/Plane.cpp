// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "Plane.h"

using namespace optix;

bool Plane::intersect(const Ray& r, HitInfo& hit, unsigned int prim_idx) const
{
	float t = -(dot(r.origin, onb.m_normal) + d) / dot(r.direction, onb.m_normal);

	if (r.tmin < t && t < r.tmax)
	{
		hit.has_hit = true;
		hit.dist = t;
		hit.material = &material;
    hit.geometric_normal = onb.m_normal;
    hit.shading_normal = onb.m_normal;
    hit.position = r.origin + r.direction * t;


    if (material.has_texture)
    {
      float u;
      float v;
      get_uv(hit.position, u, v);
      hit.texcoord = make_float3(u, v, 0);
    }


		return true;
	}


  // Implement ray-plane intersection here.
  // It is fine to intersect with the front-facing side of the plane only.
  //
  // Input:  r                    (the ray to be checked for intersection)
  //         prim_idx             (index of the primitive element in a collection, not used here)
  //
  // Output: hit.has_hit          (set true if the ray intersects the plane)
  //         hit.dist             (distance from the ray origin to the intersection point)
  //         hit.position         (coordinates of the intersection point)
  //         hit.geometric_normal (the normalized normal of the plane)
  //         hit.shading_normal   (the normalized normal of the plane)
  //         hit.material         (pointer to the material of the plane)
  //        (hit.texcoord)        (texture coordinates of intersection point, not needed for Week 1)
  //
  // Return: True if the ray intersects the plane, false otherwise 
  //         (do not return hit.has_hit as it is shared between all primitives and remains true once it is set true)
  //
  // Relevant data fields that are available (see Plane.h and OptiX math library reference)
  // r.origin                     (ray origin)
  // r.direction                  (ray direction)
  // r.tmin                       (minimum intersection distance allowed)
  // r.tmax                       (maximum intersection distance allowed)
  // position                     (origin of the plane)
  // onb                          (orthonormal basis of the plane: normal [n], tangent [b1], binormal [b2])
  // d                            (displacement of the plane, d in the equation of the plane)
  // material                     (material of the plane)
  //
  // Hint: The OptiX math library has a function dot(v, w) which returns
  //       the dot product of the vectors v and w.

  return false;
}

void Plane::transform(const Matrix4x4& m)
{
  onb = Onb(normalize(make_float3(m*make_float4(onb.m_normal, 0.0f))));
  position = make_float3(m*make_float4(position, 1.0f));
  d = -dot(position, onb.m_normal);
}

Aabb Plane::compute_bbox() const
{
  return Aabb(make_float3(-1e37f), make_float3(1e37f));
}

void Plane::get_uv(const float3& hit_pos, float& u, float& v) const 
{ 
  // Do an inverse mapping from hit position to texture coordinates.
  //
  // Input:  hit_pos    (position where a ray intersected the plane)
  //
  // Output: u          (texture coordinate in the tangent direction of the plane)
  //         v          (texture coordinate in the binormal direction of the plane)
  //
  // Relevant data fields that are available (see Plane.h and OptiX math library reference)
  // position           (origin of the plane)
  // onb                (orthonormal basis of the plane: normal [n], tangent [b1], binormal [b2])
  // tex_scale          (constant for scaling the texture coordinates)

  // u = 0.0f;
  // v = 0.0f;

  u = (dot((hit_pos - position), onb.m_tangent))*tex_scale;
  v = (dot((hit_pos - position), onb.m_binormal))*tex_scale;

  
}