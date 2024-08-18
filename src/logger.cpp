#include "logger.hpp"

using namespace engine;

std::string Logger::last_message = "";
unsigned Logger::last_message_count = 0;
bool Logger::VERBOSE = false;

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

void Logger::logVerbose(std::string message) {
    if (!VERBOSE) {
        return;
    }
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
    message = "\033[1;31mError: " + message +"\033[0m";
    if (last_message == message) {
        last_message_count += 1;
        std::cerr << "\r" << "[" << last_message_count << "] " << message << std::flush;
    } else {
        last_message = message;
        last_message_count == 1;
        std::cerr << "\n" << message << std::flush;
    }
}

void Logger::logWarning(std::string message) {
    message = "\033[1;33mWarning: " + message +"\033[0m";
    if (last_message == message) {
        last_message_count += 1;
        std::cerr << "\r" << "[" << last_message_count << "] " << message << std::flush;
    } else {
        last_message = message;
        last_message_count == 1;
        std::cerr << "\n" << message << std::flush;
    }
}