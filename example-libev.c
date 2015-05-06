#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libev.h>

void delCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    int status;

    printf("%s() \n", __FUNCTION__);

    if (reply == NULL) return;
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);

    /* Disconnect after receiving the reply to DEL */
    redisAsyncDisconnect(c);

    return;
}

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    int status;

    printf("%s() \n", __FUNCTION__);

    if (reply == NULL) return;
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);

    /* DEL Key after receiving the reply to GET */
    status = redisAsyncCommand((redisAsyncContext *)c, delCallback, NULL, "DEL mcc-key-0");
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }

    return;
}

void setCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    int status;

    printf("%s() \n", __FUNCTION__);

    if (reply == NULL) return;
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);

    /* GET key after receiving the reply to SET */
    status = redisAsyncCommand((redisAsyncContext *)c, getCallback, (char *)"end-1", "GET mcc-key-0");
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }

    return;
}

void connectCallback(const redisAsyncContext *c, int status) {

    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");

    status = redisAsyncCommand((redisAsyncContext *)c, setCallback, NULL, "SET mcc-key-0 %b nx", "FooBarMCC", strlen("FooBarMCC"));
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }

    return;
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

// another callback, this time for a time-out
static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
    printf("Timeout Callback...\n");
    // this causes the innermost ev_run to stop iterating
    ev_break (EV_A_ EVBREAK_ONE);

    return;
}

int main (int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    ev_timer timeout_watcher;
    struct ev_loop *loop = EV_DEFAULT;
    redisAsyncContext *c;

    //c = redisAsyncConnect("127.0.0.1", 6379);
#define SERVER_NAME "xmpp.mccdomain.com"
    printf("Using DNS name %s, not 127.0.0.1\n", SERVER_NAME);
    c = redisAsyncConnect(SERVER_NAME, 6379);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    redisLibevAttach(EV_DEFAULT_ c);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);

    // initialise a timer watcher, then start it
    // simple non-repeating 5.5 second timeout
    ev_timer_init (&timeout_watcher, timeout_cb, 5.5, 0.);
    ev_timer_start (loop, &timeout_watcher);

    // now wait for events to arrive
    ev_run (loop, 0);
    
    // break was called, so exit
    return 0;
}
