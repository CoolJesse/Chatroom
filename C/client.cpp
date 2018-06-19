
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> //contains definitions for a number of data types used in the system calls
#include <sys/socket.h> //includes number of definitions of structures needed for sockets
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses
#include <netdb.h> //defines the structure hostent, which is used below

using namespace std;

static int PORTNUMBER = 18305;

void error(const char *msg) //called when a system call fails, displays message on stderr and then aborts program
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) //allows for command line arguements
{
	/*Declare all variables*/
    int sockfd; //file descriptor stores the value returned by the socket system call and the accept system call 
	int portno; //stores the port number on which the server accepts connections
	int n; //return value for the read() and write() calls. Contains the number of characters read or written
    struct sockaddr_in serv_addr; //will contain the address of the server to which we wish to connect. Of type struct sockaddr_in
	
    struct hostent *server; //pointer to a structure of type hostent. This struct is defined in the header file netdb.h 

    char buffer[256]; //buffer to read message from server
	
	/*Checks to make sure enough arguements were passed in the command line*/
    //if (argc < 3) //command line arguement to run client.exe must include name of the host on which the server is running and the port number it is listening on
	
	if (argc < 2)
	{ 	fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
    }
	
	/*Assigns the port number*/
    portno = PORTNUMBER;
	//portno = atoi(argv[2]);//atoi() function converts string of digits to an integer. Takes port number on which server listens for connection passed as argument
						  //and sets portno equal to this integer value

	/*socket system call creates a new socket. Takes three arguments; the first being the address domain. either AF_UNIX or AF_INET
	the second being the type of socket, either SOCK_STREAM or SOCK_DGRAM for either stream or datagram sockets respectively
	the third being the protocal, almost always zero. OS will choose most appropriate protocal, TCP for stream and UDP for datagram
	Socket system call returns an entry into the file descriptor table. This value is used for all subsequent references to this socket*/
	/***sock()**************************************************************************************************************************************/
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 											 
	if (sockfd < 0)                         
        error("ERROR opening socket");

	/*argv[1] contains name of a host. gethostbyname() takes such a name as an argument and returns a pointer to a hostent containg info about that 
	host. If this structure is NULL the system could not locate a host by that name*/
	
	/***server = gethostbyname()********************************************************************************************************************/
    server = gethostbyname(argv[1]); 
    if (server == NULL)
	{	fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	
	
	/*sets all values in a buffer to zero. Initializes serv_addr to all zeros*/
    bzero((char *) &serv_addr, sizeof(serv_addr)); 

/***Modifies sockaddr_in serv_addr******************************************************************************************************************/	
	 
	/*serv_addr is a structure of type struct sockaddr_in. This structure has four fields,the first of which short sin_family. should always be set
	to the symbolic constant AF_INET*/
    serv_addr.sin_family = AF_INET;
								   
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); //
	
	/*htons() function converts port number from host byte order to to network byte order and passes value to second field of serv_addr*/
    serv_addr.sin_port = htons(portno); //
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/	
	/*connect() function is called by the client to establish a conection to the server It takes three arguments, the socket file descriptor, 
	the address of the host it wants to connect including the port number, and the size of this address*/
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");											  
																			 
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
/*Now our client out client should be connected to the server*/	
	/*Initial welcome from server*/
	bzero(buffer,256); //Initilizes buffer using bzero() function, all values = 0
	n = read(sockfd,buffer,255); //reads from server, writes to buffer
	printf("%s\n",buffer);
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/

	cout << "connected to server on port number: " << n << endl;
	/*Enter into infinite loop to keep communicating with server*/
	while(1)
	{
		/*Formats buffer and reads message from stdin"keyboard," stores it in buffer, reads a maximum of 255 characters*/
		/*takes input*/
		bzero(buffer,256);
		fgets(buffer,255,stdin); 
/*-------------------------------------------------------------------------------------------------------------------------------------------------*/       
		/*Passes message to server. Last arguement of write() is the size of the message*/
        n = write(sockfd,buffer,strlen(buffer));  
		if (n < 0) 
			error("ERROR writing to socket");
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
		/*Formats buffer and reads from server, writes to buffer, reads a maximum of 255 characters*/
		bzero(buffer,256);
		n = read(sockfd,buffer,255);
		if (n < 0) 
			error("ERROR reading from socket");
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
		/*test for Goodbye message indicating logout*/
		if(buffer[0]=='G'&&buffer[1]=='o'&&buffer[2]=='o'&&buffer[3]=='d'&&buffer[4]=='b'&&buffer[5]=='y'&&buffer[6]=='e')
		{	cout << "you are logged out " << endl;
		    printf("%s\n",buffer);
			break;
		}
/*--------------------------------------------------------------------------------------------------------------------------------------------------*/
		/*outputs contents of buffer*/
		printf("%s\n",buffer); //
	}
	
    close(sockfd); //close socket
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* defined in the header file netinet/in.h
struct sockaddr_in
{
  short   sin_family; // must be AF_INET
  u_short sin_port;
  struct  in_addr sin_addr; //contains only one field, an unsigned long called s_addr
  char    sin_zero[8]; // Not used, must be zero 
};
*/
/* defined in the heade file netdb.h, defines a host computer on the Internet
struct  hostent
{
  char    *h_name;        // official name of host 
  char    **h_aliases;    // alias list 
  int     h_addrtype;     // host address type
  int     h_length;       // length of address 
  char    **h_addr_list;  // list of addresses from name server 
  #define h_addr  h_addr_list[0]  // address, for backward compatiblity 
};
*/
