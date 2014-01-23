#ifndef OBLNETWORKING_CLASS
#define OBLNETWORKING_CLASS

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sockLib.h"
#include "inetLib.h"
#include "hostLib.h"

#define PORT 12345

class CRioNetworking
{
private:
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in remaddr;	/* remote address */
	int addrlen;
	int fd;				/* our socket */
	char *server;

public:
	CRioNetworking(void);
	int connect(void);
	void disconnect(void);
	void send(char* message);
	void receive(char * buf, int bufLen);
};

#endif
