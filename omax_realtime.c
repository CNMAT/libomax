/*
  Copyright (c) 2009, 13.  The Regents of the University of California (Regents).
  All Rights Reserved.
 
  Permission to use, copy, modify, and distribute this software and its
  documentation for educational, research, and not-for-profit purposes, without
  fee and without a signed licensing agreement, is hereby granted, provided that
  the above copyright notice, this paragraph and the following two paragraphs
  appear in all copies, modifications, and distributions.  Contact The Office of
  Technology Licensing, UC Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley,
  CA 94720-1620, (510) 643-7201, for commercial licensing opportunities.
 
  Written by John MacCallum, Andy Schmeder, The Center for New Music and
  Audio Technologies, University of California, Berkeley.
 
  IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
  SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
  ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
  REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
  REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
  DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
  REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
  ENHANCEMENTS, OR MODIFICATIONS.
*/

#ifndef WIN_VERSION
//#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#else
#include <windows.h>
#endif

#include <math.h>

#include "ext.h"
#include "z_dsp.h"

#include "omax_realtime.h"
#include "osc.h"
#include "osc_timetag.h"
#include "osc_error.h"
#include "osc_mem.h"
/*
  #ifndef NAME
  #define NAME "omax_realtime_clock"
  #endif
*/

//#define __OMAX_REALTIME_DEBUG__

t_symbol* _sym_omax_realtime_clock_master;

t_omax_realtime_clock* omax_realtime_clock_get_master()
{
	if(! _sym_omax_realtime_clock_master) {
		_sym_omax_realtime_clock_master = gensym("_omax_realtime_clock_master");
	}
    
	if(! _sym_omax_realtime_clock_master->s_thing) {
		_sym_omax_realtime_clock_master->s_thing = (void*)osc_mem_alloc(sizeof(t_omax_realtime_clock));
	}
    
	return (t_omax_realtime_clock*)(_sym_omax_realtime_clock_master->s_thing);   
}

void omax_realtime_clock_init()
{
	t_omax_realtime_clock* x = omax_realtime_clock_get_master();
    
	x->dt0 = 0.;
	x->n = 0;
    
	x->running = -1;
	x->trigger = NULL;
}

void omax_realtime_clock_register(void* trigger)
{
	t_omax_realtime_clock* x = omax_realtime_clock_get_master();
	// only set the first one.
    
	// running or never run
	if(x->running != 0) {
		// not running
		x->running = 0;

		t_dspchain *c = dspchain_get();
		if(c) {
			if(c->c_patcher != 0) {
				//error("attempting to use realtime object as master clock in a poly~, bad idea!");
			}
		}
        
		// this pointer will trigger tick
		x->trigger = trigger;

		// dt0 needs to be recalculated, n resets to zero
		x->dt0 = 0.;
		x->n = 0;
	}
}

void omax_realtime_clock_set_fc(double fc)
{
	t_omax_realtime_clock *x = omax_realtime_clock_get_master();
    
	// on init:
	//fc = 0.100000, w = 0.628319, b = 0.888577, c = 0.394784
	x->fc = fc;
	x->w = 2. * M_PI * x->fc;
	x->b = M_SQRT2 * x->w;
	x->c = x->w * x->w;
}

void omax_realtime_printTimetag(t_osc_timetag t, const char *id, const char *func, long line)
{
#ifdef __OMAX_REALTIME_DEBUG__
	char *buf = osc_timetag_format(t);
	printf("%s:%ld: %s = %s (%f)\n", func, line, id, buf, osc_timetag_timetagToFloat(t));
	osc_mem_free(buf);
#endif
}

void omax_realtime_printFloat(double f, const char *id, const char *func, long line)
{
#ifdef __OMAX_REALTIME_DEBUG__
	printf("%s:%ld: %s = %f\n", func, line, id, f);
#endif
}

void omax_realtime_clock_tick(void *trigger)
{
	/*
	struct ntptime now;
	//struct ntptime t1_neg;
	struct ntptime t1_next;
	struct ntptime dt1;
	//struct ntptime loop_err_ntp;
	*/
	t_osc_timetag t1_next, dt1;
    
	double loop_err;

	t_omax_realtime_clock *x = omax_realtime_clock_get_master();

	if(x->running == 0) {
		if(trigger != x->trigger) {
			error("clock is not running but trigger is not first");
		} else {
			x->running = 1;
			x->dt0 = (sys_getblksize() * (1./sys_getsr()));
		}
	}
    
	// tick only applies for the first caller 
	if(trigger != x->trigger) {
		return;
	}
	//printf("%s:%d: x->dt0 = %f\n", __func__, __LINE__, x->dt0);

	if(x->running != 1) {
		error("clock should be running now");
	}
        
	// whats the time now
	//cmmjl_osc_timetag_now_to_ntp(&now);
	t_osc_timetag now = osc_timetag_now();
	omax_realtime_printTimetag(now, "now", __func__, __LINE__);
    
	// reinitialize if sample rate changes or n == 0.
	if(x->n == 0) {
		omax_realtime_clock_set_fc(0.1);

		// expected time per frame
		x->dt = x->dt0;

		// convert to ntp
		dt1 = osc_timetag_floatToTimetag(x->dt);

		// now -> t0
		x->t0 = now;
        
		// time we expect next frame to occur
		x->t1 = osc_timetag_add(x->t0, dt1);
		omax_realtime_printTimetag(x->t0, "x->t0", __func__, __LINE__);
		omax_realtime_printTimetag(x->t1, "x->t1", __func__, __LINE__);

		// no error on this tick
		x->dt_error = loop_err = 0.;

		// advance frame count
		x->n++;
		
		// everything in the struct has been updated, so just return
		return;
	}

	// compare the time now to what we expected it to be on the last tick
        if(osc_timetag_compare(now, x->t1) == 1){
		loop_err = osc_timetag_timetagToFloat(osc_timetag_subtract(now, x->t1));
	}else{
		loop_err = osc_timetag_timetagToFloat(osc_timetag_subtract(x->t1, now)) * -1.;
	}
	omax_realtime_printFloat(loop_err, "loop_err", __func__, __LINE__);

	// if the error is this big, then something bad has happened
	// such as a buffer-underrun
	// or the system clock was adjusted by a lot
	if(fabs(loop_err) > (2.*x->dt)) {
		omax_realtime_printFloat(2. * x->dt, "large loop error > 2. * x->dt", __func__, __LINE__);
		if(fabs(loop_err) > (5.*x->dt)) {
			omax_realtime_printFloat(5. * x->dt, "very large loop error > 5. * x->dt: hard reset", __func__, __LINE__);
			x->dt = x->dt0;
			dt1 = osc_timetag_floatToTimetag(x->dt);
			x->t0 = now;
			x->t1 = osc_timetag_add(x->t0, dt1);
			x->dt_error = loop_err = 0.;
			x->n = 1;
			omax_realtime_clock_set_fc(0.1);
			// everything in the struct has been updated, so just return
			return;
		} else {
			// soft reset by backing off the filter for a while
			omax_realtime_printFloat(5. * x->dt, "large loop error <= 5. * x->dt: soft reset", __func__, __LINE__);
			x->n = 10;
			omax_realtime_clock_set_fc(0.1);
			// x->n = 100;
			// omax_realtime_clock_set_fc(0.01);
		}
	}

	// update loop
	// t1 -> t0
	x->t0 = x->t1;
	omax_realtime_printTimetag(x->t0, "x->t0", __func__, __LINE__);
    
	// estimate t1
	dt1 = osc_timetag_floatToTimetag(loop_err * x->b + x->dt);
	t1_next = osc_timetag_add(dt1, x->t1);
	x->t1 = t1_next;
	omax_realtime_printTimetag(t1_next, "t1_next", __func__, __LINE__);

	// update estimated frame delta time
	x->dt += x->c * loop_err;
	x->dt_error = loop_err;
	x->n++;
    
	// if the error is this big, then something bad has happened
	// such as a buffer-underrun
	// or the system clock was adjusted by a lot
	// printf("%s:%d: loop_err (%f) > %f == %d\n", __func__, __LINE__, fabs(loop_err), 2. * x->dt, fabs(loop_err) > (5.*x->dt));
	// if(fabs(loop_err) > (2.*x->dt)) {
	// 	printf("%s:%d: loop_err (%f) > %f\n", __func__, __LINE__, fabs(loop_err), 2. * x->dt);
	// 	if(fabs(loop_err) > (5.*x->dt)) {
	// 		printf("%s:%d: hard reset\n", __func__, __LINE__);
	// 		// hard reset to initial conditions
	// 		x->dt = x->dt0;
	// 		//cmmjl_osc_timetag_float_to_ntp(x->dt, &dt1);
	// 		dt1 = osc_timetag_floatToTimetag(x->dt);
	// 		//cmmjl_osc_timetag_cpy(&(x->t0), &now); 
	// 		x->t0 = now;
	// 		//cmmjl_osc_timetag_add(&(x->t0), &dt1, &(x->t1));
	// 		x->t1 = osc_timetag_add(x->t0, dt1);
	// 		x->n = 0;
	// 		omax_realtime_clock_set_fc(0.01);
	// 	} else {
	// 		printf("%s:%d: soft reset\n", __func__, __LINE__);
	// 		// soft reset by backing off the filter for a while
	// 		x->n = 100;
	// 		omax_realtime_clock_set_fc(0.01);
	// 	}
	// }
    
	// adjust filter cutoff dynamically
	if(x->n == 100) {
		omax_realtime_clock_set_fc(0.01);
	}
	if(x->n == 1000) {
		omax_realtime_clock_set_fc(0.001);
	}
	if(x->n == 10000) {
		omax_realtime_clock_set_fc(0.0001);
	}
}

// when is now
void omax_realtime_clock_now(t_osc_timetag *t)
{
	t_omax_realtime_clock *x = omax_realtime_clock_get_master();
	*t = x->t0;
	//cmmjl_osc_timetag_cpy(t, &(x->t0));
}

// when is next
void omax_realtime_clock_next(t_osc_timetag *t)
{

	t_omax_realtime_clock *x = omax_realtime_clock_get_master();
	*t = x->t1;
	//cmmjl_osc_timetag_cpy(t, &(x->t1));
}

// how long until next in seconds
double omax_realtime_clock_dt(void)
{
	t_omax_realtime_clock *x = omax_realtime_clock_get_master();
	return x->dt;
}

