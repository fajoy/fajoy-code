/*
 * ClientQueue.h
 *
 *  Created on: 2011/11/13
 *      Author: fajoy
 */

#ifndef CLIENTQUEUE_H_
#define CLIENTQUEUE_H_
#include "Client.h"
#define ClientQueueLength 30
int clietnQueueFlowId=0;
static Client clientQueue[ClientQueueLength];
static void InitClientQueue(){
	memset(&clientQueue[0], 0, ClientQueueLength * sizeof(Client));
}
#endif /* CLIENTQUEUE_H_ */
