/*
  Written by John MacCallum, The Center for New Music and Audio Technologies,
  University of California, Berkeley.  Copyright (c) 2010-13, The Regents of
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

/** 	\file omax_dict.c
	\author John MacCallum

*/

#ifdef OMAX_PD_VERSION
//no dicts in pd
#else


#ifndef WIN_VERSION
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#else
#include <windows.h>
#endif

#include <inttypes.h>

#include "osc.h"
#include "osc_mem.h"
#include "osc_message_u.h"
#include "osc_atom_u.h"
#include "osc_bundle_iterator_s.h"

#include "omax_dict.h"
#include "omax_util.h"
#include "ext_dictionary.h"
#include "ext_dictobj.h"

t_symbol *omax_dict_ps_FullPacket = NULL;

//#ifndef WIN_VERSION
static int omax_dict_haveDict;
static int omax_dict_dictStubsResolved;
static t_dictionary *(*omax_dict_dictobj_findregistered_retain)(t_symbol *name);
static t_max_err (*omax_dict_dictobj_release)(t_dictionary *d);
//#endif

#ifdef WIN_VERSION
int omax_dict_resolveDictStubs(void)
{
	//something like this, i assume

	// A simple program that uses LoadLibrary and 
	// GetProcAddress to access myPuts from Myputs.dll. 
 
	//#include <windows.h> 
	//#include <stdio.h> 
 
	//typedef int (__cdecl *MYPROC)(LPWSTR); 
 
	//int main( void ) 
	//{ 
	if(omax_dict_dictStubsResolved){
		return omax_dict_haveDict;
	}
	omax_dict_dictStubsResolved = 1;
	char *dllpath = NULL;
	short version = maxversion();
	HINSTANCE hinstLib; 
	//MYPROC ProcAdd; 
	BOOL fFreeResult, fRunTimeLinkSuccess = FALSE; 

	if((version & 0xFF0) == 0x710){
		dllpath = "C:\\Program Files (x86)\\Cycling '74\\Max 7\\resources\\support\\MaxAPI.dll";
	}else if((version & 0xFF0) == 0x700){
		dllpath = "C:\\Program Files (x86)\\Cycling '74\\Max\\support\\MaxAPI.dll";
	}else if((version & 0xFF0) == 0x610){
		dllpath = "C:\\Program Files (x86)\\Cycling '74\\Max 6.1\\support\\MaxAPI.dll";
	}else if((version & 0xFF0) == 0x600){
		dllpath = "C:\\Program Files (x86)\\Cycling '74\\Max 6.0\\support\\MaxAPI.dll";
	}else if((version & 0xF00) == 0x500){
		return 0;
		dllpath = "C:\\Program Files (x86)\\Cycling '74\\Max 5.0\\support\\MaxAPI.dll";
	}else{
		return 0;
	}
 
	// Get a handle to the DLL module.
	hinstLib = LoadLibrary(TEXT(dllpath));
 
	// If the handle is valid, try to get the function address.
 
	if (hinstLib != NULL) 
		{ 
			omax_dict_dictobj_findregistered_retain = GetProcAddress(hinstLib, "dictobj_findregistered_retain"); 
			if(!omax_dict_dictobj_findregistered_retain){
				return 0;
			}

			omax_dict_dictobj_release = GetProcAddress(hinstLib, "dictobj_release"); 
			if(!omax_dict_dictobj_release){
				return 0;
			}

			omax_dict_dictobj_register = GetProcAddress(hinstLib, "dictobj_register"); 
			if(!omax_dict_dictobj_register){
				return 0;
			}
			omax_dict_haveDict = 1;
 
			// If the function address is valid, call the function.
			/*
			  if (NULL != ProcAdd) 
			  {
			  fRunTimeLinkSuccess = TRUE;
			  (ProcAdd) (L"Message sent to the DLL function\n"); 
			  }
			*/
			// Free the DLL module.
 
			fFreeResult = FreeLibrary(hinstLib); 
		} 

	// If unable to call the DLL function, use an alternative.
	/*
	  if (! fRunTimeLinkSuccess) 
	  printf("Message printed from executable\n"); 
	*/
	return omax_dict_haveDict;

	//}
	//*/
}
#else
int omax_dict_resolveDictStubs(void)
{
	if(omax_dict_dictStubsResolved){
		return omax_dict_haveDict;
	}
	omax_dict_dictStubsResolved = 1;
	char *frameworkpath = NULL;
	short version = maxversion();

	// I don't think this will work for standalones
	if((version & 0xF00) == 0x700){
		printf("7\n");
		frameworkpath = "/Applications/Max.app/Contents/Frameworks/MaxAPI.framework";
	}else if((version & 0xFF0) == 0x610){
		frameworkpath = "/Applications/Max 6.1/Max.app/Contents/Frameworks/MaxAPI.framework";
	}else if((version & 0xFF0) == 0x600){
		frameworkpath = "/Applications/Max6/Max.app/Contents/Frameworks/MaxAPI.framework";
	}else if((version & 0xF00) == 0x500){
		frameworkpath = "/Applications/Max5/MaxMSP.app/Contents/Frameworks/MaxAPI.framework";
	}else{
		return 0;
	}

	int frameworkpath_len = strlen(frameworkpath);
	OSStatus err;
	short path;
	char name[MAX_PATH_CHARS];
	long type = 0;

	omax_dict_haveDict = 0;
	//char frameworkpath[MAX_PATH_CHARS];
	//sprintf(frameworkpath,"/Applications/Max6/Max.app/Contents/Frameworks/MaxAPI.framework");
	//if (!locatefile_extended(name, &path, &type, 0L, 0)){
	char natname[MAX_PATH_CHARS];

	//if(!path_topathname(path, name, frameworkpath)){
	CFStringRef str;
	CFURLRef url;
	CFBundleRef maxapi_bundle_ref;

	path_nameconform(frameworkpath, natname, PATH_STYLE_NATIVE, PATH_TYPE_PATH);
	str = CFStringCreateWithCString(kCFAllocatorDefault, natname, kCFStringEncodingUTF8);
	if((url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str, kCFURLPOSIXPathStyle, true))){
		// we need to get rid of this in a quitmethodthingy
		if((maxapi_bundle_ref = CFBundleCreate(kCFAllocatorDefault, url))){
			omax_dict_dictobj_findregistered_retain = CFBundleGetFunctionPointerForName(maxapi_bundle_ref, CFSTR("dictobj_findregistered_retain"));
			if(!omax_dict_dictobj_findregistered_retain){
				return 0;
			}

			omax_dict_dictobj_release = CFBundleGetFunctionPointerForName(maxapi_bundle_ref, CFSTR("dictobj_release"));
			if(!omax_dict_dictobj_release){
				return 0;
			}

			omax_dict_dictobj_register = CFBundleGetFunctionPointerForName(maxapi_bundle_ref, CFSTR("dictobj_register"));
			if(!omax_dict_dictobj_register){
				return 0;
			}
			/*
			  The above file explicitly loads each function to a typed function pointer like the following:
			  pf_cgCreateContext = CFBundleGetFunctionPointerForName(g_cg_bundle_ref, CFSTR("cgCreateContext"));
			  if(!pf_cgCreateContext) {
			  error("jit.gl.shader: unable to load CG framework function 'cgCreateContext'");
			  }
			*/
			omax_dict_haveDict = 1;
		}
	broken:
		CFRelease(url);
	}
	CFRelease(str);
	//}
	//}
	return omax_dict_haveDict;
}
#endif

void omax_dict_dictionaryToOSC(t_dictionary *dict, t_osc_bndl_u *bndl_u)
{
	if(!dict || !bndl_u){
		return;
	}
	long nkeys = 0;
	t_symbol **keys = NULL;
	dictionary_getkeys(dict, &nkeys, &keys);
	if(nkeys && keys){
		for(int i = 0; i < nkeys; i++){
			char *key = keys[i]->s_name;
			int keylen = strlen(key);
			char addy[keylen + 2];
			if(*key != '/'){
				*addy = '/';
				strncpy(addy + 1, key, keylen + 1);
			}else{
				strncpy(addy, key, keylen + 1);
			}
			long argc = 0;
			t_atom *argv = NULL;
			dictionary_getatoms(dict, keys[i], &argc, &argv);
			/*
			  if(argc == 1){
			  if(atom_gettype(argv) == A_OBJ){
			  t_dictionary *dict2 = object_dictionaryarg(1, argv);
			  if(dict2){
			  t_osc_bndl_u *bndl2_u = osc_bundle_u_alloc();
			  omax_dict_dictionaryToOSC(dict2, bndl2_u);
			  long len = 0; 
			  char *bndl2 = NULL;
			  osc_bundle_u_serialize(bndl2_u, &len, &bndl2);
			  t_osc_msg_u *msg = osc_message_u_alloc();
			  osc_message_u_setAddress(msg, addy);
			  osc_message_u_appendBndl(msg, len, bndl2);
			  osc_bundle_u_addMsg(bndl_u, msg);
			  osc_bundle_u_free(bndl2_u);
			  osc_mem_free(bndl2);
			  continue;
			  }
			  }
			  }
			*/
			t_osc_msg_u *msg = NULL;
			omax_util_maxAtomsToOSCMsg_u(&msg, gensym(addy), argc, argv);
			osc_bundle_u_addMsg(bndl_u, msg);
		}
	}
	dictionary_freekeys(dict, nkeys, keys);
}

void omax_dict_processDictionary(void *x, t_symbol *name, void (*fp)(void *x, t_symbol *msg, int argc, t_atom *argv))
{
	//#ifdef WIN_VERSION
	//t_dictionary *dict = omax_dict_dictobj_findregistered_retain(name);
	//#else
	t_dictionary *dict = dictobj_findregistered_retain(name);//omax_dict_dictobj_findregistered_retain(name);
	//#endif
	t_osc_bndl_u *bndl_u = osc_bundle_u_alloc();
	omax_dict_dictionaryToOSC(dict, bndl_u);
	//long len = 0;
	//char *bndl = NULL;
	t_osc_bndl_s *bs = osc_bundle_u_serialize(bndl_u);
	t_atom a[2];
	if(bs){
		atom_setlong(a, osc_bundle_s_getLen(bs));
		atom_setlong(a + 1, (long)osc_bundle_s_getPtr(bs));
		fp(x, NULL, 2, a);
		osc_bundle_s_deepFree(bs);
	}
	osc_bundle_u_free(bndl_u);
	//osc_mem_free(bndl);
	//#ifdef WIN_VERSION
	//dictobj_release(dict);
	//#else
	dictobj_release(dict);//omax_dict_dictobj_release(dict);
	//#endif
}

void omax_dict_bundleToDictionary(t_osc_bndl_s *bndl, t_dictionary *dict, bool stripLeadingSlash)
{
	if(!bndl || !dict){
		return;
	}
	t_osc_bndl_it_s *it = osc_bndl_it_s_get(osc_bundle_s_getLen(bndl), osc_bundle_s_getPtr(bndl));
	while(osc_bndl_it_s_hasNext(it)){
		t_osc_msg_s *m = osc_bndl_it_s_next(it);
		int n = omax_util_getNumAtomsInOSCMsg(m);
		t_atom a[n];
		t_atom aa[n];
		int nn = 0;
		omax_util_oscMsg2MaxAtoms(m, a);
		t_symbol *k = gensym(atom_getsym(a)->s_name + stripLeadingSlash);
		for(int i = 1; i < n; i++){
			if(atom_gettype(a + i) == A_SYM){
				if(!omax_dict_ps_FullPacket){
					omax_dict_ps_FullPacket = gensym("FullPacket");
				}
				if(atom_getsym(a + i) == omax_dict_ps_FullPacket){
					t_osc_bndl_s *bndl2 = osc_bundle_s_alloc(atom_getlong(a + i + 1),
										 (char *)atom_getlong(a + i + 2));
					t_dictionary *dict2 = dictionary_new();
					omax_dict_bundleToDictionary(bndl2, dict2, stripLeadingSlash);
					atom_setobj(aa + nn++, (t_object *)dict2);
					i += 2;
					osc_bundle_s_free(bndl2);
					continue;
				}
			}
			aa[nn++] = a[i];
		}
		switch(nn){
		case 0:
			break;
		case 1:
			dictionary_appendatom(dict, k, aa);
			break;
		default:
			dictionary_appendatoms(dict, k, nn, aa);
			break;
		}
	}
	osc_bndl_it_s_destroy(it);
}

#endif
