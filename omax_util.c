/*
  Written by John MacCallum, The Center for New Music and Audio Technologies,
  University of California, Berkeley.  Copyright (c) 2010-12, The Regents of
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

/** 	\file omax_util.c
	\author John MacCallum

*/

#ifndef WIN_VERSION
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>
#else
#include <windows.h>
#endif

#include <inttypes.h>
#include "osc.h"
#include "osc_mem.h"
#include "osc_byteorder.h"
#include "osc_match.h"
#include "osc_bundle_s.h"
#include "osc_bundle_u.h"
#include "osc_bundle_iterator_s.h"
#include "osc_bundle_iterator_u.h"
#include "osc_message_u.h"
#include "osc_message_iterator_u.h"
#include "osc_message_s.h"
#include "osc_message_iterator_s.h"
#include "osc_atom_s.h"
#include "omax_util.h"


#ifdef OMAX_PD_VERSION
#define OMAX_PD_MAXSTRINGSIZE (1<<16)

#define NUMATOMSINMESS 3
#define A_LONG -666
#define A_SYM A_SYMBOL

extern void atom_setfloat(t_atom *atom, t_float f);
extern void atom_setlong(t_atom *atom, long l);
extern void atom_setsym(t_atom *atom, t_symbol *s);
extern t_int atom_getlong(t_atom *atom);
extern t_symbol *atom_getsym(t_atom *atom);
extern t_atomtype atom_gettype(t_atom *atom);
#else
#define NUMATOMSINMESS 2

#endif


t_symbol *omax_util_ps_FullPacket = NULL;

//#define __ODOT_PROFILE__
//#include "osc_profile.h"

int omax_util_liboErrorHandler(const char * const errorstr)
{
	// stupid max window doesn't respect newlines
	int len = strlen(errorstr) + 1;
	char buf[len];
	strncpy(buf, errorstr, len);

	char *s = buf;
	char *e = buf;
	while(*e){
		if(*e == '\n'){
			*e = '\0';
			error("%s", s);
			s = e + 1;
		}
		e++;
	}
	if(e != s){
		error("%s", s);
	}
	return 0;
}

void omax_util_oscLenAndPtr2Atoms(t_atom *argv, long len, char *ptr)
{
#ifdef OMAX_PD_VERSION
	uint32_t l = (uint32_t)len;
	atom_setfloat(argv, *((t_float *)&l));

#if UINTPTR_MAX == 0xffffffff
	uint32_t i2 = (uint32_t)ptr;
	float f1 = 0;
	float f2 = *((float *)&i2);
#elif UINTPTR_MAX == 0xffffffffffffffff
	uint32_t i1 = (uint32_t)((((uint64_t)ptr) & 0xffffffff00000000) >> 32);
	uint32_t i2 = (uint32_t)((((uint64_t)ptr) & 0xffffffff));
	float f1 = *((float *)&i1);
	float f2 = *((float *)&i2);
#else
#error This is BAD BAD BAD
#endif    
	atom_setfloat(argv+1, f1);
	atom_setfloat(argv+2, f2);

#else
	atom_setlong(argv, len);
	atom_setlong(argv + 1, (long)ptr);
#endif
    
}


void omax_util_outletOSC(void *outlet, long len, char *ptr)
{
	if(!omax_util_ps_FullPacket){
		omax_util_ps_FullPacket = gensym("FullPacket");
	}
	t_atom out[NUMATOMSINMESS];
	omax_util_oscLenAndPtr2Atoms(out, len, ptr);
	outlet_anything(outlet, omax_util_ps_FullPacket, NUMATOMSINMESS, out);
}

t_osc_err omax_util_outletOSC_u(void *outlet, t_osc_bndl_u *bndl)
{
	if(!omax_util_ps_FullPacket){
		omax_util_ps_FullPacket = gensym("FullPacket");
	}
	long len = 0;
	char *ptr = NULL;
	t_osc_err e = osc_bundle_u_serialize(bndl, &len, &ptr);
	if(e){
		return e;
	}
	if(len && ptr){
		omax_util_outletOSC(outlet, len, ptr);
		osc_mem_free(ptr);
	}
	return OSC_ERR_NONE;
}

int omax_util_getNumAtomsInOSCMsg(t_osc_msg_s *m)
{
	int n = 1; // address;
	t_osc_msg_it_s *it = osc_msg_it_s_get(m);
	while(osc_msg_it_s_hasNext(it)){
		t_osc_atom_s *a = osc_msg_it_s_next(it);
		switch(osc_atom_s_getTypetag(a)){
		case OSC_BUNDLE_TYPETAG:
			n += (NUMATOMSINMESS+1); // FullPacket <len> <address>
			break;
		case 'b':
			{
				char *data = osc_atom_s_getData(a);
				n += ntoh32(*((int32_t *)data));	
			}
			break;
		default:
			n += 1;
			break;
		}
	}
	osc_msg_it_s_destroy(it);
	return n;
}

int omax_util_braceError(char *s)
{
    int i, len = strlen(s);
    for( i = 0; i < len; i++ )
    {
        if(s[i] == '{' || s[i] == '}')
            return 1;
    }
    return 0;
}

void omax_util_hashBrackets2Curlies(char *s)
{
    
	char c;
	int len = strlen(s);
    
	int i, j = 0;
	for( i = 0; i < len; i++ )
    {
        c = s[i];
        if( i < len-1 )
        {
            if(s[i] == '#' && s[i+1] == '['){
                c = '{';
                i++;
            } else if(s[i] == ']' && s[i+1] == '#') {
                c = '}';
                i++;
            }
        }
        s[j++] = c;
    }
    
	while(j < len)
		s[j++] = '\0';
    
}

void omax_util_curlies2hashBrackets(char **ptr, long bufsize)
{
	//   printf("%s", __func__);
    
	char *str = (*ptr);
	if(!str)
    {
        error("no string in buffer");
        return;
    }
    
	int i, j = 0;
	int len = strlen(str);
	char buf[len * 2]; //<< max possible size with every character being a { or }
	memset(buf, '\0', len * 2);
    
	for( i = 0; i < len; i++ )
    {
        if (str[i] == '{')
        {
            buf[j++] = '#';
            buf[j++] = '[';
        }
        else if (str[i] == '}')
        {
            buf[j++] = ']';
            buf[j++] = '#';
        } else {
            buf[j++] = str[i];
        }
    }
	if(j != i)
    {
        memset(*ptr, '\0', bufsize);
        //        *ptr = (char *)realloc(*ptr, sizeof(char) * j);
        strcpy(*ptr, buf);
    }
}



int omax_util_oscMsg2MaxAtoms(t_osc_msg_s *m, t_atom *av)
{
	t_atom *ptr = av;
	if(osc_message_s_getAddress(m)){
		atom_setsym(ptr, gensym(osc_message_s_getAddress(m)));
	}else{
		// there are some cases when a message won't have an address
	}
	ptr++;

	t_osc_msg_it_s *it = osc_msg_it_s_get(m);
	while(osc_msg_it_s_hasNext(it)){
		t_osc_atom_s *a = osc_msg_it_s_next(it);
		switch(osc_atom_s_getTypetag(a)){
		case 'i':
		case 'I':
		case 'h':
		case 'H':
		case 'T':
		case 'F':
		case 'N':
			atom_setlong(ptr++, osc_atom_s_getInt32(a));
			break;
		case 'f':
		case 'd':
			atom_setfloat(ptr++, osc_atom_s_getFloat(a)); 
			break;
		case 's':		
			{
				int len = osc_atom_s_getStringLen(a);
				char buf[len + 1];
				char *bufptr = buf;
				osc_atom_s_getString(a, len + 1, &bufptr);
#ifdef OMAX_PD_VERSION
                char extrabuf[OMAX_PD_MAXSTRINGSIZE];
                strcpy(extrabuf, bufptr);
                char *pdbufptr = extrabuf;
                omax_util_curlies2hashBrackets(&pdbufptr, OMAX_PD_MAXSTRINGSIZE);
                
                if(omax_util_braceError(pdbufptr))
                    return 1;
                
                atom_setsym(ptr++, gensym(extrabuf));
#else
				atom_setsym(ptr++, gensym(buf));
#endif
			}
			break;
		case 'b':
			/*
			{
				int j, n = osc_atom_s_sizeof(a);
				char *data = osc_atom_s_getBlob(a);
				for(j = 0; j < n - 4; j++){
					atom_setlong(ptr++, (long)data[j + 4]);
				}
			}
			 */
			break;
		case OSC_BUNDLE_TYPETAG:
			{
				char *data = osc_atom_s_getData(a);
				atom_setsym(ptr++, gensym("FullPacket"));
                
				t_atom bnddata[NUMATOMSINMESS];
				omax_util_oscLenAndPtr2Atoms(bnddata, (long)ntoh32(*((uint32_t *)data)), (char *)((long)(data + 4)));
				int i;
				for(i = 0; i < NUMATOMSINMESS; i++)
					{
						*ptr++ = bnddata[i];
					}
				/*
				  atom_setlong(ptr++, ntoh32(*((uint32_t *)data)));
				  atom_setlong(ptr++, (long)(data + 4));
				*/
			}
			break;
		}
	}
	osc_msg_it_s_destroy(it);
    return 0;
}

// encode a FullPacket <len> <ptr> message as a nested bundle
void omax_util_maxFullPacketToOSCAtom_u(t_osc_atom_u **osc_atom, t_atom *len, t_atom *ptr)
{
	if(!(*osc_atom)){
		*osc_atom = osc_atom_u_alloc();
	}
#ifdef OMAX_PD_VERSION
    float ff = atom_getfloat(len);
    long l = (long)*((uint32_t *)&ff);
    ff = atom_getfloat(ptr);
    uint64_t l1 = *((uint64_t *)&ff);
    l1 <<= 32;
    ff = atom_getfloat(ptr+1);
    uint64_t l2 = *((uint64_t *)&ff);
    char *p = (char *)(l1 | l2);
#else
    long l = atom_getlong(len);
    char *p = (char *)atom_getlong(ptr);
#endif
	osc_atom_u_setBndl(*osc_atom, l, p);
}

void omax_util_maxAtomToOSCAtom_u(t_osc_atom_u **osc_atom, t_atom *max_atom)
{
	if(!(*osc_atom)){
		*osc_atom = osc_atom_u_alloc();
	}
	switch(atom_gettype(max_atom)){
	case A_FLOAT:
		osc_atom_u_setDouble(*osc_atom, atom_getfloat(max_atom));
		break;
	case A_LONG:
		osc_atom_u_setInt32(*osc_atom, atom_getlong(max_atom));
		break;
	case A_SYM:
		{
#ifdef OMAX_PD_VERSION
			t_symbol *sym = atom_getsym(max_atom);
			char buf[ strlen(sym->s_name) + 1 ];
			strcpy(buf, sym->s_name);
			omax_util_hashBrackets2Curlies(buf);
			osc_atom_u_setString(*osc_atom, buf);
#else
			t_symbol *s = atom_getsym(max_atom);
			if(s && s->s_name){
				osc_atom_u_setString(*osc_atom, s->s_name);
				break;
			}
#endif
			// intentional fall-through to default
		}
	default:
		osc_atom_u_free(*osc_atom);
		*osc_atom = NULL;
		break;
	}
}

t_osc_err omax_util_maxAtomsToOSCMsg_u(t_osc_msg_u **msg, t_symbol *address, int argc, t_atom *argv)
{
	if(!(*msg)){
		*msg = osc_message_u_alloc();
		if(address){
			t_osc_err e = osc_message_u_setAddress(*msg, address->s_name);
			if(e){
				return e;
			}
		}
		int i;
		for(i = 0; i < argc; i++){
			t_osc_atom_u *a = NULL;
			if(atom_gettype(argv + i) == A_SYM){
				if((atom_getsym(argv + i) == gensym("FullPacket")) && argc - i >= 3){
					// FullPacket to be encoded as nested bundle
					omax_util_maxFullPacketToOSCAtom_u(&a, argv + 1, argv + 2);
#ifdef OMAX_PD_VERSION
					i += 3;
#else
					i += 2;
#endif
				}else{
					omax_util_maxAtomToOSCAtom_u(&a, argv + i);
				}
			}else{
				omax_util_maxAtomToOSCAtom_u(&a, argv + i);
			}
			if(a){
				osc_message_u_appendAtom(*msg, a);
			}
		}
	}
	return OSC_ERR_NONE;
}

t_osc_err omax_util_copyBundleWithSubs_u(t_osc_bndl_u **dest, t_osc_bndl_u *src, int argc, t_atom *argv, int *hassubs)
{
	if(!src){
		return -1;
	}
	*hassubs = 0;
	t_osc_bndl_u *bcopy = osc_bundle_u_alloc();
	t_osc_bndl_it_u *bit = osc_bndl_it_u_get(src);

	while(osc_bndl_it_u_hasNext(bit)){
		t_osc_msg_u *m = osc_bndl_it_u_next(bit);
		t_osc_msg_u *mcopy = osc_message_u_alloc();
		{
			char *address = osc_message_u_getAddress(m);
			int addresslen = strlen(address) + 1;
			char copy[addresslen];
			char *copyptr = copy;
			memcpy(copy, address, addresslen);
			int newaddresslen = addresslen;
			int dosub = 0;
			int addedtolist = 0;
			char *tok = NULL;
			while((tok = strsep(&copyptr, "$"))){
				if(copyptr){
					if(copyptr[0] < 58 && copyptr[0] > 47){
						dosub = 1;
						char *endp = NULL;
						long l = strtol(copyptr, &endp, 0) - 1;
						if(l < argc){
							switch(atom_gettype(argv + l)){
							case A_LONG:
								newaddresslen += snprintf(NULL, 0, "%lld", (long long)atom_getlong(argv + l));
								break;
							case A_FLOAT:
								newaddresslen += snprintf(NULL, 0, "%f", atom_getfloat(argv + l));
								break;
							case A_SYM:
								newaddresslen += strlen(atom_getsym(argv + l)->s_name);
								break;
							default:
								break;
							}
						}else{
							newaddresslen += snprintf(NULL, 0, "$%ld", l + 1);
						}
					}else{
						newaddresslen++;
					}
				}
			}
			if(dosub){
				*hassubs = 1;
				//newaddresslen += 16; // never can be too careful...
				char *newaddress = osc_mem_alloc(newaddresslen);
				char *ptr = newaddress;
				memcpy(copy, address, addresslen);
				copyptr = copy;
				char *lasttok = copy, *tok = NULL;
				while((tok = strsep(&copyptr, "$"))){
					ptr += sprintf(ptr, "%s", lasttok);
					if(copyptr){
						if(copyptr[0] < 58 && copyptr[0] > 47){
							char *endp = NULL;
							long l = strtol(copyptr, &endp, 0) - 1;
							if(l < argc){
								switch(atom_gettype(argv + l)){
								case A_LONG:
									ptr += sprintf(ptr, "%lld", (long long)atom_getlong(argv + l));
									break;
								case A_FLOAT:
									ptr += sprintf(ptr, "%f", atom_getfloat(argv + l));
									break;
								case A_SYM:
									ptr += sprintf(ptr, "%s", atom_getsym(argv + l)->s_name);
									break;
								default:
									break;
								}
							}else{
								ptr += sprintf(ptr, "$%ld", l + 1);
							}
							lasttok = endp;
						}else{
							*ptr++ = '$';
							*ptr = '\0';
						}
					}
				}
				osc_message_u_setAddressPtr(mcopy, newaddress, NULL);
			}else{
				osc_message_u_setAddress(mcopy, osc_message_u_getAddress(m));
			}
		}

		t_osc_msg_it_u *mit = osc_msg_it_u_get(m);
		while(osc_msg_it_u_hasNext(mit)){
			t_osc_atom_u *a = osc_msg_it_u_next(mit);
			t_osc_atom_u *acopy = NULL;
			if(osc_atom_u_getTypetag(a) == 's'){
				char *s = osc_atom_u_getStringPtr(a);
				if(s && s[0] == '$' && strlen(s) > 1){
					long l = strtol(s + 1, NULL, 0);
					if(l > 0){
						*hassubs = 1;
						if(l < argc + 1){
							omax_util_maxAtomToOSCAtom_u(&acopy, argv + (l - 1));
						}else{
							osc_atom_u_copy(&acopy, a);
						}
					}else{
						osc_atom_u_copy(&acopy, a);
					}
				}else{
					osc_atom_u_copy(&acopy, a);
				}
			}else if(osc_atom_u_getTypetag(a) == OSC_BUNDLE_TYPETAG){
				t_osc_bndl_u *ncopy = NULL;
				omax_util_copyBundleWithSubs_u(&ncopy, osc_atom_u_getBndl(a), argc, argv, hassubs);
				acopy = osc_atom_u_alloc();
				osc_atom_u_setBndl_u(acopy, ncopy);
			}else{
				osc_atom_u_copy(&acopy, a);
			}
			osc_message_u_appendAtom(mcopy, acopy);
		}
		osc_msg_it_u_destroy(mit);
		osc_bundle_u_addMsg(bcopy, mcopy);
	}
	osc_bndl_it_u_destroy(bit);
	*dest = bcopy;
	return OSC_ERR_NONE;
}
