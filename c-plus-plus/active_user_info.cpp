#include <string>
#include "active_user_info.h"

using namespace std;

active_user_info::active_user_info(string new_user_name, int new_socket)
    :user_name{new_user_name}, socket{new_socket} {}

string active_user_info::get_user_name() const
{
	return user_name;
}

int active_user_info::get_socket() const
{
	return socket;
}
       
void active_user_info::set_user_name(string new_user_name)
{
	user_name = new_user_name;
}

void active_user_info::set_socket(int new_socket)
{
	socket = new_socket;
}

bool active_user_info::operator==(const int& rhs_socket)
{
	return (socket == rhs_socket);
}

