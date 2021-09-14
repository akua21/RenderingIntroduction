#include <iostream> // debug
#include <valarray>
#include <algorithm>
#include "cdf_bsearch.h"
#include "Distribution1D.h"

using namespace std;

Distribution1D::Distribution1D(const valarray<float>& f)
  : pdf(f), cdf(f.size())
{
  float f_sum = f.sum()/f.size();
  cdf[0] = f[0]/f.size();
  for(unsigned int i = 1; i < cdf.size(); ++i)
    cdf[i] = cdf[i - 1] + f[i]/f.size();
  cdf /= f_sum;
  pdf /= f_sum;
  cdf[cdf.size() - 1] = 1.0f;
}

float Distribution1D::sample_continuous(float xi, float& probability) const
{
  unsigned int idx = cdf_bsearch(xi, cdf);
  float du = idx > 0 ? (xi - cdf[idx - 1])/(cdf[idx] - cdf[idx - 1]) : xi/cdf[idx];
  if(pdf[idx] < 1.0e-5f)
    cerr << idx << ": " << pdf[idx] << " ";
  probability = pdf[idx];
  return (idx + du)/pdf.size();
}

unsigned int Distribution1D::sample_discrete(float xi, float& probability) const
{
  unsigned int idx = cdf_bsearch(xi, cdf);
  probability = pdf[idx]/pdf.size();
  return idx;
}

float Distribution1D::get_pdf(float u) const
{
  int count = pdf.size();
  int u_idx = static_cast<int>(u*pdf.size());
  u_idx = min(max(u_idx, 0), count - 1);
  return pdf[u_idx];
}
