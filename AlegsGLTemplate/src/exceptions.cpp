#include "exceptions.hpp"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <new>   

LameAssString::LameAssString(char* buf) : buffer(nullptr){
	size_t size = strlen(buf);
	try {
		buffer = new char[size + 1 + 500];
		strcpy(buffer, buf);
	}
	catch (std::bad_alloc& ba) {
		(void)ba;
		throw;
	}
}

void LameAssString::resize(size_t newSize) {
	// too leet to handle
	if (newSize == 1337) {
		throw TooLeetToHandle();
	}
}