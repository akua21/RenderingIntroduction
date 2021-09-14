#include <string>
#include <sstream>
#include <list>
#include <valarray>
#include <optix_world.h>
#include "string_utils.h"
#include "RayTracer.h"
#include "mt_random.h"
#include "sampler.h"
#include "luminance.h"
#include "Light.h"
#include "PanoramicLight.h"

using namespace std;
using namespace optix;

namespace
{
  const float M_2PIf = 2.0f*M_PIf;
  const float M_2PIPIf = M_2PIf*M_PIf;
}

PanoramicLight::PanoramicLight(RayTracer* ray_tracer, const PanoramicTexture& panoramic, unsigned int no_of_samples)
  : Light(ray_tracer, no_of_samples), envtex(panoramic), env_filename(""), distribution(0)
{
  /*
  if(!env_filename.empty())
  {
    list<string> dot_split;
    split(env_filename, dot_split, ".");
    if(dot_split.back() == "hdr")
      envtex.load_hdr(env_filename.c_str());
    else
      envtex.load(env_filename.c_str());
  }*/
  env_filename = envtex.get_filename();
  unsigned int width = envtex.get_width();
  unsigned int height = envtex.get_height();
  valarray< valarray<float> > f_luminance(height);
  for(unsigned int i = 0; i < f_luminance.size(); ++i)
  {
    float v = (i + 0.5f)/height;
    float theta = M_PIf*v;
    float sin_theta = sin(theta);
    f_luminance[i].resize(width);
    for(unsigned int j = 0; j < width; ++j)
    {
      float u = (j + 0.5f)/width;
      float phi = M_2PIf*u;
      float3 dir = make_float3(sin_theta*sin(phi), -cos(theta), -sin_theta*cos(phi));
      float4 texel = envtex.sample_nearest(dir);
      float luminance = static_cast<float>(get_luminance_NTSC(texel.x, texel.y, texel.z));
      f_luminance[i][j] = luminance;
      f_luminance[i][j] *= sin_theta;
    }
  }
  distribution = new Distribution2D(f_luminance);
}

bool PanoramicLight::sample(const float3& pos, float3& dir, float3& L) const
{
  float xi1 = mt_random(), xi2 = mt_random(), prob;
  float2 uv = distribution->sample_continuous(xi1, xi2, prob);
  float theta = uv.y*M_PIf;
  float phi = uv.x*M_2PIf;
  float sin_theta = sin(theta);
  dir = make_float3(sin_theta*sin(phi), -cos(theta), -sin_theta*cos(phi));
  if(shadows)
  {
    Ray s(pos, dir, 0, 1.0e-4f);
    HitInfo hit;
    if(tracer->trace_to_closest(s, hit))
      return false;
  }
  L = make_float3(envtex.sample_linear(dir))*sin_theta*M_2PIPIf/prob;
  return true;
}

bool PanoramicLight::emit(Ray& r, HitInfo& hit, float3& Phi) const
{
  return false;
}

string PanoramicLight::describe() const
{
  ostringstream ostr;
  ostr << "Panoramic environment light (texture image file " << env_filename << ", no. of samples " << samples << ").";
  return ostr.str();
}
