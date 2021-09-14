#ifndef PANORAMICLIGHT_H
#define PANORAMICLIGHT_H

#include <string>
#include <valarray>
#include <optix_world.h>
#include "RayTracer.h"
#include "Distribution2D.h"
#include "PanoramicTexture.h"
#include "Light.h"

class PanoramicLight : public Light
{
public:
  PanoramicLight(RayTracer* ray_tracer, const PanoramicTexture& panoramic, unsigned int no_of_samples = 1);
  ~PanoramicLight() { delete distribution; }

  virtual bool sample(const optix::float3& pos, optix::float3& dir, optix::float3& L) const;
  virtual bool emit(optix::Ray& r, HitInfo& hit, optix::float3& Phi) const;

  std::string describe() const;

protected:
  const PanoramicTexture& envtex;
  std::string env_filename;
  Distribution2D* distribution;
};

#endif // PANORAMICLIGHT_H
