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

char client_to_dealer_name[30];
char dealer_to_worker1_name[30];
char dealer_to_worker2_name[30];
char worker_to_dealer_name[30];

/*struct mq_attr{ 
int mq_maxmsg = MQ_MAX_MESSAGES;

}attr; 
*/

int main (int argc, char * argv[])
{
  // argument safety
  if (argc != 1)
  {
    fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  }
  
  printf("dealer ready\n\n");

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

  // open client to dealer message queue
  struct mq_attr cdattr;
  cdattr.mq_maxmsg = MQ_MAX_MESSAGES;
  cdattr.mq_msgsize = sizeof(CDMessage);
  client_to_dealer_name[30] = REQ_QUEUE_NAME;
  mqd_t cd_channel = mq_open(client_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &cdattr);

  // open dealer to worker message queue
  struct mq_attr dwattr;
  dwattr.mq_maxmsg = MQ_MAX_MESSAGES;
  dwattr.mq_msgsize = sizeof(DWMessage);
  char DWChannelName[40] = W1_QUEUE_NAME;
  mqd_t dw_channel = mq_open(DWChannelName, O_CREAT | O_WRONLY | O_EXCL, 0600, &dwattr);

  // open worker to dealer message queue
  struct mq_attr wdattr;
  wdattr.mq_maxmsg = MQ_MAX_MESSAGES;
  wdattr.mq_msgsize = sizeof(WDMessage);
  char WDChannelName[40] = RESP_QUEUE_NAME;
  mqd_t wd_channel = mq_open(WDChannelName, O_CREAT | O_RDONLY | O_EXCL, 0600, &wdattr);

  // test channels
  if (cd_channel == -1)
    printf("cd channel creation error\n");

  if (dw_channel == -1)
    printf("dw channel creation error\n");
  

  if (wd_channel == -1)
    printf("wd channel creation error\n");
  



  // create client process
  char client_name[40];
  execlp("./client", client_name, client_to_dealer_name, NULL);
  
  // initialize workers
  for (int i=0; i<1; i++) {
    int pid = fork();

    if (pid == 0) {
      // initialize worker name by index
      char worker_name[40];
      sprintf(worker_name, "serv1worker%d", i); 

      // execute the worker code
      int retcode = execlp("./worker_s1", worker_name, DWChannelName, WDChannelName, NULL);
      printf("code %d\n", retcode);
    }
  }





  // big while responses are not done

    // read request from clients

    // pass request to workers 

    // read reasponse from woarkers 

    // print response

  // end while



  // receive message from client
  CDMessage cd_message;
  mq_receive(cd_channel, (char*)&cd_message, sizeof(CDMessage), 0);

  // send message to all workers to close
  DWMessage terminate_message;
  terminate_message.reqest_id = 3;
  terminate_message.data = 16;
  mq_send(dw_channel, (char*)&terminate_message, sizeof(DWMessage), 0);

  // receive message from workers
  WDMessage wd_message;
  mq_receive(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
  printf("received %d %d\n", wd_message.request_id, wd_message.result);

  // wait for them to close
  int worker_id;
  while (0 < (worker_id = wait(NULL))) {
    printf("%d ", worker_id);
  }
  printf("\nall workers terminated\n");

  // close channels
  mq_close(cd_channel);
  mq_close(dw_channel);
  mq_close(wd_channel);

  // unlink channels
  mq_unlink(client_to_dealer_name);
  mq_unlink(DWChannelName);
  mq_unlink(WDChannelName);

  printf("peace out\n");

  return (0);
}
