#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hiredis.h"
#include "hashs.h"
#include "singlelinkedlist.h"
  
void switch_database(redisContext *c, int num_database) {
	/* Function to switch between databases
	 * Input: c - Redis connection host, num_database - database index to switch to
	 * Output: Void */
	
    redisReply *switch_data;
    switch_data = redisCommand(c, "SELECT %i", num_database);
}

char *get_from_database(redisContext *c, char *actor_name) {
	/* Function to get actor name from id, or vice versa
	 * Input: c - Redis connection host, actor_name - actor name or id
	 * Output: Actor name or id */
	
    redisReply *start_actor;
    start_actor = redisCommand(c,"GET %s", actor_name);
    return start_actor->str;
}

int main(int argc, char **argv) {
	/* Main function to connect to Redis databases and run bidirectional search on two inputted actors
	 * Input: Actor names
	 * Output: 0 */
	
	// set up connection to Redis database    
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
    
    //Initialize two paths for searching, one from start actor and one from end actor
    Node *start_queue;
    start_queue = make_node(start, NULL);
    
    Node *end_queue;
    end_queue = make_node(end, NULL);
    
    //Initialize two linked lists for start path and end path recreation
    Node *path_to_meet_s;
    path_to_meet_s = make_node(start, NULL);
    
    Node *path_to_meet_e; 
    path_to_meet_e = make_node(end, NULL);
    
    //Create hashtables for visited actors in start and end queues
    hashtable_t *hashtable_start = ht_create( 65536 );
    hashtable_t *hashtable_end = ht_create( 65536 );
    
    //Create hashtables for parent actors of each actor in order to recreate path
    hashtable_t *hashtable_parents_start = ht_create( 65536 );
    hashtable_t *hashtable_parents_end = ht_create( 65536 );
       
    //Save start actor and ending actor
    char *start_name = start;
    char *end_name = end;
        
    unsigned int k; //variable for looping through adjacency lists of each actor
    
    //Loop through queues until one is empty
    while ((strcmp(start_name, "NULL") != 0) && (strcmp(end_name, "NULL") != 0)) {
		//Get next value in each queue, peek at heads
        start_name = peek(&start_queue);
		end_name = peek(&end_queue);
		
		//Check if current start actor has been visited in end_queue, vice versa
		char *visited_in_end = ht_get(hashtable_end, start_name);
		char *visited_in_start = ht_get(hashtable_start, end_name);
		
		//If start actor has been visited in end queue, break
		if (strcmp(visited_in_end, "NULL") != 0) {
			// printf("met at %s \n", start_name);
			end_name = start_name; //set end name to same value for path recreation
			break;
		}
		
		//If end actor has been visited in start queue, break
		if (strcmp(visited_in_start, "NULL") != 0) {
			// printf("met at %s \n", end_name);
			start_name = end_name; //set end name to same value for path recreation
			break;
		}
		
		//Check if current start actor has been visited in start queue, end in end queue
		char *check_start = ht_get(hashtable_start, start_name);
		char *check_end = ht_get(hashtable_end, end_name);
		
		/*If current start actor has not been visited do BFS
		 * code explained in bfs_FINAL_names.c */
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
        
        /*If current end actor has not been visited do BFS
         * code explained in bfs_FINAL_names.c */
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
		
		//Check if current start and end actors are equal
		if (strcmp(start_name, end_name) != 0) {
			//If not, pop each one from their respective queues
			pop(&start_queue); 
			pop(&end_queue);
		} else { 
			//If they are, print meeting point and break
			printf("met %s %s \n", start_name, end_name);
			break; 
		}
    }
    
    //Switch to DB2 for actor lookup
    switch_database(c,1);

	//Push name of actor the two paths met at to beginning of their path lists
    push_begin(&path_to_meet_s, get_from_database(c,start_name));
	push_begin(&path_to_meet_e, get_from_database(c,end_name));
    
    //New variables for parents values
    char *start_parent = start_name;
    char *end_parent = end_name;
    
	//Reverse both paths to pop last values in queues
	reverse(&path_to_meet_s);
	pop(&path_to_meet_s);
	
	reverse(&path_to_meet_e);
	pop(&path_to_meet_e);
	
	//Get parent actors for current start_parent and end_parent (meeting point)
	char *parent_st = ht_get(hashtable_parents_start, start_parent);
	char *parent_en = ht_get(hashtable_parents_end, end_parent);
	
	//While current actor is not the start actor, recreate start path
	while (strcmp(parent_st, start) != 0) {
		push(&path_to_meet_s, get_from_database(c,parent_st));
		parent_st = ht_get(hashtable_parents_start, parent_st);
	}
	
	//While current actor is not the end actor, recreate end path
	while (strcmp(parent_en, end) != 0) {
		push(&path_to_meet_e, get_from_database(c, parent_en));
		parent_en = ht_get(hashtable_parents_end, parent_en);
	}
	
	//Push meeting name to start path and reverse for path to go in order from start actor to meeting
	push(&path_to_meet_s, get_from_database(c,parent_st));
	reverse(&path_to_meet_s);
	
	//Push last value to end path and pop meeting actor name (to remove repitition)
	push(&path_to_meet_e, get_from_database(c,parent_en));
	pop(&path_to_meet_e);
	
	//Print entire path between the start and end actors
	print_list(path_to_meet_s);
	print_list(path_to_meet_e);
		
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}
