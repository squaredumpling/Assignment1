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

char client_to_dealer_name[30] = REQ_QUEUE_NAME;
char dealer_to_worker1_name[30] = W1_QUEUE_NAME;
char dealer_to_worker2_name[30] = W2_QUEUE_NAME;
char worker_to_dealer_name[30] = RESP_QUEUE_NAME;

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

  // mq_unlink(client_to_dealer_name);
  // mq_unlink(dealer_to_worker1_name); 
  // mq_unlink(worker_to_dealer_name);




  // open client to dealer message queue
  struct mq_attr cdattr;
  cdattr.mq_maxmsg = MQ_MAX_MESSAGES;
  cdattr.mq_msgsize = sizeof(CDMessage);
  mqd_t cd_channel = mq_open(client_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &cdattr);

  // open dealer to worker message queue
  struct mq_attr dwattr;
  dwattr.mq_maxmsg = MQ_MAX_MESSAGES;
  dwattr.mq_msgsize = sizeof(DWMessage);
  mqd_t dw_channel = mq_open(dealer_to_worker1_name, O_CREAT | O_WRONLY | O_EXCL, 0600, &dwattr);

  // open worker to dealer message queue
  struct mq_attr wdattr;
  wdattr.mq_maxmsg = MQ_MAX_MESSAGES;
  wdattr.mq_msgsize = sizeof(WDMessage);
  mqd_t wd_channel = mq_open(worker_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &wdattr);

  // test channels
  if (cd_channel == -1)
    printf("cd channel creation error\n");

  if (dw_channel == -1)
    printf("dw channel creation error\n");
  
  if (wd_channel == -1)
    printf("wd channel creation error\n");
  



  // create client process
  int pid = fork();
  if (pid == 0){
    execlp("./client", "client", client_to_dealer_name, NULL);
  }

  // initialize workers
  for (int i=0; i<1; i++) {
    int pid = fork();

    if (pid == 0) {
      // initialize worker name by index
      char worker_name[40];
      sprintf(worker_name, "serv1worker%d", i);

      // execute the worker code
      int retcode = execlp("./worker_s1", worker_name, dealer_to_worker1_name, worker_to_dealer_name, NULL);
      printf("worker error %d\n", retcode);
    }
  }





  // while responses are not done manage everyone
  bool once = true;
  while (once){

    // receive request from client
    Request request;
    mq_receive(cd_channel, (char*)&request, sizeof(Request), 0);
    printf("request %d %d %d\n", request.job, request.data, request.service);

    // pass request to workers 
    DWMessage dw_message;
    dw_message.reqest_id = 3;
    dw_message.data = 16;
    mq_send(dw_channel, (char*)&dw_message, sizeof(DWMessage), 0);

    // receive message from workers
    WDMessage wd_message;
    mq_receive(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
    

    // print response
    printf("response %d %d\n", wd_message.request_id, wd_message.result);

    once = false;
  }



  

  // send message to all workers to close
   DWMessage terminate_message;
   terminate_message.reqest_id = -1;
   terminate_message.data = 2;
   mq_send(dw_channel, (char*)&terminate_message, sizeof(DWMessage), 0);

  // wait for them to close
  int worker_id;
  while (0 < (worker_id = wait(NULL))) {
    
  }
  printf("\nall workers terminated\n");

  // close channels
  mq_close(cd_channel);
  mq_close(dw_channel);
  mq_close(wd_channel);

  // unlink channels
  mq_unlink(client_to_dealer_name);
  mq_unlink(dealer_to_worker1_name);
  mq_unlink(worker_to_dealer_name);

  printf("\npeace out\n");

  return (0);
}
