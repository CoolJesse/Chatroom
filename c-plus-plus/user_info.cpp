#include <fstream>
#include <iostream>

#include "user_info.h"

user_info::user_info(std::string new_user_name, std::string new_password)
{
	user_name = new_user_name;
	password = new_password;
}

std::string user_info::get_user_name() const
{
	return user_name;
}

std::string user_info::get_password() const
{
	return password;
}
     
void user_info::set_user_name(std::string new_user_name)
{
	user_name = new_user_name;
}

void user_info::set_password(std::string new_password)
{
	password = new_password;
}

