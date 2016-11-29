#ifndef HINTS_H
#define HINTS_H
#include <iostream>
#include <typeinfo>
#include <sstream>

int str_to_int(std::string s)
{
	std::stringstream convert;
	int result;
	if ((convert >> result).fail() || !(convert >> std::ws).eof())
	{
		throw std::bad_cast();
	}
	return result;
}
#endif // HINTS_H
