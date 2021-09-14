// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <list>
#include <string>
#include <optix_world.h>
#include "mt_random.h"
#include "my_glut.h"
#include "string_utils.h"
#include "ObjMaterial.h"
#include "TriMesh.h"
#include "IndexedFaceSet.h"
#include "obj_load.h"
#include "Plane.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Light.h"
#include "AreaLight.h"
#include "HitInfo.h"
#include "Texture.h"
#include "RayTracer.h"
#include "InvSphereMap.h"
#include "Scene.h"

#ifdef _OPENMP
  #include <omp.h>
#endif

using namespace std;
using namespace optix;

Scene::~Scene()
{
  for(unsigned int i = 0; i < objects.size(); ++i)
    delete objects[i];
  for(unsigned int i = 0; i < planes.size(); ++i)
    delete planes[i];
  for(unsigned int i = 0; i < light_meshes.size(); ++i)
    delete light_meshes[i];
  for(unsigned int i = 0; i < extracted_lights.size(); ++i)
    delete lights[extracted_lights[i]];
  for(map<string, Texture*>::iterator i = textures.begin(); i != textures.end(); ++i)
  {
    delete i->second;
    i->second = 0;
  }
  for(map<string, MerlTexture*>::iterator i = brdfs.begin(); i != brdfs.end(); ++i)
    delete i->second;
}

void Scene::load_mesh(const string& filename, const Matrix4x4& transform)
{
  cout << "Loading " << filename << endl;

  TriMesh* mesh = new TriMesh; 
  obj_load(filename, *mesh);
  if(!mesh->has_normals())
  {
    cout << "Computing normals" << endl;
    mesh->compute_normals();
  }
  mesh->transform(transform);
  mesh->compute_areas();
  cout << "No. of triangles: " << mesh->geometry.no_faces() << endl;
  meshes.push_back(mesh);
  objects.push_back(mesh);
  transforms.push_back(Matrix4x4::identity());

  // Correct scene bounding box
  Aabb mesh_bbox = mesh->compute_bbox();
  bbox.include(mesh_bbox);
}

void Scene::load_texture(const ObjMaterial& mat, bool is_sphere)
{
  if(mat.has_texture && textures.find(mat.tex_name) == textures.end())
  {
    string path_and_name = mat.tex_path + mat.tex_name;
    list<string> file_type;
    split(mat.tex_name, file_type, ".");
    if(file_type.back() == string("binary"))
    {
      MerlTexture*& brdf = brdfs[mat.tex_name];
      brdf = new MerlTexture;
      textures[mat.tex_name] = brdf;
      brdf->load(path_and_name.c_str());
    }
    else
    {
      Texture*& tex = textures[mat.tex_name];
      tex = is_sphere ? new InvSphereMap : new Texture;
      tex->load(path_and_name.c_str());
    }
  }
}

void Scene::load_textures()
{
  for(unsigned int i = 0; i < meshes.size(); ++i)
    for(unsigned int j = 0; j < meshes[i]->materials.size(); ++j)
      load_texture(meshes[i]->materials[j]);
  for(unsigned int i = 0; i < planes.size(); ++i)
    load_texture(planes[i]->get_material());
  for(unsigned int i = 0; i < spheres.size(); ++i)
    load_texture(spheres[i]->get_material(), true);
  for(unsigned int i = 0; i < triangles.size(); ++i)
    load_texture(triangles[i]->get_material());
}

void Scene::add_plane(const float3& position, const float3& normal, const string& mtl_file, unsigned int idx, float tex_scale)
{
  vector<ObjMaterial> m;
  if(!mtl_file.empty())
    mtl_load(mtl_file, m);
  if(m.size() == 0)
	  m.push_back(ObjMaterial());
  Plane* plane = new Plane(position, normal, idx < m.size() ? m[idx] : m.back(), tex_scale);
  planes.push_back(plane);
}

void Scene::add_sphere(const float3& center, float radius, const string& mtl_file, unsigned int idx)
{
  vector<ObjMaterial> m;
  if(!mtl_file.empty())
    mtl_load(mtl_file, m);
  if(m.size() == 0)
	  m.push_back(ObjMaterial());
  Sphere* sphere = new Sphere(center, radius, idx < m.size() ? m[idx] : m.back());
  spheres.push_back(sphere);
  objects.push_back(sphere);
  bbox.include(sphere->compute_bbox());
}

void Scene::add_triangle(const float3& v0, const float3& v1, const float3& v2, const string& mtl_file, unsigned int idx)
{
  vector<ObjMaterial> m;
  if(!mtl_file.empty())
    mtl_load(mtl_file, m);
  if(m.size() == 0)
	  m.push_back(ObjMaterial());
  Triangle* triangle = new Triangle(v0, v1, v2, idx < m.size() ? m[idx] : m.back());
  triangles.push_back(triangle);
  objects.push_back(triangle);
  bbox.include(triangle->compute_bbox());
}

unsigned int Scene::extract_area_lights(RayTracer *tracer, unsigned int samples_per_light)
{
  for(unsigned int i = 0; i < meshes.size(); ++i)
  {
    TriMesh* mesh = new TriMesh;
    const vector<int>& indices = meshes[i]->mat_idx;
    for(unsigned int j = 0; j < indices.size(); ++j)
    {
      const ObjMaterial& mat = meshes[i]->materials[indices[j]];
      if(mat.name == "default")
        continue;

      bool emissive = false;
      for(unsigned int k = 0; k < 3; ++k)
        emissive = emissive || (mat.ambient[k] > 0.0f);

      if(emissive)
      {
    	  uint3 g_face = meshes[i]->geometry.face(j);
        g_face.x = mesh->geometry.add_vertex(meshes[i]->geometry.vertex(g_face.x));
        g_face.y = mesh->geometry.add_vertex(meshes[i]->geometry.vertex(g_face.y));
        g_face.z = mesh->geometry.add_vertex(meshes[i]->geometry.vertex(g_face.z));
        int idx = mesh->geometry.add_face(g_face);
        if(meshes[i]->normals.no_faces() > j)
        {
          uint3 n_face = meshes[i]->normals.face(j);
          n_face.x = mesh->normals.add_vertex(meshes[i]->normals.vertex(n_face.x));
          n_face.y = mesh->normals.add_vertex(meshes[i]->normals.vertex(n_face.y));
          n_face.z = mesh->normals.add_vertex(meshes[i]->normals.vertex(n_face.z));
          mesh->normals.add_face(n_face, idx);
        }
        if(meshes[i]->texcoords.no_faces() > j)
        {
          uint3 t_face = meshes[i]->texcoords.face(j);
          t_face.x = mesh->texcoords.add_vertex(meshes[i]->texcoords.vertex(t_face.x));
          t_face.y = mesh->texcoords.add_vertex(meshes[i]->texcoords.vertex(t_face.y));
          t_face.z = mesh->texcoords.add_vertex(meshes[i]->texcoords.vertex(t_face.z));
          mesh->texcoords.add_face(t_face, idx);
        }
        
        bool material_exists = false;
        for(unsigned int k = 0; k < mesh->materials.size(); ++k)
          if(mesh->materials[k].name == meshes[i]->materials[indices[j]].name)
          {
            mesh->mat_idx.push_back(k);
            material_exists = true;
            break;
          }
        if(!material_exists)
        {
          mesh->mat_idx.push_back(mesh->materials.size());
          mesh->materials.push_back(meshes[i]->materials[indices[j]]);
        }
      }
    }
    if(mesh->geometry.no_faces() == 0)
      delete mesh;
    else
    {
      mesh->compute_areas();
      light_meshes.push_back(mesh);
      extracted_lights.push_back(lights.size());
      lights.push_back(new AreaLight(tracer, mesh, samples_per_light));
    }
  }
  return lights.size();
}

void Scene::toggle_shadows()
{
  for(unsigned int i = 0; i < lights.size(); ++i)
    lights[i]->toggle_shadows();
}

void Scene::set_shader(int model, Shader* s)
{
  if(model < 0)
  {
    cout << "Negative identification numbers are not supported for illumination models." << endl;
    return;
  }
  while(model >= static_cast<int>(shaders.size()))
    shaders.push_back(0);
  shaders[model] = s;
}

const Shader* Scene::get_shader(const HitInfo& hit) const
{
  unsigned int model = hit.material->illum;
  if(model < shaders.size())
    return shaders[model];
  return 0;
}

void Scene::get_bsphere(float3& c, float& r) const
{
  c = bbox.center();
  r = length(bbox.extent())*0.5f;
}

void Scene::draw()
{
  static unsigned int disp_list = 0;
  if(redraw)
  {
    cout << "Generating scene display list";
    if(glIsList(disp_list))
      glDeleteLists(disp_list, 1);
    disp_list = glGenLists(1);
    glNewList(disp_list, GL_COMPILE);

    for(unsigned int i = 0; i < meshes.size(); ++i)
      draw_mesh(meshes[i]);
    for(unsigned int i = 0; i < planes.size(); ++i)
      draw_plane(planes[i]);
    for(unsigned int i = 0; i < spheres.size(); ++i)
      draw_sphere(spheres[i]);
    for(unsigned int i = 0; i < triangles.size(); ++i)
      draw_triangle(triangles[i]);

    glEndList();
    redraw = false;
    cout << endl;
  }
  glCallList(disp_list);
}

void Scene::init_accelerator()
{
  acc.init(objects, planes);
}

bool Scene::is_specular(const ObjMaterial* m) const
{
  return m && ((m->illum > 1 && m->illum < 10) || m->illum > 10);
}

void Scene::draw_mesh(const TriMesh* mesh) const
{
  const IndexedFaceSet& geometry = mesh->geometry;
  const IndexedFaceSet& normals = mesh->normals;
  vector<float3*> shades(geometry.no_vertices(), static_cast<float3*>(0));
  const int faces = geometry.no_faces();
  const unsigned int indices = faces*3;
  vector<float3> verts(indices);
  vector<float3> norms(indices);
  vector<float3> colors(indices);
  #pragma omp parallel for private(randomizer)
  for(int i = 0; i < faces; ++i)
  {
    const unsigned int* g_face = &geometry.face(i).x;
    const unsigned int* n_face = &normals.face(i).x;
    for(unsigned int j = 0; j < 3; ++j)
    {
      unsigned int idx = i*3 + j;
      verts[idx] = geometry.vertex(g_face[j]);
      norms[idx] = normalize(normals.vertex(n_face[j]));
      colors[idx] = make_float3(0.5f);

      if(!shades[g_face[j]])
      {
        const ObjMaterial* m = &mesh->materials[mesh->mat_idx[i]];
        unsigned int model = m->illum;
        if(model < shaders.size() && shaders[model])
        {
          float3 ray_vec = verts[idx] - cam->get_position();
          Ray r(cam->get_position(), normalize(ray_vec), 0, 0.0f);
          HitInfo hit;
          hit.has_hit = true;
          hit.dist = length(ray_vec);
          hit.position = verts[idx];
          hit.geometric_normal = hit.shading_normal = norms[idx];
          hit.material = m;
          hit.texcoord = make_float3(0.0f);
          colors[idx] = shaders[model]->shade(r, hit);
        }
        if(!shades[g_face[j]])
        #pragma omp critical (store_shade)
        {
          shades[g_face[j]] = new float3(colors[idx]);
        }
      }
      else
        colors[idx] = *shades[g_face[j]];
    }
    if(faces > 100 && (i + 1) % (faces/10) == 0)
      cout << ".";
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  
  glVertexPointer(3, GL_FLOAT, 0, &verts[0].x);
  glNormalPointer(GL_FLOAT, 0, &norms[0].x);
  glColorPointer(3, GL_FLOAT, 0, &colors[0].x);

  glDrawArrays(GL_TRIANGLES, 0, indices);

  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  for(unsigned int i = 0; i < shades.size(); ++i)
    delete shades[i];
}

void Scene::draw_plane(const Plane* plane)
{
  if(!plane)
    return;

  float3 c;
  float rad;
  get_bsphere(c, rad);
  rad *= 100.0f;
  float3 shade = make_float3(0.5f);
  float3 vert = plane->get_origin();
  float3 v0 = vert - rad*plane->get_tangent() - rad*plane->get_binormal();
  float3 v1 = vert + rad*plane->get_tangent() - rad*plane->get_binormal();
  float3 v2 = vert - rad*plane->get_tangent() + rad*plane->get_binormal();
  float3 v3 = vert + rad*plane->get_tangent() + rad*plane->get_binormal();
  const float3& normal = plane->get_normal();
  const ObjMaterial* m = &plane->get_material();
  unsigned int model = m->illum;
  if(model < shaders.size() && shaders[model])
  {
    float3 ray_vec = vert - cam->get_position();
    float dist = length(ray_vec);
    Ray r(cam->get_position(), ray_vec/dist, 0, 0.0f);
    HitInfo hit;
    hit.has_hit = true;
    hit.dist = dist;
    hit.position = vert;
    hit.shading_normal = normal;
    hit.material = m;
    hit.texcoord = make_float3(0.0f);
    shade = shaders[model]->shade(r, hit);
  }
  const Texture* tex = do_textures && m->has_texture ? textures[m->tex_name] : 0;
  if(tex)
  {
    tex->enable();
    tex->bind();
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  }
  float2 uv;
  glBegin(GL_TRIANGLE_STRIP);
    glNormal3fv(&normal.x);
    glColor3fv(&shade.x);
    plane->get_uv(v0, uv.x, uv.y);
    glTexCoord2fv(&uv.x);
    glVertex3fv(&v0.x);
    plane->get_uv(v1, uv.x, uv.y);
    glTexCoord2fv(&uv.x);
    glVertex3fv(&v1.x);
    plane->get_uv(v2, uv.x, uv.y);
    glTexCoord2fv(&uv.x);
    glVertex3fv(&v2.x);
    plane->get_uv(v3, uv.x, uv.y);
    glTexCoord2fv(&uv.x);
    glVertex3fv(&v3.x);
  glEnd();
  if(tex)
    tex->disable();
}

void Scene::draw_sphere(const Sphere *sphere) const
{
  const ObjMaterial* m = &sphere->get_material();
  glColor3fv(m->diffuse);
  const float3& pos = sphere->get_center();
  glTranslatef(pos.x, pos.y, pos.z);
  glutSolidSphere(sphere->get_radius(), 64, 64);
  glTranslatef(-pos.x, -pos.y, -pos.z);
}

void Scene::draw_triangle(const Triangle *triangle) const
{
  const float3* pvert = triangle->get_vertices();
  glBegin(GL_POLYGON);
    for(unsigned int i = 0; i < 3; ++i)
    {
      float3 vertex = *(pvert + i);	  
      float3 shade = make_float3(0.5f);

      float3 normal = normalize(triangle->compute_normal());
      glNormal3fv(&normal.x);

      const ObjMaterial* m = &triangle->get_material();
      unsigned int model = m->illum;
      if(model < shaders.size() && shaders[model])
      {
        float3 ray_vec = vertex - cam->get_position();
        Ray r(cam->get_position(), normalize(ray_vec), 0, 0.0f);
        HitInfo hit;
        hit.has_hit = true;
        hit.dist = length(ray_vec);
        hit.position = vertex;
        hit.shading_normal = normal;
        hit.material = m;
        hit.texcoord = make_float3(0.0f);
        shade = shaders[model]->shade(r, hit);
      }
      glColor3fv(&shade.x);
      glVertex3fv(&vertex.x);
    }    
  glEnd();
}