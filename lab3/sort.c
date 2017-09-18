#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>



static double sec(void)
{
	struct timeval tval;
	gettimeofday(&tval, NULL);
	return tval.tv_sec + (tval.tv_usec / 1000000.0);
}

void partition(double* a, size_t n, double* smaller, double* larger, size_t* small_count, size_t* large_count)
{
	int r = rand() % n;
	double pivot = a[r];
	for (int i = 0; i < n; i++) {
		if (a[i] < pivot) {
			smaller[*small_count] = a[i];
			*small_count++;
		} else {
			larger[*large_count] = a[i];
			*large_count++;
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
	size_t small_count = 0;
	double* large = malloc(n * s);
	size_t large_count = 0;
	partition(a, n, small, large, &small_count, &large_count);

	double* small_l = malloc(small_count * s);
	size_t small_count_l = 0;
	double* large_l = malloc(small_count * s);
	size_t large_count_l = 0;
	partition(small, small_count, small_l, large_l, &small_count_l, &large_count_l);


	double* small_r = malloc(large_count * s);
	size_t small_count_r = 0;
	double* large_r = malloc(large_count * s);
	size_t large_count_r = 0;
	partition(large, large_count, small_r, large_r, &small_count_r, &large_count_r);

	pthread_t* small_l_t;
	arg_struct_t args_small_l = {(void*)small_l, small_count_l, s, cmp};

	pthread_t* large_l_t;
	arg_struct_t args_large_l = {(void*)large_l, large_count_l, s, cmp};

	pthread_t* small_r_t;
	arg_struct_t args_small_r = {(void*)small_r, small_count_r, s, cmp};
	
	pthread_t* large_r_t;
	arg_struct_t args_large_r = {(void*)large_r, large_count_r, s, cmp};

	
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

int main(int ac, char** av)
{
	int		n = 2000000;
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

	printf("%1.2f s\n", end - start);

	free(a);
	
	return 0;
}
