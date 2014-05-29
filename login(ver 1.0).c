// 20123381 Hyesu Shin

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

#define MAX_USER	100
#define BUFSIZE		1024

int main(int argc, char* argv[]) {
  //var list for login server
  int loginServer_sock;
  struct sockaddr_in loginServer_addr;
  const int loginServer_port = 40512;
  fd_set reads, temp;
  int fd_max, nFound;

  //var list for user
  int addrlen, user_sock[MAX_USER]; 
  struct sockaddr_in user_addr[MAX_USER];
  int numUser = 0, portnum = 10000;
  char buf[BUFSIZE];

  int i;


  // make login server socket
  memset((void*)&loginServer_addr, 0, sizeof(loginServer_addr));
  loginServer_sock = socket(PF_INET, SOCK_STREAM, 0);
  loginServer_addr.sin_family = AF_INET;
  loginServer_addr.sin_addr.s_addr = INADDR_ANY;
  loginServer_addr.sin_port = htons((u_short)loginServer_port);


  // bind
  if(bind(loginServer_sock, (struct sockaddr*)&loginServer_addr, sizeof(loginServer_addr)) < 0) {
	perror("bind");
	exit(1);
  }


  // listen
  if(listen(loginServer_sock, SOMAXCONN) < 0) {
	perror("listen");
	exit(1);
  }


  FD_ZERO(&reads);
  FD_SET(loginServer_sock, &reads);
  
  while(1) {
	temp = reads;
	nFound = select(FD_SETSIZE, &temp, (fd_set*)0, (fd_set*)0, NULL);

	//if there is trial of connection
  	if(FD_ISSET(loginServer_sock, &temp)) {
	   addrlen = sizeof(user_addr);
   	   user_sock[numUser] = accept(loginServer_sock, (struct sockaddr*)&user_addr[numUser], &addrlen);
  	   if(user_sock[numUser] < 0) {
	 	 perror("accept");
	  	exit(1);
  	   }

  	   printf("Connection : Host IP %s, Port %d, socket %d\n",
		inet_ntoa(user_addr[numUser].sin_addr), ntohs(user_addr[numUser].sin_port), user_sock[numUser]);

	    strcpy(buf, "\0");
		sprintf(buf, "%d",portnum+numUser);
		if(write(user_sock[numUser], buf, strlen(buf)) < 0) {
		perror("write");
		exit(1);
		}

	   FD_SET(user_sock[numUser], &reads);
	   numUser++;
	}
	else {
	   for(i=0; i<numUser; i++) 
	   {
		//message from user
		if(FD_ISSET(user_sock[i], &temp)) {
		   //int bytesread = read(user_sock[i], buf, BUFSIZE);
		   //buf[bytesread] = '\0';
		   //
		   //printf("%s\n", buf);

		   if(!strcmp(buf, "bye")) {
			FD_CLR(user_sock[i], &reads);
			(void) close(user_sock[i]);
			numUser--;
		   }
		   buf[0] = '\0';
		}
	   }
	} //end if

  }//end whild(1)
}//end main
