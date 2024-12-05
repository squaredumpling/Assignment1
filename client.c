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
#include <errno.h>      // for perror()
#include <unistd.h>     // for getpid()
#include <mqueue.h>     // for mq-stuff

#include "messages.h"
#include "request.h"


int main (int argc, char * argv[])
{
    // create channel
    mqd_t request_channel = mq_open(argv[1], O_WRONLY);
    if (request_channel == -1) { perror("request channel open error\n"); exit(10); }

    // repeatingly get the next job and send the request to the dealer queue
    Request request;
    while (getNextRequest(&request.job, &request.data, &request.service) == 0) {
        // make a client dealer message to standardize communication
        CDMessage cd_message;
        cd_message.request_id = request.job;
        cd_message.data = request.data;
        cd_message.service_id = request.service;
        int retcode6 = mq_send(request_channel, (char*)&cd_message, sizeof(CDMessage), 0);
        if (retcode6 == -1) { perror("client send error\n"); exit(6); }
    }     

    // send end of requests message
    CDMessage requests_finished = {-1, 0, 0};
    int retcode6 = mq_send(request_channel, (char*)&requests_finished, sizeof(CDMessage), 0);
    if (retcode6 == -1) { perror("client send error\n"); exit(6); }

    // close message queue
    if (mq_close(request_channel) == -1)  { perror("close channel error\n"); exit(8); }
    
    exit(0);
}
