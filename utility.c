#include "utility.h"
#include <stdio.h>

Queue *queueCreate(int max_size)
{
    Queue *queue = (Queue *)malloc(sizeof(*queue));
    queue->size = 0;
    queue->max_size = max_size;
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void queueDestroy(Queue *queue)
{
    while (queue->head)
    {
        Node *temp = queue->head;
        queue->head = queue->head->next;
        close(temp->data->connfd);
        free(temp->data);
        free(temp);
    }
    free(queue);
}

void queueInsert(Queue *queue, Request *data, int thread_id) // need to add thread_id
{
    Node *newNode = (Node *)malloc(sizeof(*newNode));
    newNode->data = data;
    newNode->next = NULL;
    // newNode->prev = NULL;
    newNode->thread_id = thread_id;
    if (queue->size == 0)
    {
        queue->head = newNode;
    }
    else
    {
        queue->tail->next = newNode;
    }
    newNode->prev = queue->tail;
    queue->tail = newNode;
    queue->size++;
}

Request *queuePopHead(Queue *queue)
{
    if (queue->size == 0)
    {
        return NULL;
    }
    Request *data = queue->head->data;
    Node *temp = queue->head;
    queue->head = queue->head->next;
    free(temp);
    queue->size--;
    return data;
}

Request *queueRemoveById(Queue *queue, int thread_id)
{
    if (queue->size == 0)
    {
        return NULL;
    }
    Node *temp = queue->head;
    while (temp != NULL)
    {
        if (temp->thread_id == thread_id)
        {
            if (temp->prev == NULL)
            {
                queue->head = temp->next;
            }
            else
            {
                temp->prev->next = temp->next;
            }
            if (temp->next == NULL)
            {
                queue->tail = temp->prev;
            }
            else
            {
                temp->next->prev = temp->prev;
            }
            Request *data = temp->data;
            free(temp);
            queue->size--;
            return data;
        }
        temp = temp->next;
    }
    return NULL;
}

ProcessQueue *processQueueCreate(int max_threads, int max_queue, int dynamic_max_size, POLICY policy)
{
    ProcessQueue *pq = (ProcessQueue *)malloc(sizeof(*pq));
    if (!pq)
    {
        exit(1); // maybe change to return NULL?
    }
    pq->max_size = max_threads + max_queue;
    pq->dynamic_max_size = dynamic_max_size;
    pq->running_queue = queueCreate(max_threads);
    pq->waiting_queue = queueCreate(max_queue);
    pq->policy = policy;
    if (!(pq->running_queue) || !(pq->waiting_queue))
    {
        free(pq->waiting_queue);
        free(pq->running_queue);
        free(pq);
        exit(1);
    }
    pthread_mutex_init(&(pq->mutex), NULL);
    pthread_cond_init(&(pq->empty), NULL);
    pthread_cond_init(&(pq->not_empty), NULL);
    pthread_cond_init(&(pq->not_full), NULL);
    return pq;
}

void processQueueDestroy(ProcessQueue *pq)
{
    pthread_cond_destroy(&(pq->empty));
    pthread_cond_destroy(&(pq->not_empty));
    pthread_cond_destroy(&(pq->not_full));
    pthread_mutex_destroy(&(pq->mutex));
    queueDestroy(pq->running_queue);
    queueDestroy(pq->waiting_queue);
    free(pq);
}

void getNewRequest(ProcessQueue *pq, Request *request)
{
    pthread_mutex_lock(&(pq->mutex));
    switch (pq->policy)
    {
    case BLOCK:
        while (pq->waiting_queue->size + pq->running_queue->size >= pq->max_size)
        {
            pthread_cond_wait(&pq->not_full, &pq->mutex);
        }
        break;
    case DROP_TAIL:
        if (pq->waiting_queue->size + pq->running_queue->size >= pq->max_size)
        {
            pthread_mutex_unlock(&(pq->mutex));
            close(request->connfd);
            free(request);
            return;
        }
        break;
    case DROP_HEAD:
        if (pq->waiting_queue->size + pq->running_queue->size >= pq->max_size)
        {
            // pthread_mutex_unlock(&(pq->mutex));
            Request *temp = queuePopHead(pq->waiting_queue);
            close(temp->connfd);
            free(temp);
        }
        break;
    case BLOCK_FLUSH:
        if (pq->waiting_queue->size + pq->running_queue->size >= pq->max_size)
        {
            while (pq->waiting_queue->size + pq->running_queue->size > 0)
            {
                printf("waiting: %d. running: %d.", pq->waiting_queue->size, pq->running_queue->size);
                pthread_cond_wait(&pq->empty, &pq->mutex);
            }
        }
        break;
    case DYNAMIC:
        if (pq->waiting_queue->size + pq->running_queue->size >= pq->max_size)
        {
            pthread_mutex_unlock(&(pq->mutex));
            close(request->connfd);
            free(request);
            if (pq->max_size < pq->dynamic_max_size)
            {
                pq->max_size++;
                pq->waiting_queue->max_size++;
            }
            return;
        }
        break;
    case DROP_RANDOM:
        if (pq->waiting_queue->size + pq->running_queue->size >= pq->max_size)
        {
            for (int i = 0; i < pq->waiting_queue->max_size / 2; i++)
            {
                int random = rand() % pq->waiting_queue->size;
                Node *temp = pq->waiting_queue->head;
                for (int j = 0; j < random; j++)
                {
                    temp = temp->next;
                }
                temp->thread_id = 1;
                Request *temp_request = queueRemoveById(pq->waiting_queue, 1);
                close(temp_request->connfd);
                free(temp_request);
            }
        }
        break;
    }
    queueInsert(pq->waiting_queue, request, -1);
    pthread_cond_signal(&(pq->not_empty));
    pthread_mutex_unlock(&(pq->mutex));
}

Request *runRequest(ProcessQueue *pq /*Stats *stats*/)
{
    pthread_mutex_lock(&(pq->mutex));
    while (pq->waiting_queue->size == 0)
    {
        pthread_cond_wait(&(pq->not_empty), &(pq->mutex));
    }
    Request *request = queuePopHead(pq->waiting_queue);
    queueInsert(pq->running_queue, request, (int)(unsigned long)pthread_self());

    // struct timeval end;
    // gettimeofday(&end, NULL);
    // stats->arrival_time = request->arrival_time;
    // timersub(&end, &(request->arrival_time), &(stats->dispatch_time));

    pthread_mutex_unlock(&(pq->mutex));
    return request;
}

void removeRequest(ProcessQueue *pq, int thread_id)
{
    pthread_mutex_lock(&(pq->mutex));
    Request *request = queueRemoveById(pq->running_queue, (int)(unsigned long)pthread_self());
    if (request)
    {
        close(request->connfd);
    }
    free(request);
    if (pq->waiting_queue->size + pq->running_queue->size == 0)
    {
        pthread_cond_signal(&(pq->empty));
    }
    pthread_cond_signal(&(pq->not_full));
    pthread_mutex_unlock(&(pq->mutex));
}