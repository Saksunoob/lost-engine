#include "logger.hpp"

using namespace engine;

std::string Logger::last_message = "";
unsigned Logger::last_message_count = 0;

void Logger::log(std::string message) {
    if (last_message == message) {
        last_message_count += 1;
        std::cout << "\r" << "[" << last_message_count << "] " << message << std::flush;
    } else {
        last_message = message;
        last_message_count = 1;
        std::cout << "\n" << message << std::flush;
    }
}

void Logger::logError(std::string message) {
    message = "Error: " + message;
    if (last_message == message) {
        last_message_count += 1;
        std::cerr << "\r" << "[" << last_message_count << "] " << message << std::flush;
    } else {
        last_message = message;
        last_message_count == 1;
        std::cerr << "\n" << message << std::flush;
    }
    
}