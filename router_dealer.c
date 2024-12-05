/* 
 * Operating Systems  (2INCO)  Practical Assignment
 * Interprocess Communication
 *
 * Mihnea Buzoiu 1923552
 * Filip Cuciuc 1659626
 * Tudor Suteu 1961233
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

char client_to_dealer_name[30] = "/request_queue_group48";
char dealer_to_worker1_name[30] = "/worker1_queue_group48";
char dealer_to_worker2_name[30] = "/worker2_queue_group48";
char worker_to_dealer_name[30] = "/response_queue_group48";


int main (int argc, char * argv[])
{
  // argument safety
  if (argc != 1) { fprintf (stderr, "%s: invalid arguments\n", argv[0]); exit(1); }

  // open client to dealer message queue
  struct mq_attr cdattr;
  cdattr.mq_maxmsg = MQ_MAX_MESSAGES;
  cdattr.mq_msgsize = sizeof(CDMessage);
  mqd_t cd_channel = mq_open(client_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &cdattr);

  // open dealer to worker message queues
  struct mq_attr dwattr;
  dwattr.mq_maxmsg = MQ_MAX_MESSAGES;
  dwattr.mq_msgsize = sizeof(DWMessage);
  mqd_t dw1_channel = mq_open(dealer_to_worker1_name, O_CREAT | O_WRONLY | O_EXCL, 0600, &dwattr);
  mqd_t dw2_channel = mq_open(dealer_to_worker2_name, O_CREAT | O_WRONLY | O_EXCL, 0600, &dwattr);
  

  // open worker to dealer message queue
  struct mq_attr wdattr;
  wdattr.mq_maxmsg = MQ_MAX_MESSAGES;
  wdattr.mq_msgsize = sizeof(WDMessage);
  mqd_t wd_channel = mq_open(worker_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &wdattr);

  // test channels
  if (cd_channel == -1) { perror("cd channel creation error\n"); exit(2); }
  if (dw1_channel == -1) { perror("dw1 channel creation error\n"); exit(2); }
  if (dw2_channel == -1) { perror("dw2 channel creation error\n"); exit(2); }
  if (wd_channel == -1) { perror("wd channel creation error\n"); exit(2); }

  // create client process
  int pid = fork();
  if (pid == 0) execlp("./client", "client", client_to_dealer_name, NULL);
  if (pid == -1) { perror("client creation error\n"); exit(3); }

  // initialize service 1 workers
  for (int i=0; i<N_SERV1; i++) {
    int pid = fork();

    if (pid == 0) {
      // initialize worker name by index
      char worker_name[40];
      sprintf(worker_name, "serv1worker%d", i);

      // execute the worker code
      int retcode11 = execlp("./worker_s1", worker_name, dealer_to_worker1_name, worker_to_dealer_name, NULL);
      if (retcode11 == -1) { perror("execlp error"); exit(11); }
    }

    if (pid == -1) { perror("worker creation error"); exit(4); }
  }

  // initialize service 2 workers
  for (int i=0; i<N_SERV2; i++) {
    int pid = fork();

    if (pid == 0) {
      // initialize worker name by index
      char worker_name[40];
      sprintf(worker_name, "serv2worker%d", i);

      // execute the worker code
      int retcode11 = execlp("./worker_s2", worker_name, dealer_to_worker2_name, worker_to_dealer_name, NULL);
      if (retcode11 == -1) { perror("execlp error"); exit(11); }
    }

    if (pid == -1) { perror("worker creation error\n"); exit(4); }
  }


  // while responses are not finished manage everyone
  int requests_todo = 0;
  bool requests_ongoing = true;
  while (requests_ongoing || (0 < requests_todo)){

    for (int i=0; i<MQ_MAX_MESSAGES; i++){
      // receive request from client
      CDMessage cd_message;
      int retcode5 = mq_receive(cd_channel, (char*)&cd_message, sizeof(CDMessage), 0);
      if (retcode5 == -1) { perror("dealer receive error\n"); exit(5); }
      // printf("dealer got request %d %d %d\n", cd_message.request_id, cd_message.data, cd_message.service_id);

      // if client sent end message stop reading
      if (cd_message.request_id == -1){
        requests_ongoing = false;
        break;
      }

      // pass request to workers
      DWMessage dw_message;
      dw_message.request_id = cd_message.request_id;
      dw_message.data = cd_message.data;
      
      // select type of worker
      int retcode6;
      switch (cd_message.service_id){
        case 1: retcode6 = mq_send(dw1_channel, (char*)&dw_message, sizeof(DWMessage), 0); break;
        case 2: retcode6 = mq_send(dw2_channel, (char*)&dw_message, sizeof(DWMessage), 0); break;
      }
      if (retcode6 == -1) { perror("dealer send error\n"); exit(6); }
      requests_todo++;
    }

    for (int i=0; i<MQ_MAX_MESSAGES && (0 < requests_todo); i++){
      // receive message from workers
      WDMessage wd_message;
      int retcode5 = mq_receive(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
      if (retcode5 == -1) { perror("dealer receive error\n"); exit(5); }
      requests_todo--;

      // print response
      printf("%d -> %d\n", wd_message.request_id, wd_message.result);
    }  
  }

  // printf("read well\n");

  // send message to all workers1 to close
  for (int i=0; i<N_SERV1; i++){
    DWMessage terminate_message = {-1, 0};
    int retcode6 = mq_send(dw1_channel, (char*)&terminate_message, sizeof(DWMessage), 0);
    if (retcode6 == -1) { perror("dealer send error\n"); exit(6); }
  }

  // send message to all workers2 to close
  for (int i=0; i<N_SERV2; i++){
    DWMessage terminate_message = {-1, 0};
    int retcode6 = mq_send(dw2_channel, (char*)&terminate_message, sizeof(DWMessage), 0);
    if (retcode6 == -1) { perror("dealer send error\n"); exit(6); }
  }

  // wait for them to close
  int worker_id;
  while (0 < (worker_id = wait(NULL))) {
    if (worker_id == -1) { perror("dealer wait error\n"); exit(7); }
  }
  //printf("\nall workers terminated\n");

  // close channels
  if (mq_close(cd_channel) == -1) { perror("close channel error\n"); exit(8); }
  if (mq_close(dw1_channel) == -1) { perror("close channel error\n"); exit(8); }
  if (mq_close(dw2_channel) == -1)  { perror("close channel error\n"); exit(8); }
  if (mq_close(wd_channel) == -1)  { perror("close channel error\n"); exit(8); }

  // unlink channels
  if (mq_unlink(client_to_dealer_name) == -1) { perror("cd channel unlink error\n"); exit(9); }
  if (mq_unlink(dealer_to_worker1_name) == -1) { perror("dw1 channel unlink error\n"); exit(9); }
  if (mq_unlink(dealer_to_worker2_name) == -1) { perror("dw2 channel unlink error\n"); exit(9); }
  if (mq_unlink(worker_to_dealer_name) == -1) { perror("wd channel unlink error\n"); exit(9); }

  return (0);
}
