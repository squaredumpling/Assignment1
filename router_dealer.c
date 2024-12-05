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
#include <time.h>      // for time()


#include "settings.h"  
#include "messages.h"

char client_to_dealer_name[30] = "/request_queue_group48";
char dealer_to_worker1_name[30] = "/worker1_queue_group48";
char dealer_to_worker2_name[30] = "/worker2_queue_group48";
char worker_to_dealer_name[30] = "/response_queue_group48";


int main (int argc, char * argv[])
{
  // argument safety
  if (argc != 1) fprintf (stderr, "%s: invalid arguments\n", argv[0]);
  
  printf("dealer ready\n\n");

  mq_unlink(client_to_dealer_name);
  mq_unlink(dealer_to_worker1_name); 
  mq_unlink(worker_to_dealer_name);

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
  if (cd_channel == -1) printf("cd channel creation error\n");

  if (dw_channel == -1) printf("dw channel creation error\n");
  
  if (wd_channel == -1) printf("wd channel creation error\n");



  // create client process
  int pid = fork();
  if (pid == 0){
    execlp("./client", "client", client_to_dealer_name, NULL);
  }

  // initialize service 1 workers
  for (int i=0; i<N_SERV1; i++) {
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

  // initialize service 2 workers
  for (int i=0; i<N_SERV2; i++) {
    int pid = fork();

    if (pid == 0) {
      // initialize worker name by index
      char worker_name[40];
      sprintf(worker_name, "serv2worker%d", i);

      // execute the worker code
      int retcode = execlp("./worker_s2", worker_name, dealer_to_worker1_name, worker_to_dealer_name, NULL);
      printf("worker error %d\n", retcode);
    }
  }


  // while responses are not finished manage everyone
  int requests_todo = 0;
  bool requests_ongoing = true;
  while (requests_ongoing || (0 < requests_todo)){

    for (int i=0; i<MQ_MAX_MESSAGES; i++){
      // receive request from client
      CDMessage cd_message;
      mq_receive(cd_channel, (char*)&cd_message, sizeof(CDMessage), 0);
      printf("dealer got request %d %d %d\n", cd_message.request_id, cd_message.data, cd_message.service_id);

      if (cd_message.request_id == -1){
        requests_ongoing = false;
        // terminate workers
        break;
      }

      // pass request to workers
      DWMessage dw_message;
      dw_message.request_id = cd_message.request_id;
      dw_message.data = cd_message.data;
      mq_send(dw_channel, (char*)&dw_message, sizeof(DWMessage), 0);
      requests_todo++;
    }
    
    usleep(1000000); // 1 second

    for (int i=0; i<MQ_MAX_MESSAGES && (0 < requests_todo); i++){
      // receive message from workers
      WDMessage wd_message;
      mq_receive(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
      requests_todo--;

      // print response
      printf("dealer got response %d %d\n", wd_message.request_id, wd_message.result);
    }  

    usleep(1000000); // 1 second
  }

  printf("read well\n");

  // send message to all workers to close
  for (int i=0; i<N_SERV1; i++){
    DWMessage terminate_message = {-1, 0};
    mq_send(dw_channel, (char*)&terminate_message, sizeof(DWMessage), 0);
  }

  // wait for them to close
  int worker_id;
  while (0 < (worker_id = wait(NULL))) {}
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
