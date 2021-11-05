// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef FRESNEL_H
#define FRESNEL_H

inline float fresnel_r_s(float cos_theta1, float cos_theta2, float ior1, float ior2)
{
  // Compute the perpendicularly polarized component of the Fresnel reflectance
  return (ior1*cos_theta1 - ior2*cos_theta2) / (ior1*cos_theta1 + ior2*cos_theta2);
}

inline float fresnel_r_p(float cos_theta1, float cos_theta2, float ior1, float ior2)
{
  // Compute the parallelly polarized component of the Fresnel reflectance
  return (ior2*cos_theta2 - ior1*cos_theta1) / (ior2*cos_theta2 + ior1*cos_theta1);
}

inline float fresnel_R(float cos_theta1, float cos_theta2, float ior1, float ior2)
{
  // Compute the Fresnel reflectance using fresnel_r_s(...) and fresnel_r_p(...)
  auto r_s = fresnel_r_s(cos_theta1, cos_theta2, ior1, ior2);
  auto r_p = fresnel_r_p(cos_theta1, cos_theta2, ior1, ior2);
  return 0.5f * (r_s*r_s + r_p*r_p);
}

#endif // FRESNEL_H