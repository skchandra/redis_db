#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hiredis.h"
#include "hashs.h"
#include "singlelinkedlist.h"
  
// int main( int argc, char **argv ) {

//     hashtable_t *hashtable = ht_create( 65536 );

//     ht_set( hashtable, "key1", "inky" );
//     ht_set( hashtable, "key2", "pinky" );
//     ht_set( hashtable, "key3", "blinky" );
//     ht_set( hashtable, "key4", "floyd" );
//     ht_set( hashtable, "key1", "nother");

//     printf( "%s\n", ht_get( hashtable, "key1" ) );
//     printf( "%s\n", ht_get( hashtable, "key2" ) );
//     printf( "%s\n", ht_get( hashtable, "key3" ) );
//     printf( "%s\n", ht_get( hashtable, "key4" ) );

//     char *ret = ht_get(hashtable, "key1");
//     printf("%s\n", ret);

//     char *ret2 = ht_get(hashtable, "keyslk");
//     if(strcmp(ret2, "nothing")!=0){
//         printf("not same\n" );
//     }
//     else{
//         printf("same\n");
//     }
//     // printf("done with ifelse");
//     // printf("%s\n", ret2);

//     return 0;
// }
int main(int argc, char **argv) {
    
    unsigned int j;
    unsigned int k;
    redisContext *c;
    redisReply *reply;
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
    // reply = redisCommand(c,"PING");
    // printf("PING: %s\n", reply->str);
    // freeReplyObject(reply);

    // /* Set a key */
    // reply = redisCommand(c,"SET %s %s", "foo", "hello world");
    // printf("SET: %s\n", reply->str);
    // freeReplyObject(reply);

    // /* Set a key using binary safe API */
    // reply = redisCommand(c,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    // printf("SET (binary API): %s\n", reply->str);
    // freeReplyObject(reply);

    /* Try a GET and two INCR */
    redisReply *all_keys;
    all_keys = redisCommand(c, "KEYS *");
    
    Node *actors;
    Node *path;
    hashtable_t *hashtable = ht_create( 65536 );
    
    int count = 1;
    
    actors = make_node("0000138", NULL);
    char *name = "0000138";
    
    while (strcmp(name, "0356017") != 0) {
    	name = peek(&actors);

        char *check = ht_get(hashtable, name);
        if(strcmp(check, "nothing")!=0){
            printf("not same\n" );
        }
        else{
            printf("same\n");
        }

    	printf("%s %i\n", name, actors->isVisited);
    	if (actors->isVisited != 1) {
			if (strcmp(name, "0356017") == 0) { 
				return; 
			}
			printf("%s %i\n", name, actors->isVisited);
			char word_reply[100] = "";
			strcat(word_reply, "LRANGE ");
			strcat(word_reply, name);
			strcat(word_reply, " 0 -1");
			reply = redisCommand(c,word_reply);
			if (reply->type == REDIS_REPLY_ARRAY) {
				for (k = 0; k < reply->elements; k++) {
					push(&actors, reply->element[k]->str);
				}
			}
			actors->isVisited = 1;
		} else {
			printf("visited %s %i\n", name, actors->isVisited);
		}
		pop(&actors);
	}
	printf("%s", "success");
    /*if (all_keys->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < all_keys->elements; j++) {
			if (j == 0) {
				actors = make_node(all_keys->element[j]->str, NULL);
			} else {
				push(&actors, all_keys->element[j]->str);
				count = count+1;
			}
			
            //printf("%u) %s\n", j, all_keys->element[j]->str);
            char word_reply[100] = "";
            strcat(word_reply, "LRANGE ");
            strcat(word_reply, all_keys->element[j]->str);
            strcat(word_reply, " 0 -1");
            reply = redisCommand(c,word_reply);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    push(&actors, reply->element[k]->str);
                    count = count+1;
                }
            }
            freeReplyObject(reply);
        }
    }
    freeReplyObject(all_keys);*/

	// make a list of even numbers
    /*Node *test_list = make_node("2", NULL);
    test_list->next = make_node("4", NULL);
    test_list->next->next = make_node("6", NULL);
    
    print_list(test_list);
    char *val = pop(&test_list);
    print_list(test_list);
    push(&test_list, "3");
    print_list(test_list);*/
    
    print_list(actors);
    printf("%i\n", count);
    
    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}

