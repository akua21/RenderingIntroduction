// 02576 Rendering Framework
// Written by Jeppe Revall Frisvad, 2012
// Copyright (c) DTU Informatics 2012

#ifndef SAMPLER_H
#define SAMPLER_H

#include <cmath>
#include <optix_world.h>
#include "mt_random.h"

// Given a direction vector v sampled on the hemisphere
// over a surface point with the z-axis as its normal,
// this function applies the same rotation to v as is
// needed to rotate the z-axis to the actual normal
// [Frisvad, Journal of Graphics Tools 16, 2012;
//  Duff et al., Journal of Computer Graphics Techniques 6, 2017].
inline void rotate_to_normal(const optix::float3& normal, optix::float3& v)
{
  const float sign = copysignf(1.0f, normal.z);
  const float a = -1.0f/(1.0f + fabsf(normal.z));
  const float b = normal.x*normal.y*a;
  v =   optix::make_float3(1.0f + normal.x*normal.x*a, b, -sign*normal.x)*v.x 
      + optix::make_float3(sign*b, sign*(1.0f + normal.y*normal.y*a), -normal.y)*v.y 
      + normal*v.z;
}

inline void onb(const optix::float3& normal, optix::float3& b1, optix::float3& b2)
{
  const float sign = copysignf(1.0f, normal.z);
  const float a = -1.0f/(1.0f + fabsf(normal.z));
  const float b = normal.x*normal.y*a;
  b1 = optix::make_float3(1.0f + normal.x*normal.x*a, b, -sign*normal.x);
  b2 = optix::make_float3(sign*b, sign*(1.0f + normal.y*normal.y*a), -normal.y);
}

// Given spherical coordinates, where theta is the 
// polar angle and phi is the azimuthal angle, this
// function returns the corresponding direction vector
inline optix::float3 spherical_direction(double sin_theta, double cos_theta, double phi)
{
  return optix::make_float3(static_cast<float>(sin_theta*cos(phi)),
                            static_cast<float>(sin_theta*sin(phi)),
                            static_cast<float>(cos_theta));
}

inline optix::float3 sample_hemisphere(const optix::float3& normal)
{
  // Get random numbers

  // Calculate new direction as if the z-axis were the normal

  // Rotate from z-axis to actual normal and return
  return optix::make_float3(0.0f);
}

inline optix::float3 sample_cosine_weighted(const optix::float3& normal)
{
  // Get random numbers

  // Calculate new direction as if the z-axis were the normal

  // Rotate from z-axis to actual normal and return
  return optix::make_float3(0.0f);
}

inline optix::float3 sample_Phong_distribution(const optix::float3& normal, const optix::float3& dir, float shininess)
{
  // Get random numbers

  // Calculate sampled direction as if the z-axis were the reflected direction

  // Rotate from z-axis to actual reflected direction
  return optix::make_float3(0.0f);
}

inline optix::float3 sample_Blinn_distribution(const optix::float3& normal, const optix::float3& dir, float shininess)
{
  // Get random numbers

  // Calculate sampled half-angle vector as if the z-axis were the normal

  // Rotate from z-axis to actual normal

  // Make sure that the half-angle vector points in the right direction

  // Return the reflection of "dir" around the half-angle vector
  return optix::make_float3(0.0f);
}

inline optix::float3 sample_isotropic()
{
  // Use rejection sampling to find an arbitrary direction
  return optix::make_float3(0.0f);
}

inline optix::float3 sample_HG(const optix::float3& forward, double g)
{
  // Get random numbers

  // Calculate new direction as if the z-axis were the forward direction

  // Rotate from z-axis to forward direction
  return optix::make_float3(0.0f);
}

#endif
