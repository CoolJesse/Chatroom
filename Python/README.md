# python_sockets
This project utilizes the socket API and multi-threading to create a chatroom for multiple users. The server starts up, and reads the user info from "client_user_info.csv" into a list. This list will be used to verify a user id when a client attempts to login. The server then listens for an incomming connection, and spawns a thread for each client that connects. Upon connecting with the server, each client must first login with one of the user id and password credentials included in the "client_user_info.csv" file. They are allowed three attempts to login, if they fail the client program terminates. If they user successfully logs in they are prompted to enter one of four commands: "who" to see who else is logged in to the chat room, "send all" to send a message to all clients who are logged in, "send username" to send a message only to the specified client, and "logout" to logout of the chatroom. 