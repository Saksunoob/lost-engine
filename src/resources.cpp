#include "resources.hpp"

void engine::Input::handleKeyEvent(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        keys[event.key.keysym.sym] = {true, true};
        return;
    }
    if (event.type == SDL_KEYUP) {
        keys[event.key.keysym.sym] = {false, true};
        return;
    }
}

void engine::Input::newFrame() {
    for (auto& [keycode, state] : keys) {
        state.justChanged = false;
    }
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