// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2019
// Copyright (c) DTU Compute 2019

#ifndef HOLDOUT_H
#define HOLDOUT_H

#include <optix_world.h>
#include "HitInfo.h"
#include "Light.h"
#include "PathTracer.h"
#include "MCGlossy.h"

class Holdout : public MCGlossy
{
public:
  Holdout(PathTracer* path_tracer, const std::vector<Light*>& light_vector, unsigned int no_of_samples)
    : MCGlossy(path_tracer, light_vector, 10), samples(no_of_samples)
  { }

  virtual optix::float3 shade(const optix::Ray& r, HitInfo& hit, bool emit = true) const;

protected:
  unsigned int samples;
};

#endif // HOLDOUT_H
