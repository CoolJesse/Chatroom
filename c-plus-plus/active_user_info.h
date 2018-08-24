#ifndef _ACTIVE_USER_INFO_H
#define _ACTIVE_USER_INFO_H

#include <string>

using namespace std;

class active_user_info
{
    public:

        active_user_info(string new_user_name = "", int new_socket = -1);
            
        string get_user_name() const;
        int get_socket() const;
        
        void set_user_name(string new_user_name);
        void set_socket(int new_socket);

	bool operator==(const int& rhs_socket);

	~active_user_info() {};
        
    private:
        string user_name;
	int socket;
 
};

#endif





