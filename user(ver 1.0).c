#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE		1024

void Start_Chatting(int port);

int main(int argc, char* argv[]) {
  int user_sock, proxyServer_sock;
  struct sockaddr_in user_addr;
  const int loginServer_port = 40520;
  int myport;
  struct hostent *loginServer_host;
  char buf[BUFSIZE];

  if((user_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
	perror("user socket");
	exit(1);
  }


  if((loginServer_host = gethostbyname("localhost")) == 0) {
	fprintf(stderr, "%d: unknown host\n", loginServer_port);
	exit(1);
  }
  memset((void*)&user_addr, 0, sizeof(user_addr));
  user_addr.sin_family = AF_INET;
  memcpy((void*)&user_addr.sin_addr, loginServer_host->h_addr, loginServer_host->h_length);
  user_addr.sin_port = htons((u_short)loginServer_port);


  if(connect(user_sock, (struct sockaddr*)&user_addr, sizeof(user_addr)) < 0) {
	printf("connect\n");
	(void) close(user_sock);
	fprintf(stderr, "connect");
	exit(1);
  }

  int bytesread = read(user_sock, buf, BUFSIZE);
   buf[bytesread] = '\0';

   //printf("%s\n", buf);

  //strcpy(buf, "\0");
  //strcat(buf, "user user\n\0");
  //if(write(user_sock, buf, strlen(buf)) < 0) {
	//perror("write");
	//exit(1);
  //}
  myport = atoi(buf);
  printf("My Port Number : %d\n",myport);

  buf[0] = '\0';
  
  sleep(2);
  Start_Chatting(myport);


  strcat(buf, "bye");
  printf("%s\n", buf);
  if(write(user_sock, buf, strlen(buf)) < 0) {
	perror("write");
	exit(1);
  }
  (void) close(user_sock);
} //end main

void Start_Chatting(int portnum)
{

	int proxyServer_sock;
	int peertcpSocket = -1;
	int clnt_len;
	struct sockaddr_in proxyServer_addr;
	struct hostent *loginServer_host;

	char command[BUFSIZE];

	fd_set reads, temps;
	
	proxyServer_sock = socket(PF_INET, SOCK_STREAM, 0);
	proxyServer_addr.sin_family = AF_INET;
	proxyServer_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	proxyServer_addr.sin_port = htons(portnum);

	if (bind(proxyServer_sock, (struct sockaddr *)&proxyServer_addr, sizeof proxyServer_addr) < 0) {
		perror("bind");
		exit(1);
	}
	
	if (listen(proxyServer_sock, 5) < 0) {
		perror("listen");
		exit(1);
	}

	int fd_max;

	FD_ZERO(&reads);
	FD_SET(fileno(stdin), &reads);
	FD_SET(proxyServer_sock, &reads);
	fd_max = proxyServer_sock;

	while(1)
	{
		int port;
		struct sockaddr_in clnt_addr;

		temps = reads;
	
		select(fd_max+1, &temps, 0, 0, NULL);
		
		memset( command, '\0', sizeof(command) );

		if ( FD_ISSET( proxyServer_sock, &temps ) )
		{
				clnt_len = sizeof(clnt_addr);
				peertcpSocket = accept(proxyServer_sock, (struct sockaddr *)&clnt_addr, &clnt_len);
				FD_SET(peertcpSocket, &reads);
				if(fd_max<peertcpSocket)
					fd_max=peertcpSocket;
				printf("connection from host %s, port %d, socket %d \n", inet_ntoa(clnt_addr.sin_addr), port, peertcpSocket);
				
				FD_CLR(proxyServer_sock, &temps);

		}
		else if ( FD_ISSET( fileno(stdin), &temps ) )
		{
			fgets( command, sizeof (command), stdin );
			
			if ( strncmp(command, "@invite", 7) == 0 )
			{
				char Info[1024];
				sscanf( &command[8], "%d", &port);

				if(peertcpSocket > 0)
				{
					printf( "Close Socket %d\n", peertcpSocket );
					close(peertcpSocket);
					FD_CLR(peertcpSocket, &reads);
				}
					
				peertcpSocket = tcpConnect(port);
				FD_SET(peertcpSocket, &reads);

				if( fd_max < peertcpSocket )
					fd_max = peertcpSocket;
				
				
			}
			else if ( strncmp(command, "@quit", 5) == 0 )
			{	
				printf( "Connection Closed %d\n", peertcpSocket );
				close(peertcpSocket);
				peertcpSocket = -1;
				fd_max = proxyServer_sock;
				return;
			}
			else
			{
			char message[BUFSIZE];
			//memmove(message, userid, strlen(userid));
			memmove(message, command, BUFSIZE);

				if (write(peertcpSocket, message, strlen(message)) < 0) 
				{
					perror("write");
					exit(1);
				}
			}
		}
		else
		{
			clnt_len = read(peertcpSocket, command, BUFSIZE);
			if (clnt_len<0) {
				perror("read");
				exit(1);
				/* fall through */
			}
			if(clnt_len <=0) // 연결 종료 요청인 경우
			{ 
				FD_CLR(peertcpSocket, &reads);
				close(peertcpSocket);
				printf("Close Socket %d\n", peertcpSocket);
			}
			printf( "\n%d : %s\n",portnum, command );
		}
	}
}


int tcpConnect(int port)
{
	struct sockaddr_in tcpClient_addr;
	struct hostent* host;
	int sock;

	if ( ( sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0){
		perror("socket");
		/* fall through */
	}

	if((host = gethostbyname("localhost")) == 0) {
	fprintf(stderr, "%d: unknown host\n", port);
	exit(1);
	}
	memset(&tcpClient_addr, 0, sizeof(tcpClient_addr));
	tcpClient_addr.sin_family=AF_INET;
	memcpy((void *) &tcpClient_addr.sin_addr, host->h_addr, host->h_length);
	tcpClient_addr.sin_port=htons(port);

	if( connect( sock, (struct sockaddr *)&tcpClient_addr, sizeof(tcpClient_addr) ) < 0 ){
		perror("connect");
		/* fall through */
	}
	
	printf("connection is made successfully\n");

	return sock;
}
