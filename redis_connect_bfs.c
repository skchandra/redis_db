#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hiredis.h"
#include <assert.h>

int length = 0;
typedef struct node {
    int val;
    struct node * next;
} Node;

Node *make_node(int val, Node *next) {
    Node *node = malloc(sizeof(Node));
    node->val = val;
    node->next = next;
    length = length + 1;
    return node;
}

void print_list(Node *head) {
    Node *current = head;

    while (current != NULL) {
        printf("%d\n", current->val);
        current = current->next;
    }
}

int pop(Node **head) {
    int retval;
    Node *next_node;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;
    length = length -1;
    return retval;
}

// Add a new element to the beginning of the list.
void push(Node **head, int val) {
    Node *new_node = make_node(val, *head);
    length = length + 1;
    *head = new_node;
}

// Remove the first element with the given value; return the number
// of nodes removed.
int remove_by_value(Node **head, int val) {
    Node *node = *head;
    Node *victim;

    if (node == NULL) {
    return 0;
    }

    if (node->val == val) {
    pop(head);
    length = length -1;
    return 1;
    }

    for(; node->next != NULL; node = node->next) {
    if (node->next->val == val) {
        victim = node->next;
        node->next = victim->next;
        free(victim);
        return 1;
    }
    }
    return 0;
}

// Reverse the elements of the list without allocating new nodes.
void reverse(Node **head) {
    Node *node = *head;
    Node *next, *temp;

    if (node == NULL || node->next == NULL) {
    return;
    }

    next = node->next;
    node->next = NULL;

    while (next != NULL) {
    temp = next->next;
    next->next = node;
    node = next;
    next = temp;
    }
    *head = node;
}

// Adds a new element to the list before the indexed element.
// Index 0 adds an element to the beginning.  Index 1 adds an
// element between the first and second elements.
// Returns 0 if successful, -1 if the index is out of range.
int insert_by_index(Node **head, int val, int index) {
    int i;
    Node *node = *head;

    if (index == 0) {
    push(head, val);
    return 0;
    }

    for (i=0; i<index-1; i++) {
    if (node == NULL) return -1;
    node = node->next;
    }
    if (node == NULL) return -1;
    node->next = make_node(val, node->next);
    return 0;
}


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



    /* Try a GET and two INCR */
    redisReply *all_keys;
    all_keys = redisCommand(c, "KEYS *");
    if (all_keys->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < all_keys->elements; j++) {
            printf("%u) %s\n", j, all_keys->element[j]->str);
            char word_reply[100] = "";
            strcat(word_reply, "LRANGE ");
            strcat(word_reply, all_keys->element[j]->str);
            strcat(word_reply, " 0 -1");
            reply = redisCommand(c,word_reply);
            if (reply->type == REDIS_REPLY_ARRAY) {
                for (k = 0; k < reply->elements; k++) {
                    printf("%u) %s\n", k, reply->element[k]->str);
                }
            }
            freeReplyObject(reply);
        }
    }
    freeReplyObject(all_keys);




    /* Disconnects and frees the context */
    redisFree(c);

    return 0;
}
