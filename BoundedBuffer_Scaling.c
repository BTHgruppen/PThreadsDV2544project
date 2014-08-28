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

#define NO_PRODUCERS 4 //16
#define NO_CONSUMERS 4 //32
#define KILO 1024
#define MEGA (KILO*KILO)
#define ITEMS_TO_SEND 8*KILO // number of items to pass through the buffer.
#define DEBUG 1

static int NUM_BUFFERS = 2;
static int BUFFER_SIZE = 20;


static pthread_mutex_t g_lock;
static int g_pot = 0;
static int g_toSend = 10000;
static int g_toReceive = 10000;



// Structs.
typedef struct
{
    int in, out;
    int no_elems;
    int *buf;
    pthread_mutex_t lock;
	int items_sent;
	int items_received;
	int items_to_send;
} buffer_t;

// Variables.
//static int no_items_sent = 0;
//static int no_items_recieved = 0;
static int print_flag = 0;	// 1 = printouts, 0 = no printouts.
static buffer_t* buffers;

// Buffer initialization.
void init_buffers(void)
{
	printf("Initializing buffers...\n");
	printf("%d buffers will ne used.\n", NUM_BUFFERS);
	buffers = malloc(sizeof(buffer_t)* NUM_BUFFERS);
	int i;
	for (i = 0; i < NUM_BUFFERS; i++)
	{
		buffers[i].buf = malloc(sizeof(BUFFER_SIZE));
		buffers[i].in = 0;
		buffers[i].out = 0;
		buffers[i].no_elems = 0;
		buffers[i].items_to_send = ITEMS_TO_SEND / NUM_BUFFERS;
		buffers[i].items_sent = 0;
		buffers[i].items_received = 0;
		pthread_mutex_init(&buffers[i].lock, NULL);
		printf("Buffer %d has been initialized and tasked with sending %d items.\n", i, buffers[i].items_to_send);
	}
	pthread_mutex_init(&g_lock, NULL);
}

void *miniConsumer(void *thr_id)
{
	int quit = 0;

	while (!quit)
	{
		pthread_mutex_lock(&g_lock);
		if (g_toReceive > 0)
		{
			if (g_pot > 0)
			{
				g_pot--;
				g_toReceive--;
			}
		}
		else
		{
			quit = 1;
		}
		pthread_mutex_unlock(&g_lock);
	}
}

void *miniProducer(void *thr_id)
{
	int quit = 0;

	while (!quit)
	{
		pthread_mutex_lock(&g_lock);
		if (g_toSend > 0)
		{
			g_pot++;
			g_toSend--;
		}
		else
		{
			quit = 1;
		}
		pthread_mutex_unlock(&g_lock);
	}
}

// Consumer code.
void *consumer(void *thr_id)
{
	int item;
	int c_quit = 0;
	long my_id = (long)thr_id;
	int myIndex = 0;
	int consumersPerBuffer = NO_CONSUMERS / NUM_BUFFERS;


	long n = (long)thr_id;
	while (n > consumersPerBuffer)
	{
		n -= consumersPerBuffer;
		myIndex++;
	}

    
	if (print_flag)
	{
		printf("Cstart 4: tid %ld\n", (long)thr_id);
	}

	while(!c_quit) 
	{
		pthread_mutex_lock(&buffers[myIndex].lock);

		if (buffers[myIndex].items_received < buffers[myIndex].items_to_send)
		{
			// check if there is empty buffer places.
			if (buffers[myIndex].no_elems > 0)
			{
				//delay_in_buffer();
				item = buffers[myIndex].buf[buffers[myIndex].out];
				buffers[myIndex].out = (buffers[myIndex].out + 1) % BUFFER_SIZE;
				buffers[myIndex].no_elems--;
				buffers[myIndex].items_received++;

				if (print_flag)
				{ 
					printf("Consumer %ld got number %d from buffer (%d items)\n", (long)thr_id, item, buffers[myIndex].no_elems);
				}
			}
		} 
		
		else 
		{
			c_quit = 1;
		}
		pthread_mutex_unlock(&buffers[myIndex].lock);
	}


	if (my_id == 0)
		printf("[C0] All done!\n");

	if (print_flag)
	{
		printf("CBreak 4: tid %ld\n", (long)thr_id);
	}

	pthread_exit(0);
}

// Producer code.
void *producer(void *thr_id)
{
	
	int myIndex = 0;
	int producersPerBuffer = NO_PRODUCERS / NUM_BUFFERS;

	long n = (long)thr_id;
	while (n > producersPerBuffer)
	{
		n -= producersPerBuffer;
		myIndex++;
	}

	
	int item;
	int p_quit = 0;
	long my_id = (long) thr_id;

	if (print_flag)
	{
		printf("Pstart 4: tid %ld\n", (long)thr_id);
	}
	
	while(!p_quit) 
	{
		pthread_mutex_lock(&buffers[myIndex].lock);

		if (buffers[myIndex].items_received < buffers[myIndex].items_to_send)
		{
			// Check if there is empty buffer places.
			if (buffers[myIndex].no_elems < BUFFER_SIZE)
			{
				//delay_in_buffer();
				item = buffers[myIndex].items_sent++;
				
				printf("buffers[%d].buf[%d] = %d\n", myIndex, buffers[myIndex].in, item);
				buffers[myIndex].buf[buffers[myIndex].in] = item;

				printf("buffers[%d].in = (%d + 1) mod %d (%d)\n", myIndex, buffers[myIndex].in, BUFFER_SIZE, (buffers[myIndex].in + 1) % BUFFER_SIZE);
				buffers[myIndex].in = (buffers[myIndex].in + 1) % BUFFER_SIZE;
				buffers[myIndex].no_elems++;
				if (print_flag)
					printf("Producer %ld put number %d in buffer (%d items)\n", (long)thr_id, item, buffers[myIndex].no_elems);
			}
		} 
	
		else 
		{
			p_quit = 1;
		}

		pthread_mutex_unlock(&buffers[myIndex].lock);
		//usleep(10);
	}

	if (print_flag)
	{
		printf("PBreak 4: tid %ld\n", (long)thr_id);
	}

	printf("Producer thread %ld is exiting.\n", my_id);
	
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

	printf("g_pot: %d \ng_toSend: %d\ng_toReceive: %d\n", g_pot, g_toSend, g_toReceive);
}
