/*
  Written by John MacCallum, The Center for New Music and Audio Technologies,
  University of California, Berkeley.  Copyright (c) 2009-11, The Regents of
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

/** 	\file omax_util.h
	\author John MacCallum

*/

#ifndef __OMAX_UTIL_H__
#define __OMAX_UTIL_H__

#include "osc.h"
#ifndef ulong
#define ulong
#endif
#ifndef uint
#define uint
#endif
#ifndef ushort
#define ushort
#endif
#include "ext.h"
#ifdef ulong
#undef ulong
#endif
#ifdef uint
#undef uint
#endif
#ifdef ushort
#define ushort
#endif
#include "ext_obex.h"
#include "osc_message_s.h"

#ifdef __cplusplus
extern "C" {
#endif

// this is a workaround for a bug in Max.  the function that passes arguments to functions
// declared with static types (ie, not with A_GIMME) is not thread safe.  This has been fixed in
// max 6, but not in earlier versions.
#define OMAX_UTIL_GET_LEN_AND_PTR \
	if(argc != 2){\
		object_error((t_object *)x, "%s: expected 2 arguments but got %d", __func__, argc);\
		return;\
	}\
	if(atom_gettype(argv) != A_LONG){\
		object_error((t_object *)x, "%s: argument 1 should be an int", __func__);\
		return;\
	}\
	if(atom_gettype(argv + 1) != A_LONG){\
		object_error((t_object *)x, "%s: argument 2 should be an int", __func__);\
		return;\
	}\
	long len = atom_getlong(argv);\
	long ptr = atom_getlong(argv + 1);

// that stupid macro above used to be defined in osc.h.  I moved it here but 
// was too lazy to change all the files that used it, so we have this #define below.
// As I visit each of the files, I'll change it and then this can be removed...
#define OSC_GET_LEN_AND_PTR OMAX_UTIL_GET_LEN_AND_PTR

void omax_util_outletOSC(void *outlet, long len, char *ptr);
void omax_util_maxFullPacketToOSCAtom_u(t_osc_atom_u **osc_atom, t_atom *len, t_atom *ptr);
void omax_util_maxAtomToOSCAtom_u(t_osc_atom_u **osc_atom, t_atom *max_atom);
t_osc_err omax_util_maxAtomsToOSCMsg_u(t_osc_msg_u **msg, t_symbol *address, int argc, t_atom *argv);
int omax_util_getNumAtomsInOSCMsg(t_osc_msg_s *msg);
void omax_util_oscMsg2MaxAtoms(t_osc_msg_s *m, t_atom *av);

int omax_util_liboErrorHandler(const char * const errorstr);

#ifdef __cplusplus
}
#endif

#endif 
