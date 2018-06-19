import socket
import time
import threading

from threading import *

#-- Declare user id a=to be changed once client logs in ------------------------------------------------#
USER_ID = 'userID'
#-------------------------------------------------------------------------------------------------------#
#-- Semaphore to allow only one thread to print at a time ----------------------------------------------#
#screen_lock = Semaphore(value=1)
#-- Establish connection to server ---------------------------------------------------------------------#
server = ('localhost', 50000)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(server)
#--------------------------------------------------------------------------------------------------------#
#-- Function listens for incoming message from server ---------------------------------------------------#
def incoming_communication(connection):

    logout = False

    while logout is False:

        incoming_message = connection.recv(4096)
        incoming_message = incoming_message.decode()
        user_id, command, message = incoming_message.split("|", 3)
    
        logout = incoming_message_handler(connection, user_id, command, message)
#--------------------------------------------------------------------------------------------------------#
#-- Function to handle incoming messages from server or peer clients ------------------------------------#
def incoming_message_handler(connection, user_id, command, message):

    logout = False
    #-- Client request list of active members, message contains that list --#
    if command == "list":
        print("Active Peers: %s" %(message))
    #-- message sent by client was successfully relayed to receiver --#
    elif command == "sent":
        print("%s: %s" %(user_id, message))
    #-- message failed to reach recipient --#
    elif command == "nosend":
        print("%s: %s" %(user_id, message))
    #-- Client is receiving message from peer client --#
    elif command == "recv":
        print("%s: %s" %(user_id, message))
    #-- Client has gotten to okay to logout from the server --#
    elif command == "logo":
        print("%s: %s" %(user_id, message))
        time.sleep(2)
        connection.close()
        logout = True
    #-- Client receives an error message from the server indicating a problem with the last message sent --#
    elif command == "erro":
        print("%s: %s" %(user_id, message))
    #-- Client receives unrecognized command --#
    else:
        print("Received unrecognized command.")
        
    return logout
#--------------------------------------------------------------------------------------------------------#
#-- Loop executes until logout issued by user -----------------------------------------------------------#
def outgoing_communication(connection):

    command = ""
    while command != "logout":

        print("Please type one of the following commands: send all, send \"user id\", who, logout")
        command = input("Command --> ")
	
        #---------------------------------------------------------#
        #-- User wishes to send a message to one or more users ---#
        if command[0:4] == "send":
            message = input("message --> ")
        #---------------------------------------------------------#
        #-- User would like a list of all active members ---------#
        elif command == "who":
            message = ""
        #---------------------------------------------------------#
        #-- User would like to logout ----------------------------#
        elif command == "logout":
            message = ""
        #---------------------------------------------------------#
        #-- Invalid command entered ------------------------------#	  
        else:
            print("Sorry, you entered an invalid command, please try again.")
            continue
        
        #-- Send message to server -------------------------------#
        outgoing_message = USER_ID + "|" + command + "|" + message
        connection.send(outgoing_message.encode('utf-8'))
        #---------------------------------------------------------#
        #incoming_communication(connection)
#--------------------------------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------------------------------#
#-- User must first login, they have three attempts -----------------------------------------------------#
attempts = 3
login_successful =  False
#-- Loops up to three times for use to attempt to login -------------------------------------------------#
while attempts > 0:
    print("Please login")
	
    user_name = input("User Name--> ")
    password = input("Password--> ")
    outgoing_message = user_name + "|" + "login" + "|" + password
	
    sock.send(outgoing_message.encode("utf-8"))
	
    incoming_message = sock.recv(4096)
    incoming_message = incoming_message.decode()
    user_id, command, message = incoming_message.split("|", 3)
	
    if command == "logi":
        print("%s: %s" %(user_id, message))
        login_successful = True
        USER_ID = user_name
        break
        
    elif command == "fail":
        print("%s: %s" %(user_id, message))
        attempts -= 1
        
    else:
        print("%s: %s" %(user_id, message))
#--------------------------------------------------------------------------------------------------------#
#-- If login was unsuccessful ---------------------------------------------------------------------------#
if login_successful is False:
    print("Failed to login after three attempts.")
    sys.exit()
#--------------------------------------------------------------------------------------------------------#
    
try:
    t_one = threading.Thread(target=outgoing_communication, args=(sock,))  
except:
    print("unable to create outgoing communication thread.")
   
try:
    t_two = threading.Thread(target=incoming_communication, args=(sock,))
except:
    print("unable to create incoming communication thread.") 

try:    
    t_one.start() 
except:
    print("Unable to start first thread.")
    
try:
   t_two.start()
except:
    print("Unable to start second thread")

  
    #outgoing_communication(sock)
#-- End of loop and program -----------------------------------------------------------------------------#	
