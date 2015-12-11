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


    //SETTING UP LOOKUP TABLE:
    redisReply *actors_ids;
    actors_ids = redisCommand(c, "SELECT 2");

    redisReply *start_actor;
    redisReply *end_actor;

    //Get the starting actor
    char *start_act = argv[1];
    start_actor = redisCommand(c,"GET %s", start_act);
    char *start = start_actor->str;
    // printf("%s\n", start);
    // freeReplyObject(start_actor);

    //Get the ending actor
    char *end_act = argv[2];
    end_actor = redisCommand(c,"GET %s", end_act);
    char *end = end_actor->str;
    // printf("%s\n", end);
    // freeReplyObject(end_actor);


    //Switch back to DB0 for breadth-first search
    redisReply *switch_back;
    switch_back = redisCommand(c, "SELECT 0");

    Node *actors;
    Node *path;
    hashtable_t *hashtable = ht_create( 65536 );
    hashtable_t *hashtable_parents = ht_create( 65536 );
    
    
    actors = make_node(start, NULL);
    char *name = start;
    
    while (strcmp(name, end) != 0) {
        name = peek(&actors);

        char *check = ht_get(hashtable, name);

        if(strcmp(check, "NULL")==0){
            if (strcmp(name, end) == 0) { 
                break; 
            }
            
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
                        ht_set(hashtable_parents, reply->element[k]->str, name);
                    }
                }
            }
            
            ht_set(hashtable, name, "visited");
        }
        pop(&actors);
    }

    //Switch to DB2 for actor lookup
    redisReply *switch_actor_name;
    switch_actor_name = redisCommand(c, "SELECT 1");


    //GET FIRST ACTOR NAME
    redisReply *first_actor_name;
    first_actor_name = redisCommand(c,"GET %s", end);
    char *first_parent_name = first_actor_name->str;

    Node *final_path = make_node(first_parent_name, NULL);



    //Gets first parent to make linked list
    char *parent = ht_get(hashtable_parents, end);

    while (strcmp(parent, start) != 0) {
        redisReply *actor_name;

        //Get the actor name
        actor_name = redisCommand(c,"GET %s", parent);
        char *parent_name = actor_name->str;

        //push parent name
        push(&final_path, parent_name);
        //get new parent
        parent = ht_get(hashtable_parents, parent);
    }

    //Get the final name
    redisReply *actor_name;
    actor_name = redisCommand(c,"GET %s", parent);
    char *parent_name = actor_name->str;

    push(&final_path, parent_name);

    reverse(&final_path);
    print_list(final_path);
    
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}