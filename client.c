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
#include <time.h>       // for time()

#include "messages.h"
#include "request.h"

//static void rsleep (int t);

int main (int argc, char * argv[])
{
    mqd_t request_channel = mq_open(argv[1], O_WRONLY);

    // test channels
    if (request_channel == -1) printf("request channel creation error\n");

    // repeatingly get the next job and send the request to the dealer queue
    Request request;
    getNextRequest(&request.job, &request.data, &request.service);
    //while (0 < getNextRequest(&request.job, &request.data, &request.service)) {
        printf("client has request %d %d %d\n", request.job, request.data, request.service);
        
        // make a client dealer message to standardize communication
        CDMessage cd_message;
        cd_message.request_id = request.job;
        cd_message.data = request.data;
        cd_message.service_id = request.service;
        mq_send(request_channel, (char*)&cd_message, sizeof(CDMessage), 0);
    //}     

    // close message queue
    mq_close(request_channel);

    //unlink message queue
    mq_unlink(argv[1]);
    
    exit(0);
}
