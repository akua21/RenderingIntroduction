/* ----------------------------------------------------------------------- *
 * This file is part of GEL, http://www.imm.dtu.dk/GEL
 * Copyright (C) the authors and DTU Informatics
 * For license and list of authors, see ../../doc/intro.pdf
 * ----------------------------------------------------------------------- */

#include <optix_world.h>
#include "my_glut.h"
#include "quaternion.h"
#include "GLViewController.h"

using namespace std;
using namespace optix;


void GLViewController::reset_projection()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV_DEG, aspect, znear, zfar);
  glMatrixMode(GL_MODELVIEW);
}

GLViewController::GLViewController(int _WINX, int _WINY, const float3& centre, float rad)
  : FOV_DEG(53),
    WINX(_WINX), WINY(_WINY), 
    aspect(WINX/float(WINY)),
    button_down(false),
    spin(false),
    ball(centre, rad, WINX, WINY)
{
  znear = 0.01f*rad;
  zfar  = 3*rad;
  reset_projection();
}

void GLViewController::grab_ball(TrackBallAction action, const uint2& pos)
{
  ball.grab_ball(action,pos);
  if(action==DOLLY_ACTION)
    set_near_and_far();

  spin = false;
  button_down = true;
  last_action = action;
}

void GLViewController::roll_ball(const uint2& pos)
{
  static uint2 old_pos = pos;
  float2 dir = make_float2(pos - old_pos);
  float len = length(dir);
  if (len < 1.0e-8)
    return;
    
  ball.roll_ball(pos);
  if(last_action==DOLLY_ACTION)
    set_near_and_far();
    
  spin = len>=1.1f;
  old_pos = pos;  
}


void GLViewController::release_ball()
{
  ball.release_ball();
  if(last_action==DOLLY_ACTION)
    set_near_and_far();
}

bool GLViewController::try_spin()
{
  if(spin && !ball.is_grabbed()) 
  {
    ball.do_spin();
    return true;
  }
  return false;
}
  
void GLViewController::set_gl_modelview()
{
  // Modify the current gl matrix by the trackball rotation and translation.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  float3 eye;
  float3 centre;
  float3 up;
  ball.get_view_param(eye, centre, up);
  gluLookAt(eye.x, eye.y, eye.z,
            centre.x, centre.y, centre.z,
            up.x, up.y, up.z);
}


void GLViewController::reshape(unsigned int W, unsigned int H)
{
  WINX = W;
  WINY = H;
  aspect = WINX/static_cast<float>(WINY);
  glViewport(0,0,WINX,WINY);
  reset_projection();
  ball.set_screen_window(WINX, WINY);
}  

void GLViewController::set_near_and_far()
{  
  float rad = ball.get_eye_dist();
  znear = 0.01f*rad;
  zfar = 3*rad;
  reset_projection();
}

void GLViewController::set_view_param(const float3& e, const float3& c, const float3& u)
{
  // native viewing direction is the negative z-axis
  // while right is the x-axis and up is the y-axis
  float3 view = c - e;
  float eye_dist = length(view);
  view /= eye_dist;
  float3 right = normalize(cross(view, u));
  float3 up = cross(right, view);
  Matrix3x3 rot;
  rot.setCol(0, right);
  rot.setCol(1, up);
  rot.setCol(2, -view);

  // convert the change-of-basis matrix to a quaternion
  quaternion qrot;
  qrot.make_rot(rot);
  set_rotation(qrot);
  set_centre(c);
  set_eye_dist(eye_dist);
}

bool GLViewController::load(std::ifstream& ifs)
{
  if(ifs)
  {
    ifs.read(reinterpret_cast<char*>(this), sizeof(GLViewController));
    reset_projection();
    ball.set_screen_window(WINX, WINY);
    return true;
  }
  return false;
}

bool GLViewController::save(std::ofstream& ofs) const
{
  if(ofs)
  {
    ofs.write(reinterpret_cast<const char*>(this), sizeof(GLViewController));
    return true;
  }
  return false;
}

