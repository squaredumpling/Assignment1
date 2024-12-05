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
#include "service1.h"

static void rsleep (int t);


int main (int argc, char * argv[])
{
    // open chennels
    mqd_t dw1_channel = mq_open(argv[1], O_RDONLY);
    mqd_t wd_channel = mq_open(argv[2], O_WRONLY);

    // test channels
    if (dw1_channel == -1) { perror("dw2 channel open error\n"); exit(10); }
    if (wd_channel == -1) { perror("wd channel open error\n"); exit(10); }

    DWMessage dw_message;
    WDMessage wd_message;

    // loop until you recieve terminate message
    while (true) {

        // read from queue
        int retcode5 = mq_receive(dw1_channel, (char*)&dw_message, sizeof(DWMessage), 0);
        if (retcode5 == -1) { perror("worker receive error\n"); exit(5); }

        if (dw_message.request_id == -1) break;

        // sleep max 10 miliseconds
        rsleep(10000);

        //write response with service 1
        wd_message.request_id = dw_message.request_id;
        wd_message.result = service(dw_message.data);
        int retcode6 = mq_send(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
        if (retcode6 == -1) { perror("dealer send error\n"); exit(6); }
    }

    // close message queues
    if (mq_close(dw1_channel) == -1) { perror("close channel error\n"); exit(8); }
    if (mq_close(wd_channel) == -1) { perror("close channel error\n"); exit(8); }

    exit(0);
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
