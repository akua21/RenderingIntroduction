#ifndef PANORAMICTEXTURE_H
#define PANORAMICTEXTURE_H

#include <optix_world.h>
#include "SphereTexture.h"

class PanoramicTexture : public SphereTexture
{
public:
  virtual void project_direction(const optix::float3& d, float& u, float& v) const;
};

#endif // PANORAMICTEXTURE_H