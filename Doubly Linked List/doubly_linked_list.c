#include <stdio.h>
#include <stdlib.h>

struct node{
  int value;
  struct node *next;  //adress of next node
  struct node *prev;  //adress of prev node
};

void print_list(struct node* cur);
struct node* create_list(int value);
struct node* push(struct node* head, int value);


int main(){
  struct node *head = create_list(1);
  head = push(head, 2);
  head = push(head, 3);
  head = push(head, 4);
  head = push(head, 5);

  print_list(head);


  free(head);
  while(head->next != NULL){
    head = head->next;
    free(head);
  }
}

void print_list(struct node* cur){
  printf("%d\n", cur->value);
  while(cur->next != NULL){
    cur = cur->next;
    printf("%d\n", cur->value);
  }
}

/* creates a new list */
struct node* create_list(int value){
  struct node* new = malloc(sizeof(struct node));
  new->value = value;
  new->next = NULL;
  new->prev = NULL;
  return new;
}

/* inserts a node at the begining of a list*/
struct node* push(struct node* head, int value){
  struct node* new = malloc(sizeof(struct node));
  new->value = value;
  new->next = head;
  new->prev = NULL;
  head->prev = new;
  return new;
}

/* inserts a node after a given node*/
struct node* insert_after_node(struct node* node, int value){

  
  new->value = value;
  new->next = head;
  new->prev = NULL;
  head->prev = new;
  return new;
}

struct node* get_node(struct node* head, int value){
  if(head->value == value) return head;
  while (head->next != NULL) {
    if(head->next->value == value) return head->next;
    head = head->next;
  }
}
