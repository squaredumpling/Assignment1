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

static void rsleep (int t);


int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the message queue (whose name is provided in the
    //    arguments)
    //  * repeatingly:
    //      - get the next job request 
    //      - send the request to the Req message queue
    //    until there are no more requests to send
    //  * close the message queue

    char *request_mq_name = argv[3];
    mqd_t request_channel = mq_open(request_mq_name, O_WRONLY);

    // check for channel opening errors
    if (request_channel == -1)  printf("open request channel error\n");

    // repeatingly get the next job and send the request to the Req message queue
    Request req;
    while (getNextRequest(&req.job, &req.data, &req.service) == NO_ERR) {
        if (mq_send(request_channel, (char*)&req, sizeof(Request), 0) == -1) {
            perror("client mq_send failed");
            break;
        }
    }

    // Notify dealer that client has no requests
    Request no_requests = {-1, 0, 0};
    if (mq_send(request_channel, (char*)&no_requests, sizeof(Request), 0) == -1) perror("client termination mq_send failed");

    // close message queue
    mq_close(request_channel);

    //unlink message queue
    mq_unlink(request_mq_name); //request_mq_name instead of argv[3]???

    exit(53);
    
    return (0);
}
