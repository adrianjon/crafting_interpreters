#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Node {
    char* str;
    struct Node* next;
    struct Node* prev;
} Node;

void insert(Node** head_ref, const char* new_data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->str = malloc(strlen(new_data) + 1);
    memcpy(new_node->str, new_data, strlen(new_data) + 1);
    new_node->next = (*head_ref);
    new_node->prev = NULL;
    (*head_ref) = new_node;
}
Node* find(Node* head, const char* key) {
    Node* current = head;
    while(current != NULL) {
        if(strcmp(current->str, key) == 0) {
            return current;
            printf("Found: %s\n", key);
        }
        current = current->next;
    }
    return NULL;
}
void delete(Node** head_ref, const char* key) {
    Node* target = find(*head_ref, key);
    if (target != NULL) {
        if (target->prev != NULL) {
            target->prev->next = target->next;
        } else {
            *head_ref = target->next;
        }
        if (target->next != NULL) {
            target->next->prev = target->prev;
        }
        free(target->str);
        free(target);
    }    
}
void print_list(Node* node) {
    while (node != NULL) {
        printf("%s\n", node->str);
        node = node->next;
    }
    printf("\n");
}
int main(void) {
    Node* doubly_linked_list = NULL;
    insert(&doubly_linked_list, "Hello");
    insert(&doubly_linked_list, "World");
    print_list(doubly_linked_list);
    delete(&doubly_linked_list, "Hello");
    printf("Deleted 'Hello'\n");
    print_list(doubly_linked_list);
}