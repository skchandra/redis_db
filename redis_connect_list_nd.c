#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis.h"

int main(int argc, char **argv) {
    unsigned int j;
    redisContext *c;
    redisReply *reply;
    redisReply *reply1;
    const char *hostname = (argc > 1) ? argv[1] : "127.0.0.1";
    int port = (argc > 2) ? atoi(argv[2]) : 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* PING server */
    reply = redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    // /* Set a key */
    // reply = redisCommand(c,"SET %s %s", "foo", "hello world");
    // printf("SET: %s\n", reply->str);
    // freeReplyObject(reply);

    // /* Set a key using binary safe API */
    // reply = redisCommand(c,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    // printf("SET (binary API): %s\n", reply->str);
    // freeReplyObject(reply);

    // /* Try a GET and two INCR */
    // reply = redisCommand(c,"GET foo");
    // printf("GET foo: %s\n", reply->str);
    // freeReplyObject(reply);

    // reply = redisCommand(c, "LRANGE 0000008 -100 100 ");
    // printf("get actor: %s\n", reply->str);
    // freeReplyObject(reply);


    // reply = redisCommand(c,"INCR counter");
    // printf("INCR counter: %lld\n", reply->integer);
    // freeReplyObject(reply);
    // /* again ... */
    // reply = redisCommand(c,"INCR counter");
    // printf("INCR counter: %lld\n", reply->integer);
    // freeReplyObject(reply);

    // /* Create a list of numbers, from 0 to 9 */
    // reply = redisCommand(c,"DEL mylist");
    // freeReplyObject(reply);
    // for (j = 0; j < 10; j++) {
    //     char buf[64];

    //     snprintf(buf,64,"%d",j);
    //     reply = redisCommand(c,"LPUSH mylist element-%s", buf);
    //     freeReplyObject(reply);
    // }

    // reply = redisCommand(c, "KEYS *");
    // printf("get actor: %s\n", reply->str);
    // freeReplyObject(reply);

    reply = redisCommand(c,"KEYS *");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);

            reply1 = redisCommand(c,"LRANGE 0000008 0 -1"); //don't print out same one's 
		    if (reply1->type == REDIS_REPLY_ARRAY) {
		        for (j = 0; j < reply1->elements; j++) {
		            printf("%u) %s\n", j, reply1->element[j]->str);
		        }
		    }
		    freeReplyObject(reply1);

        }
    }
    freeReplyObject(reply);

    /* Let's check what we have inside the list */
    

    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}
