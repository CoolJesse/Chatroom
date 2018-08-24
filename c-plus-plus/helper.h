#ifndef _HELPER_H
#define _HELPER_H

#include <list>
#include <string>

#include "user_info.h"

namespace tools
{
	//class user_info;

	std::list<user_info> read_from_file(char const *name_of_file);

	void string_to_char_array(const std::string& message, char buffer[], const size_t& size);

}
#endif
