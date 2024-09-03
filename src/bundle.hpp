#pragma once

namespace engine {
    struct Bundle {
        std::unordered_map<std::type_index, std::shared_ptr<std::any>> components;

        template <typename...T>
        Bundle(T... with) {
            (insert_component<T>(with), ...);
        }

        private:
        template <typename T>
        void insert_component(T& value) {
            components[typeid(T)] = std::make_shared<std::any>(std::forward<T>(value));
        }
    };
}