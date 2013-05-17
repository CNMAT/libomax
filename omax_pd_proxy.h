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
	t_omax_pd_proxy_class *c;
	long inletnum;
	long *inletloc;
}t_omax_pd_proxy;


t_omax_pd_proxy_class *omax_pd_class_new(t_symbol *name, t_newmethod *new, t_method *free, size_t size, int flags);
void omax_pd_class_addanything(void *c, t_method fp);
void omax_pd_class_addmethod(void *c, t_method fp, t_symbol *msg);
void omax_pd_class_addfloat(void *c, t_method fp);
void omax_pd_class_addbang(void *c, t_method fp);
t_omax_pd_proxy *omax_pd_proxynew(t_object *x, long inletnum, long *inletloc, t_omax_pd_proxy_class *class);


int omax_pd_proxygetinlet(t_object *xx);
#endif
