//==================================================//
//		    PARALLELL GAUSSIAN ELIMINATION			//
//==================================================//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>

#define MAX_SIZE 8
#define THREADS 8

typedef double matrix[MAX_SIZE][MAX_SIZE];

int	N;					// Matrix size.
int	maxnum;				// max number of element.
char *Init;				// matrix init type.
int PRINT;				// print switch.
matrix A;				// matrix A.
double b[MAX_SIZE];		// vector b.
double y[MAX_SIZE];		// vector y.

// Thread variables.
pthread_t thread[THREADS];
typedef struct
{
	int k;
	int start;
	int stop;
} ThreadData;

ThreadData threadData[THREADS];

void Work(void);
void* ThreadWork(void*);
void Init_Matrix(void);
void Print_Matrix(void);
void Read_Options(int, char **);

void Init_Default()
{
    N = MAX_SIZE;
    Init = "rand";
    maxnum = 15.0;
    PRINT = 0;
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	double start_time, end_time;

    int i;

	for(i = 0; i < THREADS; i++)
	{
		threadData[i].k = 0;
		threadData[i].start = 0;
		threadData[i].stop = 0;
	}

	// Init default values.
    Init_Default();

	// Read arguments.
    Read_Options(argc, argv);

	// Init the matrix.
    Init_Matrix();

	// Start timer.
	start_time = MPI_Wtime();

	// Do guassian elimination.
    Work();

	// Stop timer.
	end_time = MPI_Wtime();

    if (PRINT == 1)
	{
		printf("===== AFTER GUASSIAN ELIMINATION ======\n");
		Print_Matrix();
		printf("=======================================\n\n");
	}

	double time_taken = (end_time - start_time);
	printf("Execution time: %f\n", time_taken);
}

void Work(void)
{
    int i, j, k, offset;

    // Gaussian elimination algorithm, Algo 8.4 from Grama.
    for (k = 0; k < N; k++) 
	{
		for (j = k+1; j < N; j++)
		{
			// Division step.
			A[k][j] = A[k][j] / A[k][k]; 
		}

		y[k] = b[k] / A[k][k];
		A[k][k] = 1.0;

		// Calculate offset
		offset = (N - (k+1)) / THREADS;

		// Thread the elimination step.
		for(i = 0; i < (THREADS - 1); i++)
		{
			// Set data and create the thread.
			threadData[i].k = k;
			threadData[i].start	= (k + 1) + (offset * i);
			threadData[i].stop	= (k + 1) + (offset * (i + 1));
			pthread_create(&thread[i], NULL, ThreadWork, (void*)&threadData[i]);
		}

		// Set data, and create last thread.
		threadData[(THREADS - 1)].k = k;
		threadData[(THREADS - 1)].start	= (k + 1) + (offset * (THREADS - 1));
		threadData[(THREADS - 1)].stop	= N;
		pthread_create(&thread[THREADS - 1], NULL, ThreadWork, (void*)&threadData[(THREADS - 1)]);

		// Wait for all threads to terminate.
		for (i = 0; i < THREADS; i++)
		{
			pthread_join(thread[i], NULL);
		}
    }
}

void* ThreadWork(void* input)
{
	int i, j;
	ThreadData threadData = *(ThreadData*) input;
		
	for (i = threadData.start; i < threadData.stop; i++) 
	{
		for (j = threadData.k+1; j < N; j++)
		{
			// Elimination step.
			A[i][j] = A[i][j] - A[i][threadData.k] * A[threadData.k][j]; 
		}

		b[i] = b[i] - A[i][threadData.k] * y[threadData.k];
		A[i][threadData.k] = 0.0;
	}
}

void Init_Matrix()
{
    int i, j;

 	printf("Mode      = Parallell");
    printf("\nSize      = %dx%d ", N, N);
    printf("\nMaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...\n");
 
    if (strcmp(Init, "rand") == 0) 
	{
		for (i = 0; i < N; i++)
		{
			for (j = 0; j < N; j++) 
			{
				if (i == j)
				{
					 // Diagonal dominance.
					A[i][j] = (double)(rand() % maxnum) + 5.0;
				}

				else
				{
					A[i][j] = (double)(rand() % maxnum) + 1.0;
				}
			}
		}
    }

    if (strcmp(Init, "fast") == 0) 
	{
		for (i = 0; i < N; i++) 
		{
			for (j = 0; j < N; j++) 
			{
				if (i == j)
				{
					// Diagonal dominance.
					A[i][j] = 5.0;
				}

				else
				{
					A[i][j] = 2.0;
				}
			}
		}
    }

    // Initialize vectors b and y.
    for (i = 0; i < N; i++) 
	{
		b[i] = 2.0;
		y[i] = 1.0;
    }

    printf("Done!\n\n");

    if (PRINT == 1)
	{
		printf("===== BEFORE GUASSIAN ELIMINATION =====\n");
		Print_Matrix();
	}
}

void Print_Matrix()
{
    int i, j;
 
    printf("\nMatrix A:\n");

    for (i = 0; i < N; i++) 
	{
		printf("[");

		for (j = 0; j < N; j++)
		{
			printf(" %5.2f,", A[i][j]);
		}

		printf("]\n");
    }

    printf("\nVector b:\n[");

    for (j = 0; j < N; j++)
	{
		printf(" %5.2f,", b[j]);
	}

    printf("]\n");
    printf("\nVector y:\n[");

    for (j = 0; j < N; j++)
	{
		printf(" %5.2f,", y[j]);
	}

    printf("]\n");
    printf("\n");
}
 
void Read_Options(int argc, char **argv)
{
    char *prog;
    prog = *argv;

    while (++argv, --argc > 0)
	{
		if (**argv == '-')
		{
			switch ( *++*argv ) 
			{
				case 'n':
					--argc;
					N = atoi(*++argv);
				break;

				case 'h':
					printf("\nHELP: try sor -u \n\n");
					exit(0);
				break;

				case 'u':
					printf("\nUsage: sor [-n problemsize]\n");
					printf("           [-D] show default values \n");
					printf("           [-h] help \n");
					printf("           [-I init_type] fast/rand \n");
					printf("           [-m maxnum] max random no \n");
					printf("           [-P print_switch] 0/1 \n");
					exit(0);
				break;

				case 'D':
					printf("\nDefault:  n         = %d ", N);
					printf("\n          Init      = rand" );
					printf("\n          maxnum    = 5 ");
					printf("\n          P         = 0 \n\n");
					exit(0);
				break;

				case 'I':
					--argc;
					Init = *++argv;
				break;

				case 'm':
					--argc;
					maxnum = atoi(*++argv);
				break;

				case 'P':
					--argc;
					PRINT = atoi(*++argv);
				break;

				default:
					printf("%s: ignored option: -%s\n", prog, *argv);
					printf("HELP: try %s -u \n\n", prog);
				break;
			} 
		}
	}
}