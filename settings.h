#ifndef _SETTINGS_H_
#define _SETTINGS_H_


// settings for interprocess communications
// (note: be sure that MQ_MAX_MESSAGES <= /proc/sys/fs/mqueue/msg_max (= 10 on most systems))
#define N_SERV1    	        4
#define N_SERV2    	        3
#define MQ_MAX_MESSAGES     10
#define REQ_QUEUE_NAME      "/request_queue"
#define W1_QUEUE_NAME       "/worker1_queue"
#define W2_QUEUE_NAME       "/worker2_queue"
#define RESP_QUEUE_NAME     "/response_queue" 
#define STOP_MSG            "exit"    


#endif

