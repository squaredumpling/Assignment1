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

#ifndef MESSAGES_H
#define MESSAGES_H

// client to dealer message
typedef struct{
    int request_id, service_id, data;
}CDMessage;

// dealer to worker message
typedef struct{
    int reqest_id, data;
}DWMessage;

// worker to dealer message
typedef struct{
    int request_id, result; 
}WDMessage;

#endif
