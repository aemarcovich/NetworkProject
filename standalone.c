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
 #include <time.h>
 #include <arpa/inet.h>
#include <netinet/ip_icmp.h>

#define PORT "3490" // the port client will be connecting to

 #define MAXDATASIZE 1000 // max number of bytes we can get at once

int UDP_packets=20;//match with server
 // get sockaddr, IPv4 or IPv6:
char serverip[MAXDATASIZE];
int sport;
int dport;
int dport_tcp;
int dport_tcp_t;
char tcpport[MAXDATASIZE];
int udpsize;
int intertime;
int TTL;
char data[MAXDATASIZE]={0};
double time_taken;
double check=.1;
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void setvar(FILE *fp){
  char local_data[MAXDATASIZE]={0};
  char* token;
  fread(data,1,MAXDATASIZE,fp);
  strcpy(local_data,data);
  char* rest =local_data;
  int line=0;
  while(token=strtok_r(rest,"\n",&rest)){
    if(line==0)
      strcpy(serverip,token);
    else if(line==1)
      sport=atoi(token);
    else if(line==2)
      dport=atoi(token);
    else if(line==3)
      dport_tcp=atoi(token);
     else if(line==4)
      dport_tcp_t=atoi(token);
     else if(line==5)
      strcpy(tcpport,token);
     else if(line==6)
      udpsize=atoi(token);
     else if(line==7)
      intertime=atoi(token);
     else if(line==8)
      UDP_packets=atoi(token);
     else if(line==9)
      TTL=atoi(token);
    //printf("%s\n",serverip );
    line++;
  }
}
void send_file(FILE *fp,int sockfd){
	 //char data[MAXDATASIZE] = {0};

	// while(fgets(data,MAXDATASIZE,fp) != NULL){
   //fread(data,1,MAXDATASIZE,fp);
		if(send(sockfd,data, strlen(data),0) == -1){
			perror("-Error with sending data");
			exit(1);
		}
		printf("file sent \n");
		bzero(data, MAXDATASIZE);
	// }
	//sleep(10);
}

void send_udp(){

  int port = 8765;
  int sockfd;
  struct sockaddr_in serverAddr,srcaddr;
  char buffer[1000];
  socklen_t addr_size;
  clock_t start,end;

  sockfd = socket(PF_INET, SOCK_DGRAM, 0);

  //set destport
  memset(&serverAddr, '\0', sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(dport);
  serverAddr.sin_addr.s_addr = inet_addr(serverip);
  printf("destination port set\n");
  
  //set Don't flag
  int val = IP_PMTUDISC_DO;
  setsockopt(sockfd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
  printf("don't flag set\n");

  //set the TTL
  setsockopt(sockfd, IPPROTO_IP, IP_TTL, &TTL, sizeof(int));
  printf("TTL set to %d\n",TTL );


  //set source port
   memset(&srcaddr, 0, sizeof(srcaddr));
   srcaddr.sin_family = AF_INET;
   srcaddr.sin_addr.s_addr = inet_addr("192.168.137.128");
   srcaddr.sin_port = htons(sport);
   printf("source port set\n");

    if (bind(sockfd, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
        perror("bind");
        exit(1);
    }
    char zero_train[UDP_packets][udpsize];
    // host1(config)#ip ttl TTL;

    for(int t=0; t<UDP_packets; t++)
    {
    	//buffe
    	char zeropacket[udpsize];
      memset(zeropacket,0,udpsize);
    	uint16_t *packet_id = (uint16_t*) zeropacket;
    	*packet_id = t;
    	memcpy(zero_train[t], zeropacket, udpsize);
    }
    for (int i = 0; i < UDP_packets; ++i)
    {
      sendto(sockfd, zero_train[i], udpsize, 0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    }
    sleep(intertime);

    for(int t=0; t<UDP_packets; t++)
    {
      
      char randpacket[udpsize];
     uint16_t *packet_id = (uint16_t*) randpacket;
     *packet_id = t;
     for(int i=2;i<udpsize;i++)
     {
     //genterate rand int random bit
       char num = (rand() % (256));
       randpacket[i]=num;
     }
      memcpy(zero_train[t], randpacket, udpsize);
    }
    for (int i = 0; i < UDP_packets; ++i)
    {
      sendto(sockfd, zero_train[i], udpsize, 0,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    }



}

void recv_value(int sockfd){
  char message[500];
  char timer[1000];
  strcpy(message,"send the time\n");
    if(send(sockfd,message, strlen(message),0) == -1){
      perror("-Error with sending data");
      exit(1);
    }
    printf("message  sent\n");
    recv(sockfd,timer,strlen(message),0);
    time_taken=atof(timer);
    printf("%lf seconds\n",time_taken );
    if(time_taken>check)
    {
      printf("Compression detected");
    }
    else{
      printf("Compression not detected\n");
    }
    bzero(message, 500);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	FILE *fp;
	char *filename=argv[1];

	char buffer[1024];
	struct sockaddr_in serverAddr;


	if (argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}

 fp= fopen(filename,"r");

 if(fp==NULL){
  perror("error with reading");
  exit(1);
 }
  setvar(fp);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;


	if ((rv = getaddrinfo(serverip, tcpport, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

 // loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("socket");
		continue;
	}

	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
		close(sockfd);
		perror("connect");
		continue;
	}

	break;
}
if (p == NULL) {
	fprintf(stderr, "failed to connect\n");
	return 2;
}

inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	s, sizeof s);
printf("connecting to %s\n", s);

 freeaddrinfo(servinfo); // all done with this structure

 if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
 	perror("recv");
 	exit(1);
 }

 send_file(fp,sockfd);
 fclose(fp);
 sleep(10);

 send_udp();
 //recv_value(sockfd);
 if(time_taken>check)
 {
  printf("Compression detected");
}else{
    printf("Compression not detected\n");
    }


 buf[numbytes] = '\0';

 printf("received '%s'\n",buf);



 close(sockfd);

 return 0;
}