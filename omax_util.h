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

#ifdef OMAX_PD_VERSION
#include "m_pd.h"
#else //MAXVERSION
#include "ext.h"
#include "ext_obex.h"
#endif

#ifdef ulong
#undef ulong
#endif
#ifdef uint
#undef uint
#endif
#ifdef ushort
#define ushort
#endif
#include "osc_message_s.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined WIN_VERSION || defined(__linux__)
  char *strsep(char **stringp, const char *delim);
#endif

void omax_util_oscLenAndPtr2Atoms(t_atom *argv, long len, char *ptr);
void omax_util_curlies2hashBrackets(char **ptr, long bufsize);
void omax_util_hashBrackets2Curlies(char *s);

void omax_util_outletOSC(void *outlet, long len, char *ptr);
t_osc_err omax_util_outletOSC_u(void *outlet, t_osc_bndl_u *bndl);
void omax_util_maxFullPacketToOSCAtom_u(t_osc_atom_u **osc_atom, t_atom *len, t_atom *ptr);
void omax_util_maxAtomToOSCAtom_u(t_osc_atom_u **osc_atom, t_atom *max_atom);
t_osc_err omax_util_maxAtomsToOSCMsg_u(t_osc_msg_u **msg, t_symbol *address, int argc, t_atom *argv);
int omax_util_getNumAtomsInOSCMsg(t_osc_msg_s *msg);
int omax_util_oscMsg2MaxAtoms(t_osc_msg_s *m, t_atom *av);

int omax_util_liboErrorHandler(void *context, t_osc_err errorcode, const char * const errorstr);

t_osc_err omax_util_copyBundleWithSubs_u(t_osc_bndl_u **dest, t_osc_bndl_u *src, int argc, t_atom *argv, int *hassubs);

#ifdef __cplusplus
}
#endif

#endif
