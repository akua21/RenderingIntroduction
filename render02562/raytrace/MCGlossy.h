// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef MCGLOSSY_H
#define MCGLOSSY_H

#include <vector>
#include <optix_world.h>
#include "PathTracer.h"
#include "HitInfo.h"
#include "Light.h"
#include "Glossy.h"

class MCGlossy : public Glossy
{
public:
  MCGlossy(PathTracer* pathtracer, const std::vector<Light*>& light_vector, int max_trace_depth = 500) 
    : Glossy(pathtracer, light_vector, max_trace_depth) 
  { }

  virtual optix::float3 shade(const optix::Ray& r, HitInfo& hit, bool emit = true) const;
};

#endif // MCGLOSSY_H
