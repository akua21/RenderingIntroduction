// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "ObjMaterial.h"
#include "fresnel.h"
#include "RayTracer.h"

using namespace optix;

bool RayTracer::trace_reflected(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
  // Initialize the reflected ray and trace it.
  //
  // Input:  in         (the ray to be reflected)
  //         in_hit     (info about the ray-surface intersection)
  //
  // Output: out        (the reflected ray)
  //         out_hit    (info about the reflected ray)
  //
  // Return: true if the reflected ray hit anything
  //
  // Hints: (a) There is a reflect function available in the OptiX math library.
  //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
  
  float3 r = reflect(in.direction, in_hit.shading_normal);
  out = Ray(in_hit.position, r, 0, 0.01f);


  bool does_hit = trace_to_closest(out, out_hit);

  if (does_hit)
  {
    out_hit.ray_ior = in_hit.ray_ior;
    out_hit.trace_depth = in_hit.trace_depth + 1;
    return true;
  }

  return false;
  
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit) const
{
  // Initialize the refracted ray and trace it.
  //
  // Input:  in         (the ray to be refracted)
  //         in_hit     (info about the ray-surface intersection)
  //
  // Output: out        (the refracted ray)
  //         out_hit    (info about the refracted ray)
  //
  // Return: true if the refracted ray hit anything
  //
  // Hints: (a) There is a refract function available in the OptiX math library.
  //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
  //        (c) Remember that the function must handle total internal reflection.
  
  /*
  bool refraction = refract(out.direction, in.direction, in_hit.shading_normal, in_hit.ray_ior);
  
  float3 dir = out.direction;
  out = Ray(in_hit.position, dir, 0, 0.01f);

  bool does_hit = trace_to_closest(out, out_hit);
  float3 out_normal;

  if (does_hit)
  {
    out_hit.ray_ior = get_ior_out(in, in_hit, out_normal);
    out_hit.trace_depth = in_hit.trace_depth + 1;
    return true;
  }
  else
  {
    return false;
  }*/

  
  float3 r;
  float3 out_normal;
  out_hit.ray_ior = get_ior_out(in, in_hit, out_normal);

  bool refraction = refract(r, in.direction, out_normal, (out_hit.ray_ior / in_hit.ray_ior));
  
  if (refraction)
  {
    out = Ray(in_hit.position, r, 0, 0.01f);
    bool does_hit = trace_to_closest(out, out_hit);

    if (does_hit)
    {
    out_hit.trace_depth = in_hit.trace_depth + 1;
    return true;
    }
  
  }  

  return false;
  
}

bool RayTracer::trace_refracted(const Ray& in, const HitInfo& in_hit, Ray& out, HitInfo& out_hit, float& R) const
{
  // Initialize the refracted ray and trace it.
  // Compute the Fresnel reflectance (see fresnel.h) and return it in R.
  //
  // Input:  in         (the ray to be refracted)
  //         in_hit     (info about the ray-surface intersection)
  //
  // Output: out        (the refracted ray)
  //         out_hit    (info about the refracted ray)
  //
  // Return: true if the refracted ray hit anything
  //
  // Hints: (a) There is a refract function available in the OptiX math library.
  //        (b) Set out_hit.ray_ior and out_hit.trace_depth.
  //        (c) Remember that the function must handle total internal reflection.
  R = 0.1;

  float3 r;
  float3 normal;

  float ior_1 = in_hit.ray_ior;
  float ior_2 = get_ior_out(in, in_hit, normal);
  bool total_internal_reflection = refract(r, in.direction, normal, ior_2/in_hit.ray_ior);
  if (!total_internal_reflection){
    R = 1.0;
  } 
  else {
    R = fresnel_R(dot(in.direction, normal), dot(r, normal), ior_1, ior_2);
  }

  return trace_refracted(in, in_hit, out, out_hit);
}

float RayTracer::get_ior_out(const Ray& in, const HitInfo& in_hit, float3& normal) const
{
  normal = in_hit.shading_normal;
	if(dot(normal, in.direction) > 0.0)
	{
    normal = -normal;
    return 1.0f;
  }
  const ObjMaterial* m = in_hit.material;
  return m ? m->ior : 1.0f;
}
