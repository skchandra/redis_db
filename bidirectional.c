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
    
    Node *start_queue;
    Node *end_queue;
    Node *start_path;
    Node *end_path;
    hashtable_t *hashtable = ht_create( 65536 );
    hashtable_t *hashtable_parents = ht_create( 65536 );
    /*char *start = argv[1];
    char *end = argv[2];*/
    char *start = "0000147";
    char *end = "0000012";
    
    start_queue = make_node(start, NULL);
    end_queue = make_node(end, NULL);
    char *start_name = start;
    char *end_name = end;
    Node *path_to_meet_s;
    Node *path_to_meet_e;
    path_to_meet_s = make_node(start, NULL);
    path_to_meet_e = make_node(end, NULL);
    char *meet_st = start_name;
    char *meet_en = end_name;
    int count = 0;
    
    while (strcmp(start_name, end_name) != 0) {
    //while (strcmp(start_name, end_name) != 0) {
        start_name = peek(&start_queue);
		end_name = peek(&end_queue);
		printf("start %s, end %s\n", start_name, end_name);
		push(&path_to_meet_s, start_name);
		push(&path_to_meet_e, end_name);
		
		/*if (strcmp(start_name, end_name) == 0) {
			count=count+1;
		} else {
			printf("DJSAKLSJDG");
			break;
		}*/
		printf("%i\n", count);

        char *check_start = ht_get(hashtable, start_name);
        char *check_end = ht_get(hashtable, end_name);
				
		char word_reply1[100] = "";
		strcat(word_reply1, "LRANGE ");
		strcat(word_reply1, start_name);
		strcat(word_reply1, " 0 -1");
		reply = redisCommand(c,word_reply1);
		if (reply->type == REDIS_REPLY_ARRAY) {
			for (k = 0; k < reply->elements; k++) {
				char *check_start = ht_get(hashtable, reply->element[k]->str);
				if(strcmp(check_start, "NULL")==0){
					push(&start_queue, reply->element[k]->str);
					//printf("start pushed %s\n", reply->element[k]->str);
					char *check2_st = ht_get(hashtable_parents, reply->element[k]->str);
					if (strcmp(check2_st, "NULL") == 0) {
						ht_set(hashtable_parents, reply->element[k]->str, start_name);
						//printf("hashparents Keychild %s, Valueparent %s\n", reply->element[k]->str, start_name);
					}
				}
			}
            
            ht_set(hashtable, start_name, "visited");
            //printf("Popping start %s\n", start_name);
        }
        pop(&start_queue);
        
        if(strcmp(check_end, "NULL")==0){
            char word_reply[100] = "";
            strcat(word_reply, "LRANGE ");
            strcat(word_reply, end_name);
            strcat(word_reply, " 0 -1");
            reply = redisCommand(c,word_reply);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    push(&end_queue, reply->element[k]->str);
                    //printf("end pushed %s\n", reply->element[k]->str);
                    char *check2_end = ht_get(hashtable_parents, reply->element[k]->str);
                    if (strcmp(check2_end, "NULL") == 0) {
                        ht_set(hashtable_parents, reply->element[k]->str, end_name);
                    }
                }
            }
            
            ht_set(hashtable, end_name, "visited");
            //printf("Popping end %s\n", start_name);
        }
        pop(&end_queue);
        pop(&path_to_meet_s);
        pop(&path_to_meet_e);
    }
    printf("%i\n", strcmp(start_name, end_name));
	printf("meeting %s %s\n", start_name, end_name);
    /*printf("%s", "success\n");
    print_list(path_to_meet_s);
    printf("\n");
    print_list(path_to_meet_e);*/

/*    char *parent = ht_get(hashtable_parents, end);
    Node *final_path = make_node(end, NULL);
    while (strcmp(parent, start) != 0) {
        push(&final_path, parent);
        parent = ht_get(hashtable_parents, parent);
    }
    push(&final_path, parent);
    reverse(&final_path);
    print_list(final_path);
    */
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}

