/**
 * @file GLViewController.h
 * @brief Class for setting view transform.
 */
/* ----------------------------------------------------------------------- *
 * This file is part of GEL, http://www.imm.dtu.dk/GEL
 * Copyright (C) the authors and DTU Informatics
 * For license and list of authors, see ../../doc/intro.pdf
 * ----------------------------------------------------------------------- */

#ifndef GLVIEWCONTROLLER_H
#define GLVIEWCONTROLLER_H

#include <optix_world.h>
#include <fstream>
#include "quaternion.h"
#include "QuatTrackBall.h"

/** The GLViewController is a more high level component than a trackball.
	The idea behind GLViewController is to handle setting up the projection
	and changing the viewport when the window is reshaped. Basically the raw
	mouse position and related info is sent to the view controller which takes
	care of the rest.
	*/
class GLViewController
{
	float FOV_DEG;
	int WINX, WINY;
	float znear, zfar;
	float aspect;
	bool button_down;
	TrackBallAction last_action;
	bool spin;

	QuatTrackBall ball;
		

public:

	/** Constructor which accepts the window dimensions as well as the world center and the 
		radius which should be construed as the distance to the observer */
	GLViewController(int _WINX, int _WINY,
										const optix::float3& _centre, float _rad);
										 
	/// Grab ball takes an action and a mouse position.
	void grab_ball(TrackBallAction action, const optix::uint2& pos);
		
	/// Roll virtual trackball (pass just mouse position).
	void roll_ball(const optix::uint2& pos);
		
	/// Release the virtual trackball
	void release_ball();
		
	/// Try to spind the trackball - called from idle.
	bool try_spin();
				
	/// Setup GL modelview matrix.
	void set_gl_modelview();

	/// Reset projection. Called initially, when window size has changed or when user zooms. 
	void reset_projection();

	/// Reshape window.
	void reshape(unsigned int W, unsigned int H);
		
	/// Set near and far planes.
	void set_near_and_far();
		
	/// Set centre of ball.
	void set_centre(const optix::float3& c)
	{
		ball.set_centre(c);
	}

	/// Set rotation of ball.
	void set_rotation(const quaternion& qrot)
	{
		ball.set_rotation(qrot);
	}
		
	/// Set eye distance.
	void set_eye_dist(float rad)
	{
		ball.set_eye_dist(rad);
		set_near_and_far();
	}

	/// Returns eye distance
	float get_eye_dist() const
	{
		return ball.get_eye_dist();
	}

	/// Get viewing parameters: eye, centre, up
	void get_view_param(optix::float3& e, optix::float3& c, optix::float3& u) const
	{
		ball.get_view_param(e,c,u);
	}

	/// Set viewing parameters: eye centre, up
	void set_view_param(const optix::float3& e, const optix::float3& c, const optix::float3& u);

	/// Load trackball from stream
	bool load(std::ifstream&);
		
	/// Save trackball to stream.
	bool save(std::ofstream&) const;
};

#endif // GLVIEWCONTROLLER_H
