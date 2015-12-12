#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hiredis.h"
#include "hashs.h"
#include "singlelinkedlist.h"
  
void switch_database(redisContext *c, int num_database) {
    redisReply *switch_data;
    switch_data = redisCommand(c, "SELECT %i", num_database);
}

char *get_from_database(redisContext *c, char *actor_name) {
    redisReply *start_actor;
    start_actor = redisCommand(c,"GET %s", actor_name);
    return start_actor->str;
}

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
    
	//SETTING UP LOOKUP TABLE, switch to database 2:
    switch_database(c,2);
    //Get the starting actor
    char *start_act = argv[1];
    char *start = get_from_database(c,start_act);
    //Get the ending actor
    char *end_act = argv[2];
    char *end = get_from_database(c,end_act);


    //Switch back to DB0 for breadth-first search
    switch_database(c,0);
    
    Node *start_queue;
    start_queue = make_node(start, NULL);
    
    Node *end_queue;
    end_queue = make_node(end, NULL);
    
    Node *path_to_meet_s;
    path_to_meet_s = make_node(start, NULL);
    
    Node *path_to_meet_e; 
    path_to_meet_e = make_node(end, NULL);
    
    hashtable_t *hashtable_start = ht_create( 65536 );
    hashtable_t *hashtable_end = ht_create( 65536 );
    
    hashtable_t *hashtable_parents_start = ht_create( 65536 );
    hashtable_t *hashtable_parents_end = ht_create( 65536 );
       
    char *start_name = start;
    char *end_name = end;
        
    while ((strcmp(start_name, "NULL") != 0) && (strcmp(end_name, "NULL") != 0)) {
        start_name = peek(&start_queue);
		end_name = peek(&end_queue);
		
		char *visited_in_end = ht_get(hashtable_end, start_name);
		char *visited_in_start = ht_get(hashtable_start, end_name);
		
		if (strcmp(visited_in_end, "NULL") != 0) {
			// printf("met at %s \n", start_name);
			end_name = start_name;
			break;
		}
		
		if (strcmp(visited_in_start, "NULL") != 0) {
			// printf("met at %s \n", end_name);
			start_name = end_name;
			break;
		}
		
		char *check_start = ht_get(hashtable_start, start_name);
		char *check_end = ht_get(hashtable_end, end_name);
		
		if(strcmp(check_start, "NULL")==0){	
			char word_reply1[100] = "";
			strcat(word_reply1, "LRANGE ");
			strcat(word_reply1, start_name);
			strcat(word_reply1, " 0 -1");
			reply = redisCommand(c,word_reply1);
			if (reply->type == REDIS_REPLY_ARRAY) {
				for (k = 0; k < reply->elements; k++) {
					push(&start_queue, reply->element[k]->str);
					char *check2_st = ht_get(hashtable_parents_start, reply->element[k]->str);
					if (strcmp(check2_st, "NULL") == 0) {
						ht_set(hashtable_parents_start, reply->element[k]->str, start_name);
					}
				}
			}
			
			ht_set(hashtable_start, start_name, "visited");
		} //else { printf("already visited, skipping"); }
        
        if(strcmp(check_end, "NULL")==0){	
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
			ht_set(hashtable_end, end_name, "visited");
		} //else { printf("already visited, skipping\n"); }
		
		if (strcmp(start_name, end_name) != 0) {
			pop(&start_queue);
			pop(&end_queue);
		} else { 
			printf("met %s %s \n", start_name, end_name);
			break; 
		}
    }
    
    //Switch to DB2 for actor lookup
    switch_database(c,1);

    push_begin(&path_to_meet_s, get_from_database(c,start_name));
	push_begin(&path_to_meet_e, get_from_database(c,end_name));
    
    char *start_parent = start_name;
    char *end_parent = end_name;
    
	reverse(&path_to_meet_s);
	pop(&path_to_meet_s);
	
	reverse(&path_to_meet_e);
	pop(&path_to_meet_e);
	
	char *parent_st = ht_get(hashtable_parents_start, start_parent);
	char *parent_en = ht_get(hashtable_parents_end, end_parent);
	
	while (strcmp(parent_st, start) != 0) {
		push(&path_to_meet_s, get_from_database(c,parent_st));
		parent_st = ht_get(hashtable_parents_start, parent_st);
	}
	
	while (strcmp(parent_en, end) != 0) {
		push(&path_to_meet_e, get_from_database(c, parent_en));
		parent_en = ht_get(hashtable_parents_end, parent_en);
	}
	
	push(&path_to_meet_s, get_from_database(c,parent_st));
	reverse(&path_to_meet_s);
	
	push(&path_to_meet_e, get_from_database(c,parent_en));
	pop(&path_to_meet_e);
	
	print_list(path_to_meet_s);
	print_list(path_to_meet_e);
		
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}