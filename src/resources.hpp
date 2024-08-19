#include <chrono>
#include <iostream>


namespace engine {
    class Time {

        double delta_time;
        int64_t last_frame;


        public:
        double deltaTime() {
            return delta_time;
        }
        void newFrame() {
            int64_t new_frame = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            delta_time = static_cast<double>(new_frame-last_frame) / 1e6;
            last_frame = new_frame;
        }
    };
}