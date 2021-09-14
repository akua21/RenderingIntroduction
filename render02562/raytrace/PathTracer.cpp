#include <optix_world.h>
#include "mt_random.h"
#include "PathTracer.h"

using namespace optix;

void PathTracer::update_pixel(unsigned int x, unsigned int y, float sample_number, float3& L) const
{
  float2 ip_coords = make_float2(x + mt_random(), y + mt_random())*win_to_ip + lower_left;
  Ray r = scene->get_camera()->get_ray(ip_coords);
  HitInfo hit;

  L *= sample_number;
  if(trace_to_closest(r, hit))
  {
    const Shader* shader = get_shader(hit);
    if(shader)
      L += shader->shade(r, hit);
  }
  else
    L += get_background(r.direction);
  L /= sample_number + 1.0f;
}

