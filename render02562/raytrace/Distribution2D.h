#ifndef DISTRIBUTION2D_H
#define DISTRIBUTION2D_H

#include <valarray>
#include <optix_world.h>
#include "Distribution1D.h"

class Distribution2D
{
public:
  Distribution2D(const std::valarray< std::valarray<float> >& f);
  ~Distribution2D();

  optix::float2 sample_continuous(float xi1, float xi2, float& probability) const;
  float get_pdf(float u, float v) const;

private:
  Distribution1D* marginal;
  std::valarray< Distribution1D* > conditional;
};

#endif // DISTRIBUTION2D_H