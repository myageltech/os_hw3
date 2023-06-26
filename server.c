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

typedef struct
{
    int id;
    ProcessQueue *pq;
    pthread_t thread;
    Stats stats;
} threadAux;

void *thread_handler(void *t_args)
{
    threadAux *args = (threadAux *)t_args;
    ProcessQueue *pq = args->pq;
    while (1)
    {
        Request *data = runRequest(pq, &args->stats);
        int connfd = data->connfd;
        requestHandle(connfd, &args->stats);
        removeRequest(pq, (args->stats).id);
    }
    return NULL;
}

void getargs(int *port, int *thread_max, int *process_max, int *dynamic_max_size, POLICY *policy, int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *thread_max = atoi(argv[2]);
    *process_max = atoi(argv[3]);
    *dynamic_max_size = *process_max;
    char *policy_str = argv[4];
    if (strcmp(policy_str, "block") == 0)
    {
        *policy = BLOCK;
    }
    else if (strcmp(policy_str, "dt") == 0)
    {
        *policy = DROP_TAIL;
    }
    else if (strcmp(policy_str, "dh") == 0)
    {
        *policy = DROP_HEAD;
    }
    else if (strcmp(policy_str, "bf") == 0)
    {
        *policy = BLOCK_FLUSH;
    }
    else if (strcmp(policy_str, "dynamic") == 0)
    {
        *policy = DYNAMIC;
        if (argc != 6)
        {
            fprintf(stderr, "Error: invalid number of arguments\n");
            exit(1);
        }
        *dynamic_max_size = atoi(argv[5]);
    }
    else if (strcmp(policy_str, "random") == 0)
    {
        *policy = DROP_RANDOM;
    }
    else
    {
        fprintf(stderr, "Error: invalid policy\n");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, thread_max, process_max, procces_real_max;
    POLICY policy;
    struct sockaddr_in clientaddr;

    getargs(&port, &thread_max, &process_max, &procces_real_max, &policy, argc, argv);
    ProcessQueue *pq = processQueueCreate(process_max, procces_real_max, policy);
    if (!pq)
        exit(1);
    threadAux *thrd_args = malloc(thread_max * sizeof(*thrd_args));
    if (!thrd_args || thread_max <= 0)
    {
        processQueueDestroy(pq);
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    for (int i = 0; i < thread_max; i++)
    {
        thrd_args[i].id = i;
        thrd_args[i].pq = pq;
        thrd_args->stats = {{0, 0}, {0, 0}, i, 0, 0, 0};
        pthread_create(&(thrd_args[i].thread), NULL, thread_handler, (void *)&thrd_args[i]);
    }
    listenfd = Open_listenfd(port);
    while (1)
    {
        Request *rqst = malloc(sizeof(*rqst));
        if (!rqst)
        {
            processQueueDestroy(pq);
            for (int i = 0; i < thread_max; i++)
            {
                pthread_join(thrd_args[i].thread, NULL);
            }
            free(thrd_args);
            fprintf(stderr, "Error: malloc failed\n");
            exit(1);
        }
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        gettimeofday(&(rqst->req_arrival), NULL);
        rqst->connfd = connfd;
        getNewRequest(pq, rqst);
    }
    processQueueDestroy(pq);
    for (int i = 0; i < thread_max; i++)
    {
        pthread_join(thrd_args[i].thread, NULL);
    }
    free(thrd_args);
    return 0;
}