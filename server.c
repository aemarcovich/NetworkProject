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

#define PORT "3490" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 1024 // max number of bytes we can get at once

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
void udp_server(){
	int port = 8765;
  	int sockfd;
  	struct sockaddr_in si_me, si_other;
  	char buffer[1024];
  	socklen_t addr_size;

  	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  	memset(&si_other, '\0', sizeof(si_me));
  	si_other.sin_family = AF_INET;
  	si_other.sin_port = htons(9876);
  	si_other.sin_addr.s_addr = inet_addr("192.168.137.128");


  	memset(&si_me, '\0', sizeof(si_me));
  	si_me.sin_family = AF_INET;
  	si_me.sin_port = htons(port);
  	si_me.sin_addr.s_addr = inet_addr("192.168.137.129");

  	bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me));
  	addr_size = sizeof(si_other);

  	if(recvfrom(sockfd, buffer, 1024, 0,(struct sockaddr*)& si_other, &addr_size) < 0);{
  		perror("Error udp received\n");
  	}
  	printf("test\n");
  	printf("[+]Data Received: %s\n", buffer);
  	sendto(sockfd, buffer, 1024, 0,(struct sockaddr*)& si_other,addr_size);
  	printf("[+]Data Send: %s", buffer);
  	//for(int t=0; t<6000; t++)
   //  {
   //  	if(recvfrom(sockfd, buffer, 1024, 0,(struct sockaddr*)& si_other, &addr_size)< 0){
   //  		printf("check\n");
   //  	}
   //  }
   //  printf("data sent\n");
}
void write_file(int sockfd)
{
	int n;
	FILE *fp;
	char *filename ="test2.txt";
	char buffer[MAXDATASIZE];

	fp= fopen(filename,"w");
	if(fp==NULL){
		perror("error with create file");
		exit(1);
	}
	while (1){
		n= recv(sockfd,buffer,MAXDATASIZE,0);
		if(n <= 0){
			break;
			return;
		}
		printf("%s\n", buffer);
		fprintf(fp, "%s", buffer);
		printf("File created.\n");
		udp_server();
		bzero(buffer, MAXDATASIZE);
	}

	//printf("File created.\n");
	return;
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

while(1) { // main accept() loop
	sin_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if (new_fd == -1) {
		perror("accept");
		continue;
	}

	inet_ntop(their_addr.ss_family,
		get_in_addr((struct sockaddr *)&their_addr),
		s, sizeof s);
	printf("server: got connection from %s\n", s);

if (!fork()) { // this is the child process
close(sockfd); // child doesn't need the listener
if (send(new_fd, "Hello, world!", 13, 0) == -1)
	perror("send");
//printf(" data received: %s\n", buffer);
close(new_fd);
exit(0);
}

// write_file(new_fd);
printf("File created.\n");

udp_server();
// hints.ai_socktype = SOCK_DGRAM;
//sockfd = socket(p->ai_family, SOCK_DGRAM,p->ai_protocol);
// new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

//close(sockfd);
close(new_fd); // parent doesn't need this
}


return 0;
}