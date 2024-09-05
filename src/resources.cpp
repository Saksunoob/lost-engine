#include "resources.hpp"

void engine::Input::handleKeyEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            if (!keys[event.key.keysym.sym].pressed) {
                keys[event.key.keysym.sym] = {true, true};
            }
            return;
        case SDL_KEYUP:
            keys[event.key.keysym.sym] = {false, true};
            return;
        case SDL_MOUSEBUTTONDOWN:
            buttons[event.button.button] = {true, true};
            return;
        case SDL_MOUSEBUTTONUP:
            buttons[event.button.button] = {false, true};
            return;
        case SDL_MOUSEWHEEL:
            scroll = IVector2(event.wheel.x, event.wheel.y);
            return;
        case SDL_MOUSEMOTION:
            mouse_delta = IVector2(event.motion.xrel, event.motion.yrel);
            mouse_pos = IVector2(event.motion.x, event.motion.y);
            return;
    }
}

void engine::Input::newFrame() {
    for (auto& [keycode, state] : keys) {
        state.justChanged = false;
    }
    for (auto& [button, state] : buttons) {
        state.justChanged = false;
    }
    mouse_delta = IVector2(0, 0);
    scroll = IVector2(0, 0);
}

bool engine::Input::getKey(SDL_KeyCode key) const {
    if (keys.find(key) == keys.end()) {
        return false;
    }
    return keys.at(key).pressed;
}

bool engine::Input::getKeyJustPressed(SDL_KeyCode key) const {
    if (keys.find(key) == keys.end()) {
        return false;
    }
    return keys.at(key).pressed && keys.at(key).justChanged;
}
bool engine::Input::getKeyJustReleased(SDL_KeyCode key) const {
    if (keys.find(key) == keys.end()) {
        return false;
    }
    return !keys.at(key).pressed && keys.at(key).justChanged;
}

bool engine::Input::getMouseButton(uint8_t button) const {
    if (buttons.find(button) == buttons.end()) {
        return false;
    }
    return buttons.at(button).pressed;
}
bool engine::Input::getMouseButtonJustPressed(uint8_t button) const {
    if (buttons.find(button) == buttons.end()) {
        return false;
    }
    return buttons.at(button).pressed && buttons.at(button).justChanged;
}
bool engine::Input::getMouseButtonJustReleased(uint8_t button) const {
    if (buttons.find(button) == buttons.end()) {
        return false;
    }
    return !buttons.at(button).pressed && buttons.at(button).justChanged;
}

engine::IVector2 engine::Input::getMousePos() const {
    return mouse_pos;
}
engine::IVector2 engine::Input::getMouseDelta() const {
    return mouse_delta;
}

engine::IVector2 engine::Input::getMouseScroll() const {
    return scroll;
}