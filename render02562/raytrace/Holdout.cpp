// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2019
// Copyright (c) DTU Compute 2019

#include <optix_world.h>
#include "HitInfo.h"
#include "sampler.h"
#include "Holdout.h"

using namespace optix;

float3 Holdout::shade(const Ray& r, HitInfo& hit, bool emit) const
{
  float ambient = 0.0f;

  // Implement ambient occlusion here.
  //
  // Input:  r    (the ray that hit the material)
  //         hit  (info about the ray-surface intersection)
  //         emit (passed on to Emission::shade)
  //
  // Return: radiance reflected to where the ray was coming from
  //
  // Relevant data fields that are available (see Holdout.h):
  // samples      (number of times to trace a sample ray)
  // tracer       (pointer to ray tracer)
  //
  // Hint: Use the function tracer->trace_to_closest(...) to trace
  //       a new ray in a direction sampled on the hemisphere around the
  //       surface normal according to the function sample_cosine_weighted(...).
  // float3 rho_d = get_diffuse(hit);
  // optix::float3 wj;
  // float3 Li = make_float3(0.0f, 0.0f, 0.0f);
  // Ray ray;

  // for (unsigned i = 0; i < samples; i++) {
  //   wj = sample_cosine_weighted(hit.shading_normal);
  //   ray = Ray(hit.position, wj, 0, 0.01f);
  //   HitInfo hit_ray = HitInfo();
  //   tracer->trace_to_closest(ray, hit_ray);

  //   Li += shade_new_ray(ray, hit_ray, false);

  //   ambient += Li;
  // }

  return ambient*tracer->get_background(r.direction);
}
