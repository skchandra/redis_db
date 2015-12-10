#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hiredis.h"
#include "hashs.h"
#include "singlelinkedlist.h"
  
int main(int argc, char **argv) {
    //Set up connection
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
    
    //For bidirectional, need to use 2 queues, 
    //1 for BFS from the start, and 1 for BFS
    //from the end
    Node *start_queue;
    Node *end_queue;

    //Create hashtables for the visited
    hashtable_t *hashtable_start = ht_create( 65536 );
    //hashtable_t *hashtable_end = ht_create( 65536 );

    //Create hashtable to store hashtable parents
    hashtable_t *hashtable_parents = ht_create( 65536 );
    hashtable_t *hashtable_parents_end = ht_create( 65536 );

    //Path will be found between these two
    char *start = "0000138";
    char *end = "0000197";
    
    //initialize the queues
    start_queue = make_node(start, NULL);
    end_queue = make_node(end, NULL);

    char *start_name = start;
    char *end_name = end;

    //LL for paths
    Node *path_to_meet_s;
    Node *path_to_meet_e;
    path_to_meet_s = make_node(start, NULL);
    path_to_meet_e = make_node(end, NULL);

    //Where do they meet
    char *met1 = start_name;
    char *met2 = end_name;
     
    //while start and end are not NULL
    //while (strcmp(start_name, end_name) != 4389605843690) {
    while ((strcmp(start_name, "NULL") != 0) && (strcmp(end_name, "NULL") != 0)) {

        //kick off the BFS with the start and the end
        start_name = peek(&start_queue);
        end_name = peek(&end_queue);
        //printf("start %s, end %s\n", start_name, end_name);

        //push to their BFS queues
        push(&path_to_meet_s, start_name);
        push(&path_to_meet_e, end_name);
        
        //Check if they exist in the visited hashtable, which is named
        //hashtable_start
        char *check_start = ht_get(hashtable_start, start_name);
        char *check_end = ht_get(hashtable_start, end_name);
        //char *check_end = ht_get(hashtable_end, end_name);
        
        //Same BFS
        if(strcmp(check_start, "NULL")==0){ 
            if (strcmp(end_name, start_name) == 0) {
                met1 = start_name;
                met2 = end_name;
                break;
            } 
            char word_reply1[100] = "";
            strcat(word_reply1, "LRANGE ");
            strcat(word_reply1, start_name);
            strcat(word_reply1, " 0 -1");
            reply = redisCommand(c,word_reply1);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    push(&start_queue, reply->element[k]->str);
                    char *check2_st = ht_get(hashtable_parents, reply->element[k]->str);
                    if (strcmp(check2_st, "NULL") == 0) {
                        ht_set(hashtable_parents, reply->element[k]->str, start_name);
                    }
                }
            }
            
            ht_set(hashtable_start, start_name, "visited");
        }
        
        //Same BFS
        if(strcmp(check_end, "NULL")==0){   
            if (strcmp(end_name, start_name) == 0) {
                met1 = start_name;
                met2 = end_name;
                break;
            } 
            char word_reply2[100] = "";
            strcat(word_reply2, "LRANGE ");
            strcat(word_reply2, end_name);
            strcat(word_reply2, " 0 -1");
            reply = redisCommand(c,word_reply2);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    push(&end_queue, reply->element[k]->str);
                    char *check2_en = ht_get(hashtable_parents_end, reply->element[k]->str);
                    if (strcmp(check2_en, "NULL") == 0) {
                        ht_set(hashtable_parents_end, reply->element[k]->str, end_name);
                    }
                }
            }
            ht_set(hashtable_start, end_name, "visited");
            //ht_set(hashtable_end, end_name, "visited");
        }
        
        //if you haven't reached the same actor from the start and the end
        //keep BFS traversing the front and end until you do
        if (strcmp(start_name, end_name) != 0) {
            pop(&start_queue);
            pop(&end_queue);
        } 
        //Otherwise, you have met, and you should break out of the while
        //loop to build the path
        else { 
            printf("met %s %s", start_name, end_name);
            break; 
        }
        pop(&path_to_meet_s);
        pop(&path_to_meet_e);
    }


    //After the while loop, you have paths to meet
    char *start_parent = peek(&path_to_meet_s);
    char *end_parent = peek(&path_to_meet_e);
    printf("%s ", "success\n");
    /*print_list(path_to_meet_s);
    printf("\n");
    print_list(path_to_meet_e);*/

    reverse(&path_to_meet_s);
    pop(&path_to_meet_s);
    reverse(&path_to_meet_e);
    
    char *parent_st = ht_get(hashtable_parents, start_parent);
    char *parent_en = ht_get(hashtable_parents_end, end_parent);
    while (strcmp(parent_st, start) != 0) {
        push(&path_to_meet_s, parent_st);
        parent_st = ht_get(hashtable_parents, parent_st);
    }
    while (strcmp(parent_en, end) != 0) {
        push(&path_to_meet_e, parent_en);
        parent_en = ht_get(hashtable_parents_end, parent_en);
    }
    push(&path_to_meet_s, parent_st);
    reverse(&path_to_meet_s);
    push(&path_to_meet_e, parent_en);

    //This is the final path
    //print_list(path_to_meet_s);
    print_list(path_to_meet_e);
        
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}

