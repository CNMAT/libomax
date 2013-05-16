#ifdef OMAX_PD_VERSION
#include "m_pd.h"
#include "m_imp.h"
#include "omax_pd_proxy.h"
#include "osc_mem.h"

void omax_pd_class_addmethod(void *c, t_method fp, t_symbol *msg)
{
	class_addmethod(c, (t_method)omax_pd_proxydispatchanything, msg, A_GIMME, 0);
}

void omax_pd_class_addfloat(void *c, t_method fp)
{
	class_addfloat(c, (t_method)omax_pd_proxydispatchfloat);
}

void omax_pd_class_addbang(void *c, t_method fp)
{
	class_addbang(c, (t_method)omax_pd_proxydispatchbang);
}

t_omax_pd_proxy *omax_pd_proxynew(t_object *x, long inletnum, long *inletloc)
{
	t_omax_pd_proxy *p = osc_mem_alloc(sizeof(t_omax_pd_proxy));
	p->x = x;
	p->inletnum = inletnum;
	p->inletloc = inletloc;
	inlet_new(x, (t_pd *)p->x->ob_pd, 0, 0);
	return p;
}

t_gotfn omax_pd_getfunctionforsymbol(t_object *x, t_symbol *msg)
{
	t_gotfn f = NULL;
	for(int i = 0; i < ((t_pd)(x->ob_pd))->c_nmethod; i++){
		if(((t_pd)(x->ob_pd))->c_methods[i].me_name == msg){
			f = ((t_pd)(x->ob_pd))->c_methods[i].me_fun;
			break;
		}
	}
	return f;
}

void omax_pd_proxydispatchanything(t_object *xx, t_symbol *msg, int argc, t_atom *argv)
{
    t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	printf("%s: %p\n", __func__, x);
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, msg);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, msg, argc, argv);
	}
}

void omax_pd_proxydispatchfloat(t_object *xx, double ff)
{
    t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	printf("%s: %p\n", __func__, x);
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, &s_float);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, ff);
	}
}

void omax_pd_proxydispatchbang(t_object *xx)
{
    t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	printf("%s: %p\n", __func__, x);
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, &s_bang);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x);
	}
}

int omax_pd_proxygetinlet(t_object *xx)
{
    t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	printf("%s: %p\n", __func__, x);
	return *(x->inletloc);
}

#endif