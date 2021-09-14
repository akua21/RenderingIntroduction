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
