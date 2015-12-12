#ifndef _SLL_H_
#define _SLL_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct node {
    char val[100];
    struct node * next;
} Node;

Node *make_node(char *val, Node *next) {
    Node *node = malloc(sizeof(Node));
    strcpy(node->val, val);
    node->next = next;
    return node;
}

void print_list(Node *head) {
    Node *current = head;

    while (current != NULL) {
        printf("%s\n", current->val);
        current = current->next;
    }
}

char *pop(Node **head) {
    char *retval;
    Node *next_node;

    if (*head == NULL) {
        return "no";
    }
    
    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}

char *peek(Node **head) {
    return (*head)->val;
}

// Add a new element to the beginning of the list.
void push_begin(Node **head, char *val) {
    Node *new_node = make_node(val, *head);
    *head = new_node;
}

void push(Node **head, char *val) {
    Node *new_node = make_node(val, NULL);
    //*head = new_node;
    Node *temp = *head;
    while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = new_node;
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
#endif
