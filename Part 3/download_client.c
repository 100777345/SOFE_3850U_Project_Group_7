/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>/* A simple echo client using TCP */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		100	/* buffer length */

int main(int argc, char **argv)
{
	int 	n, i, bytes_to_read;
	int 	sd, port;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];
	size_t value_received;
	FILE *fp;
	
	switch(argc){
	case 2:
		host = argv[1];
		port = SERVER_TCP_PORT;
		break;
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
		exit(1);
	}

	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}

	// Ask for the file
	printf("Enter the name of the file: \n");
	if((n = read(0, sbuf, BUFLEN)) != -1){	/* get user message */
	  sbuf[strcspn(sbuf, "\n")] = '\0'; // Add null character at end
	  write(sd, sbuf, n);		/* send it out */
	  value_received = read(sd, rbuf, BUFLEN);

	  // Check to see if error, if so print message	  
	  if(value_received > 0 && strcmp(rbuf, "ENOENT") == 0){
	  	fprintf(stderr, "The file was not found\n");
	  	exit(1);
	  }
	  
	  // Create a new file with the name provided by user
	  fp = fopen(sbuf, "w");
	  if(fp == NULL){
	  	perror("failed to create file");
	  	exit(1);
	  }


	 // If the file exists, write the first line to the file
	 if(fwrite(rbuf, 1, value_received, fp) != value_received){
	  	perror("fwrite");
	  	exit(1);
	  }
	
	  // Write the remaining lines to the file
	  while((value_received = read(sd, rbuf, BUFLEN)) >0){
	  	if(fwrite(rbuf, 1, value_received, fp) <0){
	  		perror("fwrite");
	  		exit(1);
	  	}
	  }

	fclose(fp);
	puts("File Successfully downloaded!");
	}

	close(sd);
	return(0);
}
