#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT "3490" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 1000 // max number of bytes we can get at once
int UDP_packet=6000; //Match with client's
char serverip[MAXDATASIZE];
int sport;
int dport;
int dport_tcp;
int dport_tcp_t;
char tcpport[MAXDATASIZE];
int udpsize;
int intertime;
int TTL;
char buffer[MAXDATASIZE]={0};
double time_taken;
void sigchld_handler(int s)
{
// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
double stopWatch(int sockfd,struct sockaddr_in* si_other,socklen_t* addr_size)
{
	clock_t start,end;
	start=clock();
  	for(int t=0; t<UDP_packet; t++)
    {
    	if(recvfrom(sockfd, buffer, 1000, 0,(struct sockaddr*) si_other, addr_size)< 0){
    		printf("check\n");
    	}
    	uint16_t packid= *((uint16_t*) buffer);
    	//printf("%d: %hu\n",t , packid);
    	if(packid==UDP_packet-1){
    		break;
    	}
    }
    end=clock();
    //time in seconds
    return ((double) (end-start))/ CLOCKS_PER_SEC;
}
void udp_server(){
	//int port = 8765;
	int sockfd;
	struct sockaddr_in si_me, si_other;
	char buffer[udpsize];
	socklen_t addr_size;

	clock_t start,end;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	//destination
	memset(&si_other, '\0', sizeof(si_me));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(dport);
	si_other.sin_addr.s_addr = inet_addr("192.168.137.128");

	//soruce
	memset(&si_me, '\0', sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(sport);
	si_me.sin_addr.s_addr = inet_addr(serverip);

	bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me));
	addr_size = sizeof(si_other);

	double lowEntorpy=stopWatch(sockfd, &si_other, &addr_size);
    printf("data sent time_taken for lowEntorpy: %f\n",lowEntorpy);
    double highEntorpy=stopWatch(sockfd, &si_other, &addr_size);
    printf("data sent time_taken for highEntorpy: %f\n",highEntorpy);
    time_taken=highEntorpy-lowEntorpy;


}
void setvar(){
  char local_data[MAXDATASIZE]={0};
  char* token;
  strcpy(local_data,buffer);
  char* rest =local_data;
  int line=0;
  while(token=strtok_r(rest,"\n",&rest)){
    if(line==0)
      strcpy(serverip,token);
    else if(line==1)
      dport=atoi(token);
    else if(line==2)
      sport=atoi(token);
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
      UDP_packet=atoi(token);
     else if(line==9)
      TTL=atoi(token);
    //printf("%s\n",serverip );
    line++;
  }
}
void write_file(int sockfd, FILE* fp)
{
	int n;

		n= recv(sockfd,buffer,MAXDATASIZE,0);
		setvar();
		printf("file received\n%s\n", buffer);
		fprintf(fp, "%s", buffer);
		//dp_server();
		bzero(buffer, MAXDATASIZE);
	return;
}

void send_value(int sockfd)
{
	char message[1000];
	int n=recv(sockfd,message,MAXDATASIZE,0);
	printf("%s\n",message);
	//setting time taken to be sent.
	char arr[sizeof(time_taken)];
	printf("%f\n",time_taken);
	snprintf(arr, 1000, "%f", time_taken);
	send(sockfd,arr,MAXDATASIZE,0);
	printf("number char sent\n");

}

int main(void)
{
int sockfd, sockfd2, new_fd; // listen on sock_fd, new connection on new_fd
struct addrinfo hints, *servinfo, *p;
struct sockaddr_storage their_addr; // connector's address information
socklen_t sin_size;
struct sigaction sa;
int yes=1;
char s[INET6_ADDRSTRLEN];
int rv;
FILE *fp;
char *filename ="test2.txt";

memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // use my IP



if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
}

// loop through all the results and bind to the first we can
for(p = servinfo; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
		p->ai_protocol)) == -1) {
		perror("server: socket");
	continue;
}

if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
	sizeof(int)) == -1) {
	perror("setsockopt");
exit(1);
}

if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	close(sockfd);
	perror("server: bind");
	continue;
}

break;
}

freeaddrinfo(servinfo); // all done with this structure

if (p == NULL) {
	fprintf(stderr, "server: failed to bind\n");
	exit(1);
}

if (listen(sockfd, BACKLOG) == -1) {
	perror("listen");
	exit(1);
}

sa.sa_handler = sigchld_handler; // reap all dead processes
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	perror("sigaction");
	exit(1);
}

printf("server: waiting for connections...\n");

 // main accept() loop
	sin_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (new_fd == -1) {
		perror("accept");
	}

	inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
		s, sizeof s);
	printf("server: got connection from %s\n", s);

if (!fork()) { // this is the child process
close(sockfd); // child doesn't need the listener
if (send(new_fd, "chat ended", 10, 0) == -1)
	perror("send");
//printf(" data received: %s\n", buffer);
close(new_fd);
exit(0);
}
fp= fopen(filename,"w");
if(fp==NULL){
	perror("error with create file");
	exit(1);
}
write_file(new_fd, fp);
fclose(fp);
printf("Incoming packets.\n");

udp_server();

send_value(new_fd);

close(new_fd); // parent doesn't need this



return 0;
}