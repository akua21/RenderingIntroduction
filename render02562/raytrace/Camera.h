// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef CAMERA_H
#define CAMERA_H

#include <optix_world.h>

const float NEAR_PLANE = 1.0e-3f;
const float FAR_PLANE = 1.0e5f;

class Camera
{
public:
  Camera(const optix::float3& eye_point = optix::make_float3(0.0f, 0.0f, 0.0f),  
	       const optix::float3& lookat_point = optix::make_float3(0.0f, 0.0f, -1.0f),
	       const optix::float3& up_vector = optix::make_float3(0.0f, 1.0f, 0.0f),      
	       float camera_constant = 1.0f)
  {
    set(eye_point, lookat_point, up_vector, camera_constant);
  }

  void set(const optix::float3& eye_point,  
	         const optix::float3& view_point,
	         const optix::float3& up_vector,      
	         float camera_constant);

  /// Get direction of viewing ray from image coords.
  optix::float3 get_ray_dir(const optix::float2& coords) const;

  /// Return position of camera.
  const optix::float3& get_position() const { return eye; }

  /// Return camera line of sight
  const optix::float3& get_line_of_sight() const { return ip_normal; }  

  /// Return the ray corresponding to a set of image coords
  optix::Ray get_ray(const optix::float2& coords) const;

  float get_fov() const { return fov; }
  float get_cam_const() const { return cam_const; }
  void set_cam_const(float camera_constant) { set(eye, lookat, up, camera_constant); }

  // OpenGL
  void glSetPerspective(unsigned int width, unsigned int height) const;
  void glSetCamera() const;

private:
  optix::float3 eye, lookat, up;
  float cam_const;
  float fov;

  // Basis of camera coordinate system (ip - image plane).
  // The normal is the viewing direction.
  optix::float3 ip_normal;
  optix::float3 ip_xaxis;
  optix::float3 ip_yaxis;
};

#endif
