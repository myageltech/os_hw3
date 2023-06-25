#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "segel.h"
#include <sys/time.h>

typedef enum
{
    BLOCK,
    DROP_TAIL,
    DROP_HEAD,
    BLOCK_FLUSH,
    DYNAMIC,
    DROP_RANDOM
} POLICY;

typedef struct
{
    int connfd;
    struct timeval arrival_time;
} Request;

typedef struct node
{
    Request *data;
    int thread_id; // default is -1
    struct node *next;
    struct node *prev;
} Node;

typedef struct
{
    Node *head;
    Node *tail;
    int max_size;
    int size;
} Queue;

typedef struct
{
    int max_size;
    int dynamic_max_size;
    Queue *waiting_queue;
    Queue *running_queue;
    POLICY policy;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t empty;
    pthread_cond_t not_empty;
} ProcessQueue;

Queue *queueCreate(int max_size); // inside c

void queueDestroy(Queue *queue); // inside c

void queueInsert(Queue *queue, Request *data, int thread_id); // inside c

Request *queuePopHead(Queue *queue); // inside c

Request *queueRemoveById(Queue *queue, int thread_id); // inside c

ProcessQueue *processQueueCreate(int max_threads, int max_size, int dynamic_max_size, POLICY policy); // inside c

void processQueueDestroy(ProcessQueue *queue); // inside c

void getNewRequest(ProcessQueue *pq, Request *request); // inside c

Request *runRequest(ProcessQueue *pq, int thread_id /*Stats *stats*/);

void removeRequest(ProcessQueue *pq, int thread_id); // inside c

#endif // QUEUE_H