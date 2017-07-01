#ifndef LOGGER_H_
#define LOGGER_H_
#include <iostream>

#define LOG(message) std::cout << "LOG: " << message << std::endl
#define ERROR_LOG(message) std::cerr << "ERROR: " << message << std::endl
#define WARNING_LOG(message) std::cout << "WARNING: " << message << std::endl;

#endif //LOGGER_H_
