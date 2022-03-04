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

    /***********************traverse list for process count********************/
    struct node* currNode = *head; // keeps track of current node
    struct node* prevNode = *head; // keeps track of previous node
    int procCount = 0; // keeps track of how many processes are in list

    while (prevNode->next != NULL) { // traverse list
      procCount++;
      prevNode = currNode; // prevNode will end on last node
      currNode = currNode->next; // currNode will end on NULL
    } // end while(currNode->next != NULL)

    /**************************put processes in array**************************/
      /* allocate mem for tied processes array */
      struct node** processArr = malloc(procCount * sizeof(struct node*));

      currNode = *head; // keeps track of current node
      prevNode = *head; // keeps track of previous node
      int index = 0; // keeps track of index of process array
      while (prevNode->next != NULL) { // traverse list
        processArr[index++] = currNode; // fill array
        prevNode = currNode; // prevNode will end on last node
        currNode = currNode->next; // currNode will end on NULL
      }

      /************************run processes***********************************/
      int time = 0; // keeps track of amount of time run
      bool ran = true; // keeps track if there was a process that ran
      while(ran) {
        ran = false; // if no process ran, ran will remain false and end loop
        for (int i = procCount - 1; i >= 0; i--) { // traverse process array
          /* if burst greater or equal to 10 */
          if (processArr[i]->task->burst >= 10) {
            run(processArr[i]->task, 10); // run process for 10
            ran = true; // set ran to true
            time += 10; // add 10 to time
            printf("        Time is now: %d\n", time); // print time
            /* decrease process burst time by 10 */
            processArr[i]->task->burst -= 10;
          }
          /* burst time less than 10 but greater than 0 */
          else if (processArr[i]->task->burst > 0) {
            // run process for the amount of burst it has left
            run(processArr[i]->task, processArr[i]->task->burst);
            ran = true; // set ran to true
            time += processArr[i]->task->burst; // increase time by burst
            printf("        Time is now: %d\n", time); // print time
            /* decrease the process burst time to 0 so won't run anymore */
            processArr[i]->task->burst -= processArr[i]->task->burst;
          }
        }
      }

      /***********************delete processes from array**********************/
      for (int i = 0; i < procCount; i++) {
        free(processArr[i]->task->name); // free name mem
        free(processArr[i]->task); // free task mem
        free(processArr[i]); // free node mem
      }

  free(head); // free node's pointer
  head = NULL;
}
