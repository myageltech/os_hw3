#ifndef QUEUE_H
#define QUEUE_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "request_struct.h" // ??
#include <stdbool.h>
#include "segel.h"
#include <sys/time.h>

typedef struct node
{
    RequestStruct *data;
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
    Queue *waiting_queue;
    Queue *running_queue;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} ProcessQueue;

Queue *queueCreate(int max_size); // inside c

void queueDestroy(Queue *queue); // inside c

void queueInsert(Queue *queue, RequestStruct *data); // inside c

RequestStruct *queuePopHead(Queue *queue); // inside c

RequestStruct *queueRemoveById(Queue *queue, int thread_id); // inside c

ProcessQueue *processQueueCreate(int max_threads, int max_size); // inside c

void processQueueDestroy(ProcessQueue *queue); // inside c

RequestStruct *getNewRequest(ProcessQueue *pq, RequestStruct *request); // inside c

RequestStruct *runRequest(ProcessQueue *pq, Stats *stats);

void removeRequest(ProcessQueue *pq, int thread_id); // inside c

#endif // QUEUE_H