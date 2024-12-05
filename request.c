/* 
 * Operating Systems (2INCO) Practical Assignment
 * Interprocess Communication
 *
 * Contains functions that are used by the clients
 *
 */

#include "request.h"

// Array of requests
const Request requests[] = { {1, 26, 1}, {2, 5, 2}, {3, 10, 2}, {5, 13, 1}, {4, 3, 1}, 
							 {8, 26, 1}, {6, 5, 2}, {7, 10, 2}, {9, 13, 1}, {10, 3, 1},
							 {11, 26, 1}, {12, 5, 2}, {13, 10, 2}, {14, 13, 1}, {15, 3, 1}};

// Places the information of the next request in the parameters sent by reference.
// Returns NO_REQ if there is no request to make.
// Returns NO_ERR otherwise.
int getNextRequest(int* jobID, int* data, int* serviceID) {
	static int i = 0;
	static int N_REQUESTS = sizeof(requests) / sizeof(Request);

	if (i >= N_REQUESTS) 
		return NO_REQ;

	*jobID = requests[i].job;
	*data = requests[i].data;
	*serviceID = requests[i].service;		
	++i;
	return NO_ERR;
		
}
