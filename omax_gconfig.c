/*
  Written by John MacCallum, The Center for New Music and Audio Technologies,
  University of California, Berkeley.  Copyright (c) 2013, The Regents of
  the University of California (Regents). 
  Permission to use, copy, modify, distribute, and distribute modified versions
  of this software and its documentation without fee and without a signed
  licensing agreement, is hereby granted, provided that the above copyright
  notice, this paragraph and the following two paragraphs appear in all copies,
  modifications, and distributions.

  IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
  SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
  OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
  BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
  HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
  MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#ifndef WIN_VERSION
// #include <Carbon/Carbon.h>
// #include <CoreServices/CoreServices.h>
#else
#include <windows.h>
#endif

#include <stdio.h>
#include "ext.h"
#include "omax_gconfig.r"
#include "omax_gconfig.h"
#include "osc.h"
#include "osc_atom_array_u.h"
#include "osc_error.h"
#include "osc_timetag.h"

//t_omax_gconfig *omax_gconfig = NULL;
t_symbol *omax_gconfig_ps = NULL;
#define omax_gconfig_string "omax_gconfig"
#define OMAX_GCONFIG_DSPSTARTTIME "/o/gconfig/dspstarttime"

t_omax_gconfig *omax_gconfig_alloc(void)
{
	return (t_omax_gconfig *)osc_bundle_u_alloc();
}

void omax_gconfig_free(t_omax_gconfig *gc)
{
	if(gc){
		osc_bundle_u_free((t_osc_bndl_u *)gc);
	}
}

void omax_gconfig_makePS(void)
{
	if(!omax_gconfig_ps){
		omax_gconfig_ps = gensym(omax_gconfig_string);
		omax_gconfig_ps->s_thing = NULL;
	}
}

t_osc_err omax_gconfig_makeGconfig()
{
	omax_gconfig_makePS();
	if(!(omax_gconfig_ps->s_thing)){
		omax_gconfig_ps->s_thing = (void *)omax_gconfig_alloc();
		if(!(omax_gconfig_ps->s_thing)){
			return OSC_ERR_OUTOFMEM;
		}
	}
	return OSC_ERR_NONE;
}

t_osc_err omax_gconfig_getGconfig(t_omax_gconfig **gc)
{
	t_osc_err e = omax_gconfig_makeGconfig();
	if(e){
		return e;
	}
	*gc = (t_omax_gconfig *)omax_gconfig_ps->s_thing;
	return OSC_ERR_NONE;
}

t_osc_err omax_gconfig_get(char *property, t_omax_gconfig **gconfig)
{
	t_osc_err e = OSC_ERR_NONE;
	if(!omax_gconfig_ps){
		return e;
	}
	if(!(omax_gconfig_ps->s_thing)){
		return e;
	}
	t_osc_msg_ar_u *ar = NULL;
	t_omax_gconfig *gc = (t_omax_gconfig *)(omax_gconfig_ps->s_thing);
	e = osc_bundle_u_lookupAddress_copy((t_osc_bndl_u *)gc, property, &ar, 1);
	if(e){
		return e;
	}
	t_osc_bndl_u *b = osc_bundle_u_alloc();
	e = osc_bundle_u_addMsgArray(b, ar);
	if(e){
		return e;
	}
	*gconfig = (t_omax_gconfig *)b;
	return e;
}

t_osc_err omax_gconfig_set(char *property, t_osc_atom_ar_u *values)
{
	t_osc_err e = OSC_ERR_NONE;
	t_omax_gconfig *gc = NULL;
	e = omax_gconfig_getGconfig(&gc);
	if(e){
		return e;
	}
	t_osc_msg_u *m = osc_message_u_alloc();
	if(!m){
		return OSC_ERR_OUTOFMEM;
	}
	osc_message_u_setAddress(m, property);
	e = osc_message_u_setArgArrayCopy(m, values);
	if(e){
		return e;
	}
	e = omax_gconfig_setMsg_u(m);
	if(e){
		return e;
	}
	osc_message_u_free(m);
	return e;
}

t_osc_err omax_gconfig_setMsg_u(t_osc_msg_u *msg)
{
	t_omax_gconfig *gc = NULL;
	t_osc_err e = omax_gconfig_getGconfig(&gc);
	if(e){
		return e;
	}
	e = osc_bundle_u_addMsgWithoutDupsCopy((t_osc_bndl_u *)gc, msg);
	return e;
}

t_osc_err omax_gconfig_setMsg_s(t_osc_msg_u *msg)
{
	return OSC_ERR_NONE;
}

t_osc_err omax_gconfig_setDspStartTime(t_osc_timetag t)
{
	t_osc_atom_ar_u *ar = osc_atom_array_u_alloc(1);
	osc_atom_u_setTimetag(osc_atom_array_u_get(ar, 0), t);
	return omax_gconfig_set(OMAX_GCONFIG_DSPSTARTTIME, ar);
}

t_osc_err omax_gconfig_getDspStartTime(t_osc_timetag *t)
{
	t_osc_err e = OSC_ERR_NONE;
	t_omax_gconfig *gc = NULL;
	e = omax_gconfig_get(OMAX_GCONFIG_DSPSTARTTIME, &gc);
	if(e){
		return e;
	}
	if(gc){
		t_osc_msg_u *m = osc_bundle_u_getFirstMsg((t_osc_bndl_u *)gc);
		t_osc_atom_u *a = NULL;
		osc_message_u_getArg(m, 0, &a);
		*t = osc_atom_u_getTimetag(a);
		omax_gconfig_free(gc);
	}
	return e;
}
