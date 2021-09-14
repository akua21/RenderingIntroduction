// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <optix_world.h>
#include "BRDF.h"
#include "MerlTexture.h"

using namespace std;
using namespace optix;

void MerlTexture::load(const char* filename)
{
  const unsigned int MERL_SIZE = BRDF_SAMPLING_RES_THETA_H*BRDF_SAMPLING_RES_THETA_D*BRDF_SAMPLING_RES_PHI_D/2*3;
  float3 rho_d;
  brdf.resize(MERL_SIZE);
  if(!read_brdf(filename, &brdf[0], MERL_SIZE, rho_d))
    cerr << "Error reading file " << filename << endl;
  width = height = 1;
  channels = 3;
  data = (unsigned char *)malloc(width*height*channels);
  data[0] = static_cast<unsigned char>(rho_d.x*255 + 0.5);
  data[1] = static_cast<unsigned char>(rho_d.y*255 + 0.5);
  data[2] = static_cast<unsigned char>(rho_d.z*255 + 0.5);

  delete [] fdata;
  fdata = new float4[1];
  fdata[0] = make_float4(rho_d);
  tex_handle = SOIL_create_OGL_texture(data, width, height, channels, tex_handle, SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS);
  tex_target = GL_TEXTURE_2D;
}

float4 MerlTexture::sample_nearest(const float3& texcoord) const
{
  if(!fdata)
    return make_float4(0.0f);
  else
    return fdata[0];
}

float4 MerlTexture::sample_linear(const float3& texcoord) const
{
  return sample_nearest(texcoord);
}

float3 MerlTexture::brdf_lookup(const float3& n, const float3& normalized_wi, const float3& normalized_wo) const
{
  return lookup_brdf_val(&brdf[0], n, normalized_wi, normalized_wo);
}
