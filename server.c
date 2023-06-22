#include "segel.h"
#include "request.h"
#include "utility.h"
//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too
typedef struct
{
    ProcessQueue *pq;
    pthread_t thread;
} threadAux;

void *thread_handler(void *t_args)
{
    threadAux *args = (threadAux *)t_args;
    ProcessQueue *pq = args->pq;
    // Stats stats;
    // stats.count = 0;
    // stats.static_count = 0;
    // stats.dynamic_count = 0;
    // stats.id = args->id; // pthread_self()?

    while (1)
    {
        Request *data = runRequest(pq /*&stats*/);
        int connfd = data->connfd;
        requestHandle(connfd /*&stats*/);
        removeRequest(pq, pthread_self()); // pthread_self()?
    }
    return NULL;
}

void getargs(int *port, int *thread_max, int *process_max, int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *thread_max = atoi(argv[2]);
    *process_max = atoi(argv[3]);
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, thread_max, process_max;
    struct sockaddr_in clientaddr;

    getargs(&port, &thread_max, &process_max, argc, argv);
    ProcessQueue *pq = processQueueCreate(thread_max, process_max);
    //
    // HW3: Create some threads...
    //
    threadAux *thrd_args = malloc(thread_max * sizeof(*thrd_args));
    // pthread_t *threads = malloc(thread_max * sizeof(*threads));
    if (!thrd_args)
    {
        processQueueDestroy(pq);
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    for (int i = 0; i < thread_max; i++)
    {
        // make sure thread id is i
        thrd_args[i].pq = pq;
        // thrd_args[i].thread = malloc(sizeof(pthread_t));
        // if (!thrd_args[i].thread)
        // {
        //     processQueueDestroy(pq);
        //     free(thrd_args); // free inner mallocs
        //     fprintf(stderr, "Error: malloc failed\n");
        //     exit(1);
        // }
        pthread_create(&(thrd_args[i].thread), NULL, thread_handler, (void *)&thrd_args[i]);
    }
    listenfd = Open_listenfd(port);
    while (1)
    {
        Request *rqst = malloc(sizeof(*rqst));
        if (!rqst)
        {
            processQueueDestroy(pq);
            free(thrd_args);
            fprintf(stderr, "Error: malloc failed\n");
            exit(1);
        }
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        gettimeofday(&(rqst->arrival_time), NULL);
        rqst->connfd = connfd;
        getNewRequest(pq, rqst);
    }
    processQueueDestroy(pq);
    free(thrd_args); // free inner mallocs
    return 0;
}