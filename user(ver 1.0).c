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

int main(int argc, char* argv[]) {
  int user_sock;
  struct sockaddr_in user_addr;
  const int loginServer_port = 40512;
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

  strcpy(buf, "\0");
  strcat(buf, "user user\n\0");
  if(write(user_sock, buf, strlen(buf)) < 0) {
	perror("write");
	exit(1);
  }
  buf[0] = '\0';

  sleep(3);

  strcat(buf, "bye");
  printf("%s\n", buf);
  if(write(user_sock, buf, strlen(buf)) < 0) {
	perror("write");
	exit(1);
  }
  (void) close(user_sock);
} //end main
