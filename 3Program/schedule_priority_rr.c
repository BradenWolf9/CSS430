/**
 * schedule_priority.c
*/

#include <stdbool.h>
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
    /***************************one process in list****************************/
    if ((*head)->next == NULL) { // only one process in list
      run((*head)->task, (*head)->task->burst); // run the processes
      time += (*head)->task->burst; // keep track of how much time has past
      printf("        Time is now: %d\n", time); // print time

      free((*head)->task->name); // free name mem
      free((*head)->task); // free task mem
      free((*head)); // free node mem

      break; // break loop
    } // end if (currNode->next == NULL)

    /****************************traverse list for priority********************/
    struct node* highestPri = *head; // node with highest priority
    struct node* prevHighestPri = NULL; // previous node of highest priority
    struct node* currNode = (*head)->next; // keeps track of current node
    struct node* prevNode = *head; // keeps track of previous node
    int countSame = 1; // keeps track of how many processes have highest pri

    while (prevNode->next != NULL) { // traverse list
      /* if curr node pri highest */
      if (currNode->task->priority > highestPri->task->priority) {
          highestPri = currNode; // set highest pri node as curr node
          prevHighestPri = prevNode; // set previous node of highest pri
          countSame = 1;
      }
      /* if priorities equal */
      else if (currNode->task->priority == highestPri->task->priority) {
        countSame++;
      }

      prevNode = currNode; // prevNode will end on last node
      currNode = currNode->next; // currNode will end on NULL
    } // end while(currNode->next != NULL)

    /***********************tied priorities into array*************************/
    if (countSame > 1) { // tie for highest priority so round robin
      /* allocate mem for tied processes array */
      struct node** tiedProcesses = malloc(countSame * sizeof(struct node*));

      currNode = *head; // keeps track of current node
      prevNode = *head; // keeps track of previous node
      int index = 0; // keeps track of index of tied priorities array
      while (prevNode->next != NULL) { // traverse list
        /* if process has highest priority */
        if (currNode->task->priority == highestPri->task->priority) {
          tiedProcesses[index++] = currNode; // fill array
        }
        prevNode = currNode; // prevNode will end on last node
        currNode = currNode->next; // currNode will end on NULL
      }

      /*******************run tied processes***********************************/
      bool ran = true; // keeps track if there was a process that ran
      while(ran) {
        ran = false; // if no process ran will remain false and end loop
        /* tied processes run in rr which is fcfs so run from back of array
           first because array was filled from front to back of list and list
           was created by placing new processes in front. */
        for (int i = countSame - 1; i >= 0; i--) {
          /* if burst greater or equal to 10 */
          if (tiedProcesses[i]->task->burst >= 10) {
            run(tiedProcesses[i]->task, 10); // run process for 10
            ran = true; // set ran to true
            time += 10; // add 10 to time
            printf("        Time is now: %d\n", time); // print time
            /* decrease process burst time by 10 */
            tiedProcesses[i]->task->burst -= 10;
          }
          /* burst time less than 10 but greater than 0 */
          else if (tiedProcesses[i]->task->burst > 0) {
            // run process for the amount of burst it has left
            run(tiedProcesses[i]->task, tiedProcesses[i]->task->burst);
            ran = true; // set ran to true
            time += tiedProcesses[i]->task->burst; // increase time by burst
            printf("        Time is now: %d\n", time); // print time
            /* decrease the process burst time to 0 so won't run anymore */
            tiedProcesses[i]->task->burst -= tiedProcesses[i]->task->burst;
          }
        }
      }

      /*******************remove tied processes from list*********************/
      currNode = *head; // keeps track of current node
      prevNode = *head; // keeps track of previous node
      int highest = highestPri->task->priority; // set highest priority number
      while (prevNode->next != NULL) { // traverse list
        if (currNode->task->priority == highest) { // if highest priority
          if (prevNode->task->tid == currNode->task->tid &&
              currNode->next == NULL) { // last process in list
                free(currNode->task->name); // free name mem
                free(currNode->task); // free task mem
                free(currNode); // free node mem
                break; // end loop
          }
          /* highest priority is fist node in list */
          if (prevNode->task->tid == currNode->task->tid) {
            *head = currNode->next; // detach first node from list
            prevNode = *head; // set prevNode as new head
            free(currNode->task->name); // free name mem
            free(currNode->task); // free task mem
            free(currNode); // free node mem
            currNode = *head; // set curr node as new head
          }
          else {
            /* release highest pri from list */
            prevNode->next = currNode->next;
            free(currNode->task->name); // free name mem
            free(currNode->task); // free task mem
            free(currNode); // free node mem
            currNode = prevNode->next;
          }
        }
        else {
          prevNode = currNode; // prevNode will end on last node
          currNode = currNode->next; // currNode will end on NULL
        }
      }
    }

    /***************************run one process********************************/
    else { // only one process with highest priority
      run(highestPri->task, highestPri->task->burst); // run processes
      time += highestPri->task->burst; // keep track of how much time has past
      printf("        Time is now: %d\n", time); // print time

      if (prevHighestPri == NULL) { // highest priority is fist node in list
        *head = highestPri->next; // detach first node from list
      }
      else {
        /* release highest pri from list */
        prevHighestPri->next = highestPri->next;
      }

      free(highestPri->task->name); // free name mem
      free(highestPri->task); // free task mem
      free(highestPri); // free node mem
    }
  } // end while(1)

  free(head); // free node's pointer
  head = NULL;
}
