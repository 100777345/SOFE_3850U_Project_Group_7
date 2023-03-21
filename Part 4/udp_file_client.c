/* time_client.c - main */

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
                                                                                
#include <netdb.h>

#define	BUFSIZE 64

#define	MSG		"Any Message \n"

struct pdu {
	char type;
	char data[100];
};

/*------------------------------------------------------------------------
 * main - UDP client for TIME service that prints the resulting time
 *------------------------------------------------------------------------
 */
int
main(int argc, char **argv)
{
	char	*host = "localhost";
	int	port = 3000;
	char	contents[100];		/* 32-bit integer to hold time	*/ 
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, n, type;	/* socket descriptor and socket type	*/
	FILE *fpt;

	switch (argc) {
	case 3:
		host = argv[1];
		port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: %s [host] [port]\n", argv[0]);
		exit(1);
	}

	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;                                                                
        sin.sin_port = htons(port);
                                                                                        
    /* Map host name to IP address, allowing for dotted decimal */
        if ( phe = gethostbyname(host) ){
                memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
        }
        else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
                                                                                
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "Can't create socket \n");
	
                                                                                
    /* Connect the socket */
        if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "Connection to %s %s \n", host, "failed.");


	struct pdu rpdu;
	struct pdu spdu;
	int option;

while (1){

	// Main menu
	fprintf(stdout, "Enter 1 to download a file, or 2 to quit.\n");
	scanf("%d", &option);
	if (option == 2)
		exit(0);

	// Take file name input
	fprintf(stdout, "Enter the name of the file to download.\n");
	spdu.type = 'C';
	scanf("%s", spdu.data);

	// Write to server
	(void) write(s, &spdu, strlen(spdu.data)+1);

	fprintf(stdout, "Packet sent. Type: \"%c\", Value: \"%s\".\n", spdu.type, spdu.data);

	// Create file
	fpt = fopen(spdu.data, "w");

	// Recieve packets from server
	do {
	n = read(s, &rpdu, sizeof(rpdu.data)+1);

	if (n < 0){
		fprintf(stderr, "Failed to receive PDU from server.\n");
		break;
	}
	
	// Write to file if type is data
	if (rpdu.type == 'D')
		fputs(rpdu.data, fpt);

	// Print received packet
	fprintf(stdout, "Packet received. Type: \"%c\", Value: \"%s\".\n", rpdu.type, rpdu.data);

	// Clear rpdu
	memset(rpdu.data, 0, sizeof(rpdu.data));

	} while (rpdu.type != 'E' && rpdu.type != 'F');
	
	// Display result
	if (rpdu.type == 'E'){
		fprintf(stderr, "Error during read.\n");
		
		// Delete created blank file if file was not found on server
		fseek(fpt,0,SEEK_END);
		int size = ftell(fpt);
		if (size == 0)
			remove(spdu.data);

	} else if (rpdu.type == 'F')
		fprintf(stdout, "File successfully downloaded.\n");

	fclose(fpt);

}

	exit(0);
}
