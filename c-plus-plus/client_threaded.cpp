
/** C++ libraries *********************************************************************************************************/
#include <iostream>
#include <string>
#include <cstring> //bcopy() and bclear()
#include <thread>
#include <mutex>
#include <queue>
/**************************************************************************************************************************/

/** User defined header "helper.h" with namespace "tools::" for helper functions ******************************************/
#include "user_info.h"
#include "helper.h"
/**************************************************************************************************************************/

/** C libraries ***********************************************************************************************************/
#include <stdlib.h>
#include <unistd.h> //read() write()
#include <stdio.h>
#include <string.h> //strlen()
/**************************************************************************************************************************/

/** Libraries for creating and using sockets ******************************************************************************/
#include <sys/types.h> //contains definitions for a number of data types used in the system calls
#include <sys/socket.h> //includes number of definitions of structures needed for sockets
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses
#include <netdb.h> //defines the structure hostent, which is used below
#include <arpa/inet.h>
/**************************************************************************************************************************/

using namespace std;

const size_t BUFFSIZE = 4028;

const int SERVER_PORT = 50000;
const char* SERVER = "127.0.0.1";
string user_name;

/** Variable for to tell incoming thread to exit *************************************************************************/
bool exit_program = false;
bool flag = false;
mutex mtx, cout_mtx;

/** Message queue for accepting incoming messages to print to cout once mutex becomes unlocked **/
queue<string> message_queue;

/** Function declarations ************************************************************************************************/

bool login_function(const int& socket);
void outgoing_communication_handler(const int& socket);
void incoming_communication_handler(const int& socket);

void string_to_char_array(const string& message, char buffer[], const size_t& size);

int main()
{
/** Declare all variables ************************************************************************************************/

	int sockfd; /* file descriptor stores the value returned by the socket system call */ 
	//int n; /* return value for the read() and write() calls. Contains the number of characters read or written */
    	//char socket_buffer[BUFFSIZE]; /* buffer to read message from server */

    	struct sockaddr_in serv_addr; /* will contain the address of the server to which we wish to connect. Of type struct sockaddr_in */

/** bzero() sets all values in a buffer to zero. Initializes serv_addr to all zeros **************************************/

    	bzero( (char *) &serv_addr, sizeof(serv_addr) );

/** Modifies sockaddr_in serv_addr ***************************************************************************************/	

	/*serv_addr is a structure of type struct sockaddr_in. This structure has four fields,the first of which short sin_family. should 		always be set to the symbolic constant AF_INET*/

	/** htons() converts port number from host byte order to network byte order and passes value to second field of serv_addr **/

    	serv_addr.sin_family = AF_INET;							   
    	serv_addr.sin_port = htons(SERVER_PORT); //

    	//bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length); //

/** Convert char string to network address using inet_pton() *************************************************************/

	if( inet_pton(AF_INET, SERVER, &serv_addr.sin_addr) <= 0) /* converts character string to network address structure in the AF_INET */ 			cerr << "ERROR, Invalid Address\n";	      /* family, then copies the network address structure into serv_addr.sin_addr */ 
		
/** Create socket *********************************************************************************************************/
	/*socket system call creates a new socket. Takes three arguments; the first being the address domain. either AF_UNIX or AF_INET
	the second being the type of socket, either SOCK_STREAM or SOCK_DGRAM for either stream or datagram sockets respectively
	the third being the protocal, almost always zero. OS will choose most appropriate protocal, TCP for stream and UDP for datagram
	Socket system call returns an entry into the file descriptor table. This value is used for all subsequent references to this socket*/
 
	sockfd = socket(AF_INET, SOCK_STREAM,0);

	if(sockfd < 0)                    
        	cerr << "ERROR opening socket\n";

/** Connect to server ****************************************************************************************************/
	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 )
		cerr << "ERROR connecting to server\n";

/** Main body of program **************************************************************************************************/
	bool login_successful = login_function(sockfd);

	if(login_successful)
	{
		//int size_of_buffer = sizeof(socket_buffer);

		auto outgoing_thread = thread( outgoing_communication_handler, ref(sockfd) );
		auto incoming_thread = thread( incoming_communication_handler, ref(sockfd) );

		outgoing_thread.join();
		incoming_thread.join();
	}

	close(sockfd);
	return 0;
/**************************************************************************************************************************/
}
/** Login Function *******************************************************************************************************/
bool login_function(const int& socket)
{
	char buffer[BUFFSIZE]= {0};
	string incoming_message, outgoing_message;
	bool login_successful = false;

    	bzero(buffer, BUFFSIZE);

	if( read(socket, buffer, BUFFSIZE) < 0) /* Read from server */
		cerr << "ERROR reading from server\n";

	incoming_message = buffer;
	cout << "Incoming message: " << incoming_message << endl;

	for(int i=1; !login_successful; i++)
	{
		/* Send message to server ************************************/
		bzero(buffer, BUFFSIZE );

		cout << "username->";
		cin >> user_name;

		string password;
		cout << "password->";
		cin >> password;

		/* flush the input buffer for getline to use later */
		cin.ignore(numeric_limits<streamsize>::max(),'\n'); 
		
		outgoing_message = user_name + ' ' + password + '\n';

		tools::string_to_char_array(outgoing_message, buffer, BUFFSIZE);
		printf("outgoing buffer contents: %s\n", buffer);

		cout <<"send to server\n";
		if( send(socket, buffer, BUFFSIZE, 0) < 0)
			cerr << "ERROR sending to server\n";
		/*************************************************************/

		/* Receive message from server *******************************/
		bzero(buffer, BUFFSIZE );

		if( read(socket, buffer, BUFFSIZE) < 0) /* Read from server */
			cerr << "ERROR reading from server\n";

		incoming_message = buffer;
		cout << "Incoming message: " << incoming_message<< endl;
		/*************************************************************/


		if(incoming_message.compare("login") == 0)
		{
			login_successful = true;
			cout << "You have successfully logged in." << endl;
			break;
		}
		
		else if(incoming_message.compare("nope") == 0)
			cout << "Login attempted failed, please try again." << endl;

		else if(incoming_message.compare("faild") == 0)
		{
			cout << "Sorry, you failed to login successfully after " << i << " attempts."<< endl;
			break;
		}

		else
			; /* Invalid command so do nothing */
	}

	return login_successful;

}	
/** Outgoing Communication Function for handling all outgoing messages  *********************************************/
void outgoing_communication_handler(const int& socket)
{
	char buffer[BUFFSIZE] = {0};
	string command, outgoing_message;

	cout << "Please enter a command, either \"who\" to receive a list of every user currently active, \"send all\" to send a message to 		every active user, \"send username\" to send a message to the specified user, or \"logout\" to logout of the chatroom.\n";

	/** Infite loop executes until user enters command to logout **************************************************/
	/** Only accepted commands are: who, send all, send "username", and logout ************************************/
	while(1)
	{
		if(flag == false) //global flag variable controls switching between output of 2 threads
		{
			cout_mtx.lock();
			cout << "-> "; /* prompt for user to enter command */

			getline(cin, command); /* takes user input as command */
			cout_mtx.unlock();
		
			if(command.compare(0,3,"who") == 0)
			{
				flag = true;

				bzero(buffer, BUFFSIZE);
				outgoing_message = (user_name + " who");
				tools::string_to_char_array(outgoing_message, buffer, BUFFSIZE);

				printf("Outgoing message: %s\n", buffer );

				if( send(socket, buffer, BUFFSIZE, 0) < 0)
					cerr << "ERROR sending to server\n";

			}

			else if(command.compare(0,8,"send all") == 0) /* command is "send all" */
			{
				flag = true;

				command[4] = '_'; /* Get rid of blank space */

				bzero(buffer, BUFFSIZE); /* clear buffer of any contents */
				outgoing_message = (user_name + ' ' + command);
				tools::string_to_char_array(outgoing_message, buffer, BUFFSIZE);/* enters command into buffer for sending */

				if( send(socket, buffer, BUFFSIZE, 0) < 0)
					cerr << "ERROR sending to server\n";

			}

			else if(command.compare(0,4,"send") == 0) /* command is "send username" */
			{
				flag = true;

				bzero(buffer, BUFFSIZE);
				outgoing_message = (user_name + ' ' + command);
				tools::string_to_char_array(outgoing_message, buffer, BUFFSIZE); /*enters command into buffer for sending */

				if( send(socket, buffer, BUFFSIZE, 0) < 0)
					cerr << "ERROR sending to server\n";

			}

			else if(command.compare(0,6,"logout") == 0) /* command is to logout */
			{
				flag = true;

				bzero(buffer, BUFFSIZE);
				outgoing_message = (user_name + " logout");
				tools::string_to_char_array(outgoing_message, buffer, BUFFSIZE);

				if( send(socket, buffer, BUFFSIZE, 0) < 0)
					cerr << "ERROR sending logout message to server\n";

				break;
			}

			else
				cout << "Sorry, you have entered an invalid command, please try again." << endl;

		}
	
	}
	/** End of while() loop  ************************************************************************************/
	mtx.lock();
	exit_program = true;
	mtx.unlock();

	cout << "Goodbye!\n";
}
/** Incoming Communication Function for handling all incoming messages  *********************************************/
void incoming_communication_handler(const int& socket)
{
	char buffer[BUFFSIZE] = {0};
	string full_message, sender_name, command, incoming_message;

	while(!exit_program)
	{
		bzero(buffer, BUFFSIZE);

		if( read(socket, buffer, BUFFSIZE) < 0) /* Read from server */
			cerr << "ERROR reading from server\n";

		incoming_message = buffer;
		message_queue.push(incoming_message);

		if(cout_mtx.try_lock())
		{
			while(!message_queue.empty())
			{
				full_message = message_queue.front();
				message_queue.pop();
				
				sender_name.clear();
				command.clear();
				incoming_message.clear();

				/* Parse sender name, command, and message out of full_message */
				size_t i = 0;
				for(; i<full_message.length() && full_message[i] != ' ';i++ )
					sender_name += full_message[i];

				++i; /* skip blank space*/

				for(; i<full_message.length() && full_message[i] != ' '; i++)
					command += full_message[i];

				++i; /* skip blank space*/

				for(; i<full_message.length(); i++)
					incoming_message += full_message[i];
				
				/**************************************************************/

				if(command.compare("broad") == 0)
					cout << sender_name << ": " << incoming_message << endl;

				else if(command.compare("recv") == 0)
					cout << sender_name << ": " << incoming_message << endl;

				else if(command.compare("list") == 0)
					cout << "Printing list of users: " << incoming_message << endl;

				else if(command.compare("error") == 0)
					cout << "Error sending message to server.\n";

				else if(command.compare("later") == 0)
					cout << "Logging out.\n";

				else if(command.compare("nfind") == 0)
					cout << "error sending message, could not locate user: " << incoming_message.substr(incoming_message.find(' ')) << endl;
	
				else
					cerr << "ERROR: invalid message received from server.\n";
			}

			cout_mtx.unlock();
		}
		flag = false;
	}
}


