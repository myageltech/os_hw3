#include "segel.h"
#include "request.h"
#include "request_struct.h"
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
    int id;
} threadAux;

void *thread_handler(void *t_args)
{
    threadAux *args = (threadAux *)t_args;
    ProcessQueue *pq = args->request;
    // Stats stats;
    // stats.count = 0;
    // stats.static_count = 0;
    // stats.dynamic_count = 0;
    // stats.id = args->id;

    while (1)
    {
        RequestStruct *data = getNewRequest(pq);
        int connfd = data->connfd;
        requestHandle(connfd);
        removeRequest(pq);
    }
    return NULL;
}

void getargs(int *port, int* thread_max, int* process_max, int argc, char *argv[])
{
    if (argc < 2) {
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

    listenfd = Open_listenfd(port);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        RequestStruct *thr = malloc(sizeof(*thr));
        if(!thr){
            fprintf(stderr, "Error: malloc failed\n");
            exit(1);
    }
    thr->connfd = connfd;
	// 
	// HW3: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
	requestHandle(connfd);
    proccessQueueInsert(pq, thr);
	Close(connfd);
    }

}
