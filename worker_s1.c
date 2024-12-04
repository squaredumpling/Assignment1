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
#include "service1.h"

static void rsleep (int t);


int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the
    //    arguments)
    //  * repeatedly:
    //      - read from the S1 message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do the job 
    //      - write the results to the Rsp message queue
    //    until there are no more tasks to do
    //  * close the message queues

    printf("I am a worker %s\n", argv[1]);

    // open chennels
    mqd_t dw_channel = mq_open(argv[0], O_RDONLY);
    mqd_t wd_channel = mq_open(argv[1], O_WRONLY);

    // channel opening errors
    if (dw_channel == -1) printf("dw channel opening error\n");
    if (wd_channel == -1) printf("wd channel opening error\n");

    DWMessage dw_message;
    WDMessage wd_message;

    // loop until you recieve terminate message
    while (dw_message.jobID != NO_REQ) {
        // read from queue S1
        if (mq_receive(dw_channel, (char*)&dw_message, sizeof(DWMessage), 0) == -1) {
            perror("worker1 mq_receive failed");
            break;
        }
        printf("read %d %d\n", dw_message.jobID, dw_message.data);

        printf("sleep\n");
        rsleep(1000000); // sleep 10 seconds
        printf("woke up\n");

        // do job
        // assigned service result directly to message

        //write resp
        wd_message.jobID = dw_message.jobID;
        wd_message.result = service(dw_message.data);
        if (mq_send(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0) == -1) {
            perror("worker1 mq_send failed");
            break;
        }
        printf("wrote to queue\n");
    }
    // end while

    WDMessage finished_jobs = {-1, 0};
    mq_send(wd_channel, (char*)&finished_jobs, sizeof(WDMessage), 0);

    // close message queues
    mq_close(dw_channel);
    mq_close(wd_channel);

    // unlink message queues
    mq_unlink(argv[0]);
    mq_unlink(argv[1]);

    exit(43);

}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;
    
    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
