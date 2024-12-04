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


#define REQ_QUEUE_NAME      "/request_queue_group_48"
#define W1_QUEUE_NAME       "/worker1_queue_group_48"
#define W2_QUEUE_NAME       "/worker2_queue_group_48"
#define RESP_QUEUE_NAME     "/response_queue_group_48"
#define RESP_QUEUE_NAME     "/response_queue_group_48"


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
  char client_to_dealer_name[25];
  char dealer_to_worker1_name[25];
  char dealer_to_worker2_name[25];
  char worker_to_dealer_name[25];

  // create and open client to dealer message queue
  struct mq_attr attr;
  attr.mq_maxmsg = MQ_MAX_MESSAGES;
  attr.mq_msgsize = sizeof(Request);
  client_to_dealer_name[25] = REQ_QUEUE_NAME;
  mqd_t cd_channel = mq_open(client_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &attr);

  // create and open dealer to worker1 message queue
  attr.mq_msgsize = sizeof(DWMessage);
  dealer_to_worker1_name[25] = W1_QUEUE_NAME;
  mqd_t w1_channel = mq_open(dealer_to_worker1_name, O_CREAT | O_WRONLY | O_EXCL, 0600, &attr);

  // create and open dealer to worker2 message queue
  dealer_to_worker2_name[25] = W2_QUEUE_NAME;
  mqd_t w2_channel = mq_open(dealer_to_worker2_name, O_CREAT | O_WRONLY | O_EXCL, 0600, &attr);

  // create and open worker to dealer message queue,
  attr.mq_msgsize = sizeof(WDMessage);
  worker_to_dealer_name[25] = RESP_QUEUE_NAME;
  mqd_t wd_channel = mq_open(worker_to_dealer_name, O_CREAT | O_RDONLY | O_EXCL, 0600, &attr);

  // check for channel opening errors
  if (cd_channel == -1) printf("cd channel creation error\n");
  if (w1_channel == -1) printf("w1 channel creation error\n");
  if (w2_channel == -1) printf("w2 channel creation error\n");
  if (wd_channel == -1) printf("wd channel creation error\n");

  if (cd_channel == -1 || w1_channel == -1 || w2_channel == -1 || wd_channel == -1) {
    perror("Channel(s) mq_open failed");
    // close channels
    mq_close(cd_channel);
    mq_close(w1_channel);
    mq_close(w2_channel);
    mq_close(wd_channel);

    // unlink channels
    mq_unlink(client_to_dealer_name);
    mq_unlink(dealer_to_worker1_name);
    mq_unlink(dealer_to_worker2_name);
    mq_unlink(worker_to_dealer_name);
    exit(1);
  }

  // create client process
  pid_t client_pid = fork();

  if (client_pid == -1) {
    perror("fork() failed - client process");
    exit(1);
  }
  if (client_pid == 0) {
    printf("client process started with pid: %d\n", getpid());
    execlp("./client", REQ_QUEUE_NAME, NULL);
    perror("execlp() failed - client process");
    exit(2);
  }

  pid_t worker_pids[];
  for (int i = 1; i <= N_SERV1 + N_SERV2; i++) {
    worker_pids[i] = fork();
    if (worker_pids[i] == -1) {
      perror("fork() failed - worker process");
      exit(1);
    }
    if (worker_pids[i] == 0) {
      if (i <= N_SERV1) {
        printf("worker1 (%d) process started with pid: %d\n", i, getpid());
        execlp("./worker_s1", W1_QUEUE_NAME, RESP_QUEUE_NAME, NULL);
        perror("execlp() failed - worker1 process");
        exit(3);
      }

      printf("worker2 (%d) process started with pid: %d\n", i - N_SERV1, getpid());
      execlp("./worker_s2", W2_QUEUE_NAME, RESP_QUEUE_NAME, NULL);
      perror("execlp() failed - worker2 process");
      exit(4);
    }
    printf("created worker process with index %d\n", i);
  }

  Request cd_message = {0, 0, 0};
  while (cd_message.job != NO_REQ) {
    if (mq_receive(cd_channel, (char*)&cd_message, sizeof(Request), 0) == -1) {
      perror("router-dealer mq_receive() failed");
      break;
    }

    DWMessage sx_message;
    sx_message.jobID = cd_message.job;
    sx_message.data = 16;
    if (cd_message.service == 1) {
      if (mq_send(w1_channel, (char *)&sx_message, sizeof(DWMessage), 0) == -1) {
        perror("router-dealer mq_send() ro S1 failed");
        break;
      }
    } else if (cd_message.service == 2) {
      if (mq_send(w2_channel, (char *)&sx_message, sizeof(DWMessage), 0) == -1) {
        perror("router-dealer mq_send() to S2 failed");
        break;
      }
    }
  }

 // TODO: think about how to signal when the response queue is empty (when both w1 and w2 are finished) and add the waitpid() stuff

  WDMessage wd_message;
  mq_receive(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
  printf("received %d %d\n", wd_message.jobID, wd_message.result);


  // initialize workers
  // for (int i=0; i<1; i++) {
  //   int pid = fork();
  //
  //   if (pid == 0) {
  //     // initialize worker name by index
  //     char worker_name[40];
  //     sprintf(worker_name, "serv1worker%d", i);
  //
  //     // execute the worker code
  //     int retcode = execlp("./worker_s1", worker_name, DWChannelName, WDChannelName, NULL);
  //     printf("code %d\n", retcode);
  //   }
  // }

  // wait for them to close
  int worker_id;
  while (0 < (worker_id = wait(NULL))) {
    printf("%d ", worker_id);
  }
  printf("\nall workers terminated\n");

  // close channels
  mq_close(cd_channel);
  mq_close(w1_channel);
  mq_close(w2_channel);
  mq_close(wd_channel);

  // unlink channels
  mq_unlink(client_to_dealer_name);
  mq_unlink(dealer_to_worker1_name);
  mq_unlink(dealer_to_worker2_name);
  mq_unlink(worker_to_dealer_name);

  printf("peace out\n");

  return (0);
}
