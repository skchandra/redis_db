#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hiredis.h"
#include "hashs.h"
#include "singlelinkedlist.h"
  
int main(int argc, char **argv) {
    unsigned int j;
    unsigned int k;
    redisContext *c;
    redisReply *reply;
    const char *hostname = (argc > 3) ? argv[3] : "127.0.0.1";
    int port = (argc > 4) ? atoi(argv[4]) : 6379;

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


    redisReply *all_keys;
    all_keys = redisCommand(c, "KEYS *");
    
    Node *actors;
    Node *path;
    hashtable_t *hashtable = ht_create( 65536 );
    hashtable_t *hashtable_parents = ht_create( 65536 );
    char *start = argv[1];
    char *end = argv[2];
    
    
    actors = make_node(start, NULL);
    char *name = start;
    
    while (strcmp(name, end) != 0) {
        name = peek(&actors);

        char *check = ht_get(hashtable, name);

        if(strcmp(check, "NULL")==0){
            if (strcmp(name, end) == 0) { 
                break; 
            }
            
            //printf("%s not visited yet\n", name);
            char word_reply[100] = "";
            strcat(word_reply, "LRANGE ");
            strcat(word_reply, name);
            strcat(word_reply, " 0 -1");
            reply = redisCommand(c,word_reply);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    push(&actors, reply->element[k]->str);
                    char *check2 = ht_get(hashtable_parents, reply->element[k]->str);
                    if (strcmp(check2, "NULL") == 0) {
                        //printf("child: %s\n", reply->element[k]->str);
                        //printf("parent: %s\n", name);
                        ht_set(hashtable_parents, reply->element[k]->str, name);
                    }
                }
            }
            
            ht_set(hashtable, name, "visited");
        } else {
            //printf("%s already visited\n", name);
        }
        pop(&actors);
    }
    printf("%s", "success\n");

    char *parent = ht_get(hashtable_parents, end);
    Node *final_path = make_node(end, NULL);
    while (strcmp(parent, start) != 0) {
        push(&final_path, parent);
        parent = ht_get(hashtable_parents, parent);
    }
    push(&final_path, parent);
    reverse(&final_path);
    print_list(final_path);
    
    // print_list(actors);
    // printf("%i\n", count);
    
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}