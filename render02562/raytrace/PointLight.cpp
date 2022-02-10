// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <optix_world.h>
#include "HitInfo.h"
#include "mt_random.h"
#include "PointLight.h"

using namespace optix;

bool PointLight::sample(const float3& pos, float3& dir, float3& L) const
{
  // Compute output and return value given the following information.
  //
  // Input:  pos (the position of the geometry in the scene)
  //
  // Output: dir (the direction toward the light)
  //         L   (the radiance received from the direction dir)
  //
  // Return: true if not in shadow
  //
  // Relevant data fields that are available (see PointLight.h and Light.h):
  // shadows    (on/off flag for shadows)
  // tracer     (pointer to ray tracer)
  // light_pos  (position of the point light)
  // intensity  (intensity of the emitted light)
  //
  // Hint: Construct a shadow ray using the Ray datatype. Trace it using the
  //       pointer to the ray tracer.

  if (shadows)
  {
    dir = normalize(light_pos - pos);

    HitInfo hit;
    Ray shadow_ray = Ray(pos, dir, 0, 0.01f);

    tracer->trace_to_any(shadow_ray, hit);

    if (hit.has_hit)
    {
      return false;
    }
    
    L = intensity / pow(length(light_pos - pos), 2);
  }
  else 
  {
    L = intensity / pow(length(light_pos - pos), 2);
  }

  return true;
  
}

bool PointLight::emit(Ray& r, HitInfo& hit, float3& Phi) const
{
  // Emit a photon by creating a ray, tracing it, and computing its flux.
  //
  // Output: r    (the photon ray)
  //         hit  (the photon ray hit info)
  //         Phi  (the photon flux)
  //
  // Return: true if the photon hits a surface
  //
  // Relevant data fields that are available (see PointLight.h and Light.h):
  // tracer     (pointer to ray tracer)
  // light_pos  (position of the point light)
  // intensity  (intensity of the emitted light)
  //
  // Hint: When sampling the ray direction, use the function
  //       mt_random() to get a random number in [0,1].

  // Sample ray direction and create ray
  float3 direction;

  do {
    r.direction.x = 2.0f*mt_random() - 1.0f;
    r.direction.y = 2.0f*mt_random() - 1.0f;
    r.direction.z = 2.0f*mt_random() - 1.0f;
  } 
  while (dot(r.direction, r.direction) > 1.0f);

  r.direction = normalize(r.direction);
  r.origin = light_pos;
  r.tmin = 0.0f;
  r.tmax = RT_DEFAULT_MAX;

  // Trace ray
  tracer->trace_to_closest(r, hit);
  
  // If a surface was hit, compute Phi and return true
  if (hit.has_hit) {
    Phi = 4 * M_PI * intensity;
    return true;
  }  
  return false;
}
