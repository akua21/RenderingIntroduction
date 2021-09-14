#include <valarray>
#include <algorithm>
#include <optix_world.h>
#include "Distribution1D.h"
#include "Distribution2D.h"

using namespace std;
using namespace optix;

Distribution2D::Distribution2D(const valarray< valarray<float> >& f)
  : marginal(0), conditional(static_cast<Distribution1D*>(0), f.size())
{
  valarray<float> marginal_f(f.size());
  for(unsigned int i = 0; i < f.size(); ++i)
  {
    marginal_f[i] = f[i].sum()/f[i].size();
    conditional[i] = new Distribution1D(f[i]);
  }
  marginal = new Distribution1D(marginal_f);
}

Distribution2D::~Distribution2D()
{
  for(unsigned int i = 0; i < conditional.size(); ++i)
    delete conditional[i];
  delete marginal;
}

float2 Distribution2D::sample_continuous(float xi1, float xi2, float& probability) const
{
  int count = marginal->size();
  float pdf_m, pdf_c;
  float v = marginal->sample_continuous(xi2, pdf_m);
  int v_idx = static_cast<int>(v*count);
  v_idx = min(max(v_idx, 0), count - 1);
  float u = conditional[v_idx]->sample_continuous(xi1, pdf_c);
  probability = pdf_m*pdf_c;
  return make_float2(u, v);
}

float Distribution2D::get_pdf(float u, float v) const
{
  int count = marginal->size();
  int v_idx = static_cast<int>(v*count);
  v_idx = min(max(v_idx, 0), count - 1);
  return (*marginal)[v_idx]*conditional[v_idx]->get_pdf(u);
}