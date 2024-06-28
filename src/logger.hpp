#pragma once

#include <iostream>

namespace engine {
    class Logger {
        static std::string last_message;
        static unsigned last_message_count;
        public:
            static void log(std::string message);
            static void logError(std::string message);
    };
}