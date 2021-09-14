#ifndef QUATTRACKBALL_H
#define QUATTRACKBALL_H

#include <optix_world.h>
#include "quaternion.h"

enum TrackBallAction
	{
		NO_ACTION = 0,
		ORBIT_ACTION,
		PAN_ACTION,
		DOLLY_ACTION
	};

// Virtual trackball
class QuatTrackBall 
{
	optix::float3 centre;
	optix::uint2 screen_centre;

	unsigned int width, height;
	quaternion qrot;
	quaternion qinc;
	optix::float2 trans;
	float scale;
	float	ballsize;
	float eye_dist;
	optix::float2 last_pos;
	TrackBallAction current_action;

	void orbit(const optix::float2&);
	void pan(const optix::float2&);
	void dolly(const optix::float2&);

	void calcRotation(const optix::float2&);
	float projectToSphere(const optix::float2&);
  optix::float2 scalePoint(const optix::uint2&) const;

	void set_position(const optix::float2&);

public:

	/** First constructor argument is the point we look at. 
			The second argument is the distance to eye point.
			The third is the scaling factor
			the last two arguments are the window dimensions. */
	QuatTrackBall(const optix::float3&, float, unsigned, unsigned);

	/// Set window dimensions.
	void set_screen_window(unsigned _width, unsigned _height)
	{
		width = _width;
		height = _height;
		screen_centre.x = width/2;
		screen_centre.y = height/2;
	}
	
	/// set the centre point of rotation
	void set_centre(const optix::float3& _centre)
	{
		centre = _centre;
	}

	void set_screen_centre(const optix::uint2& _screen_centre)
	{
		screen_centre.x = _screen_centre.x;
		screen_centre.y = height - _screen_centre.y;
	}

	const quaternion& get_rotation() const 
	{
		return qrot;
	}

	void set_rotation(const quaternion& _qrot)
	{
		qrot = _qrot;
	}

	void set_eye_dist(float _eye_dist)
	{
		eye_dist = _eye_dist;
	}

	float get_eye_dist() const
	{
		return eye_dist;
	}

	/// Call GL to set up viewing. 
	void set_gl_modelview() const;

	/** Spin. used both to spin while button is pressed and if the ball is just
			spinning while program is idling. */
	void do_spin();

	bool is_spinning() const;
	
	/// Zeroes the rotation value - makes everything stop.
	void stop_spin();
	
	/// Call this function to start action when mouse button is pressed 
	void grab_ball(TrackBallAction,const optix::uint2&);

	/// Call this function to perform action when user drags mouse
	void roll_ball(const optix::uint2&);

	/// Call this function to stop action when mouse is released.
	void release_ball() 
	{
		current_action = NO_ACTION;
	}

	/// Returns true if the ball is `grabbed' and not released yet.
	bool is_grabbed() const 
	{
		if(current_action == NO_ACTION) 
			return false;
		return true;
	}

	void get_view_param(optix::float3& eye,
											optix::float3& _centre,
                      optix::float3& up) const;

	TrackBallAction get_current_action() 
	{
		return current_action;
	}

};

#endif // QUATTRACKBALL_H
