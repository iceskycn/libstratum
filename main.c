#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <netdb.h>

#include "queue.h"
#include "vstring.h"

#include "log.h"

// Default buffer size
#define BUF_SIZE 1024

// Default timeout - http://linux.die.net/man/2/epoll_wait
#define EPOLL_RUN_TIMEOUT 10000

// Count of connections that we are planning to handle (just hint to kernel)
#define EPOLL_SIZE 10000

// Macros - exit in any error (eval < 0) case
#define CHK(eval) if(eval < 0){perror("eval"); exit(-1);}

// Macros - same as above, but save the result(res) of expression(eval)
#define CHK2(res, eval) if((res = eval) < 0){perror("eval"); exit(-1);}

#define DEBUG_MODE 1

static const size_t EOL_POS = (size_t)-1;

// Debug epoll_event
void debug_epoll_event(struct epoll_event ev){
    printf("fd(%d), ev.events:", ev.data.fd);

    if(ev.events & EPOLLIN)
        printf(" EPOLLIN ");
    if(ev.events & EPOLLOUT)
        printf(" EPOLLOUT ");
    if(ev.events & EPOLLET)
        printf(" EPOLLET ");
    if(ev.events & EPOLLPRI)
        printf(" EPOLLPRI ");
    if(ev.events & EPOLLRDNORM)
        printf(" EPOLLRDNORM ");
    if(ev.events & EPOLLRDBAND)
        printf(" EPOLLRDBAND ");
    if(ev.events & EPOLLWRNORM)
        printf(" EPOLLRDNORM ");
    if(ev.events & EPOLLWRBAND)
        printf(" EPOLLWRBAND ");
    if(ev.events & EPOLLMSG)
        printf(" EPOLLMSG ");
    if(ev.events & EPOLLERR)
        printf(" EPOLLERR ");
    if(ev.events & EPOLLHUP)
        printf(" EPOLLHUP ");
    if(ev.events & EPOLLONESHOT)
        printf(" EPOLLONESHOT ");
    if(ev.events & EPOLLRDHUP)
        printf(" EPOLLRDHUP");

    printf("\n");

}



struct stratum_ctx_t
{
    int socketfd;
    struct queue_t read_queue;
    struct queue_t write_queue;
    struct vstring_t read_buffer;
    pthread_mutex_t write_mtx;

};

static void* read_handler(void* data)
{
    struct stratum_ctx_t* ctx = (struct stratum_ctx_t*)data;

    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);

    ssize_t ret = read(ctx->socketfd, buffer, BUF_SIZE);

    if(ret == -1)
    {
        perror("read_handler");
        return data;
    }

    size_t rd = (size_t)ret;

    printf("[Server]: %s", buffer);

    size_t eol_pos = EOL_POS;
    if(rd > 0)
    {
        for(size_t i=rd-1; i != 0; --i)
        {
            if(buffer[i] == '\n') {
                eol_pos = i;
                break;
            }
        }

        if(eol_pos != EOL_POS)
        {
            vstring_append(&ctx->read_buffer, buffer, eol_pos);
            queue_push(&ctx->read_queue, ctx->read_buffer.data, ctx->read_buffer.size);
            vstring_clear(&ctx->read_buffer);

            if(rd < eol_pos+1)
            {
                vstring_append(&ctx->read_buffer, buffer+(eol_pos+1), rd);
            }
        }
        else
        {
            vstring_append(&ctx->read_buffer, buffer, rd);
        }
    }

    return data;
}

static void* write_handler(void* data)
{
    struct stratum_ctx_t* ctx = (struct stratum_ctx_t*)data;

    char* sdata = NULL;
    size_t size = 0;

    int res = queue_top(&ctx->write_queue, (void**)&sdata, &size);
    if(res != QUEUE_EMPTY)
    {
        ssize_t ret = write(ctx->socketfd, sdata, size);
        if(ret == -1)
        {
            perror("write_handler");
        }
        else
        {
            printf("[Client]: %s\n", sdata);
            queue_pop(&ctx->write_queue, NULL, NULL);
        }
    }

    return data;
}

extern void run_tests();

int main() {

    init_log(LOGLEVEL_DEBUG);

    /// test
    run_tests();

    LOG_ERROR("Hello World1");
    LOG_INFO("Hello World2");
    LOG_DEBUG("Hello World3");


    return 0;
    ///======


    // *** Define values
    //     socket connection with server(sock)
    //     process ID(pid)
    //     epoll descriptor to watch events
    int sock, pid, epfd;


    //     define ip & ports for server(addr)
    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(3333);
    //addr.sin_addr.s_addr = inet_addr("eu1-zcash.flypool.org");

    struct hostent *server;
    server = gethostbyname("eu1-zcash.flypool.org");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bcopy(server->h_addr,
          (char *)&addr.sin_addr.s_addr,
          server->h_length);

    //     event template for epoll_ctl(ev)
    //     storage array for incoming events from epoll_wait(events)
    //     and maximum events count could be 2
    //     'sock' from server and 'pipe' from parent process(user inputs)
    static struct epoll_event ev, events[2]; // Socket(in|out) & Pipe(in)
    ev.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP;

    // *** Setup socket connection with server
    CHK2(sock,socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0));
    int res = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0 && errno != EINPROGRESS)
    {
        perror("connect");
        exit(0);
    }



//    int written = send(sock, subscribe_method, strlen(subscribe_method), 0);
//
//    printf("Socket %d has written %d bytes\n", sock, written);

    // *** Create & configure epoll
    CHK2(epfd,epoll_create(EPOLL_SIZE));
    if(DEBUG_MODE) printf("Created epoll with fd: %d\n", epfd);

    //     add server connetion(sock) to epoll to listen incoming messages from server
    ev.data.fd = sock;
    CHK(epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev));
    if(DEBUG_MODE) printf("Socket connection (fd = %d) added to epoll\n", sock);

    // incoming epoll_wait's events count(epoll_events_count)
    // results of different functions(res)
    int epoll_events_count;

    struct stratum_ctx_t stratum_ctx;
    stratum_ctx.socketfd = sock;
    queue_init(&stratum_ctx.read_queue);
    queue_init(&stratum_ctx.write_queue);
    vstring_init(&stratum_ctx.read_buffer);


    const char* subscribe_method = "{\"id\":1,\"method\":\"mining.subscribe\",\"params\":[\"test\", null,\"eu1-zcash.flypool.org\",\"3333\"]}\n";
    queue_push(&stratum_ctx.write_queue, (void*)subscribe_method, strlen(subscribe_method));

    pthread_t t1;
    pthread_attr_t attr;
    //CHK(pthread_create(&t1, &attr, &read_thread, &stratum_ctx));

    char message[BUF_SIZE];

    // *** Main cycle(epoll_wait)
    while(1) {
        CHK2(epoll_events_count, epoll_wait(epfd, events, 1, EPOLL_RUN_TIMEOUT));
        if (DEBUG_MODE) printf("Epoll events count: %d\n", epoll_events_count);

        for(int i = 0; i < epoll_events_count ; i++){
            memset(&message, 0, BUF_SIZE);

            // EPOLLIN event from server( new message from server)
            if(events[i].data.fd == sock){
                if(DEBUG_MODE) printf("Server sends new message!\n");
                debug_epoll_event(events[i]);

                if(events[i].events & EPOLLIN)
                {
                    read_handler(&stratum_ctx);
                }

                if(events[i].events & EPOLLOUT)
                {
                    write_handler(&stratum_ctx);
                }

                // zero size of result means the server closed connection
                if(events[i].events & EPOLLRDHUP){
                    if(DEBUG_MODE) printf("Server closed connection: %d\n", sock);
                    CHK(close(sock));
                    exit(0);
                }
            }
            else
            {
                if(DEBUG_MODE) printf("New unknown event!\n");
            }
        }
    }


    //CHK(pthread_join(t1, NULL));

    return 0;
}
