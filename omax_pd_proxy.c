#ifdef OMAX_PD_VERSION
#include "m_pd.h"
#include "m_imp.h"
#include "omax_pd_proxy.h"
#include "osc_mem.h"
#include <string.h>

void omax_pd_proxydispatchanything(t_object *xx, t_symbol *msg, int argc, t_atom *argv);
void omax_pd_proxydispatchfloat(t_object *xx, t_float ff);
void omax_pd_proxydispatchbang(t_object *xx);
void omax_pd_proxydispatchmethod(t_object *xx, t_symbol *msg, int argc, t_atom *argv);
			

void omax_pd_class_addmethod(t_omax_pd_proxy_class *c, t_method fp, t_symbol *msg)
{
	osc_hashtab_store(c->ht, strlen(msg->s_name), msg->s_name, (void *)fp);
	class_addmethod(c->class, (t_method)omax_pd_proxydispatchmethod, msg, A_GIMME, 0);
}

void omax_pd_class_addfloat(t_omax_pd_proxy_class *c, t_method fp)
{
	char *msg = "float";
	osc_hashtab_store(c->ht, strlen(msg), msg, (void *)fp);
	class_addfloat(c->class, (t_method)omax_pd_proxydispatchfloat);
}

void omax_pd_class_addbang(t_omax_pd_proxy_class *c, t_method fp)
{
	char *msg = "bang";
	osc_hashtab_store(c->ht, strlen(msg), msg, (void *)fp);
	class_addbang(c->class, (t_method)omax_pd_proxydispatchbang);
}

void omax_pd_class_addanything(t_omax_pd_proxy_class *c, t_method fp)
{
	char *msg = "anything";
	osc_hashtab_store(c->ht, strlen(msg), msg, (void *)fp);
	class_addanything(c->class, (t_method)omax_pd_proxydispatchanything);
}


t_omax_pd_proxy *omax_pd_proxynew(t_object *x, long inletnum, long *inletloc, t_omax_pd_proxy_class *class)
{
    t_omax_pd_proxy *p = (t_omax_pd_proxy *)pd_new(class->class);
	p->x = x;
	p->class = class;
	p->inletnum = inletnum;
	p->inletloc = inletloc;
	inlet_new(p->x, &p->pd, 0, 0);
	return p;
}

t_gotfn omax_pd_getfunctionforsymbol(t_object *x, t_symbol *msg)
{
	t_omax_pd_proxy_class *c = ((t_omax_pd_proxy *)x)->class;
	void *f = osc_hashtab_lookup(c->ht, strlen(msg->s_name), msg->s_name);
	return (t_gotfn)f;
}

void omax_pd_proxydispatchmethod(t_object *xx, t_symbol *msg, int argc, t_atom *argv)
{
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, msg);
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, msg, argc, argv);
	}

}

void omax_pd_proxydispatchanything(t_object *xx, t_symbol *msg, int argc, t_atom *argv)
{
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, gensym("anything"));
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, msg, argc, argv);
	}
    
}


void omax_pd_proxydispatchfloat(t_object *xx, t_float ff)
{
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, gensym("float"));
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x, (double)ff);
	}
}

void omax_pd_proxydispatchbang(t_object *xx)
{
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, gensym("bang"));
	if(f){
		*(x->inletloc) = x->inletnum;
		f(x->x);
	}
}

int omax_pd_proxygetinlet(t_object *xx)
{
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	return *(x->inletloc);
}

#endif
