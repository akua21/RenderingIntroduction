#ifndef PATHTRACER_H
#define PATHTRACER_H

#include <optix_world.h>
#include "Scene.h"
#include "RayTracer.h"

class PathTracer : public RayTracer
{
public:
  PathTracer(unsigned int w, unsigned int h, Scene* s, unsigned int pixel_subdivs = 1)
    : RayTracer(w, h, s, pixel_subdivs)
  { }  

	virtual void update_pixel(unsigned int x, unsigned int y, float sample_number, optix::float3& L) const;
};

#endif // PATHTRACER_H
