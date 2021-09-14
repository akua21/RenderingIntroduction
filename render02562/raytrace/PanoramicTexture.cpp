#include <optix_world.h>
#include "PanoramicTexture.h"

using namespace std;
using namespace optix;

void PanoramicTexture::project_direction(const float3& d, float& u, float& v) const
{
  u = 0.5f + 0.5f*atan2(d.x, -d.z)*M_1_PIf;
  v = acos(-d.y)*M_1_PIf;
}


