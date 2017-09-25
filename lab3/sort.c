#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

typedef struct {
	void*		base;	// Array to sort.
	size_t		n;	// Number of elements in base.
	size_t		s;	// Size of each element.
	int		(*cmp)(const void*, const void*); // Behaves like strcmp
} arg_struct_t;

static double sec(void)
{
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return tval.tv_sec + (tval.tv_usec / 1000000.0);
}

static int cmp(const void* ap, const void* bp)
{	
	if (*((double*)ap) > (*(double*)bp)) {
		return 1;
	} else if ((*(double*)ap) < *((double*)bp)) {
		return -1;
	} else {
		return 0;
	}
}

void partition(double* a, size_t n, double* smaller, double* larger, size_t* small_count, size_t* large_count)
{
	double pivot = n/2;
	for (int i = 0; i < n; i++) {
		if (i < pivot) {
			smaller[*small_count] = a[i];
			*small_count = *small_count + 1;
		} else {
			larger[*large_count] = a[i];
			*large_count = *large_count + 1;
		}
	}	
}

void* thread_start(void* data) {
	arg_struct_t* args = (arg_struct_t*) data;
	qsort(args->base, args->n, args->s, cmp);
	return data;
}

void sortstuff(double* result, double* l1, double* l2, int n){
	int pos = 0;
	int x = 0;
	int y = 0;

	while (x < n && y < n) {
		if (l1[x] < l2[y]) {
			result[pos] = l1[x];
			x++;
		} else {
			result[pos] = l2[y];
			y++;
		}
		pos++;
	}

	if (x < y) {
		for (x; x < n; x++) {
			result[pos] = l1[x];
			pos++;
		} 
	} else {
		for (y; y < n; y++) {
			result[pos] = l2[y];
			pos++;
		} 
	}
}

void par_sort(
	void*		base,	// Array to sort.
	size_t		n,	// Number of elements in base.
	size_t		s,	// Size of each element.
	int		(*cmp)(const void*, const void*)) // Behaves like strcmp
{
	double* a = (double*)base;



	double* small = malloc(n * s);
	size_t* small_count = malloc(sizeof(size_t));
	*small_count = 0;
	double* large = malloc(n * s);
	size_t* large_count = malloc(sizeof(size_t));
	*large_count = 0;
	partition(a, n, small, large, small_count, large_count);


	double* small_l = malloc(*small_count * s);
	size_t* small_count_l = malloc(sizeof(size_t));
	*small_count_l = 0;
	double* large_l = malloc(*small_count * s);
	size_t* large_count_l = malloc(sizeof(size_t)); 
	*large_count_l = 0;
	partition(small, *small_count, small_l, large_l, small_count_l, large_count_l);


	double* small_r = malloc(*large_count * s);
	size_t* small_count_r = malloc(sizeof(size_t));
	*small_count_r = 0;
	double* large_r = malloc(*large_count * s);
	size_t* large_count_r = malloc(sizeof(size_t));
	*large_count_r = 0;
	partition(large, *large_count, small_r, large_r, small_count_r, large_count_r);
	
	pthread_t small_l_t;
	arg_struct_t args_small_l = {small_l, *small_count_l, s, cmp};
	pthread_create(&small_l_t, NULL, thread_start, &args_small_l);

	pthread_t large_l_t;
	arg_struct_t args_large_l = {large_l, *large_count_l, s, cmp};
	pthread_create(&large_l_t, NULL, thread_start, &args_large_l);

	pthread_t small_r_t;
	arg_struct_t args_small_r = {small_r, *small_count_r, s, cmp};
	pthread_create(&small_r_t, NULL, thread_start, &args_small_r);
	
	pthread_t large_r_t;
	arg_struct_t args_large_r = {large_r, *large_count_r, s, cmp};
	pthread_create(&large_r_t, NULL, thread_start, &args_large_r);
	
	pthread_join(small_l_t, NULL);
	pthread_join(large_l_t, NULL);
	pthread_join(small_r_t, NULL);
	pthread_join(large_r_t, NULL);
	double* temp_l = malloc((n/2) * s);
	double* temp_r = malloc((n/2) * s);
	sortstuff(temp_l, small_l, large_l, n/4);
	sortstuff(temp_r, small_r, large_r, n/4);
	sortstuff((double*) base, temp_l, temp_r, n/2);	

	printf("t1 work: %zu\nt2 work %zu\nt3 work %zu\nt4 work %zu\n", *small_count_l, *large_count_l, *small_count_r, *large_count_r);

	free(temp_l);
	free(temp_r);
	free(small_count);
	free(large_count);
	free(small_count_l);
	free(large_count_l);
	free(small_count_r);
	free(large_count_r);
	free(small);
	free(large);
	free(small_l);
	free(small_r);
	free(large_l);
	free(large_r);

}



int main(int ac, char** av)
{
	int		n = 10000;
	int		i;
	double*		a;
	double		start, end;

	if (ac > 1)
		sscanf(av[1], "%d", &n);

	srand(getpid());
	a = malloc(n * sizeof a[0]);
	for (i = 0; i < n; i++)
		a[i] = rand();

	start = sec();

#ifdef PARALLEL
	par_sort(a, n, sizeof a[0], cmp);
#else
	qsort(a, n, sizeof a[0], cmp);
#endif

	end = sec();

	printf("%1.4f s\n", end - start);

	for (i = 0; i < n-1; i++) {
		assert(a[i] <= a[i+1]);
	}

	free(a);
	
	return 0;
}
