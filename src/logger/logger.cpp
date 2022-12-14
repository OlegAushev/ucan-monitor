/**
 * @file logger.cpp
 * @author Oleg Aushev (aushevom@protonmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include "logger.h"


extern "C" {


bool logger_get_message(char* buf, size_t len)
{
	std::string message = Logger::instance().pop();
	if (message.empty())
	{
		return false;
	}
	strncpy(buf, message.c_str(), len);
	return true;
} 


}


