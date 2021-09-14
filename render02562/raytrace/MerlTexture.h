// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2019
// Copyright (c) DTU Informatics 2019

#ifndef MERLTEXTURE_H
#define MERLTEXTURE_H

#include <vector>
#include <optix_world.h>
#include "Texture.h"

class MerlTexture : public Texture
{
public:
  MerlTexture() : Texture() { }
  ~MerlTexture() { free(static_cast<void*>(data)); data = 0; delete [] fdata; fdata = 0; }

  // Load texture from file
  void load(const char* filename);

  // Look up the texel using texture space coordinates
  virtual optix::float4 sample_nearest(const optix::float3& texcoord) const;
  virtual optix::float4 sample_linear(const optix::float3& texcoord) const;

  optix::float3 brdf_lookup(const optix::float3& n, const optix::float3& normalized_wi, const optix::float3& normalized_wo) const;

protected:
  // Pointers to image data
  std::vector<float> brdf;
  optix::float3 rho_d;
};

#endif // MERLTEXTURE_H
