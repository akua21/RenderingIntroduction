#ifndef DISTRIBUTION1D_H
#define DISTRIBUTION1D_H

#include <valarray>

class Distribution1D
{
public:
  Distribution1D(const std::valarray<float>& f);

  float sample_continuous(float xi, float& probability) const;
  unsigned int sample_discrete(float xi, float& probability) const;
  float get_pdf(float u) const;

  unsigned int size() const { return pdf.size(); }
  float operator[](unsigned int i) { return pdf[i]; }

private:
  std::valarray<float> pdf;
  std::valarray<float> cdf;
};

#endif // DISTRIBUTION1D_H