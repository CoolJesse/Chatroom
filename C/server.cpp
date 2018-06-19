
//c++ headers////////////
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <map>
//////////////////////////
//c headers//////////////
#include <math.h>
#include <stdio.h> //input and output
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>  //contains definitions of a number of data types used in system calls	
#include <sys/socket.h> //includes a number of definitions of structures needed for sockets
#include <netinet/in.h> //contains constants and  structures needed for internet domain addresses "sockaddr_in"

using namespace std;

static int MAXCLIENTS = 3; //the maximum number of clients allowed for this project
static int PORTNUMBER = 18305; //port number assigned for this project

void communicate(int, struct sockaddr_in, const std::map<string, string>&, std::map<string, int>&); /* function prototype newsockfd, cli_addr, userNameAndPassword, userSocketNumber*/
void denialOfService(int);
void error(const char *msg) //called when a system call fails, displays message about error to stderr and aborts program
{
    perror(msg);
    exit(1);
}

int main() 
{
/**************************************Reads contents from text file and creates map element for each member*****************************************/
/****************************************************************************************************************************************************/
	char next;
	string name, userName;
	ifstream inStream;
	ofstream outStream;
	
	/*UserName first element, name second element*/
	map<string, string> userNameAndPassword;
	/*hold the persons' name, and the socket assigned to them*/
	map<string, int> userSocketNumber;

/*****************************************************Opens and reads out user account info file*****************************************************/
	inStream.open("userAccountInformation.txt");
	
	if(inStream.fail())
	{	cout << "Could not read user account information file!" << endl;
		exit(1);
	}

/************************************************************Gets first character from file**********************************************************/
	inStream.get(next);

	while(!inStream.eof())
	{	
		/*clears both variable each time a new line is reached in the text file*/
		name.clear(); //string.clear() function included in string library
		userName.clear();
		
/****************************************************************Gets the name of the user***********************************************************/

		/*the next character is not a comma "44", or a new line "10", carrage return "13", or space "32", and not the end of the file*/
		while(next != 44 && next != 10 && next != 13 && next != 32 && !inStream.eof()) 
		{	name = name+next;
			inStream.get(next);
		}

/********************************************Gets blank space, format is: "name, userName, name, userName etc."**************************************/
/****************************************************************************************************************************************************/
		if(inStream.eof())
			break;
		inStream.get(next);//skips to next character in input since last one was not a valid character
		inStream.get(next);//have to skip to spaces to get to next valid character
		/*If we come to the end of the file break out of loop*/
		if(inStream.eof())
			break;
/****************************************************************************************************************************************************/	
/*****************************************************************Gets the username******************************************************************/

		/*the next character is not a comma "44", or a new line "10", carrage return "13", or space "32", and not the end of the file*/
		while(next != 44 && next != 10 && next != 13 && next != 32 && !inStream.eof()) 
		{	userName = userName+next;
			inStream.get(next);
		}

/****************************************************Creates a unique map element for each userName**************************************************/
		userNameAndPassword[userName] = name;
		
		/*If we come to the end of the file break out of loop*/		
		if(inStream.eof())
			break;

		else
			inStream.get(next);
	}
/**************************************************Displays the database of members and their passwords**********************************************/
	cout <<"Contents of database:" << endl;

	for(map<string, string>::iterator it = userNameAndPassword.begin(); it != userNameAndPassword.end(); ++it)
	{	cout << "Name: " << it->second  << ", UserName: " << it->first << endl;
	}
	cout << endl;
	inStream.close();
	outStream.close();
	
/**************************************************************End of file input code****************************************************************/
/****************************************************************************************************************************************************/

/**************************************************************The code for the server***************************************************************/
/****************************************************************************************************************************************************/

    int sockfd, newsockfd; //file descriptors store the values returned by the socket system call and the accept system call
	int portno; //stores the port number on which the server accepts connections
    socklen_t clilen;//stores the size of the address of the client. Needed for the accept system call
	int pid; //for child process
	int numberOfClients(0); //the number of clients connected to the serve

	/*sockaddr_in is a structure containing an internet address. Defined in netinet/in.h. serv_addr contains address of server*/
	/*declares two sockaddr_in structs; serv_addr and cli_addr*/
    struct sockaddr_in serv_addr, cli_addr;

		
	//int clilen;
    //int n; //return value for the read() and write() calls. Contains the number of characters read or written 
    //char buffer[256]; //server reads characters from socket connection into this buffer
	
	/*if user fails to provide a port number for the server to accept connections as an argument
	this function will be unnecissary since we are providing the port number "static int PORTNUMBER = 18305";
    if (argc < 2)
		{	fprintf(stderr,"ERROR, no port provided\n");
			exit(1);
		}
	*/
	
/*******************************************************************Create Socket********************************************************************/
	
	/*Socket system call: sockfd() creates a new socket. Takes three arguments; the first being the address domain, 
	either AF_UNIX or AF_INET. The second arguement being the type of socket, either SOCK_STREAM or SOCK_DGRAM for either stream or datagram sockets, 
	respectively. The third being the protocal, almost always zero. OS will choose most appropriate protocal TCP for stream and UDP for 
	datagram Socket system call returns an entry into the file descriptor table. This value is used for all subsequent references to this socket*/
	 
    sockfd = socket(AF_INET, SOCK_STREAM,0); //socket system call returns int value to reference socket 	
	if (sockfd < 0) //If socket call fails returns -1
        error("ERROR opening socket");
		
	/*sets all values in a buffer to zero. Initializes struct sockaddr_in serv_addr to all zeros. Takes two arguments, the first being a pointer to 
	the buffer and the second is the size fo the buffer.*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
	
	//We do not need the function below since we are providing the port number
	 /*atoi() function converts string of digits to an integer. Takes port number on which server listens for connection passed as argument,
	 and sets portno equal to this integer value*/
    //portno = atoi(argv[1]); 
	
	 portno = PORTNUMBER; //we defined the port number as a const int variable at the beginning of out code on line 21

/*******************************************************Modifies struct sockaddr_in serv_addr********************************************************/

	/*serv_addr has four member variables, the first of which short sin_family should always be set to the symbolic constant AF_INET*/
    serv_addr.sin_family = AF_INET; 
								   //
	/*third member variable of sockaddr_in is struct of type struct in_addr which contains only a single filed unsigned long s_addr this field 
	contains the IP address of the host. For server this will always be IP address of machine server is running on. Symbolic constant to obtain*/ 
	/*IP address of machine server running on INADDR_ANY*/						    
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
											 											      
	/*htons() function converts port number from host byte order to to network byte order and passes value to second field of serv_addr*/									       
	serv_addr.sin_port = htons(portno); 

/********************************************************************Bind Socket*********************************************************************/

	/*bind() system call binds a socket to an address, in this case the address of the host and the port number on which the server runs*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");                                     
	
/***********************************************************************Listen***********************************************************************/
	
	/*Allows process to listen on socket for connection. The second argument is the size of the backlog queue*/
    listen(sockfd,5);
	
	/*size of future client address*/
    clilen = sizeof(cli_addr); 

/***************************************************Loop allows server to handle multiple connections************************************************/
/****************************************************************************************************************************************************/

		while(1)
		{	/*accept() system call causes the process to block until a client connects to the server It returns a new file descriptor and all 
			communication on this connection should be done using the new file descriptor. The second arguement is a reference pointer to the
			address of the client on the other end of the connection*/
			
/*******************************************************************newsocketfd = accept()**********************************************************/

			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) 				                  
				error("ERROR on accept");		                 
						                                   
/*********************************************************************Create new process************************************************************/

			numberOfClients++;
			pid = fork();

			if(pid < 0)
			{	numberOfClients--;
				error("ERROR on fork");
			}
	
/******************************************************************If child process AKA*************************************************************/

			if(pid == 0)
			{	//close(sockfd);
				cout << "number of clients: " << numberOfClients << endl;
				
/******************************************When this function returns the server and client have finished communicating*****************************/
			
				if(numberOfClients <= MAXCLIENTS)
				{
					//map<string, int> userSocketNumber
					communicate(newsockfd, cli_addr, userNameAndPassword, userSocketNumber);
					numberOfClients--;
				}	   
	
				else
					denialOfService(newsockfd);

				exit(0);
			}
/******************************************************************Parent process AKA server*********************************************************/
			else 
				close(newsockfd);
		}
/*********************************************************************End of while() loop************************************************************/
/****************************************************************************************************************************************************/

     close(sockfd); //close server socket 

	return 0;
}
/************************************************************Communication function()****************************************************************/
/****************************************************************************************************************************************************/
/****************takes as arguments the file descriptor returned by accept() function call, the map element userNameAndPassword, and the*************/

void communicate (int sock, struct sockaddr_in clientAddress, const std::map<string, string> &userAccount, std::map<string, int> &userSocket)
{
   string name, userName, buffContents;
   int n; //return value for the read() and write() calls. Contains the number of characters read or written
   char buffer[256];//server reads characters from socket connection into this buffer
   bool login(false);

/*******************************************************Initial welcome message to client************************************************************/
	bzero(buffer,256);
	n = write(sock,"You are now connected to the server. To login please type: login name username, replacing name with your name and username with your username", 141);
	if (n < 0) error("ERROR writing to socket");

/*************************************************************Just for testing***********************************************************************/
	cout << " UserNames:" << endl;
	for(map<string, string>::const_iterator it = userAccount.begin(); it != userAccount.end(); ++it)
	{	cout << " " << it->second << " " << it->first << endl;
	}
	cout << endl;

/***************************************************************Login loop***************************************************************************/
	/*Reads from client*/
	do
	{/*reads login attempt*/
		bzero(buffer,256);
		name.clear();
		userName.clear();
		n = read(sock,buffer,255);
	
		if (n < 0) error("ERROR reading from socket"); // Error check 
		
	/*****************************************************Checks for "login" command*************************************************************/
		/*If client enters "login" command*/
		if(buffer[0]=='l'&&buffer[1]=='o'&&buffer[2]=='g'&&buffer[3]=='i'&&buffer[4]=='n')
		{	/*reads buffer into string and for name for testing*/
			int i=6;
			for( ;buffer[i] != 44 && buffer[i] != 10 && buffer[i] != 13 && buffer[i] != 32; ++i)
			{	name += buffer[i]; //gets name
			}
			++i;
			for( ;buffer[i] != 44 && buffer[i] != 10 && buffer[i] != 13 && buffer[i] != 32; ++i)
			{	userName += buffer[i]; //gets password/username
			}

			cout << "name:" << name << "/userName:" << userName << endl;
			
			/*checks to see if userName exists in map container*/
			if(userAccount.find(userName) != userAccount.end())
			{	login = true;
				/*-----------------------------------------*/
				userSocket[name] = sock; //	map<string, int> userSocketNumber
			   /*----------------------------------------*/
			}
				
			else
			{	bzero(buffer,256);
				n = write(sock,"Incorrect name or password. Please try again.", 45);
				if (n < 0) error("ERROR writing to socket");
			}
		}
		/*If client does not enter "login" command*/
		else
		{	bzero(buffer,256);
			n = write(sock,"Incorrect command. Please login.", 32);
			if (n < 0) error("ERROR writing to socket");
		}
	}while(login == false); //continue loop until client successfully logins in
/**************************************************************End of login loop*********************************************************************/

/**************************************************************Login confirmation********************************************************************/
	bzero(buffer,256);
	n = write(sock,"You are now logged in. Please issue a command: logout/send all/send name/who", 76);

/*****************************************************************For Testing************************************************************************/
	cout << "Names and portnumbers" << endl;
	for(map<string, int>::iterator it = userSocket.begin(); it != userSocket.end(); ++it)
	{	cout << "Name:"<<it->first<< "/socket:" <<it->second<< endl;
		cout << "client address " << (struct sockaddr *) &clientAddress << endl;
	}
	cout << endl;
	if (n < 0) error("ERROR writing to socket");
	
/****************************************infinite loop allows server to continue communicating with client*******************************************/
/****************************************************************************************************************************************************/
	
	while(1)
    {  	/*Read call uses sock arguement passed to the function, which is the same value returned by the earlier 
		accept() function. Will block until there is something to read in the socket, and will read either the total number of characters 
		or 255, whichever is less.*/
		
		/*Reads from client*/
		bzero(buffer,256);
        n = read(sock,buffer,255);
		if (n < 0) error("ERROR reading from socket");

/**************************************************if server receives "logout" as a command**********************************************************/

		if(buffer[0]=='l'&&buffer[1]=='o'&&buffer[2]=='g'&&buffer[3]=='o'&&buffer[4]=='u'&&buffer[5]=='t')
		{	bzero(buffer,256);
			n = write(sock,"Goodbye",7); 
			if (n < 0) error("ERROR writing to socket");
			
			break;
		}		
/****************************************************if server recieves "send" as a command*********************************************************/

		else if(buffer[0]=='s'&&buffer[1]=='e'&&buffer[2]=='n'&&buffer[3]=='d'&&buffer[4]==' ')
		{
			
	/**********************************************************command is send all*************************************************************/
			if(buffer[5]=='a'&&buffer[6]=='l'&&buffer[7]=='l'&&buffer[8]==' ')
			{
				printf("Here is the message: %s\n",buffer);
				
				for(map<string, int>::iterator it = userSocket.begin(); it != userSocket.end(); ++it)
				{	bzero(buffer,256);
					n = write(it->second,buffer,strlen(buffer)); 
					if (n < 0) error("ERROR writing to socket");
				}
			}
			
	/*********************************************************command is send Name************************************************************/
			else
			{	name.clear();
				for(int i=5;buffer[i] != 44 && buffer[i] != 10 && buffer[i] != 13 && buffer[i] != 32; ++i)
					name += buffer[i];
				
				map<string, int>::iterator it = userSocket.find(name);
				if(it != userSocket.end())
					n = write(it->second,buffer,strlen(buffer));
					if (n < 0) error("ERROR writing to socket");
			}
		}

/*******************************************************if server recieve "who" command*************************************************************/
		else if(buffer[0]=='w'&&buffer[1]=='h'&&buffer[2]=='o')
		{	/*bzero(buffer,256);
			n = write(sock,"Goodbye",7); 
			if (n < 0) error("ERROR writing to socket");*/
			
			printf("Here is the message: %s\n",buffer);
			bzero(buffer,256);
			
			buffContents.clear();
			for(map<string, int>::iterator it = userSocket.begin(); it != userSocket.end(); ++it)
			{	buffContents+= it->first;
				buffContents+= " "; //formating purposes
			}
			
			char *temp = (char*)buffContents.c_str();
			n = write(sock,temp,255); 
			if (n < 0) error("ERROR writing to socket"); 
		}	
		
/************************************************************if an invalid command is intered********************************************************/
		else
		{   bzero(buffer,256);
		    n = write(sock,"Please enter a valid command.",29); 
            if (n < 0) error("ERROR writing to socket");
		}

   }
/**************************************************************End of infinite while() loop**********************************************************/
/****************************************************************************************************************************************************/
}
/*************************************************************End of communication function**********************************************************/
/****************************************************************************************************************************************************/
/**********************************************************Body of denialOfService(int) function*****************************************************/
void denialOfService (int sock)
{
   int n; //return value for the read() and write() calls. Contains the number of characters read or written
   char buffer[256];//server reads characters from socket connection into this buffer;

	/*Initial welcome message to client*/
	bzero(buffer,256);
	n = write(sock,"Sorry, there are too many people logged in, please try again later.", 67);
	if (n < 0) error("ERROR writing to socket");
}
/**************************************************************End of denialOfService function*******************************************************/

/*defined in the header file netinet/in.h
struct sockaddr_in
{
  short   sin_family; // must be AF_INET
  u_short sin_port;
  struct  in_addr sin_addr; //contains only one field, an unsigned long called s_addr
  char    sin_zero[8]; // Not used, must be zero 
};
*/
