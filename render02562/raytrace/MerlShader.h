// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef MERLSHADER_H
#define MERLSHADER_H

#include <vector>
#include <map>
#include <string>
#include <optix_world.h>
#include "PathTracer.h"
#include "HitInfo.h"
#include "Light.h"
#include "MerlTexture.h"
#include "Glossy.h"

class MerlShader : public Glossy
{
public:
  MerlShader(PathTracer* pathtracer, const std::vector<Light*>& light_vector, int max_trace_depth = 500)
    : Glossy(pathtracer, light_vector, max_trace_depth), brdfs(0)
  { }

  virtual optix::float3 shade(const optix::Ray& r, HitInfo& hit, bool emit = true) const;

  void set_brdfs(std::map<std::string, MerlTexture*>& brdf_texs) { brdfs = &brdf_texs; }

protected:
  std::map<std::string, MerlTexture*>* brdfs;
};

#endif // MERLSHADER_H
