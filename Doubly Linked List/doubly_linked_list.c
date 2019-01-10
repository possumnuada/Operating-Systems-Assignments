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
void insert_after_node(struct node* node, int value);
struct node* get_node(struct node* head, int value);
struct node* delete_node(struct node* head, struct node* node);


int main(){
  struct node* head = create_list(1);
  head = push(head, 2);
  head = push(head, 3);
  head = push(head, 4);
  head = push(head, 5);
  insert_after_node(head, 17);
  struct node* node = get_node(head, 3);
  printf("The value of the node is %d\n", node->value);
  insert_after_node(node, 30);
  head = delete_node(head, node);

  print_list(head);


  free(head);
  while(head->next != NULL){
    head = head->next;
    free(head);
  }
}

void print_list(struct node* cur){
  printf("%d", cur->value);
  while(cur->next != NULL){
    cur = cur->next;
    printf(", %d", cur->value);
  }
  printf("\n" );
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


/* inserts a node after a given node */
void insert_after_node(struct node* node, int value){
  struct node* new = malloc(sizeof(struct node));
  new->value = value;
  new->next = node->next;
  new->prev = node;
  node->next = new;
  new->next->prev = new;
}


/* finds and returns node with given value or returns NULL */
struct node* get_node(struct node* head, int value){
  if(head->value == value) return head;
  while (head->next != NULL) {
    if(head->next->value == value) return head->next;
    head = head->next;
  }

  printf("There is no node with value %d.\n", value);
  return NULL;
}

struct node* delete_node(struct node* head, struct node* node){
  if(head == node){
    head = head->next;
    head->prev = NULL;
  } else {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }
  free(node);
  return head;
}
