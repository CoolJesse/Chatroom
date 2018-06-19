import socket #socket API
import threading #for multithreading
import csv
import time

#----------------------------------------------------------------------------------------------------------------------#
#--------------------- These identifiers are hardcoded to identify the server to the clients --------------------------#
SERVER_ID = "Server0001"

HOST = "localhost"
PORT_NUMBER = "50000"
IP_ADDRESS = socket.gethostbyname(socket.getfqdn())
#----------------------------------------------------------------------------------------------------------------------#
#---------------------------------------- List of all peers and their info --------------------------------------------#
peers = []
active_peers = [] #-- List of peer connected to the chatroom --#
#----------------------------------------------------------------------------------------------------------------------#

#----------------------------------------- Lock to prevent race conditions --------------------------------------------#
lock = threading.Lock()
#----------------------------------------------------------------------------------------------------------------------#
#------------------------------------------ Class for holding client info ---------------------------------------------#
class Peer_Info:
    #-- Constructor --#
    def __init__(self, user_id, passcode):
        self.user_id = user_id
        self.passcode = passcode
		#self.connection
#----------------------------------------------------------------------------------------------------------------------#
class Active_Peer_Info:
    #-- Constructor --#
    def __init__(self, user_id, connection, address):
        self.user_id = user_id
        self.connection = connection
        self.ip_address = address[0]
        self.port_number = address[1]
#------------------- Import peer info from csv file and place into list of Client Info objects ------------------------#
def read_peer_info(CSV_file_to_read):
    global peers
	
    with open(CSV_file_to_read) as csvFile:
        readCSV = csv.reader(csvFile, delimiter=',')

        for row in readCSV:
            user_id = row[0]
            passcode = row[1]

            peers.append(Peer_Info(user_id,passcode ))
        
        for x in peers:
            print("user_id: ", x.user_id, " passcode: ", x.passcode)
			
# ---------------------------------------------------------------------------------------------------------------------#
# ------------------------------------------------ Create Socket ------------------------------------------------------#
def makeserversocket(portNumber, backlog=5):

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #-- allows us to reuse socket immediately after it is closed --#
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    #-- binds to whatever IP Address the peer is assigne --#
    s.bind(("", int(portNumber)))
    s.listen(backlog)

    print("binding peer server socket to port " + portNumber)
    print("Listening for connections...")
	
    #-- returns server socket --#
    return s	
# ---------------------------------------------------------------------------------------------------------------------#
# --------------------------------------------- Authentication Function -----------------------------------------------#
'''
Function takes the machineID and key that have been extracted from theincoming connection, and checks the list of peers 
to make sure the connecting peer is authorized to connect. If it is the function returns true, if not
returns false
'''
def verify_incoming_peer(connection, user_id, passcode):

    global peers
    found_match = False

    #-- print only for testing on server side --#
    print("User login info is: " + user_id + " " + passcode + " connection: %s" %(connection))
    # -------------- Loop through list of clients and see if a match with login info is found -------------------#
    for x in peers:
        # -- Client's machine id and private key match so it is confirmed to connect --#
        if (x.user_id == user_id) and (x.passcode == passcode):
            print("match found!")
            found_match = True  #Match found so update variable
            break
    # ------------------------------------------------------------------------------------------------------------#
    # ------------------------ No match found in list of clients, peer failed to connect -------------------------#
    if found_match is False:
        print("User: %s at connection: %s failed to provide a valid user_id and/or passcode." %(user_id,connection))
    # -----------------------------------------------------------------------------------------------------------------#
    # -------------------------------------- Client successfully connected --------------------------------------------#   
    return found_match
		
#----------------------------------------------------------------------------------------------------------------------#
#--------------------------------------- Client attempts to login to chatroom -----------------------------------------#
'''
Function calls the verify_incomin_peer() function to check the user id provided and the password 
contained in the message with the list of peers. If it finds a match, the function adds the user id
to the active_peers list and returns True the function returns true and the 
'''
def client_login(connection, address, user_id, command, incoming_message):

    #-- declare this as global so function accesses the existing object, not create local one --#
    global registered_peers
    login_successful = False
	#------------------------------ Log in to add user name to list of active peers -----------------------------------#
    print("User: %s attempting to login at connection: %s" %(user_id, connection))
    print("")
    member_verified = False

    #-- If user ID and password can be verified add to active members --#
    if verify_incoming_peer(connection, user_id, incoming_message):
        command = "logi"
        outgoing_message = "Login successful, welcome"
        already_exist = False
        # -- Add socket to list of active connections --#
        for p in active_peers:
            if p.user_id == user_id:
                p.connection = connection
                p.ip_address = address[0]
                p.port_number = address[1]
                login_successful = True
                already_exist = True
                break
				
        if already_exist is False:
            with lock:
                active_peers.append(Active_Peer_Info(user_id, connection, address))
                print("Added to list of active peers.")
                login_successful = True
        #------------------------------------------------------------------#     
    else:
        command = "fail"
        outgoing_message = "Failed to login, please try again."       
    #-- reply to client ------------------------------------------------------------------#
    handle_outgoing_peer(connection, SERVER_ID, command, outgoing_message)
	#-- return if login was successful --#
    return login_successful	
#----------------------------------------------------------------------------------------------------------------------#
# ---------------------------------------- Handle outgoing connection -------------------------------------------------#
def handle_outgoing_peer(connection, user_id, command, message=""):

    outgoing_message = user_id + "|" + command + "|" + message
    connection.send(outgoing_message.encode("utf-8"))
#----------------------------------------------------------------------------------------------------------------------#
# ------------------------------------------ Handle incoming connection -----------------------------------------------#
def handle_incoming_peer(connection):

    incoming_message = connection.recv(2048)
    incoming_message = incoming_message.decode() #convert input from bytestream to string to be parsed
	#-- parse incoming message into user id, passcode, etc ------------------------------------#
    user_id, command, message = incoming_message.split("|", 3)
    #-- call incoming command handler to process connection ----------------------#
    logout = incoming_command_handler(connection, user_id, command, message)
    return logout
#----------------------------------------------------------------------------------------------------------------------#
#------------------- Command Handler takes commands from peer and performs necessary operation ------------------------#
def incoming_command_handler(connection, user_id, command, incoming_message):

    terminate_connection = False
    #--------------------------- Send list of active members to client, command: who ----------------------------------#
    if command == "who":
        outgoing_message = ""
        print("sending list of active peers to %s at %s " %(user_id, connection))
        #for x in active_peers:
        for x in active_peers:
            outgoing_message += (x.user_id + " ")
            print("user_id: %s, ip address: %s, port number: %s" %(x.user_id, x.ip_address, x.port_number) )
		#-- reply to client -----------------------------------------------------------------#
        print(outgoing_message)
        handle_outgoing_peer(connection, SERVER_ID, "list", outgoing_message) 
    #------------------------------------------------------------------------------------------------------------------#
    #------------------------  Send message to specified user, command: SEND "user name" -----------------------------#
    elif command[0:4] == "send":
        
        outgoing_command = "recv"
        receiver = command[5:]
        
        print("Sending message: %s to %s." %(incoming_message, receiver))
		
        #-- send to all active members --#
        return_command = "sent"
        
        if  receiver == "all":
            return_message = "message sent to all active members"
            #pass through list of all active members and send message to them
            for x in active_peers:
                #-- testing function --#
                print("%s | %s | %s" %(x.user_id, return_command, incoming_message))
                #-------------------------------------------------------------------------------------------------#
                handle_outgoing_peer(x.connection, user_id, outgoing_command, incoming_message)
                #-------------------------------------------------------------------------------------------------#
           
        #-- send to specific member --#
        else:
            #-- Assume match does not exits, and change to if match is found --#
            return_command = "nosend"
            #-- Pass through list of all active members to find recipient --#
            for x in active_peers:
                if x.user_id == receiver:
                    return_message = "message sent"
                    return_command = "sent"
                    print("found recipient.")
                    print("%s | %s | %s" %(x.user_id, command, incoming_message))
                    #-------------------------------------------------------------------------------------------------#
                    handle_outgoing_peer(x.connection, user_id, outgoing_command, incoming_message)
                    #-------------------------------------------------------------------------------------------------#
                    break
            if return_command == "nosend":
                return_message = "Message failed to be sent, user could not be found."    
		#-- reply to sender -----------------------------------------------------------------#
        '''
        outgoing_message = "message sent."
        print(incoming_message + " " + return_message + " to " + receiver)
        handle_outgoing_peer(connection, SERVER_ID, return_command, return_message) 
        '''
    #------------------------------------------------------------------------------------------------------------------#
    # ------------------------- Close socket to terminate connection, command: logout ---------------------------------#
    elif command == "logout":

        for i, p in enumerate(active_peers):
            if p.user_id == user_id:
                print("Peer: %s signed off from network." %(p))
                #-- delete client from list of active peers --#
                del active_peers[i]
        #-- Tells calling function to terminate connection --#
        terminate_connection = True		
		#-- reply to client -----------------------------------------------------------------#  
        outgoing_message = "goodbye!"        
        print("%s %s" %(outgoing_message, user_id))
        handle_outgoing_peer(connection, SERVER_ID, "logo", outgoing_message) 
    #------------------------------------------------------------------------------------------------------------------#
	#------------------------------------------ Received invalid input ------------------------------------------------#
    else:
		#-- reply to client -----------------------------------------------------------------#  
        outgoing_message = "Invalid command received"
        print("Invalid input: %s" %(outgoing_message))
        handle_outgoing_peer(connection, SERVER_ID, "erro", outgoing_message) 
		
    #-- tells calling function whether or not to terminate connection --#
    return terminate_connection
    #------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-- Function to be called by a seperate thread of control for communication between client and server ------------------#
def communication_thread(client, address):

    incoming_message = client.recv(2048)
    incoming_message = incoming_message.decode()
    user_id, command, message = incoming_message.split("|", 3)
    
    #-- client attempts to login --#
    login_successful = client_login(client, address, user_id, command, message)

    if login_successful ==  True:
        logout = False
        while logout is False:
            logout = handle_incoming_peer(client)
           
    time.sleep(2)
    client.close()
#-----------------------------------------------------------------------------------------------------------------------#
#---------------------------------------------- Main Body of Program ---------------------------------------------------#
#-- Read user into into peer list --#
read_peer_info("client_user_info.csv")
#-- Create server socket to listen for connections --#
server_socket = makeserversocket(PORT_NUMBER)
#-- Infinite loop keeps listening for connections ----------------------------------------------------------------------#
while True:
#-- sever accepts incoming connection --#
    client, address = server_socket.accept()
    try:
        t = threading.Thread(target=communication_thread, args = (client, address))
        t.start()
    except:
        print("unable to spawn thread.")
    #communication_thread(client, address)
#-- End of program -----------------------------------------------------------------------------------------------------#
