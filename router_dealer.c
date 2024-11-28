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

char client2dealer_name[30];
char dealer2worker1_name[30];
char dealer2worker2_name[30];
char worker2dealer_name[30];

/*struct mq_attr{ 
int mq_maxmsg = MQ_MAX_MESSAGES;

}attr; 
*/

int main (int argc, char * argv[])
{
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
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

  printf("dealer ready\n\n");

  // open worker message queue
  struct mq_attr wattr;
  wattr.mq_maxmsg = 4;
  wattr.mq_msgsize = sizeof(WMessage);
  char sendChannelName[10] = "/send";
  mqd_t channel = mq_open(sendChannelName, O_CREAT | O_WRONLY | O_EXCL, 0600, &wattr);

  // open worker message queue
  struct mq_attr rattr;
  rattr.mq_maxmsg = 4;
  rattr.mq_msgsize = sizeof(RMessage);
  char receiveChannelName[10] = "/receive";
  mqd_t channel = mq_open(receiveChannelName, O_CREAT | O_RDONLY | O_EXCL, 0600, &rattr);

  

  for (int i=0; i<10; i++) {
    int pid = fork();

    if (pid == 0) {
      char worker_name[20];
      sprintf(worker_name, "serv1worker%d", i); 
      int retcode = execlp("./worker_s1", worker_name, sendChannelName, receiveChannelName, NULL);
      printf("code %d\n", retcode);
    }
  }

  // close all children
  // send message to all of them

  // wait for them to close
  int worker_id;
  while (0 < (worker_id = wait(NULL))) {
    printf("\nid %d terminated\n", worker_id);
  }

  printf("\nworkers finished\n");

  return (0);
}
