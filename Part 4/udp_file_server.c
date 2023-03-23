/* udp_file_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

struct pdu {
	char type;
	char data[100];
};

/*------------------------------------------------------------------------
 * main - UDP server for file transfer
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	int	sock;			/* server socket		*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
    int     s;        /* socket descriptor and socket type    */
	int 	port=3000;
	FILE *fpt;
                                                                                

	switch(argc){
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Failed to create socket.\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);

	struct pdu request;
	struct pdu response;

	while (1) {
		
		// Initialize type
		response.type = 'F';

		// Recieves packet
		if (recvfrom(s, &request, sizeof(request.data)+1, 0, (struct sockaddr *)&fsin, &alen) < 0)
		{
			fprintf(stderr, "recvfrom error\n");
			response.type = 'E';
		}
		fprintf(stdout, "Packet received. Type: \"%c\", Value: \"%s\".\n", request.type, request.data);

		// Cleans file name for fopen
		char temp[100];
		snprintf(temp, sizeof(temp), "%s", request.data);

		// Opens file
		fpt = fopen(request.data, "r");	

		if (fpt == NULL){
			strcpy(response.data, "FILE NOT FOUND");
			response.type = 'E';
		}
			
		// Send error packet
		if (response.type == 'E') {
			(void) sendto(s, &response, strlen(response.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
			fprintf(stdout, "Packet sent. Type: \"%c\", Value: \"%s\".\n", response.type, response.data);
		} else {
			response.type = 'D';

			// Send all data
			while(fgets(response.data, 100, fpt) != NULL){
				(void) sendto(s, &response, strlen(response.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
				fprintf(stdout, "Packet sent. Type: \"%c\", Value: \"%s\".\n", response.type, response.data);
			}

			// Signal end
			response.type = 'F';
			(void) sendto(s, &response, strlen(response.data+1), 0, (struct sockaddr *)&fsin, sizeof(fsin));
			fprintf(stdout, "Packet sent. Type: \"%c\", Value: \"%s\".\n", response.type, response.data);

		}

	}
}
