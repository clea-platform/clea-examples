#include<stdlib.h>

#include "data/link.h"

typedef struct _node {
    Data *data;
    struct _node *next;
} Node;

struct _linkedList {
    Node *head;
};

LinkedList *link_getLinkedListInstance() {
    LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));
    if (list == NULL) return NULL;

    list->head = NULL;
    return list;
}

void link_removeLinkedListInstance(LinkedList *list) {
    if(list == NULL) return;

    Node *tmp = list->head;

    while (tmp != NULL) {
        free(tmp->data);
        Node *current = tmp;
        tmp = tmp->next;
        free(current);
    }

    free(list);
}

void link_addNode(LinkedList *list, Data data) {
    if(list == NULL) return;

    Node *node = (Node *) malloc(sizeof(Node));
    node->data = data;

    if (list->head == NULL) {
        node->next = NULL;
        list->head = node;
    } else {
        node->next = list->head;
        list->head = node;
    }
}

void link_addNodeToEnd(LinkedList *list, Data data) {
    if(list == NULL) return;

    Node *node = (Node *) malloc(sizeof(Node));
    if(node == NULL) return;

    node->data = data;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
    } else {
        Node* tmp = list->head;

        while (tmp->next) {
            tmp = tmp->next;
        }

        tmp->next = node;
    }
}

Data link_next(LinkedList* list){
    if(list == NULL || list->head == NULL) {
        return NULL;
    }else{
        Node* tmp= list->head;
        Data* data;
        list->head = list->head->next;
        data = tmp->data;
        free(tmp);
        return data;
    }
}

Data link_nextFromOffset(LinkedList* list, int offset){
    int i = 0;
    bool found = false;
    Data* data =NULL;

    while (link_hasNext(list) && !found){
        Data* tmp = link_next(list);
        if(i == offset){
            data = tmp;
            found = true;
        }
    }
    return data;
}

bool link_hasNext(LinkedList* list){
    return list != NULL && list->head != NULL;
}

int link_getSize(LinkedList *list) {
    if (list == NULL) return 0;

    size_t size = 0;

    Node *current = list->head;

    while (current != NULL) {
        size++;
        current = current->next;
    }

    return (int) size;
}