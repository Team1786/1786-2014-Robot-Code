#include "CRioNetworking.h"

CRioNetworking::CRioNetworking(void)
{
	server = "10.17.86.6";	/* change this to use a different server */
	addrlen = sizeof(remaddr);		/* length of addresses */
}

int CRioNetworking::connect(void)
{
	/* create a UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);

	/* bind the socket to any valid IP address and a specific port */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	memset((char *) &remaddr, 0, sizeof(remaddr));
	//remaddr.sin_len = (u_char) addrlen;
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(PORT);
	if (inet_aton(server, &remaddr.sin_addr)!=0) {
		perror("inet_aton");
		return 1;
	}
	return 0;
}

void CRioNetworking::disconnect(){
	close(fd);
}

void CRioNetworking::send(char* message)
{
	printf("Sending \"%s\"\n", message);
	if (sendto(fd, message, strlen(message), 0, (struct sockaddr *)&remaddr, addrlen)==-1)
	{
		perror("sendto");
	}
}

int CRioNetworking::receive(char* buf, int bufLen)
{
	int recvlen=0;
	recvlen = recvfrom(fd, buf, bufLen, 0, (struct sockaddr *)&remaddr, &addrlen);
	//printf("received %d bytes\n", recvlen);
	if (recvlen > 0)
	{
		buf[recvlen] = 0;
	}
	return recvlen;
}
