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


    /* 
        BREADTH FIRST SEARCH IMPLEMENTATION
    */

    //Switch back to DB0 for breadth-first search
    redisReply *switch_back;
    switch_back = redisCommand(c, "SELECT 0");

    //initialize LLs and hashtables
    Node *actors; //used in BFS as the Queue
    Node *path; //used later for building the path

    //visited hashtable
    hashtable_t *hashtable = ht_create( 65536 );

    //hashtable of parents to build path later
    hashtable_t *hashtable_parents = ht_create( 65536 );
    
    //initialize the start node for the actors Queue
    actors = make_node(start, NULL);
    char *name = start;
    
    //while the start and end are not equal
    while (strcmp(name, end) != 0) {
        name = peek(&actors);

        //check-- has name been visited?
        char *check = ht_get(hashtable, name);

        //if name is unvisited
        if(strcmp(check, "NULL")==0){

            //break out if you have reached
            //finishing condition
            if (strcmp(name, end) == 0) { 
                break; 
            }
            
            //continue BFS
            //set up the query for the list of actors
            //in the adjacency list (neighbors)
            char word_reply[100] = "";
            strcat(word_reply, "LRANGE ");
            strcat(word_reply, name);
            strcat(word_reply, " 0 -1");
            //store the query response
            reply = redisCommand(c,word_reply);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    //push each neighbor onto the queue (as you
                    //do in BFS)
                    push(&actors, reply->element[k]->str);
                    //check if each neighbor is in the hashtable
                    //of parents
                    char *check2 = ht_get(hashtable_parents, reply->element[k]->str);
                    if (strcmp(check2, "NULL") == 0) {
                        //if not, then put that neighbor and it's parent, name
                        //in the hashtable. It is keeping it traceable
                        //by the path later. 
                        ht_set(hashtable_parents, reply->element[k]->str, name);
                    }
                }
            }
            //once done, set name to visited
            //recall that name is just the current node
            //we are visiting.
            ht_set(hashtable, name, "visited");
        }
        //traversing graph by popping from actors
        //then the new current node, name, will be set to 
        //the beginning of actors again at the beginning of the
        //while loop.
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

    /* 
        BUILDING UP THE PATH FROM THE PARENT HASHTABLE
    */

    //Gets first parent to make linked list
    //To trace back and print the path
    //parent is an id in hashtable_parents
    char *parent = ht_get(hashtable_parents, end);

    //Use parent hashtable to build up path
    //all the way up to but not including the final parent
    while (strcmp(parent, start) != 0) {
        //Need to store in redis reply after looking
        //up the actual name from id stored in parent
        redisReply *actor_name;

        //Get the actor name
        actor_name = redisCommand(c,"GET %s", parent);
        char *parent_name = actor_name->str;

        //push parent name to build up final path
        push(&final_path, parent_name);
        //get new parent
        parent = ht_get(hashtable_parents, parent);
    }

    //Get the final parent, the last name that needs
    //to get pushed onto the path
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