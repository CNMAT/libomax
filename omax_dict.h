/*
  Written by John MacCallum, The Center for New Music and Audio Technologies,
  University of California, Berkeley.  Copyright (c) 2009-13, The Regents of
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

/** 	\file omax_dict.h
	\author John MacCallum

*/

#ifndef __OMAX_DICT_H__
#define __OMAX_DICT_H__

#ifdef OMAX_PD_VERSION
//no dictionaries in pd
#else

#include "osc.h"
#include "osc_bundle_u.h"

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

#ifdef __cplusplus
extern "C" {
#endif

#define OMAX_DICT_DICTIONARY(obj_type, obj, fp)				\
	void omax_dict_dictionary(obj_type *obj, t_symbol *msg, int argc, t_atom *argv){ \
		if(argc != 1){ \
			object_error((t_object *)obj, "expected 1 argument to message dictionary but got %d", argc);	\
			return;\
		}		\
		if(atom_gettype(argv) != A_SYM){			\
			object_error((t_object *)obj, "the argument to the message dictionary should be a symbol"); \
			return;						\
		}							\
		t_symbol *name = atom_getsym(argv);			\
		omax_dict_processDictionary((void *)obj, name, (void (*)(void *, t_symbol*, int, t_atom*))fp); \
	}

  //#ifndef WIN_VERSION
  //t_dictionary *(*omax_dict_dictobj_register)(t_dictionary *d, t_symbol **name);
  //#endif

int omax_dict_resolveDictStubs(void);
void omax_dict_dictionaryToOSC(t_dictionary *dict, t_osc_bndl_u *bndl_u);
void omax_dict_processDictionary(void *x, t_symbol *name, void (*fp)(void *x, t_symbol *msg, int argc, t_atom *argv));
void omax_dict_bundleToDictionary(t_osc_bndl_s *bndl, t_dictionary *dict, bool stripLeadingSlash);

#ifdef __cplusplus
}
#endif

#endif

#endif
