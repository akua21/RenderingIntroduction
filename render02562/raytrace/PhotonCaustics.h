// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef PHOTONCAUSTICS_H
#define PHOTONCAUSTICS_H

#include <vector>
#include <optix_world.h>
#include "HitInfo.h"
#include "ParticleTracer.h"
#include "Light.h"
#include "Lambertian.h"

class PhotonCaustics : public Lambertian
{
public:
  PhotonCaustics(ParticleTracer* particle_tracer, 
                 const std::vector<Light*>& light_vector, 
                 float max_distance_in_estimate,
                 int no_of_photons_in_estimate) 
    : Lambertian(light_vector),
      tracer(particle_tracer),
      max_dist(max_distance_in_estimate), 
      photons(no_of_photons_in_estimate)
  { }

  virtual optix::float3 shade(const optix::Ray& r, HitInfo& hit, bool emit = true) const;

protected:
  ParticleTracer* tracer;
  float max_dist;
  int photons;
};

#endif // PHOTONCAUSTICS_H