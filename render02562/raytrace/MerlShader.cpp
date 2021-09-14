// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "mt_random.h"
#include "sampler.h"
#include "HitInfo.h"
#include "ObjMaterial.h"
#include "MerlTexture.h"
#include "MerlShader.h"

using namespace optix;

#ifndef M_1_PIf
#define M_1_PIf 0.31830988618379067154
#endif

float3 MerlShader::shade(const Ray& r, HitInfo& hit, bool emit) const
{
  if(hit.trace_depth >= max_depth)
    return make_float3(0.0f);

  const ObjMaterial* m = hit.material;
  MerlTexture* tex = brdfs && m && m->has_texture ? (*brdfs)[m->tex_name] : 0;
  float3 rho_d = get_diffuse(hit);
  float3 result = make_float3(0.0f);

  // Implement a path tracing shader here.
  //
  // Input:  r          (the ray that hit the material)
  //         hit        (info about the ray-surface intersection)
  //         emit       (passed on to Emission::shade)
  //
  // Return: radiance reflected to where the ray was coming from
  //
  // Relevant data fields that are available:
  // tracer             (pointer to ray tracer)
  //
  // Hint: Use the function tex->brdf_lookup(...) to retrieve the value of
  //       the measured BRDF for a given light-view configuration. Ensure
  //       that tex is non-zero and has texture before using it.

  return result;
}
