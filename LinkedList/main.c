#include <stdlib.h>
#include <stdio.h>
struct Node {
    int val;
    struct Node *next;
    struct Node *prev;
};

struct LinkedList {
    struct Node *head;
    struct Node *tail;
};

void setPointers (struct Node *first, struct Node *second) {
    first->next = second;
    second->prev = first;
}

void setHead(struct LinkedList *list, struct Node *newHead) {
    struct Node *newNext = list->head->next;
    setPointers(list->head, newHead);
    setPointers(newHead, newNext);
}

void setTail(struct LinkedList *list, struct Node *newTail) {
    setPointers(list->tail, newTail);
    list->tail = newTail;
}


struct Node *createNode(int val) {
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));
    node->val = val;
    return node;
}

struct LinkedList *initList(int val) {
    struct LinkedList *list;
    list = (struct LinkedList *)malloc(sizeof(struct LinkedList));
    // sentinel value
    list->head = createNode(0);
    list->tail = createNode(val);
    setPointers(list->head, list->tail);
    return list;
}

void addToFront(struct LinkedList *list, int val) {
    struct Node *newHead = createNode(val);
    setHead(list, newHead);
}

void addToBack(struct LinkedList *list, int val) {
    struct Node *newTail = createNode(val);
    setTail(list, newTail);
}

void removeFromFront(struct LinkedList *list) {
    if (list->head->next->next) {
        setPointers(list->head, list->head->next->next);
    }
    else {
        free(list->head->next);
    }
}

void removeFromBack(struct LinkedList *list) {
    list->tail = list->tail->prev;
    free(list->tail->next);
}

int getAtIndex(struct LinkedList *list, int index) {
    struct Node *current = list->head->next;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return current->val;
}

int main() {
    struct LinkedList *list = initList(2);
    //addToBack(list, 3);
    removeFromFront(list);
    printf("%d\n", getAtIndex(list, 0));
    addToBack(list, 4);
    addToFront(list, 1);
    //removeFromFront(list);
    //removeFromFront(list);
    removeFromBack(list);
    addToBack(list, 5);
    //printf("%d\n", getAtIndex(list, 3));
    //printf("%d\n", getAtIndex(list, 2));
    //printf("%d\n", list->tail->val);
    addToFront(list, -1);
    //printf("%d\n", list->head->next->val);
    //printf("%d\n", getAtIndex(list, 1));

    struct Node *node = list->head;
    while (node) {
        struct Node *temp = node->next;
        free(node);
        node = temp;
    }
    return 0;
}
