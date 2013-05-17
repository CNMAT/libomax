#ifdef OMAX_PD_VERSION
#include "osc_hashtab.h"
#define proxy_new omax_pd_proxynew
#define proxy_getinlet omax_pd_proxygetinlet

typedef struct _omax_pd_proxy_class{
	void *class;
	t_osc_hashtab *ht;
} t_omax_pd_proxy_class;

typedef struct _omax_pd_proxy{
    t_pd pd;
	t_object *x;
	t_omax_pd_proxy_class *class;
	long inletnum;
	long *inletloc;
}t_omax_pd_proxy;


#define omax_pd_class_new(classptr, name, new, free, size, flags, ...){ 	\
    classptr = (t_omax_pd_proxy_class *)osc_mem_alloc(sizeof(t_omax_pd_proxy_class)); \
    classptr->class = class_new(name, new, free, size, flags, __VA_ARGS__); \
    classptr->ht = osc_hashtab_new(0,NULL);}


void omax_pd_class_addanything(t_omax_pd_proxy_class *c, t_method fp);
void omax_pd_class_addmethod(t_omax_pd_proxy_class *c, t_method fp, t_symbol *msg);
void omax_pd_class_addfloat(t_omax_pd_proxy_class *c, t_method fp);
void omax_pd_class_addbang(t_omax_pd_proxy_class *c, t_method fp);
t_omax_pd_proxy *omax_pd_proxynew(t_object *x, long inletnum, long *inletloc, t_omax_pd_proxy_class *class);


int omax_pd_proxygetinlet(t_object *xx);
#endif
