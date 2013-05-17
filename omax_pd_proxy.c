#ifdef OMAX_PD_VERSION
#include "m_pd.h"
#include "m_imp.h"
#include "omax_pd_proxy.h"
#include "osc_mem.h"

void omax_pd_proxydispatchanything(t_object *xx, t_symbol *msg, int argc, t_atom *argv);
void omax_pd_proxydispatchfloat(t_object *xx, double ff);
void omax_pd_proxydispatchbang(t_object *xx);
void omax_pd_proxydispatchmethod(t_object *xx, t_symbol *msg, int argc, t_atom *argv);

#define omax_pd_class_new(class, name, new, free, size, flags, ...) 	\
		t_omax_pd_proxy_class *class = (t_omax_pd_proxy_class *)osc_mem_alloc(sizeof(t_osc_pd_proxy_class)); \
		class->class = class_new(name, new, free, size, flags, __VA_ARGS__); \
		class->ht = osc_hashtab_new();				

/*
t_omax_pd_proxy_class *omax_pd_class_new(t_symbol *name, t_newmethod *new, t_method *free, size_t size, int flags)
{
	t_omax_pd_proxy_class *c = (t_omax_pd_proxy_class *)osc_mem_alloc(sizeof(t_osc_pd_proxy_class));
	c->class = class_new(name, new, free, size, flags, A_GIMME, 0);
	c->ht = osc_hashtab_new();
}
*/

void omax_pd_class_addmethod(t_omax_pd_proxy_class *c, t_method fp, t_symbol *msg)
{
	osc_hashtab_store(c->ht, strlen(msg->s_name), msg->s_name, (void *)fp);
	class_addmethod(c, (t_method)omax_pd_proxydispatchmethod, msg, A_GIMME, 0);
}

void omax_pd_class_addfloat(t_omax_pd_proxy_class *c, t_method fp)
{
	char *msg = "float";
	osc_hashtab_store(c->ht, strlen(msg), msg, (void *)fp);
	class_addfloat(c, (t_method)omax_pd_proxydispatchfloat);
}

void omax_pd_class_addbang(t_omax_pd_proxy_class *c, t_method fp)
{
	char *msg = "bang";
	osc_hashtab_store(c->ht, strlen(msg), msg, (void *)fp);
	class_addbang(c, (t_method)omax_pd_proxydispatchbang);
}

void omax_pd_class_addanything(t_omax_pd_proxy_class *c, t_method fp)
{
	char *msg = "anything";
	osc_hashtab_store(c->ht, strlen(msg), msg, (void *)fp);
	class_addanything(c, (t_method)omax_pd_proxydispatchanything);
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
	t_omax_pd_proxy_class *c = ((t_omax_pd_proxy *)x)->c;
	void *f = osc_hashtab_lookup(c->ht, strlen(msg->s_name), msg->s_name);
	/*
	  for(int i = 0; i < ((t_pd)(x->ob_pd))->c_nmethod; i++){
	  post("msg %d : %s", i, ((t_pd)(x->ob_pd))->c_methods[i].me_name->s_name);
	  if(((t_pd)(x->ob_pd))->c_methods[i].me_name == msg){
	  f = ((t_pd)(x->ob_pd))->c_methods[i].me_fun;
	  break;
	  }
	  }
	*/
	return (t_gotfn)f;
}

void omax_pd_proxydispatchmethod(t_object *xx, t_symbol *msg, int argc, t_atom *argv)
{
	post("%s disp method %s", __func__,msg->s_name);
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	post("%s: %p", __func__, x);
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, msg);
    	post("f %s: %p", __func__, f);
	if(f){
		*(x->inletloc) = x->inletnum;
		post("disp meth inlet %d", x->inletnum);
		f(x->x, msg, argc, argv);
	}

}

void omax_pd_proxydispatchanything(t_object *xx, t_symbol *msg, int argc, t_atom *argv)
{
	post("%s disp any %s",__func__, msg->s_name);
	t_omax_pd_proxy *x = (t_omax_pd_proxy *)xx;
	post("%s: %p", __func__, x);
	t_gotfn f = omax_pd_getfunctionforsymbol(xx, msg);
	post(" %s: f = %p", __func__, f);
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
