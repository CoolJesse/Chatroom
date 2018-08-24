#ifndef _USER_INFO_H
#define _USER_INFO_H

#include <string>

class user_info
{
    public:

        user_info(std::string new_user_name = "", std::string new_password = "");
            
        std::string get_user_name() const;
        std::string get_password() const;
        
        void set_user_name(std::string new_user_name);
        void set_password(std::string new_password);

	~user_info() {};
        
    private:

        std::string user_name;
        std::string password;  
};

#endif





