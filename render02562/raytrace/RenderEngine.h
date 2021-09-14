// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#include <vector>
#include <string>
#include <optix_world.h>
#include "my_glut.h"
#include "GLViewController.h"
#include "Camera.h"
#include "Scene.h"
#include "Directional.h"
#include "ParticleTracer.h"
#include "Shader.h"
#include "Textured.h"
#include "Lambertian.h"
#include "PhotonCaustics.h"
#include "Glossy.h"
#include "Holdout.h"
#include "Mirror.h"
#include "Transparent.h"
#include "Volume.h"
#include "GlossyVolume.h"
#include "MCGlossy.h"
#include "MerlShader.h"
#include "PanoramicTexture.h"
#include "Gamma.h"

class RenderEngine
{
public:
  // Initialization
  RenderEngine();
  ~RenderEngine();
  void load_files(int argc, char** argv);
  void init_GLUT(int argc, char** argv);
  void init_GL();
  void init_view();
  void init_tracer();
  void init_texture();

  // Rendering
  unsigned int no_of_shaders() const { return shaders.size(); }
  bool toggle_shadows() { shadows_on = !shadows_on; scene.toggle_shadows(); return shadows_on; }
  bool is_tracing() const { return tracing; }
  bool is_done() const { return done; }
  void undo() { done = !done; }
  void increment_pixel_subdivs() { tracer.increment_pixel_subdivs(); }
  void decrement_pixel_subdivs() { tracer.decrement_pixel_subdivs(); }
  bool toggle_pathtracing() { return tracing = !tracing; }
  void clear_image();
  void apply_tone_map();
  void unapply_tone_map();
  void add_textures();
  void readjust_camera();
  void render();
  void pathtrace();

  // Export/import
  void save_view(const std::string& filename) const;
  void load_view(const std::string& filename);
  void save_as_bitmap();

  // Draw functions
  void set_gl_ortho_proj() const;
  void set_gl_perspective() const { cam.glSetPerspective(win.x, win.y); }
  void set_gl_camera() const { cam.glSetCamera(); }
  void set_gl_clearcolor() const { glClearColor(background.x, background.y, background.z, 0.0f); }
  void redo_display_list() { scene.redo_display_list(); }
  void angular_map_vertex(float x, float y) const;
  void draw_angular_map_strip(const optix::float4& quad, float no_of_steps) const;
  void draw_angular_map_tquad(float no_of_xsteps, float no_of_ysteps) const;
  void draw_background() const;
  void draw_texture();
  void draw();

  // GLUT callback functions
  static void display();
  static void reshape(int width, int height);
  static void keyboard(unsigned char key, int x, int y);
  static void mouse(int btn, int state, int x, int y);
  static void move(int x, int y);
  static void spin(int x);
  static void idle();

  // Accessors
  void set_window_size(int w, int h) { win.x = w; win.y = h; vctrl->reshape(w, h); }
  unsigned int get_current_shader() { return current_shader; }
  void set_current_shader(unsigned int shader);
  float get_cam_const() { return cam.get_cam_const(); }
  void set_cam_const(float fd) { cam.set_cam_const(fd); }
  float get_field_of_view() const { return cam.get_fov(); }
  GLViewController* get_view_controller() const { return vctrl; }
  int get_mouse_state() const { return mouse_state; }
  void set_mouse_state(int state) { mouse_state = state; }
  int get_spin_timer() const { return spin_timer; }

private:
  // Window and render resolution
  optix::uint2 win;
  optix::uint2 res;

  // Render data
  std::vector<optix::float3> image;
  unsigned int image_tex;
  float sample_number;
  double split_time;

  // View control
  int mouse_state;
  int spin_timer;
  GLViewController* vctrl;
  Camera cam;

  // Geometry container
  Scene scene;
  
  // Output file name
  std::string filename;

  // Tracer
  ParticleTracer tracer;
  unsigned int max_to_trace;
  unsigned int caustics_particles;
  bool tracing;
  bool done;

  // Light
  optix::float3 light_pow;
  optix::float3 light_dir;
  Directional default_light;
  bool use_default_light;
  bool shadows_on;

  // Environment
  optix::float3 background;
  PanoramicTexture bgtex;
  std::string bgtex_filename;

  // Shaders
  unsigned int current_shader;
  std::vector<Shader*> shaders;
  Textured reflectance;
  Lambertian lambertian;
  PhotonCaustics photon_caustics;
  Glossy glossy;
  Holdout holdout;
  Mirror mirror;
  Transparent transparent;
  Volume volume;
  GlossyVolume glossy_volume;
  MCGlossy mc_glossy;
  MerlShader merl;

  // Tone mapping
  Gamma tone_map;
};

extern RenderEngine render_engine;

#endif // RENDERENGINE