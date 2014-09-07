//==================================================//
//			   SCALING BOUNDED BUFFER				//
//==================================================//

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

// Debug print flag.
#define DEBUG 1

// Number of production lines to use.
// [PRODUCER] -> [BUFFER] -> [CONSUMER]
// Each line will create 2 threads, one for the producer, and one for the consumer.
// Thus for 4 lines, 8 threads will be executed, and so on.
#define PRODUCTION_LINES 1

// Size of each individual buffer.
#define BUFFER_SIZE 10

// Total number of items to send through the buffers.
#define ITEMS_TO_SEND 10000000
#define ITEMS_PER_LINE (ITEMS_TO_SEND / PRODUCTION_LINES)
static int ITEMS_SENT[PRODUCTION_LINES];
static int ITEMS_RECIEVED[PRODUCTION_LINES];

// Buffer struct.
typedef struct
{
	int m_input;
	int m_output;
	int m_numberOfItems;
	int m_items[BUFFER_SIZE];

	// Buffer mutex lock.
	pthread_mutex_t m_lock;
} Buffer;

// Create enough buffers to have one per production line.
static Buffer buffer[PRODUCTION_LINES];

// ===== BUFFER INITIALIZATION =====
void InitializeBuffers(void)
{
	if (DEBUG)
	{
		printf("\n(!) Initializing buffers...");
	}

	int i;
	for (i = 0; i < PRODUCTION_LINES; i++)
	{
		buffer[i].m_input = 0;
		buffer[i].m_output = 0;
		buffer[i].m_numberOfItems = 0;
		pthread_mutex_init(&buffer[i].m_lock, NULL);
	}

	if (DEBUG)
	{
		printf(" DONE");
	}
}

// ===== PRODUCER CODE =====
void *Producer(void *p_threadID)
{
	int l_item = 0;
	int l_quit = 0;

	// Individual thread ID.
	long l_threadID = (long)p_threadID;

	// Item offset, to make unordered threads send the correct item.
	const int l_itemOffset = ITEMS_PER_LINE * l_threadID;

	if (DEBUG)
	{
		printf("\nProducer thread %lu beginning work...", l_threadID);
	}

	// Producer work loop.
	while (!l_quit)
	{
		// Lock the buffer for safe item adding.
		pthread_mutex_lock(&buffer[l_threadID].m_lock);

		// Check to see if more items are to be sent.
		if (ITEMS_SENT[l_threadID] < ITEMS_PER_LINE)
		{
			// Check to make sure the buffer is not currently filled.
			if (buffer[l_threadID].m_numberOfItems < BUFFER_SIZE)
			{
				// Add new item and increment counters.
				l_item = (ITEMS_SENT[l_threadID]++) + l_itemOffset;
				buffer[l_threadID].m_items[buffer[l_threadID].m_input] = l_item;
				buffer[l_threadID].m_input = (buffer[l_threadID].m_input + 1) % BUFFER_SIZE;
				buffer[l_threadID].m_numberOfItems++;
			}
		}

		else
		{
			l_quit = 1;
		}

		// Unlock the buffer.
		pthread_mutex_unlock(&buffer[l_threadID].m_lock);
	}

	if (DEBUG)
	{
		printf("\nProducer thread %lu completed.", l_threadID);
	}

	// Exit the thread.
	pthread_exit(0);
}

// ===== CONSUMER CODE =====
void *Consumer(void *p_threadID)
{
	int l_item = 0;
	int l_quit = 0;

	// Individual thread ID.
	long l_threadID = (long)p_threadID;

	if (DEBUG)
	{
		printf("\nConsumer thread %lu beginning work...", l_threadID);
	}

	// Consumer work loop.
	while (!l_quit)
	{
		// Lock the buffer for safe item removal.
		pthread_mutex_lock(&buffer[l_threadID].m_lock);

		// Check to see if more items are to be recieved.
		if (ITEMS_RECIEVED[l_threadID] < ITEMS_PER_LINE)
		{
			// Check to make sure the buffer is not currently empty.
			if (buffer[l_threadID].m_numberOfItems > 0)
			{
				// Consume items and increment counters.
				l_item = buffer[l_threadID].m_items[buffer[l_threadID].m_output];
				buffer[l_threadID].m_output = (buffer[l_threadID].m_output + 1) % BUFFER_SIZE;
				buffer[l_threadID].m_numberOfItems--;
				ITEMS_RECIEVED[l_threadID]++;
			}
		}

		else
		{
			l_quit = 1;
		}

		// Unlock the buffer.
		pthread_mutex_unlock(&buffer[l_threadID].m_lock);
	}

	if (DEBUG)
	{
		printf("\nConsumer thread %lu completed.", l_threadID);
	}

	// Exit the thread.
	pthread_exit(0);
}

// ===== MAIN ENTRYPOINT =====
int main(int argc, char **argv)
{
	printf("\nSCALING BOUNDED BUFFERS");

	if (DEBUG)
	{
		printf(" (DEBUG IS ACTIVE)");
		printf("\nItems to send: %d", ITEMS_TO_SEND);
		printf("\nBuffer size: %d", BUFFER_SIZE);
		printf("\nProduction lines: %d", PRODUCTION_LINES);
		printf("\n(Thats %d items per line)", ITEMS_PER_LINE);
	}

	// Initialize buffers.
	InitializeBuffers();

	// Create 2 threads per line, 1 producer and 1 consumer.
	pthread_t l_producerThreads[PRODUCTION_LINES];
	pthread_t l_consumerThreads[PRODUCTION_LINES];

	pthread_attr_t l_threadAttributes;
	pthread_attr_init(&l_threadAttributes);
	
	// Start timer.
	struct timeval l_start;
	struct timeval l_end;
	gettimeofday(&l_start, NULL);

	if (DEBUG)
	{
		printf("\n(!) Creating producer threads...");
	}

	// Create the producer threads.
	long i;
	for (i = 0; i < PRODUCTION_LINES; i++)
	{
		pthread_create(&l_producerThreads[i], &l_threadAttributes, Producer, (void *)i);
	}

	if (DEBUG)
	{
		printf("\nProducer threads created.");
		printf("\n(!) Creating consumer threads...");
	}

	// Create the producer threads.
	for (i = 0; i < PRODUCTION_LINES; i++)
	{
		pthread_create(&l_consumerThreads[i], &l_threadAttributes, Consumer, (void *)i);
	}

	if (DEBUG)
	{
		printf("\nConsumer threads created.");
		printf("\n(!) Waiting to join threads...");
	}

	// Wait for both thread groups to finish.
	for (i = 0; i < PRODUCTION_LINES; i++)
	{
		pthread_join(l_producerThreads[i], NULL);
	}

	if (DEBUG)
	{
		printf("\n(!) Producer threads joined.");
	}

	for (i = 0; i < PRODUCTION_LINES; i++)
	{
		pthread_join(l_consumerThreads[i], NULL);
	}

	if (DEBUG)
	{
		printf("\n(!) Consumer threads joined.");

		int l_sumSent = 0;
		int l_sumRecieved = 0;

		for (i = 0; i < PRODUCTION_LINES; i++)
		{
			l_sumSent += ITEMS_SENT[i];
			l_sumRecieved += ITEMS_RECIEVED[i];
		}

		printf("\n%d total items sent, and %d total items recieved.", l_sumSent, l_sumRecieved);
	}

	// Stop the timer.
	gettimeofday(&l_end, NULL);
	unsigned long int l_startMSec = l_start.tv_sec * 1000000 + l_start.tv_usec;
	unsigned long int l_endMSec = l_end.tv_sec * 1000000 + l_end.tv_usec;
	unsigned long int l_difference = l_endMSec - l_startMSec;
	double l_differenceInSeconds = l_difference / 1000000.0;
	printf("\n(!) Time taken to send %d items: %f seconds.\n", ITEMS_TO_SEND, l_differenceInSeconds);
}