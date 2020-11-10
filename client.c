/*
 ** client.c 
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <errno.h>
 #include <string.h>
 #include <netdb.h>
 #include <sys/types.h>
 #include <netinet/in.h>
 #include <sys/socket.h>

 #include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

 #define MAXDATASIZE 1024 // max number of bytes we can get at once

 // get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void send_file(FILE *fp,int sockfd){
	char data[MAXDATASIZE] = {0};

	while(fgets(data,MAXDATASIZE,fp) != NULL){
		if(send(sockfd,data, sizeof(data),0) == -1){
			perror("-Error with sending data");
			exit(1);
		}
		printf("file sent %s\n", data);
		bzero(data, MAXDATASIZE);
	}
	//sleep(10);
}
void send_udp(){

  int port = 8765;
  int sockfd;
  struct sockaddr_in serverAddr,srcaddr;
  char buffer[1024];
  socklen_t addr_size;

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);

  //set destport
  memset(&serverAddr, '\0', sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("192.168.137.129");
  
  //set Don't flag
  int val = IP_PMTUDISC_DO;
  setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));

  //set source port
   memset(&srcaddr, 0, sizeof(srcaddr));
   srcaddr.sin_family = AF_INET;
   srcaddr.sin_addr.s_addr = inet_addr("192.168.137.128");
   srcaddr.sin_port = htons(9876);

    // if (bind(sockfd, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
    //     perror("bind");
    //     exit(1);
    // }
    // for(int t=0; t<6000; t++)
    // {
    // 	//buffe
    // 	int zeropacket[1000]={0};
    // 	uint16_t *packet_id = zeropacket;
    // 	*packet_id = t;
    // 	memset((zero_train+1000)*t);
    // 	sendto(sockfd, zeropacket, 1024, 0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    // }
    // sleep(15);
    // for(int t=0; t<6000; t++)
    // {
    // 	//buffe
    // 	char randtrain[1000];
    // 	uint16_t *packet_id = randtrain;
    // 	*packet_id = t;
    // 	for(int i=2;i<sizeof(randtrain);i++)
    // 	{
    // 	//genterate rand int random bit
    // 		int num = (rand() % (256));
    // 		randtrain[i]=num;
    // 	}
    // 	sendto(sockfd, randtrain 1024, 0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    // }
    // printf("data print\n");

  strcpy(buffer,"testing UDP to server\n");
  sendto(sockfd, buffer, 1024, 0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
  printf("[+]Data Send: %s", buffer);
  addr_size = sizeof(serverAddr);
  recvfrom(sockfd, buffer, 1024, 0,(struct sockaddr*)&serverAddr, &addr_size);
  printf("[+]Data Received: %s\n", buffer);
}
// void sendPackets(char *data,int sockfd){
// 	if(send(sockfd,data, sizeof(data),0) == -1){
// 		perror("-Error with sendig data");
// 		exit(1);
// 	}
// 	bzero(data, MAXDATASIZE);
// 	printf("file sent\n");
// }
int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	FILE *fp;
	char *filename="test.txt";

	char buffer[1024];
	struct sockaddr_in serverAddr;


	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;


	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

 // loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("client: socket");
		continue;
	}

	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		close(sockfd);
		perror("client: connect");
		continue;
	}

	break;
}
if (p == NULL) {
	fprintf(stderr, "client: failed to connect\n");
	return 2;
}

inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	s, sizeof s);
printf("client: connecting to %s\n", s);

 freeaddrinfo(servinfo); // all done with this structure

 if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
 	perror("recv");
 	exit(1);
 }

 fp= fopen(filename,"r");

 if(fp==NULL){
 	perror("error with reading");
 	exit(1);
 }
 // send_file(fp,sockfd);
 // sleep(10);

 send_udp();
//  sockfd = socket(p->ai_family, SOCK_DGRAM,p->ai_protocol);

// strcpy(buffer,"testing UDP to server\n");
// sendto(sockfd, buffer, 1024, 0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
// printf("Data has been sent: %s\n", buffer);


 buf[numbytes] = '\0';
 // char data[6000*1000] = {0};
 // for(int x=1,x<=6000;x++)
 // {
 // 	createPackets(data);
 // 	sendPackets(data);
 // }

 printf("client: received '%s'\n",buf);



 close(sockfd);

 return 0;
}