/**
 * schedule_priority.c
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

    struct node* shortestJob = *head; // shortest job start as head
    struct node* prevShortestJob = NULL; // previous node of shortest job
    struct node* currNode = (*head)->next; // keeps track of current node
    struct node* prevNode = *head; // keeps track of previous node

    while (prevNode->next != NULL) { // traverse list
      /* if curr node job shorter than shortest job */
      if (currNode->task->burst < shortestJob->task->burst) {
          shortestJob = currNode; // set shortest job node as curr node
          prevShortestJob = prevNode; // set previous node of shortest job
      }
      /* if bursts are equal */
      else if (currNode->task->burst == shortestJob->task->burst) {
        /* if curr node's name comes before shortest job node's name */
        if (strcmp(currNode->task->name, shortestJob->task->name) < 0) {
          shortestJob = currNode; // set shortest job node as curr node
          prevShortestJob = prevNode; // set previous node of shortest job
        }
      }

      prevNode = currNode; // prevNode will end on last node
      currNode = currNode->next; // currNode will end on NULL
    } // end while(currNode->next != NULL)

    run(shortestJob->task, shortestJob->task->burst); // run processes
    time += shortestJob->task->burst; // keep track of how much time has past
    printf("        Time is now: %d\n", time); // print time

    if (prevShortestJob == NULL) { // shortest job is fist node in list
      *head = shortestJob->next; // detach first node from list
    }
    else {
      prevShortestJob->next = shortestJob->next; // release shortest job
    }

    free(shortestJob->task->name); // free name mem
    free(shortestJob->task); // free task mem
    free(shortestJob); // free node mem
  } // end while(1)

  free(head); // free node's pointer
  head = NULL;
}
