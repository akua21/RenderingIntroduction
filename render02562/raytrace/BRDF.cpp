#include <fstream>
#include <optix_world.h>
#include "sampler.h"
#include "BRDF.h"

using namespace std;
using namespace optix;

// Rotate vector around an axis
float3 rotate_vector(const float3& vector, const float3& axis, float angle)
{
  // Implement quaternion-based rotation of a vector by an angle around an axis
  return make_float3(0.0f);
}

// Convert vectors in tangent space to half/difference coordinates
void vectors_to_half_diff_coords(const float3& in, const float3& out,
  float& theta_half, float& phi_half, float& theta_diff, float& phi_diff)
{
  // compute halfway vector

  // compute theta_half, phi_half

  // compute diff vector

  // compute theta_diff, phi_diff	
}

float3 lookup_brdf_val(const float* brdf, const float3& n, const float3& normalized_wi, const float3& normalized_wo)
{
  // Transform vectors to tangent space and use the vectors_to_half_diff_coords function
  // to get input for the other version of the lookup_brdf_val function.
  // Hint: There is an onb function for creating an orthonormal basis and a Matrix3x3 data type useful for storing a change of basis matrix.
  return make_float3(0.0f);
}

float3 integrate_brdf(float* brdf, unsigned int N)
{
  float3 sum = make_float3(0.0f);
  float3 n = make_float3(0.0f, 0.0f, 1.0f);

  // Implement Monte Carlo integration to estimate the bihemisphercial diffuse reflectance (rho).
  // Use N as the number of samples.
  // Hint: Use the functions sample_cosine_weighted and lookup_brdf_val.

  return sum;
}

// Read BRDF data
bool read_brdf(const char *filename, float* brdf, unsigned int size, float3& rho_d)
{
  FILE* f = fopen(filename, "rb");
  if(!f)
    return false;

  int dims[3];
  fread(dims, sizeof(int), 3, f);
  unsigned int n = 3*dims[0]*dims[1]*dims[2];
  if(size != n)
    return false;

  double* brdf_d = new double[n];
  fread(&brdf_d[0], sizeof(double), n, f);
  for(unsigned int i = 0; i < size; ++i)
    brdf[i] = static_cast<float>(brdf_d[i]);
  fclose(f);
  delete[] brdf_d;

  rho_d = integrate_brdf(brdf, 100000);
  return true;
}
