#pragma once
#include <iostream>
#include <stdexcept>

// #define DEBUGE_MODE

static void dprintf (const char* s) {
#ifndef DEBUGE_MODE
	return;
#endif
	while (*s) {
		if (*s == '%' && *++s != '%') {
			throw std::runtime_error("invalid format string: missing arguments");
		}
		std::cout << *s++;
	}
}

template<typename T, typename... Args>
void dprintf (const char* s, const T& value, const Args&... args) {
#ifndef DEBUGE_MODE
	return;
#endif
	while (*s) { 
		if (*s == '%' && *++s != '%') {
			std::cout << value;
			return dprintf(++s, args...);
		}
		std::cout << *s++;
	}
	throw std::runtime_error("extra arguments provided to printf");
}