#include <iostream>
#include <fstream>
#include <cstring> //bzero()

#include "helper.h"

//class user_info;

namespace tools
{
	std::list<user_info> read_from_file(char const *name_of_file)
	{
		std::list<user_info> users;
		std::string line, user_name, password;

		std::ifstream instream;
		/** Open specified file for reading from **************************************/
		instream.open(name_of_file);
		/******************************************************************************/
		/** Check that file is open for reading ***************************************/
		if(!instream.is_open())
		{
			std::cerr << "Failed to open text file: " << name_of_file << std::endl;
			exit(1);
		}
		/******************************************************************************/
		/** Create new User_Info object to add to list of users **/
		user_info new_user;
	
		/*********************************************************/
		while( getline(instream,line) )
		{
			user_name.clear();
			password.clear();
	
			size_t i=0;
			for(; line[i]!=','; i++)
				user_name += line[i];
        	
			i+=2; //skip comma and space
        	
			for(; i < line.length(); i++)
				password += line[i];
        	
			new_user.set_user_name(user_name);
			new_user.set_password(password);
        	
			users.push_back(new_user);
		}
		/**************************************************************************************/
		/** close filestream **/
		instream.close();
		/*********************/
		/** Return list of users **/
		return users;
		/**************************/
	}
	
	void string_to_char_array(const std::string& message, char buffer[], const size_t& size)
	{
		bzero(buffer, size);
	
		for(size_t i=0, j=0; i < message.length() && i < size; i++, j++)
		{
			buffer[i] = message[j];
		}
	}
}
