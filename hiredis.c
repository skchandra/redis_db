/*
 * hiredis.c
 * 
 * Copyright 2015 Kiki <kiki@kiki-Latitude-E6430>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hiredis.h"

int main(void) {
	redisReply *reply;
	long int i;

//      Start measuring time
	clock_t start = clock();

//      For local connections:
//	redisContext *c = redisConnect("127.0.0.1", 6379);
//      For connections to a remote Redis server:
	redisContext *c = redisConnect
                 ("ec2-**-**-***-**.compute-1.amazonaws.com", 6379);
	if (c->err) {
        	printf("Error: %s\n", c->errstr);
    	}else{
        	printf("Connection Made! \n");
    	}
//	Get all keys for testing
	reply = redisCommand(c, "keys %s", "*");
	if ( reply->type == REDIS_REPLY_ERROR )
  		printf( "Error: %s\n", reply->str );
	else if ( reply->type != REDIS_REPLY_ARRAY )
  		printf( "Unexpected type: %d\n", reply->type );
	else {
  		for ( i=0; ielements; ++i ){
    		   printf( "Result:%lu: %s\n", i,
                      reply->element[i]->str );
  		}
	}
	printf( "Total Number of Results: %lu\n", i );

//      Output Elapsed time
        printf ( "%f Seconds\n", ( (double)clock() - start ) /
                      CLOCKS_PER_SEC );

	freeReplyObject(reply);
}
