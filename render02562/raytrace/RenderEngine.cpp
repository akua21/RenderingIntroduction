// 02562 Rendering Framework
// Written by Jeppe Revall Frisvad, 2011
// Copyright (c) DTU Informatics 2011

#include <iostream>
#include <algorithm>
#include <list>
#include <string>
#include <optix_world.h>
#include "my_glut.h"
#include "../SOIL/stb_image_write.h"
#include "string_utils.h"
#include "Timer.h"
#include "mt_random.h"
#include "Directional.h"
#include "PointLight.h"
#include "PanoramicLight.h"
#include "RenderEngine.h"

#ifdef _OPENMP
  #include <omp.h>
#endif

using namespace std;
using namespace optix;

RenderEngine render_engine;

namespace
{
  // String utilities
	void lower_case(char& x) { x = tolower(x); }

	inline void lower_case_string(std::string& s)
	{
    for_each(s.begin(), s.end(), lower_case);
	}
}

//////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////

RenderEngine::RenderEngine() 
  : win(optix::make_uint2(512, 512)),                        // Default window size
    res(optix::make_uint2(512, 512)),                        // Default render resolution
    image(res.x*res.y),
    image_tex(0),
    mouse_state(GLUT_UP),
    spin_timer(20),
    vctrl(0),
    scene(&cam),
    filename("out.ppm"),                                     // Default output file name
    tracer(res.x, res.y, &scene, 100000),                    // Maximum number of photons in map
    max_to_trace(500000),                                    // Maximum number of photons to trace
    caustics_particles(20000),                               // Desired number of caustics photons
    done(false), 
    light_pow(optix::make_float3(M_PIf)),                    // Power of the default light
    light_dir(optix::make_float3(-1.0f)),                    // Direction of the default light
    default_light(&tracer, light_pow, light_dir),            // Construct default light
    use_default_light(true),                                 // Choose whether to use the default light or not
    shadows_on(true),
    background(optix::make_float3(0.1f, 0.3f, 0.6f)),        // Background color
    bgtex_filename(""),                                      // Background texture file name
    current_shader(0),
    lambertian(scene.get_lights()),
    photon_caustics(&tracer, scene.get_lights(), 1.0f, 50),  // Max distance and number of photons to search for
    glossy(&tracer, scene.get_lights()),
    holdout(&tracer, scene.get_lights(), 1),                 // No. of samples per path in holdout ambient occlusion
    mirror(&tracer),
    transparent(&tracer),
    volume(&tracer),
    glossy_volume(&tracer, scene.get_lights()),
    mc_glossy(&tracer, scene.get_lights()),
    merl(&tracer, scene.get_lights()),
    tone_map(1.8)                                            // Gamma for gamma correction
{ 
  shaders.push_back(&reflectance);                           // number key 0 (reflectance only)
  shaders.push_back(&lambertian);                            // number key 1 (direct lighting)
  shaders.push_back(&photon_caustics);                       // number key 2 (photon map caustics)
  shaders.push_back(&mc_glossy);                             // number key 3 (path tracing shader)
}

RenderEngine::~RenderEngine()
{
  delete vctrl;
}

void RenderEngine::load_files(int argc, char** argv)
{
  if(argc > 1)
  {
    for(int i = 1; i < argc; ++i)
    {
      // Retrieve filename without path
      list<string> path_split;
      split(argv[i], path_split, "\\");
      filename = path_split.back();
      if(filename.find("/") != filename.npos)
      {
        path_split.clear();
        split(filename, path_split, "/");
        filename = path_split.back();
      }
      lower_case_string(filename);
      Matrix4x4 transform = Matrix4x4::identity();

      // Special rules for some meshes
      if(char_traits<char>::compare(filename.c_str(), "cornell", 7) == 0)
        transform = Matrix4x4::scale(make_float3(0.025f))*Matrix4x4::rotate(M_PIf, make_float3(0.0f, 1.0f, 0.0f));
      else if(char_traits<char>::compare(filename.c_str(), "bunny", 5) == 0)
        transform = Matrix4x4::translate(make_float3(-3.0f, -0.85f, -8.0f))*Matrix4x4::scale(make_float3(25.0f));
      else if(char_traits<char>::compare(filename.c_str(), "justelephant", 12) == 0)
        transform = Matrix4x4::translate(make_float3(-10.0f, 3.0f, -2.0f))*Matrix4x4::rotate(0.5f, make_float3(0.0f, 1.0f, 0.0f));
     
      // Load the file into the scene
      scene.load_mesh(argv[i], transform);
    }
    init_view();
  }
  else
  {
    // Insert default scene
    scene.add_plane(make_float3(0.0f, 0.0f, 0.0f), make_float3(0.0f, 1.0f, 0.0f), "../models/default_scene.mtl", 1, 0.2f); // last argument is texture scale
    scene.add_sphere(make_float3(0.0f, 0.5f, 0.0f), 0.3f, "../models/default_scene.mtl", 2);
    scene.add_triangle(make_float3(-0.2f, 0.1f, 0.9f), make_float3(0.2f, 0.1f, 0.9f), make_float3(-0.2f, 0.1f, -0.1f), "../models/default_scene.mtl", 3);
    scene.add_light(new PointLight(&tracer, make_float3(M_PIf), make_float3(0.0f, 1.0f, 0.0f)));

    init_view();
    float3 eye = make_float3(2.0f, 1.5f, 2.0f);
    float3 lookat = make_float3(0.0f, 0.5, 0.0f);
    float3 up = make_float3(0.0f, 1.0f, 0.0f);
    vctrl->set_view_param(eye, lookat, up);
    cam.set(eye, lookat, up, 1.0f);
  }
}

void RenderEngine::init_GLUT(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(win.x, win.y);
  glutCreateWindow("02562 Render Framework");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(move);
  glutIdleFunc(idle);
  glutTimerFunc(spin_timer, spin, 0);
}

void RenderEngine::init_GL()
{
  glEnable(GL_CULL_FACE);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void RenderEngine::init_view()
{
  float3 c;
  float r;
  scene.get_bsphere(c, r);
  r *= 1.75f;

  // Initialize track ball
  vctrl = new GLViewController(win.x, win.y, c, r);

  // Initialize corresponding camera for tracer
  float3 eye, lookat, up;
  vctrl->get_view_param(eye, lookat, up);
  cam.set(eye, lookat, up, 1.0f);
}

void RenderEngine::init_tracer()
{
  clear_image();

  // Insert background texture/color
  tracer.set_background(background);
  if(!bgtex_filename.empty())
  {
    list<string> dot_split;
    split(bgtex_filename, dot_split, ".");
    if(dot_split.back() == "hdr")
      bgtex.load_hdr(bgtex_filename.c_str());
    else
      bgtex.load(bgtex_filename.c_str());
    tracer.set_background(&bgtex);
    //PanoramicLight* envlight = new PanoramicLight(&tracer, bgtex, 1);
    //cout << "Adding light source: " << envlight->describe() << endl;
    //scene.add_light(envlight);
    //scene.add_plane(make_float3(0.0f), make_float3(0.0f, 1.0f, 0.0f), "../models/plane.mtl", 4); // holdout plane
  }

  // Set shaders
  scene.set_shader(0, shaders[current_shader]); // shader for illum 0 (chosen by number key)
  scene.set_shader(1, shaders[current_shader]); // shader for illum 1 (chosen by number key)
  scene.set_shader(2, &glossy);                 // shader for illum 2 (calls lambertian until implemented)
  scene.set_shader(3, &mirror);                 // shader for illum 3
  scene.set_shader(4, &transparent);            // shader for illum 4
  scene.set_shader(11, &volume);                // shader for illum 11
  scene.set_shader(12, &glossy_volume);         // shader for illum 12
  scene.set_shader(30, &holdout);               // shader for illum 30
  scene.set_shader(31, &merl);                  // shader for illum 31

  // Load material textures
  scene.load_textures();

  // Add polygons with an ambient material as area light sources
  unsigned int lights_in_scene = scene.extract_area_lights(&tracer, 1);  // Set number of samples per light source here

  // If no light in scene, add default light source (shadow off)
  if(lights_in_scene == 0 && use_default_light)
  {
    cout << "Adding default light: " << default_light.describe() << endl;
    scene.add_light(&default_light);
  }

  // Build acceleration data structure
  Timer timer;
  cout << "Building acceleration structure...";
  timer.start();
  scene.init_accelerator();
  timer.stop();
  cout << "(time: " << timer.get_time() << ")" << endl; 

  // Build photon maps
  cout << "Building photon maps... " << endl;
  timer.start();
  tracer.build_maps(caustics_particles, max_to_trace);
  timer.stop();
  cout << "Building time: " << timer.get_time() << endl;
}

void RenderEngine::init_texture()
{
  if(!glIsTexture(image_tex))
    glGenTextures(1, &image_tex);
  glBindTexture(GL_TEXTURE_2D, image_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // load the texture image
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res.x, res.y, 0, GL_RGB, GL_FLOAT, &image[0].x);
}


//////////////////////////////////////////////////////////////////////
// Rendering
//////////////////////////////////////////////////////////////////////

void RenderEngine::clear_image()
{
  std::fill(&image[0], &image[0] + res.x*res.y, make_float3(0.0f));
  sample_number = 0.0f;
  split_time = 0.0;
}

void RenderEngine::apply_tone_map()
{
  if(done)
  {
    tone_map.apply(&image[0].x, res.x, res.y, 3);
    init_texture();
    glutPostRedisplay();
  }
}

void RenderEngine::unapply_tone_map()
{
  if(done)
  {
    tone_map.unapply(&image[0].x, res.x, res.y, 3);
    init_texture();
    glutPostRedisplay();
  }
}

void RenderEngine::add_textures()
{
  reflectance.set_textures(scene.get_textures());
  lambertian.set_textures(scene.get_textures());
  glossy.set_textures(scene.get_textures());
  glossy_volume.set_textures(scene.get_textures());
  mc_glossy.set_textures(scene.get_textures());
  photon_caustics.set_textures(scene.get_textures());
  merl.set_textures(scene.get_textures());
  merl.set_brdfs(scene.get_brdfs());
  scene.textures_on();
}

void RenderEngine::readjust_camera()
{
  float3 eye, lookat, up;
  vctrl->get_view_param(eye, lookat, up);
  cam.set(eye, lookat, up, cam.get_cam_const());
}

void RenderEngine::render()
{
  cout << "Raytracing";
  Timer timer;
  timer.start();
  #pragma omp parallel for private(randomizer)
  for(int y = 0; y < static_cast<int>(res.y); ++y)
  {
    for (int x = 0; x < static_cast<int>(res.x); ++x) {
        image[x+y*res.x] = tracer.compute_pixel(x, y);
    }
      

    // Insert the inner loop which runs through each pixel in a row and
    // stores the result of calling compute_pixel in the image array.
    //
    // Relevant data fields that are available (see RenderEngine.h)
    // res     (image resolution)
    // image   (flat array of rgb color vectors with res.x*res.y elements)
    // tracer  (ray tracer with access to the function compute_pixel)


    if(((y + 1) % 50) == 0) 
      cerr << ".";
  }
  timer.stop();
  cout << " - " << timer.get_time() << " secs " << endl;

  init_texture();
  done = true;
}

void RenderEngine::pathtrace()
{
  static Timer timer;
  static bool first = true;
  if(first)
  {
    cout << "Render time:" << endl;
    first = false;
  }
  int no_of_samples = static_cast<int>(sample_number)+1;
  bool print = (no_of_samples%10) == 0;
  if(print) cout << no_of_samples;
  timer.start(split_time);

  #pragma omp parallel for private(randomizer)
  for(int j = 0; j < static_cast<int>(res.y); ++j)
  {
    for(unsigned int i = 0; i < res.x; ++i)
      tracer.update_pixel(i, j, sample_number, image[i + j*res.x]);
    if(print && ((j + 1) % 50) == 0)
      cerr << ".";
  }

  timer.stop();
  split_time = timer.get_time();
  if(print) cout << ": " << split_time << endl;
  ++sample_number;

  init_texture();
  glutPostRedisplay();
}


//////////////////////////////////////////////////////////////////////
// Export/import
//////////////////////////////////////////////////////////////////////

void RenderEngine::save_view(const string& filename) const
{
  float cam_const = cam.get_cam_const();
  ofstream ofs(filename.c_str(), ofstream::binary);
  if(ofs.bad())
    return;

  vctrl->save(ofs);
  ofs.write(reinterpret_cast<const char*>(&cam_const), sizeof(float));
  ofs.close();
}

void RenderEngine::load_view(const string& filename)
{
  float cam_const;
  ifstream ifs_view(filename.c_str(), ifstream::binary);
  if(ifs_view.bad())
    return;

  vctrl->load(ifs_view);
  const istream& found_fd = ifs_view.read(reinterpret_cast<char*>(&cam_const), sizeof(float));
  ifs_view.close();

  float3 eye, lookat, up;
  vctrl->get_view_param(eye, lookat, up);
  cam.set(eye, lookat, up, found_fd ? cam_const : cam.get_cam_const());
}

void RenderEngine::save_as_bitmap()
{
  string png_name = "out.png";
  if(!filename.empty())
  {
    list<string> dot_split;
    split(filename, dot_split, ".");
    png_name = dot_split.front() + ".png";
  }
  unsigned char* data = new unsigned char[res.x*res.y*3];
  for(unsigned int j = 0; j < res.y; ++j)
    for(unsigned int i = 0; i < res.x; ++i)
    {
      unsigned int d_idx = (i + res.x*j)*3;
      unsigned int i_idx = i + res.x*(res.y - j - 1);
      data[d_idx + 0] = static_cast<unsigned int>(std::min(image[i_idx].x, 1.0f)*255.0f + 0.5f);
      data[d_idx + 1] = static_cast<unsigned int>(std::min(image[i_idx].y, 1.0f)*255.0f + 0.5f);
      data[d_idx + 2] = static_cast<unsigned int>(std::min(image[i_idx].z, 1.0f)*255.0f + 0.5f);
    }
  stbi_write_png(png_name.c_str(), res.x, res.y, 3, data, res.x*3);
  delete [] data;
  cout << "Rendered image stored in " << png_name << "." << endl;
}


//////////////////////////////////////////////////////////////////////
// Draw functions
//////////////////////////////////////////////////////////////////////

void RenderEngine::set_gl_ortho_proj() const
{
  glMatrixMode(GL_PROJECTION);	 
  glLoadIdentity();             
    
  glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);  
}

void RenderEngine::angular_map_vertex(float x, float y) const
{
  static const float far_clip = 1.0e-6f - 1.0f;
  float u, v;
  float3 dir = cam.get_ray_dir(make_float2(x - 0.5f, y - 0.5f));
  bgtex.project_direction(dir, u, v);
  float3 texColor = tracer.get_background(dir);
  glColor3fv(&texColor.x);
  glTexCoord2f(u, v);
  glVertex3f(x, y, far_clip);
}

void RenderEngine::draw_angular_map_strip(const float4& quad, float no_of_steps) const
{
  glBegin(GL_TRIANGLE_STRIP);
  float ystep = quad.w - quad.y;
  for(float y = quad.y; y < quad.w + ystep; y += ystep/no_of_steps)
  {
    angular_map_vertex(quad.x, y);
    angular_map_vertex(quad.z, y);
  }
  glEnd();
}

void RenderEngine::draw_angular_map_tquad(float no_of_xsteps, float no_of_ysteps) const
{
  float xstep = 1.0f/no_of_xsteps;
  for(float x = 0.0f; x < 1.0f + xstep; x += xstep)
    draw_angular_map_strip(make_float4(x, 0.0f, x + xstep, 1.0f), no_of_ysteps);
}

void RenderEngine::draw_background() const
{
  if(!bgtex.has_texture())
    return;

  set_gl_ortho_proj();
  glLoadIdentity();

  if(bgtex.has_texture())
  {
    bgtex.bind();
    bgtex.enable();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  draw_angular_map_tquad(15.0f, 15.0f);
  if(bgtex.has_texture())
    bgtex.disable();

  cam.glSetPerspective(win.x, win.y);
  cam.glSetCamera();
}

void RenderEngine::draw_texture()
{
  static GLfloat verts[] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f };

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  glBindTexture(GL_TEXTURE_2D, image_tex);
  glEnable(GL_TEXTURE_2D);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glVertexPointer(2, GL_FLOAT, 0, verts);
  glTexCoordPointer(2, GL_FLOAT, 0, verts);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glDisable(GL_TEXTURE_2D);
}

void RenderEngine::draw()
{ 
  if(shaders[current_shader] == &photon_caustics)
    tracer.draw_caustics_map();
  else
  {
    draw_background();
    scene.draw(); 
  }
}


//////////////////////////////////////////////////////////////////////
// GLUT callback functions
//////////////////////////////////////////////////////////////////////

void RenderEngine::display()
{
  render_engine.readjust_camera();

  if(render_engine.is_done() || render_engine.is_tracing())
  {
    render_engine.set_gl_ortho_proj();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    render_engine.draw_texture();
  }
  else
  {
    glEnable(GL_DEPTH_TEST);
    render_engine.set_gl_perspective();
    render_engine.set_gl_clearcolor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
	  render_engine.set_gl_camera();
    render_engine.draw();
    glDisable(GL_DEPTH_TEST);
  }
  glutSwapBuffers();  
}

void RenderEngine::reshape(int width, int height)
{
  render_engine.set_window_size(width, height);
  glViewport(0, 0, width, height);
}

void RenderEngine::keyboard(unsigned char key, int x, int y)
{
  // The shader to be used when rendering a material is chosen 
  // by setting the "illum" property of the material. This
  // property is part of the MTL file format ("illum" is short
  // for "illumination model"). The shader to be used with
  // each illumination model is specified in the init_tracer 
  // function above.
  //
  // Number keys switch the shader used for illumination
  // models 0 and 1 to the shader at the corresponding index
  // in the array "shaders" at the top of this file.
  //
  // When you switch shaders all previous rendering results
  // will be erased!
  if(key >= 48 && key < 48 + static_cast<unsigned char>(render_engine.no_of_shaders()))
  {
    unsigned int shader_no = key - 48;
    if(shader_no != render_engine.get_current_shader())
    {
      render_engine.set_current_shader(shader_no);
      render_engine.clear_image();
      render_engine.redo_display_list();
      cout << "Switched to shader number " << shader_no << endl;
      glutPostRedisplay();
    }
  }
  switch(key)
  { 
  // Use '+' and '-' to increase or decrease the number of
  // jitter samples per pixel in a simple ray tracing
  case '+':
    render_engine.increment_pixel_subdivs();
    break;
  case '-':
    render_engine.decrement_pixel_subdivs();
    break;
  // Press '*' to apply tone mapping
  case '*':
    render_engine.apply_tone_map();
    break;
  // Press '/' to unapply tone mapping
  case '/':
    render_engine.unapply_tone_map();
    break;
  // Press 'b' to save the render result as a bitmap called out.png.
  // If obj files are loaded, the png will be named after the obj file loaded last.
  case 'b':
    render_engine.save_as_bitmap();
    break;
  // Press 'r' to start a simple ray tracing (one pass -> done).
  // To switch back to preview mode after the ray tracing is done
  // press 'r' again.
  case 'r':
    if(render_engine.is_done())
      render_engine.undo();
    else 
      render_engine.render();
    glutPostRedisplay();
    break;
  // Press 's' to toggle shadows on/off
  case 's':
    {
      bool shadows_on = render_engine.toggle_shadows();
      render_engine.clear_image();
      render_engine.redo_display_list();
      cout << "Toggled shadows " << (shadows_on ? "on" : "off") << endl;
      glutPostRedisplay();
    }
    break;
  // Press 't' to start path tracing. The path tracer will
  // continue to improve the image until 't' is pressed again.
  case 't':
    {
      bool is_tracing = render_engine.toggle_pathtracing();
      if(!is_tracing && !render_engine.is_done())
        render_engine.undo();
    }
    break;
  // Press 'x' to switch on material textures.
  case 'x':
    render_engine.add_textures();
    cout << "Toggled textures on." << endl;
    glutPostRedisplay();
    break;
  // Press 'z' to zoom in.
  case 'z':
    {
      render_engine.set_cam_const(render_engine.get_cam_const()*1.05f);
      glutPostRedisplay();
    }
    break;
    // Press 'L' to load a view saved in the file "view".
  case 'L':
  {
    render_engine.load_view("view");
    cout << "View loaded from file: view" << endl;
    float3 eye, lookat, up;
    render_engine.get_view_controller()->get_view_param(eye, lookat, up);
    cout << "Eye   : " << eye << endl
      << "Lookat: " << lookat << endl
      << "Up    : " << up << endl
      << "FOV   : " << render_engine.get_field_of_view() << endl;
    glutPostRedisplay();
  }
  break;
  // Press 'S' to save the current view in the file "view".
  case 'S':
    render_engine.save_view("view");
    cout << "Current view stored in file: view" << endl;
    break;
  // Press 'Z' to zoom out.
  case 'Z':
    {
      render_engine.set_cam_const(render_engine.get_cam_const()/1.05f);
      glutPostRedisplay();
    }
    break;
  // Press 'space' to switch between pre-view and your last tracing result.
  case 32:
    render_engine.undo();
    glutPostRedisplay();
    break;
  // Press 'esc' to exit the program.
  case 27:
    exit(0);
  }
}

void RenderEngine::mouse(int btn, int state, int x, int y)
{
  if(state == GLUT_DOWN)
  {
    if(btn == GLUT_LEFT_BUTTON)
      render_engine.get_view_controller()->grab_ball(ORBIT_ACTION, make_uint2(x, y));
    else if(btn == GLUT_MIDDLE_BUTTON)
      render_engine.get_view_controller()->grab_ball(DOLLY_ACTION, make_uint2(x, y));
    else if(btn == GLUT_RIGHT_BUTTON)
      render_engine.get_view_controller()->grab_ball(PAN_ACTION, make_uint2(x, y));
  }
  else if(state == GLUT_UP)
    render_engine.get_view_controller()->release_ball();

  render_engine.set_mouse_state(state);
}

void RenderEngine::move(int x, int y)
{
  render_engine.get_view_controller()->roll_ball(make_uint2(x, y));
  if(render_engine.get_mouse_state() == GLUT_DOWN)
    glutPostRedisplay();
}

void RenderEngine::spin(int x)
{
  if(!render_engine.is_tracing())
  {
    if(render_engine.get_view_controller()->try_spin())
      glutPostRedisplay();
    glutTimerFunc(render_engine.get_spin_timer(), spin, 0);
  }
}

void RenderEngine::idle()
{
  if(render_engine.is_tracing())
    render_engine.pathtrace();
}

void RenderEngine::set_current_shader(unsigned int shader) 
{ 
  current_shader = shader;
  for(int i = 0; i < 2; ++i)
    scene.set_shader(i, shaders[current_shader]);
}
