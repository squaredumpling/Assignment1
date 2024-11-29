/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * STUDENT_NAME_1 (STUDENT_NR_1)
 * STUDENT_NAME_2 (STUDENT_NR_2)
 *
 * Grading:
 * Your work will be evaluated based on the following criteria:
 * - Satisfaction of all the specifications
 * - Correctness of the program
 * - Coding style
 * - Report quality
 * - Deadlock analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>    
#include <unistd.h>    // for execlp
#include <mqueue.h>    // for mq


#include "settings.h"  
#include "messages.h"
#include "request.h"

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];


int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }

  struct mq_attr attr; 
  attr.mq_maxmsg = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof(Request);

  mqd_t req_mq  = mq_open(REQ_QUEUE_NAME, O_CREAT | O_RDWR, 0600, &attr);
  mqd_t resp_mq = mq_open(RESP_QUEUE_NAME, O_CREAT | O_RDWR, 0600, &attr);
  mqd_t s1_mq   = mq_open(W1_QUEUE_NAME, O_CREAT | O_RDWR, 0600, &attr); 
  mqd_t s2_mq   = mq_open(W2_QUEUE_NAME, O_CREAT | O_RDWR, 0600, &attr); 

  if (req_mq == -1 || resp_mq == -1 || s1_mq == -1 || s2_mq == -1) perror("Channel creation failed!");



  // TODO:
    //  * create the message queues (see message_queue_test() in
    //    interprocess_basic.c)
    //  * create the child processes (see process_test() and
    //    message_queue_test())
    //  * read requests from the Req queue and transfer them to the workers
    //    with the Sx queues
    //  * read answers from workers in the Rep queue and print them
    //  * wait until the client has been stopped (see process_test())
    //  * clean up the message queues (see message_queue_test())

    // Important notice: make sure that the names of the message queues
    // contain your goup number (to ensure uniqueness during testing)


    // open message queues

    printf("dealer ready\n\n");

    int pid = fork();

    if (pid == 0) {
      int retcode = execlp("./worker_s1", "worker1", NULL);
      printf("code %d\n", retcode);
    }
    else {
      wait(NULL);
      printf("\nchild finished\n");
    }


    mq_close(req_mq);
    mq_close(resp_mq);
    // Close s1_mq and s2_mq in worker code?
  
  return (0);
}
