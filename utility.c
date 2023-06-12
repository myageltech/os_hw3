#include "utility.h"

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
        free(temp);
    }
    free(queue);
}

void queueInsert(Queue *queue, RequestStruct *data) //need to add thread_id
{
    Node *newNode = (Node *)malloc(sizeof(*newNode));
    newNode->data = data;
    newNode->next = NULL;
    newNode->prev = NULL;
    // newNode->thread_id = data->thread_id;
    if (queue->size == 0)
    {
        queue->head = newNode;
        queue->tail = newNode;
    }
    else
    {
        queue->tail->next = newNode;
        newNode->prev = queue->tail;
        queue->tail = newNode;
    }
    queue->size++;
}

RequestStruct *queuePopHead(Queue *queue)
{
    if (queue->size == 0)
    {
        return NULL;
    }
    RequestStruct *data = queue->head->data;
    Node *temp = queue->head;
    queue->head = queue->head->next;
    free(temp);
    queue->size--;
    return data;
}

RequestStruct *queueRemoveById(Queue *queue, int thread_id)
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
            RequestStruct *data = temp->data;
            free(temp);
            queue->size--;
            return data;
        }
        temp = temp->next;
    }
    return NULL;
}

ProcessQueue *processQueueCreate(int max_threads, int max_size)
{
    ProcessQueue *pq = (ProcessQueue *)malloc(sizeof(*pq));
    if(!pq){
        exit(1); //maybe change to return NULL?
    }
    pq->max_size = max_size;
    pq->max_threads = max_threads;
    pq->running_queue = createQueue(max_threads);
    pq->waiting_queue = createQueue(max_size - max_threads);
    if(!(pq->running_queue) || !(pq->waiting_queue)){
        exit(1);
    }
    pthread_mutex_init(&(pq->mutex), NULL);
    pthread_cond_init(&(pq->waiting_queue_empty), NULL);
    pthread_cond_init(&(pq->waiting_queue_full), NULL);
    return pq;
}

void processQueueDestroy(ProcessQueue *pq)
{
    queueDestroy(pq->running_queue);
    queueDestroy(pq->waiting_queue);
    free(pq);
}

RequestStruct *getNewRequest(ProcessQueue *pq, RequestStruct *request)
{
    pthread_mutex_lock(&(pq->mutex));
    while (pq->waiting_queue->size + pq->running_queue->size == pq->max_size)
    {
        pthread_cond_wait(pq->waiting_queue_full, pq->mutex);
    }
    if (pq->running_queue->size < pq->running_queue->max_size)
    {
        queueInsert(pq->running_queue, request);
    }
    else
    {
        queueInsert(pq->waiting_queue, request);
    }  
    pthread_cond_signal(&(pq->waiting_queue_empty));
    pthread_mutex_unlock(&(pq->mutex));
}

RequestStruct *removeRequest(ProcessQueue *pq, int thread_id)
{
    pthread_mutex_lock(&(pq->mutex));
    RequestStruct *request = queueRemoveById(pq->running_queue, thread_id);
    if (pq->running_queue->size == 0)
    {
        pthread_cond_signal(&(pq->waiting_queue_empty));
    }
    pthread_cond_destroy(&(pq->waiting_queue_full));
    pthread_mutex_unlock(&(pq->mutex));
    return request;
}