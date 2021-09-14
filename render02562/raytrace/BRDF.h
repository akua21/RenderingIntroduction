// Copyright 2005 Mitsubishi Electric Research Laboratories All Rights Reserved.

// Modified by Jeppe Revall Frisvad, 2019

// Permission to use, copy and modify this software and its documentation without
// fee for educational, research and non-profit purposes, is hereby granted, provided
// that the above copyright notice and the following three paragraphs appear in all copies.

// To request permission to incorporate this software into commercial products contact:
// Vice President of Marketing and Business Development;
// Mitsubishi Electric Research Laboratories (MERL), 201 Broadway, Cambridge, MA 02139 or 
// <license@merl.com>.

// IN NO EVENT SHALL MERL BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL,
// OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
// ITS DOCUMENTATION, EVEN IF MERL HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

// MERL SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED
// HEREUNDER IS ON AN "AS IS" BASIS, AND MERL HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
// UPDATES, ENHANCEMENTS OR MODIFICATIONS.
#ifndef BRDF_H
#define BRDF_H

#include <optix_world.h>

// Read BRDF data
bool read_brdf(const char *filename, float* brdf, unsigned int size, optix::float3& rho_d);

// Convert vectors in tangent space to half/difference coordinates
void vectors_to_half_diff_coords(const optix::float3& in, const optix::float3& out, float& theta_half, float& phi_half, float& theta_diff, float& phi_diff);

// Given a pair of incoming/outgoing angles, look up the BRDF.
optix::float3 lookup_brdf_val(const float* brdf, float theta_half, float phi_half, float theta_diff, float phi_diff);

// Given surface normal, direction of incidence, and direction of emergence, look up the BRDF
optix::float3 lookup_brdf_val(const float* brdf, const optix::float3& n, const optix::float3& normalized_wi, const optix::float3& normalized_wo);

#define BRDF_SAMPLING_RES_THETA_H   90
#define BRDF_SAMPLING_RES_THETA_D   90
#define BRDF_SAMPLING_RES_PHI_D     360

#define RED_SCALE (1.0f/1500.0f)
#define GREEN_SCALE (1.15f/1500.0f)
#define BLUE_SCALE (1.66f/1500.0f)

// Lookup theta_half index
// This is a non-linear mapping!
// In:  [0 .. pi/2]
// Out: [0 .. 89]
inline unsigned int theta_half_index(float theta_half)
{
  theta_half = fmaxf(theta_half, 0.0f);
  float theta_half_deg = theta_half*M_1_PIf*2.0f*BRDF_SAMPLING_RES_THETA_H;
  float temp = sqrtf(theta_half_deg*BRDF_SAMPLING_RES_THETA_H);
  int idx = static_cast<int>(temp);
  return optix::min(idx, BRDF_SAMPLING_RES_THETA_H - 1);
}

// Lookup theta_diff index
// In:  [0 .. pi/2]
// Out: [0 .. 89]
inline unsigned int theta_diff_index(float theta_diff)
{
  theta_diff = fmaxf(theta_diff, 0.0f);
  int idx = static_cast<int>(theta_diff*M_1_PIf*2.0f*BRDF_SAMPLING_RES_THETA_D);
  return optix::min(idx, BRDF_SAMPLING_RES_THETA_D - 1);
}

// Lookup phi_diff index
inline unsigned int phi_diff_index(float phi_diff)
{
  // Because of reciprocity, the BRDF is unchanged under
  // phi_diff -> phi_diff + M_PIf
  phi_diff = phi_diff < 0.0f ? phi_diff + M_PIf : phi_diff;
  phi_diff = fmaxf(phi_diff, 0.0f);

  // In: phi_diff in [0 .. pi]
  // Out: tmp in [0 .. 179]
  int half_res = BRDF_SAMPLING_RES_PHI_D/2;
  int idx = static_cast<int>(phi_diff*M_1_PIf*half_res);
  return optix::min(idx, half_res - 1);
}

inline optix::float3 lookup_brdf_val(const float* brdf, float theta_half, float phi_half, float theta_diff, float phi_diff)
{
  // Find index.
  // Note that phi_half is ignored, since isotropic BRDFs are assumed
  unsigned int idx = phi_diff_index(phi_diff)
    + theta_diff_index(theta_diff)*BRDF_SAMPLING_RES_PHI_D/2
    + theta_half_index(theta_half)*BRDF_SAMPLING_RES_PHI_D/2*BRDF_SAMPLING_RES_THETA_D;

  optix::float3 result;
  result.x = brdf[idx]*RED_SCALE;
  result.y = brdf[idx + BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D/2]*GREEN_SCALE;
  result.z = brdf[idx + BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D]*BLUE_SCALE;
  return fmaxf(result, optix::make_float3(0.0f));
}

#endif // BRDF_H