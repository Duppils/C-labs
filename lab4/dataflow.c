#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include "dataflow.h"
#include "error.h"
#include "list.h"
#include "set.h"

typedef struct vertex_t	vertex_t;
typedef struct task_t	task_t;

/* cfg_t: a control flow graph. */
struct cfg_t {
	size_t			nvertex;	/* number of vertices		*/
	size_t			nsymbol;	/* width of bitvectors		*/
	vertex_t*		vertex;		/* array of vertex		*/
};

/* vertex_t: a control flow graph vertex. */
struct vertex_t {
	size_t			index;		/* can be used for debugging	*/
	set_t*			set[NSETS];	/* live in from this vertex	*/
	set_t*			prev;		/* alternating with set[IN]	*/
	size_t			nsucc;		/* number of successor vertices */
	vertex_t**		succ;		/* successor vertices 		*/
	list_t*			pred;		/* predecessor vertices		*/
	bool			listed;		/* on worklist			*/
	pthread_mutex_t		v_lock;
};

static void clean_vertex(vertex_t* v);
static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ);



cfg_t* new_cfg(size_t nvertex, size_t nsymbol, size_t max_succ)
{
	size_t		i;
	cfg_t*		cfg;

	cfg = calloc(1, sizeof(cfg_t));
	if (cfg == NULL)
		error("out of memory");

	cfg->nvertex = nvertex;
	cfg->nsymbol = nsymbol;

	cfg->vertex = calloc(nvertex, sizeof(vertex_t));
	if (cfg->vertex == NULL)
		error("out of memory");

	for (i = 0; i < nvertex; i += 1)
		init_vertex(&cfg->vertex[i], i, nsymbol, max_succ);

	return cfg;
}

static void clean_vertex(vertex_t* v)
{
	int		i;

	for (i = 0; i < NSETS; i += 1)
		free_set(v->set[i]);
	free_set(v->prev);
	free(v->succ);
	free_list(&v->pred);
}

static void init_vertex(vertex_t* v, size_t index, size_t nsymbol, size_t max_succ)
{
	int		i;

	v->index	= index;
	v->succ		= calloc(max_succ, sizeof(vertex_t*));

	if (v->succ == NULL)
		error("out of memory");
	
	for (i = 0; i < NSETS; i += 1)
		v->set[i] = new_set(nsymbol);

	v->prev = new_set(nsymbol);

	

	
}

void free_cfg(cfg_t* cfg)
{
	size_t		i;

	for (i = 0; i < cfg->nvertex; i += 1)
		clean_vertex(&cfg->vertex[i]);
	free(cfg->vertex);
	free(cfg);
}

void connect(cfg_t* cfg, size_t pred, size_t succ)
{
	vertex_t*	u;
	vertex_t*	v;

	u = &cfg->vertex[pred];
	v = &cfg->vertex[succ];

	u->succ[u->nsucc++ ] = v;
	insert_last(&v->pred, u);
}

bool testbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	return test(cfg->vertex[v].set[type], index);
}

void setbit(cfg_t* cfg, size_t v, set_type_t type, size_t index)
{
	set(cfg->vertex[v].set[type], index);
}

typedef struct {
	list_t*			worklist;
	pthread_mutex_t*	mutex;
	pthread_mutex_t*	lock;
} arg_struct_t;

void* start_thread(void* data)
{
	vertex_t*	u;
	vertex_t*	v;
	set_t*		prev;
	size_t		j;
	list_t*		p;
	list_t*		h;
	set_t* 		temp;
	

	arg_struct_t* args = (arg_struct_t*) data;
	pthread_mutex_t* mutex = args->mutex;
	pthread_mutex_lock(mutex);
	u = remove_first(&(args->worklist));
	pthread_mutex_unlock(mutex);	
	
	while (u != NULL) {
		pthread_mutex_lock(&(u->v_lock));
		u->listed = false;

		reset(u->set[OUT]);

		for (j = 0; j < u->nsucc; ++j) {
			//pthread_mutex_lock(u->succ[j]->v_lock);
			or(u->set[OUT], u->set[OUT], u->succ[j]->set[IN]);
			//pthread_mutex_unlock(u->succ[j]->v_lock);
		}

		prev = u->prev;
		u->prev = u->set[IN];
		u->set[IN] = prev;

		
		/* in our case liveness information... */
		propagate(u->set[IN], u->set[OUT], u->set[DEF], u->set[USE]);	
		

		if (u->pred != NULL && !equal(u->prev, u->set[IN])) {
			p = h = u->pred;
			do {
				v = p->data;
				if (!v->listed) {
					pthread_mutex_lock(mutex);
					v->listed = true;
					insert_last(&(args->worklist), v);
					pthread_mutex_unlock(mutex);
				}

				p = p->succ;

			} while (p != h);
		}
		pthread_mutex_unlock(&(u->v_lock));
		pthread_mutex_lock(mutex);
		u = remove_first(&(args->worklist));
		pthread_mutex_unlock(mutex);
	}
	return data;
}	

void liveness(cfg_t* cfg)
{
	list_t*		worklist;
	pthread_mutex_t mutex;
	size_t		i;
	vertex_t*	u;
	worklist = NULL;
	size_t thread_count = 4;
	pthread_t threads[thread_count];

	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i];
	


		insert_last(&worklist, u);
		u->listed = true;
	}
	

	for (i = 0; i < cfg->nvertex; i++) {
		u = &cfg->vertex[i];
		pthread_mutex_init(&(u->v_lock), NULL);
	}


	pthread_mutex_init(&mutex, NULL); //check
	arg_struct_t args = {worklist, &mutex};
	
	for (i = 0; i < thread_count; i++)
		pthread_create(&(threads[i]), NULL, start_thread, &args);

	for (i = 0; i < thread_count; i++)
		pthread_join(threads[i], NULL);

	pthread_mutex_destroy(&mutex); 
}

void print_sets(cfg_t* cfg, FILE *fp)
{
	size_t		i;
	vertex_t*	u;

	for (i = 0; i < cfg->nvertex; ++i) {
		u = &cfg->vertex[i]; 
		fprintf(fp, "use[%zu] = ", u->index);
		print_set(u->set[USE], fp);
		fprintf(fp, "def[%zu] = ", u->index);
		print_set(u->set[DEF], fp);
		fputc('\n', fp);
		fprintf(fp, "in[%zu] = ", u->index);
		print_set(u->set[IN], fp);
		fprintf(fp, "out[%zu] = ", u->index);
		print_set(u->set[OUT], fp);
		fputc('\n', fp);
	}
}
