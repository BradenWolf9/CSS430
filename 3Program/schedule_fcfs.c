/**
 * schedule_fcfs.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "list.h"
#include "schedulers.h"
#include "task.h"


struct node** head = NULL; // head

int tid = 1; // tid

// add a task to the list
void add(char *name, int priority, int burst) {
  struct task* task = malloc(sizeof(struct task)); // allocate mem for task
  task->name = malloc(sizeof(name) * strlen(name)); // allocate mem for name
  strcpy(task->name,name); // set the new task's name
  task->tid = tid++; // set the new tasks tid
  task->priority = priority; // set the new tasks priority
  task->burst = burst; // set the new tasks burst

  if (head != NULL) { // not first node
    insert(head, task); // insert task into front of list
  }

  else { // first node
    struct node* newNode = malloc(sizeof(struct node)); // allocate mem for node
    newNode->task = task; // set node's task
    newNode->next = NULL; // have node's next be NULL
    head = malloc(sizeof(struct node*)); // allocate mem for head's pointer
    *head = newNode; // set head's pointer to point at created node
  }
}

// invoke the scheduler
void schedule() {
  if (head == NULL) { // no tasks in list
    return;
  }

  int time = 0; // keeps track of amount of time run
  while(1) { // until ran all processes
    if ((*head)->next == NULL) { // only one node in list
      run((*head)->task, (*head)->task->burst); // run the processes
      time += (*head)->task->burst; // keep track of how much time has past
      printf("        Time is now: %d\n", time); // print time

      free((*head)->task->name); // free name mem
      free((*head)->task); // free task mem
      free((*head)); // free node mem

      break; // break loop
    } // end if (currNode->next == NULL)

    struct node* currNode = *head; // keep track of current node
    struct node* prevNode = *head; // keeps track of previous node
    while (currNode->next != NULL) { // get to end of list
      prevNode = currNode; // prevNode will end on second to last node
      currNode = currNode->next; // currNode will end on last node in list
    } // end while(currNode->next != NULL)

    run(currNode->task, currNode->task->burst); // run processes
    time += currNode->task->burst; // keep track of how much time has past
    printf("        Time is now: %d\n", time); // print time
    prevNode->next = NULL; // release last node from list

    free(currNode->task->name); // free name mem
    free(currNode->task); // free task mem
    free(currNode); // free node mem
  } // end while(true)

  free(head); // free node's pointer
  head = NULL;
}
