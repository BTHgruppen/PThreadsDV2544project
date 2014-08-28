//==================================================//
//			   SCALING BOUNDED BUFFER				//
//==================================================//

/*

[X] static ints for NUM_BUFFERS and BUFFER_SIZE
[ ] Set the amount of buffers to a sensible number based on NO_PRODUCERS and NO_CONSUMERS
[ ] Each consumer thread selects an appropriate buffer to use. 
	If there are 2 buffers and 16 consumers, the first 8 should use buffer 1, and the other 8 buffer 2.
[ ] Same as previous, but for producers.


*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NO_PRODUCERS 16
#define NO_CONSUMERS 32
#define KILO 1024
#define MEGA (KILO*KILO)
#define ITEMS_TO_SEND 8*KILO // number of items to pass through the buffer.
#define DEBUG 1

static int NUM_BUFFERS = 2;
static int BUFFER_SIZE = 20;

// Structs.
typedef struct
{
    int in, out;
    int no_elems;
    int *buf;
    pthread_mutex_t lock;
} buffer_t;

// Variables.
static int no_items_sent = 0;
static int no_items_recieved = 0;
static int print_flag = 0;	// 1 = printouts, 0 = no printouts.
static buffer_t* buffers;

// Buffer initialization.
void init_buffers(void)
{
	buffers = malloc(sizeof(buffer_t)* NUM_BUFFERS);
	int i;
	for (i = 0; i < NUM_BUFFERS; i++)
	{
		buffers[i].buf = malloc(sizeof(BUFFER_SIZE));
		buffers[i].in = 0;
		buffers[i].out = 0;
		buffers[i].no_elems = 0;
		pthread_mutex_init(&buffers[i].lock, NULL);
	}
}

// Consumer code.
void *consumer(void *thr_id)
{
	sleep(1);
	pthread_exit(0);
}

// Producer code.
void *producer(void *thr_id)
{

	sleep(1);
	pthread_exit(0);
}

// Main entry point.
int main(int argc, char **argv)
{
	if (DEBUG)
	{
		printf("Debug is active.\n");
		printf("NO_PRODUCERS: %d\n", NO_PRODUCERS);
		printf("NO_CONSUMERS: %d\n", NO_CONSUMERS);
		printf("NUM_BUFFERS: %d\n\n", NUM_BUFFERS);
	}

	clock_t start_time = clock();

    long i;
    pthread_t prod_thrs[NO_PRODUCERS];
    pthread_t cons_thrs[NO_CONSUMERS];
    pthread_attr_t attr;

    init_buffers();
    pthread_attr_init (&attr);

    printf("Buffer size = %d, Buffer amount = %d, items to send = %d\n", BUFFER_SIZE, NUM_BUFFERS, ITEMS_TO_SEND);

	printf("Creating producer threads.\n");
    // Create the producer threads.
    for(i = 0; i < NO_PRODUCERS; i++)
	{
		if(pthread_create(&prod_thrs[i], &attr, producer, (void *)i) != 0)
		{
			printf("Producer thread creation failed. (t_id: %ld)\n", i);
		}
	}

	printf("Creating consumer threads.\n");
	// Create the consumer threads.
    for(i = 0; i < NO_CONSUMERS; i++)
	{
		if (pthread_create(&cons_thrs[i], &attr, consumer, (void *)i) != 0)
		{
			printf("Consumer thread creation failed. (t_id: %ld)\n", i);
		}
	}


	printf("Joining producer threads.\n");
    // Wait for all threads to terminate.
    for (i = 0; i < NO_PRODUCERS; i++)
	{
		printf("Things go wrong between this...\n");

		pthread_join(prod_thrs[i], NULL);

		printf("...And this!\n");
	}

	printf("Joining consumer threads.\n");
    for (i = 0; i < NO_CONSUMERS; i++)
	{
		pthread_join(cons_thrs[i], NULL);
	}

	clock_t end_time = clock();
	printf("Elapsed time: %ld\n", (end_time - start_time)/CLOCKS_PER_SEC);
}
