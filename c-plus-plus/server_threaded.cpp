
/** C++ libraries *********************************************************************************************************/
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <cstring> //bcopy() and bclear()
#include <thread>
#include <mutex>
/**************************************************************************************************************************/

/** User defined classes **************************************************************************************************/
#include "user_info.h"
#include "active_user_info.h"
/**************************************************************************************************************************/

/** User defined header "helper.h" with namespace "tools::" for helper functions ******************************************/
#include "helper.h"
/**************************************************************************************************************************/

/** C libraries ***********************************************************************************************************/
#include <stdio.h> //for printf()
#include <unistd.h>
#include <string.h> //for strlen() and bzero()
/**************************************************************************************************************************/

/** Libraries for creating and using sockets ******************************************************************************/
#include <sys/types.h> //contains definitions for a number of data types used in the system calls
#include <sys/socket.h> //includes number of definitions of structures needed for sockets
#include <netinet/in.h> //contains constants and structures needed for internet domain addresses
#include <netdb.h> //defines the structure hostent, which is used below
/**************************************************************************************************************************/

using namespace std;

const size_t BUFFSIZE = 4028;
const int MAX_NUMBER_THREADS = 10;

const char* HOST = "127.0.0.1";
const int PORT_NUMBER = 50000; 

/** Function Declarations *************************************************************************************************/
void server_function(int & client_socket, list<user_info> users);
bool login_function(int client_socket, list<user_info>& users, string& member_id);
void incoming_communication_handler(const int& client_socket, list<user_info>& users);
void outgoing_communication_handler(const int& client_socket, list<user_info>& users, char buffer[], const int& size,  const string& sender, const string& command, const string& message);

/** Global Objects Every Thread Can Access ********************************************************************************/
char const *read_file = "user_records.txt";

list<user_info> users = tools::read_from_file(read_file); /** use read_from_file() from helper.h namespace tools to populate list **/
list<active_user_info> active_users; /** users are added and removed from list when they log on and logoff respectively **/

int number_of_threads; /** number of active user threads **/
thread thread_ids[MAX_NUMBER_THREADS]; /** maximum number of users who can connect to chatroom **/

mutex mtx; /** thread lock for securing resources **/

/**************************************************************************************************************************/
/** Main body of program **************************************************************************************************/
int main()
{
	/** List all registered members **************************************************************************/
	for(list<user_info>::iterator it = users.begin(); it != users.end(); it++)
		cout << "User name: " << ( *it).get_user_name() << " Password: " << (*it).get_password() << endl;
    	/*********************************************************************************************************/

/** The code for the server ************************************************************************************************/

    	struct sockaddr_in serv_addr; /** structure containing an internet address. Defined in netinet/in.h **/
	struct sockaddr_in cli_addr; /** structure containing an internet address. Defined in netinet/in.h **/

    	int server_socket, client_socket; //file descriptors store the values returned by the socket system call and the accept system call
	int clilen; /** size of future client address **/
    	//int n; //return value for the read() and write() calls. Contains the number of characters read or written 

/** Create Socket ************************************************************************************************************/	

	/** Socket system call: socket() creates a new socket. Takes three arguments; the first being the address domain, 
	either AF_UNIX or AF_INET. The second argument being the type of socket, either SOCK_STREAM or SOCK_DGRAM for either stream or 		datagram sockets, respectively. The third being the protocol, almost always zero. OS will choose most appropriate protocal TCP for 		stream and UDP for datagram Socket system call returns an entry into the file descriptor table. This value is used for all 		subsequent references to this socket **/
	 
	if( (server_socket = socket(AF_INET, SOCK_STREAM,0)) < 0) 
        	cerr << "ERROR opening socket\n";

/** Setsockopt to allow for reusing of port **************************************************************************************/

	int opt = 1;
	if( setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        	cerr << "ERROR implementin setsockopt()\n";

/** Modifies sockaddr_in serv_addr ************************************************************************************************/

	/** sets all values in a buffer to zero. Initializes serv_addr to all zeros. Takes two arguments, the first being a pointer to the 		buffer and the second is the size fo the buffer. **/

	bzero((char *) &serv_addr, sizeof(serv_addr));
	bzero((char *) &cli_addr, sizeof(cli_addr));

	/** serv_addr is a structure of type struct sockaddr_in. This structure has four fields,the first of which short sin_family **/
	/** should always be set to the symbolic constant AF_INET **/

	/** third field of sockaddr_in is struct of type struct in_addr which contains only a single filed unsigned long s_addr this field 		contains the IP address of the host. For server this will always be IP address of machine server is running on. Symbolic constant 		to obtain IP address of machine server running on INADDR_ANY **/

	/** htons() function converts port number from host byte order to to network byte order and passes value to second field of 		serv_addr **/

    	serv_addr.sin_family = AF_INET; 					    
	serv_addr.sin_addr.s_addr = INADDR_ANY;				       
	serv_addr.sin_port = htons(PORT_NUMBER); 

	cli_addr.sin_family = AF_INET;
	clilen = sizeof(cli_addr);

/** Bind server socket ************************************************************************************************************/

	/** bind() system call binds a socket to an address and port **/
	if (bind(server_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        	cerr << "ERROR on binding\n";                                     
	
/** Listen ************************************************************************************************************************/	
	/** Allows process to listen on socket for connection. The second argument is the size of the backlog queue **/
	listen(server_socket,5);

/** Infinite loop continues to listen for connections to server socket *************************************************************/

	/** user arrary to keep track of every file descriptor in use as a socket **/
	int socket_array[MAX_NUMBER_THREADS] = {-1};

	/** Infinte loop allows server_socket to continue to listen for connections **/
	while(1)
	{	
		cout << "Waiting for connection..." << endl;

		/*accept() system call causes the process to block until a client connects to the server It returns a new file descriptor 			and all communication on this connection should be done using the new file descriptor. The second argument is a reference 			pointer to the address of the client on the other end of the connection*/

		if( (client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen)) < 0 )
		{                  
 			cerr << "ERROR on accept\n";
			continue;
		}

		cout << "Accepted connection on socket: " << client_socket << endl;

		/* function to allow communication between server and client */
		if(number_of_threads < MAX_NUMBER_THREADS)
		{
			/** iterate through socket_array to find empty position **/
			int i = 0;
			for(; socket_array[i] != -1 && i < MAX_NUMBER_THREADS; i++)
			{}

			socket_array[i] = client_socket; /** empty position found at index i **/

			/** spawn thread to handle client that just connected **/
			for(int j=0; j < MAX_NUMBER_THREADS; j++)
			{
				if( thread_ids[j].get_id() == thread::id{} )
				{
					/* just for testing that this conditional does as expected */
					cout << "thread_ids index is: " << i << endl;
					/***********************************************************/

					/** spawn thread and set thread_ids[j] equal to new thread id **/
					thread_ids[j] = ( thread(server_function, ref(socket_array[i]), users) );
					break;
				}
			}
		}
	
		else
		{
			cout << "Too many clients attempting to connect\n.";
			send(client_socket, "Sorry, to many people are already connected, try again later.", 61 , 0);
		}

	}

    return 0;
}

/** End of main body of program **********************************************************************************/
/*****************************************************************************************************************/

/** Login function insures client successfully logs in before continuing ****************************************************/
bool login_function(int client_socket, list<user_info>& users)
{	
	bool login_successful = false;
	char buffer[BUFFSIZE] = {0};
	string user_name, password;

	send(client_socket, "hello", 5, 0);

	cout << "Login welcome message sent." << endl;

	for(int j=0; j<3; j++)
	{
		bzero(buffer, sizeof(buffer) );
		user_name = "";
		password = "";

		if( read(client_socket, buffer, BUFFSIZE) < 0 )
			cerr << "ERROR reading from socket\n";

		printf("Buffer contents: %s\n", buffer);

		size_t i=0; /* same type as that returned by string.length() */
		/** Check that the next character is not a comma, or a new line, carrage return, or space **/
		for(; i < strlen(buffer) && buffer[i] != 44 && buffer[i] != 10 && buffer[i] != 13 && buffer[i] != 32; i++)
		{
			user_name += buffer[i];
		}	

		cout << "user_name: " << user_name << endl;

		i++; /** skip white space between username and password **/

		for(; i < strlen(buffer) && buffer[i] != 44 && buffer[i] != 10 && buffer[i] != 13 && buffer[i] != 32; i++)
		{
			password += buffer[i];
		}

		cout << "password: " << password << endl;

		cout << "Searching for match...\n";

		/** Search list of users to look for a match **/
		for(list<user_info>::iterator it = users.begin(); it != users.end(); it++)
		{
			if( (*it).get_user_name() == user_name && (*it).get_password() == password)
			{
				login_successful = true;
				break;
			}
		}
		cout << "Done searching\n";
		if(login_successful)
		{
			/** Create new active_user_info object to add to list of active_users **/
			active_user_info new_joiner;

			new_joiner.set_user_name(user_name);
			new_joiner.set_socket(client_socket);

			active_users.push_back( new_joiner );

			cout << "Match found for: " << user_name << " " << password << endl;
			break;
		}
		
		if(j<2)
		{
			cout << "FAILED to find match for user info: " << user_name << " " << password << endl;
			send(client_socket, "nope", 4, 0);
		}
			
	}

	return login_successful;	
}
/** Server function spawns thread to handle communicating between server and client allowing for multiple clients **********/
void server_function(int & client_socket, list<user_info> users)
{
	/** Increment the number of threads currently running *************************************************/
	cout << "Thread spawned: " << this_thread::get_id() << endl;

	mtx.lock();
	number_of_threads++;
	mtx.unlock();

	cout << "Number of active threads: " << number_of_threads << endl;

	/** Main body of server function. If users succeeds in logging in allowed to enter chat room **********/

	bool login_successful = login_function( client_socket, users);

	if(login_successful)
	{
		cout << "Login successful on socket " << client_socket << endl;
		send(client_socket, "login", 5 , 0);
		cout << "Login message sent." << endl;

		/* function that carries out sending and receiving with client */
		incoming_communication_handler(client_socket, users);
		/***************************************************************/
	}

	else
	{
		cout << "Login failed" << endl;
		send(client_socket, "faild", 5 , 0);
		cout << "Failed login message sent." << endl;
	}

	/** Deincrement the number of active users ***********************************************************/
	mtx.lock();
	number_of_threads--; /* decrease count of active threads */
	mtx.unlock();

	close(client_socket); /* We are finished with the socket so we now close it */

	client_socket = -1;

}
/** Communication function for client to communicate with server ************************************************************/
void incoming_communication_handler(const int& client_socket, list<user_info>& users)
{
	bool logout = false;
	string incoming_message;//, outgoing_message;
	char buffer[BUFFSIZE] = {0};
	string user_name, command, message, full_message;

	while(!logout)
	{
		/** Clear buffer of any contents **********************************************************/
		bzero(buffer, sizeof(buffer) );

		/** read message from client *************************************************************/
		if( read(client_socket, buffer, BUFFSIZE) < 0 )
			cerr << "ERROR reading from socket\n";

		/** Print received message from client ***************************************************/
		full_message = buffer;
		cout << "Message received: " << full_message << endl;

		/** Parse user_name of sender, command, and message out of  ******************************/
		user_name.clear();
		command.clear();
		message.clear();

		size_t i=0;

		for(; i<full_message.length() && full_message[i] != ' ';i++ )  /* user_name */
			user_name += full_message[i];

		++i; /* skip blank space*/
	
		for(; i<full_message.length() && full_message[i] != ' ';i++)  /* command */
			command += full_message[i];

		++i; /* skip blank space*/

		for(; i<full_message.length();i++)  /* message */
			message += full_message[i];

		cout << "user_name: " << user_name << " command: " << command << " message: "<< message << endl;

		/** Possible commands received are: who, send all, send "username", or logout. Anything else is read as an error. **/

 		/** Incoming command is "send_all" ********************************************************/
		if(command.compare("send_all") == 0)
			outgoing_communication_handler(client_socket, users, buffer, sizeof(buffer), user_name, "broad", message);

		/** Incoming command is "send to specified user" ******************************************/
		else if(command.compare("send") == 0) 
			outgoing_communication_handler(client_socket, users, buffer, sizeof(buffer), user_name, "recv", message);

		/** Incoming command is "who" *************************************************************/
		else if(command.compare("who") == 0) 
			outgoing_communication_handler(client_socket, users, buffer, sizeof(buffer), user_name, "list", "");

		/** Incoming command is logout ************************************************************/
		else if(command.compare(0,6,"logout") == 0) 
		{
			outgoing_communication_handler(client_socket, users, buffer, sizeof(buffer), user_name, "done", "");
			logout = true;			
		}

		/** Incoming command is not recognized ****************************************************/
		else 
		{
			cerr << "Received unrecognized command: " << incoming_message << " from client: " << client_socket << endl;
			outgoing_communication_handler(client_socket, users, buffer, sizeof(buffer), user_name, "error", "");
		}
	} 
	/** End of infinite loop *******************************************************************************/

	/** Search list of active users for this user and delete them from the list **************************/

	cout << "Deleting active user on socket " << client_socket << endl;
	
	for(list<active_user_info>::iterator it = active_users.begin(); it != active_users.end(); it++)
	{
		if( ( *it).get_socket() == client_socket )
		{
			cout << "Deleting active user: " << ( *it).get_user_name() << endl;
			mtx.lock();
			it = active_users.erase(it); 
			mtx.unlock();
			break;
		}
	}
}

void outgoing_communication_handler(const int& client_socket, list<user_info>& users, char buffer[], const int& size, const string& sender, const string& command, const string& message)
{
	bzero(buffer, size);
	string outgoing_message;

	/** Client requested list of active users ****************************************************************/
	if(command.compare("list") == 0)
	{
		string list_of_active_users;
	
		for(list<active_user_info>::iterator it = active_users.begin(); it != active_users.end(); it++)
			list_of_active_users += ( " " + (*it).get_user_name() );

		outgoing_message = ("server " + command + list_of_active_users);

		//cout << "outgoing_message: " << outgoing_message << endl;
		tools::string_to_char_array(outgoing_message, buffer, size);
		printf("Outgoing_message: %s", buffer);		

		if( send(client_socket, buffer, strlen(buffer) , 0) < 0)
			cerr << "ERROR writing to socket\n";
	}

	/** Client requested to broadcast message to all active users ********************************************/
	else if(command.compare("broad") == 0)
	{	
		string outgoing_message = (sender + ' ' + command + ' ' + message);

		tools::string_to_char_array(outgoing_message, buffer, size);
		//cout << "outgoing_message:" << outgoing_message << endl;
		printf("Outgoing_message: %s", buffer);

		for(list<active_user_info>::iterator it = active_users.begin(); it != active_users.end(); it++)
		{
			if( send( (*it).get_socket(), buffer, strlen(buffer), 0 ) < 0 )
				cerr << "Error writing to socket: " << (*it).get_socket() << endl;
		}
	}

	/** Client requested to send message to specified user ***************************************************/
	else if(command.compare("recv") == 0)
	{
		bool found_user = false;

		string outgoing_message = (sender + ' ' + command);
		string receiver_name = message.substr(0, message.find(' '));

		outgoing_message += message.substr(message.find(' ')) ;

		tools::string_to_char_array(outgoing_message, buffer, size);
		//cout << "outgoing_message:" << outgoing_message << endl;
		printf("Outgoing_message: %s", buffer);
 
		cout << "Looking for member: " << receiver_name << endl;

		for(list<active_user_info>::iterator it = active_users.begin(); it != active_users.end(); it++)
		{
			if( (*it).get_user_name() == receiver_name )
			{
				found_user = true;
				if( send((*it).get_socket(), buffer, strlen(buffer) , 0) < 0)
					cerr << "ERROR writing to socket\n";
				break;
			}
		}

		if(found_user)
			cout << "found: " << receiver_name << endl;

		else
		{
			cout << "Could not find: " << receiver_name << endl;

			outgoing_message = ("nfind " + receiver_name);
			tools::string_to_char_array(outgoing_message, buffer, size);
			printf("Outgoing_message: %s", buffer);

			if( send(client_socket, buffer, strlen(buffer) , 0) < 0)
				cerr << "ERROR writing to socket\n";
			
		}
	}

	/** Unable to recognize command sent by client *****************************************************************/
	else if(command.compare("error") == 0)
	{
		outgoing_message = ("server " + command + "");

		//cout << "outgoing_message: " << outgoing_message << endl;
		tools::string_to_char_array(outgoing_message, buffer, size);
		printf("Outgoing_message: %s", buffer);

		if( send(client_socket, buffer, strlen(buffer), 0) < 0)
			cerr << "ERROR writing to socket\n";
	}

	/** Client requested to logout *********************************************************************************/
	else if(command.compare("done") == 0)
	{
		outgoing_message = ("server later ");

		//cout << "outgoing_message: " << outgoing_message << endl;
		tools::string_to_char_array(outgoing_message, buffer, size);
		printf("Outgoing_message: %s", buffer);

		if( send(client_socket, buffer, strlen(buffer), 0) < 0)
			cerr << "ERROR writing to socket\n";
	}

	/** Unrecognized command issued to outgoing command handler ****************************************************/
	else
		cerr << "ERROR calling outgoing_communication_handler(),";

}



