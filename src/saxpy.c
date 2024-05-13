/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed} 
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum itertions} 
 *
 * @author     Danny Munera
 * @date       2020
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

struct Result {
	double Y_avgs;
	double* Y;
};

struct arg {
	int j;
	int p;
	double* Y;
	double a;
	double* X;
	double* Y_avgs;
	int max_iters;
    int n;
};

void *saxpy(void *argY) {
	// Recibo los datos de afuera del hilo
	struct arg *arg = (struct arg *)argY;
    int j = arg->j;

	pthread_mutex_t mutex;

	double Y_avgs2;
	int i;
	int it;
	
	for(it = 0; it < arg->max_iters; it++){

		double Y_avgs2 = 0.0;
		for(i = j; i < arg->p; i+=arg->n){				//Recorre toda el vector Y y hace la suma de Ys

			arg->Y[i] = arg->Y[i] + arg->a * arg->X[i];
			Y_avgs2 += arg->Y[i];
		}

		//Solución race condition
		pthread_mutex_lock(&mutex);
		arg->Y_avgs[it] = arg->Y_avgs[it] + (Y_avgs2/arg->p);
		pthread_mutex_unlock(&mutex);

	}
}


int main(int argc, char* argv[]){
	// Variables to obtain command line parameters
	unsigned int seed = 1;
  	int p = 10000000;
  	int n_threads = 2;
  	int max_iters = 1000;
  	// Variables to perform SAXPY operation
	double* X;
	double a;
	double* Y;
	double* Y_avgs;
	int i, it;
	// Variables to get execution time
	struct timeval t_start, t_end;
	double exec_time;

	// Getting input values
	int opt;
	//Lee uno por uno argumentos del cmd hasta llegar al final -1
	while((opt = getopt(argc, argv, ":p:s:n:i:")) != -1){  
		switch(opt){  
			case 'p':  
			printf("vector size: %s\n", optarg);
			// Convierte el string optarg a entero largo ignorado lo que no sea numeros
			p = strtol(optarg, NULL, 10);
			assert(p > 0 && p <= 2147483647);
			break;  
			case 's':  
			printf("seed: %s\n", optarg);
			seed = strtol(optarg, NULL, 10);
			break;
			case 'n':  
			printf("threads number: %s\n", optarg);
			n_threads = strtol(optarg, NULL, 10);
			break;  
			case 'i':  
			printf("max. iterations: %s\n", optarg);
			max_iters = strtol(optarg, NULL, 10);
			break;  
			case ':':  
			printf("option -%c needs a value\n", optopt);  
			break;  
			case '?':  
			fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum itertions>]\n", argv[0]);
			exit(EXIT_FAILURE);
		}  
	}  

	//Generar numeros aleatorios con la semilla seed
	srand(seed);

	printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", \
	 p, seed, n_threads, max_iters);	

	// initializing data
	X = (double*) malloc(sizeof(double) * p);
	Y = (double*) malloc(sizeof(double) * p);
	Y_avgs = (double*) malloc(sizeof(double) * max_iters);

	//Lleno el vector X y Y con numeros aleatorios con tamaño p
	for(i = 0; i < p; i++){
		X[i] = (double)rand() / RAND_MAX;
		Y[i] = (double)rand() / RAND_MAX;
	}
	//Inicializo el vector Y_avgs con 0.0 en todas las casillas
	for(i = 0; i < max_iters; i++){
		Y_avgs[i] = 0.0;
	}
	a = (double)rand() / RAND_MAX;

#ifdef DEBUG
	printf("vector X= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ",X[i]);
	}
	printf("%f ]\n",X[p-1]);

	printf("vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);

	printf("a= %f \n", a);	
#endif

	
	

	gettimeofday(&t_start, NULL);
	//SAXPY iterative SAXPY mfunction

    struct arg *arg = malloc(sizeof (struct arg));
    arg->a = a;
    arg->X = X;
    arg->Y = Y;
    arg->p = p;
    arg->Y_avgs = Y_avgs;
    arg->max_iters = max_iters;
    arg->n = n_threads;

    // Crear un array de hilos y argumentos para pasar a los hilos ...
    pthread_t threads[n_threads];

    //inicializador de todos los hilos
    for (i = 0; i < n_threads; i++){
		
        arg->j = i;

        // Crear hilo
        pthread_create(&threads[i], NULL, saxpy, arg);

        // Esperar un poco para que el hilo se cree antes de crear el siguiente hilo
        usleep(9000);

    }
    
    
    // Esperar a que todos los hilos terminen ...
    for(int i = 0; i < n_threads; i++) {

        pthread_join(threads[i], NULL);

    }
	
	gettimeofday(&t_end, NULL);

#ifdef DEBUG
	printf("RES: final vector Y= [ ");
	for(i = 0; i < p-1; i++){
		printf("%f, ", Y[i]);
	}
	printf("%f ]\n", Y[p-1]);
#endif
	
	// Computing execution time
	exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
	exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
	printf("Execution time: %f ms \n", exec_time);
	printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
	printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);
	return 0;
}	



