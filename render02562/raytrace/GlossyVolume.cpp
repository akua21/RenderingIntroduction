// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "int_pow.h"
#include "GlossyVolume.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 GlossyVolume::shade(const Ray& r, HitInfo& hit, bool emit) const
{
  // Compute the specular part of the glossy shader and attenuate it
  // by the transmittance of the material if the ray is inside (as in
  // the volume shader).

  if (hit.trace_depth >= max_depth){
    return make_float3(0.0f);
  }
  
  // Remove lambertian from  Phong
  float3 rho_s = get_specular(hit);
  float s = get_shininess(hit);

  float3 result = make_float3(0.0f);
  float3 w_i, L_i;

  optix::float3 normal = hit.shading_normal;
  optix::float3 w_o = -r.direction;
  
  for(auto light : lights){
    bool not_in_shadow = light->sample(hit.position, w_i, L_i);

    if (not_in_shadow){
      optix::float3 w_r = reflect(-w_i, normal);
      optix::float3 normal = hit.shading_normal;

      float3 value = (0.5f * rho_s * (s+2) * M_1_PIf * pow(fmaxf(0.0f, dot(w_o, w_r)), s)) * L_i * fmaxf(0.0f, dot(w_i, normal));
      result += value;
    }
  }

  float R;
  Ray reflected;
  Ray refracted;
  HitInfo hit_reflected;
  HitInfo hit_refracted;

  tracer->trace_reflected(r, hit, reflected, hit_reflected);
  tracer->trace_refracted(r, hit, refracted, hit_refracted, R);
  
  optix::float3 glossy = R*shade_new_ray(reflected, hit_reflected) + (1.0f - R)*shade_new_ray(refracted, hit_refracted) + R * result;
  optix::float3 t = get_transmittance(hit);

  float inside = dot(r.direction, hit.shading_normal) > 0.0f;
  if (inside){
    return expf(-t*hit.dist) * glossy;
  } 
  else{
    return glossy;
  }
  return Volume::shade(r, hit, emit);
}
