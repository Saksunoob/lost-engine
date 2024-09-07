#include <chrono>
#include <unordered_map>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_events.h>

#include "utils.hpp"

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

    struct KeyState {
        bool pressed;
        bool justChanged;
    };

    class Input {
        std::unordered_map<int, KeyState> keys;
        std::unordered_map<uint8_t, KeyState> buttons;
        IVector2 mouse_pos, mouse_delta, scroll;
        public:
        void handleKeyEvent(SDL_Event& event);
        void newFrame();

        bool getKey(SDL_KeyCode) const;
        bool getKeyJustPressed(SDL_KeyCode) const;
        bool getKeyJustReleased(SDL_KeyCode) const;

        bool getMouseButton(uint8_t button) const;
        bool getMouseButtonJustPressed(uint8_t button) const;
        bool getMouseButtonJustReleased(uint8_t button) const;

        IVector2 getMousePos() const;
        IVector2 getUIMousePos() const;
        IVector2 getMouseDelta() const;

        IVector2 getMouseScroll() const;
    };
}