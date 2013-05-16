#include "omax_pd_proxy.h"

void omax_pd_class_addmethod(void *c, t_method fp, t_symbol *msg)
{
	addmethod(c->class, (method)omax_pd_proxydispatchanything, msg, A_GIMME, 0);
}

void omax_pd_class_addfloat(void *c, t_method fp)
{
	addmethod(c->class, (method)omax_pd_proxydispatchfloat);
}

void omax_pd_class_addbang(void *c, t_method fp)
{
	addmethod(c->class, (method)omax_pd_proxydispatchbang);
}

t_omax_pd_proxy *omax_pd_proxynew(t_object *x, long inletnum, long *inletloc)
{
	t_omax_pd_proxy *p = osc_mem_alloc(sizeof(t_omax_pd_proxy));
	p->x = x;
	p->inletnum = inletnum;
	p->inletloc = inletloc;
	inlet_new(x, p->x->ob_pd, 0, 0);
	return p;
}

method omax_pd_getfunctionforsymbol(t_object *x, t_symbol *msg)
{
	method f = NULL;
	for(int i = 0; i < x->ob_pd.c_nmethod; i++){
		if(x->ob_pd.c_methods[i]->me_name == msg){
			f = x->ob_pd.c_methods[i]->me_fun;
			break;
		}
	}
	return f;
}

void omax_pd_proxydispatchanything(t_object *x, t_symbol *msg, int argc, t_atom *argv)
{
	printf("%s: %p\n", __func__, x);
	method f = omax_pd_getfunctionforsymbol(x, msg);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, msg, argc, argv);
	}
}

void omax_pd_proxydispatchfloat(t_object *x, double ff)
{
	printf("%s: %p\n", __func__, x);
	method f = omax_pd_getfunctionforsymbol(x, msg);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, ff);
	}
}

void omax_pd_proxydispatchbang(t_object *x)
{
	printf("%s: %p\n", __func__, x);
	method f = omax_pd_getfunctionforsymbol(x, msg);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x);
	}
}

int omax_pd_proxygetinlet(t_object *x)
{
	printf("%s: %p\n", __func__, x);
	return *(x->inletloc);
}
