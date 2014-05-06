//==================================================//
//			  NON-SCALING BOUNDED BUFFER			//
//==================================================//
#include <stdio.h>
#include <pthread.h>

#define BUFFER_SIZE 10
#define NO_PRODUCERS 16
#define NO_CONSUMERS 32
#define KILO 1024
#define MEGA (KILO*KILO)
#define ITEMS_TO_SEND 8*MEGA // number of items to pass through the buffer.

// Structs.
typedef struct buffer_t
{
    int in, out;
    int no_elems;
    int buf[BUFFER_SIZE];
    pthread_mutex_t lock;
};

// Variables.
static int no_items_sent = 0;
static int no_items_recieved = 0;
static int print_flag = 0;	// 1 = printouts, 0 = no printouts.
static buffer_t buffer;

// Buffer initialization.
void init_buffer(void)
{
    buffer.in = 0;
    buffer.out = 0;
    buffer.no_elems = 0;
    pthread_mutex_init(&buffer.lock, NULL);
}

// Consumer code.
void *consumer(void *thr_id)
{
	int item;
	int c_quit = 0;
	long my_id = (long) thr_id;
    
	if (print_flag)
	{
		printf("Cstart 4: tid %d\n", my_id);
	}

	while(!c_quit) 
	{
		pthread_mutex_lock(&buffer.lock);

		if (no_items_recieved < ITEMS_TO_SEND) 
		{
			// check if there is empty buffer places.
			if (buffer.no_elems > 0) 
			{
				//delay_in_buffer();
				item = buffer.buf[buffer.out];
				buffer.out = (buffer.out + 1) % BUFFER_SIZE;
				buffer.no_elems--;
				no_items_recieved++;

				if (print_flag)
				{
					printf("Consumer %d got number %d from buffer (%d items)\n", my_id, item, buffer.no_elems);
				}
			}
		} 
		
		else 
		{
			c_quit = 1;
		}

		pthread_mutex_unlock(&buffer.lock);
		//usleep(10);
	}

	if (print_flag)
	{
		printf("CBreak 4: tid %d\n", my_id);
	}

	pthread_exit(0);
}

// Producer code.
void *producer(void *thr_id)
{
	int item;
	int p_quit = 0;
	long my_id = (long) thr_id;

	if (print_flag)
	{
		printf("Pstart 4: tid %d\n", my_id);
	}

	while(!p_quit) 
	{
		pthread_mutex_lock(&buffer.lock);

		if (no_items_sent < ITEMS_TO_SEND) 
		{
			// Check if there is empty buffer places.
			if (buffer.no_elems < BUFFER_SIZE) 
			{
				//delay_in_buffer();
				item = no_items_sent++;
				buffer.buf[buffer.in] = item;
				buffer.in = (buffer.in + 1) % BUFFER_SIZE;
				buffer.no_elems++;
				if (print_flag)
				printf("Producer %d put number %d in buffer (%d items)\n", my_id, item, buffer.no_elems);
			}
		} 
	
		else 
		{
			p_quit = 1;
		}

		pthread_mutex_unlock(&buffer.lock);
		//usleep(10);
	}

	if (print_flag)
	{
		printf("PBreak 4: tid %d\n", my_id);
	}

	pthread_exit(0);
}

// Main entry point.
int main(int argc, char **argv)
{
    long i;
    pthread_t prod_thrs[NO_PRODUCERS];
    pthread_t cons_thrs[NO_CONSUMERS];
    pthread_attr_t attr;

    init_buffer();
    pthread_attr_init (&attr);

    printf("Buffer size = %d, items to send = %d\n", BUFFER_SIZE, ITEMS_TO_SEND);

    // Create the producer threads.
    for(i = 0; i < NO_PRODUCERS; i++)
	{
		pthread_create(&prod_thrs[i], &attr, producer, (void *)i);
	}

	// Create the consumer threads.
    for(i = 0; i < NO_CONSUMERS; i++)
	{
		pthread_create(&cons_thrs[i], &attr, consumer, (void *)i);
	}

    // Wait for all threads to terminate.
    for (i = 0; i < NO_PRODUCERS; i++)
	{
		pthread_join(prod_thrs[i], NULL);
	}

    for (i = 0; i < NO_CONSUMERS; i++)
	{
		pthread_join(cons_thrs[i], NULL);
	}
}