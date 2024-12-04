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
    mqd_t dw_channel = mq_open(argv[1], O_RDONLY);
    mqd_t wd_channel = mq_open(argv[2], O_WRONLY);

    // test channels
    if (dw_channel == -1)
        printf("dw channel creation error\n");

    if (wd_channel == -1)
        printf("wd channel creation error\n");

    DWMessage dw_message;
    WDMessage wd_message;

    // loop until you recieve terminate message
    while (true) {

        // read from queue
        mq_receive(dw_channel, (char*)&dw_message, sizeof(DWMessage), 0);
        printf("workers1 read %d %d\n", dw_message.request_id, dw_message.data);

        if (dw_message.request_id == -1){
            break;
        }

        // sleep max 10 miliseconds
        rsleep(10000);

        //write response with service 1
        wd_message.request_id = dw_message.request_id;
        wd_message.result = service(dw_message.data);
        mq_send(wd_channel, (char*)&wd_message, sizeof(WDMessage), 0);
        printf("%s wrote %d %d\n", argv[0], wd_message.request_id, wd_message.result);
    }

    // close message queues
    mq_close(dw_channel);
    mq_close(wd_channel);

    // unlink message queues
    mq_unlink(argv[1]);
    mq_unlink(argv[2]);

    printf("%s done\n", argv[0]);

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
