#pragma once
#include <iostream>
#include <stdexcept>

static bool debug_print;
void dprintf (const char* s) {
	while (*s) {
		if (*s == '%' && *++s != '%') {
			throw std::runtime_error("invalid format string: missing arguments");
		}
		std::cout << *s++;
	}
}

template<typename T, typename... Args>
void dprintf (const char* s, const T& value, const Args&... args) { 
	while (*s) { 
    	if (*s == '%' && *++s != '%') {
			std::cout << value;
			return dprintf(++s, args...);
		}
		std::cout << *s++;
	}
	throw std::runtime_error("extra arguments provided to printf");
}