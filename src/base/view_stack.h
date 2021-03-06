#pragma once

#include "view.h"
#include "base/utility/time.h"
#include "base/math/box.h"
#include <map>
#include <type_traits>
#include "base/utility/maybe.h"
#include <SFML/Window.hpp>

namespace gorc {

template <typename LayerIdT> class view_stack {
    using LayerIdUT = typename std::underlying_type<LayerIdT>::type;
private:
    box<2, int> view_size = make_box(make_vector(0, 0), make_vector(0, 0));
    std::map<LayerIdUT, maybe<view*>> views;
    sf::Window& window;

public:
    view_stack(sf::Window& window) : window(window) {
        return;
    }

    void set_layer(LayerIdT index, view& value) {
        views[static_cast<LayerIdUT>(index)] = &value;
        value.resize(view_size);
    }

    void clear_layer(LayerIdT index) {
        views[static_cast<LayerIdUT>(index)] = nothing;
        resize(view_size);
    }

    void resize(const box<2, int>& view_size) {
        this->view_size = view_size;
        for(const auto& view_pair : views) {
            if(view* v = std::get<1>(view_pair)) {
                v->resize(view_size);
            }
        }
    }

    void draw(const time& time, const box<2, int>& view_size, graphics::render_target& target) const {
        for(const auto& view_pair : views) {
            if(view* v = std::get<1>(view_pair)) {
                v->draw(time, view_size, target);
            }
        }
    }

    void handle_input(const time& time, const vector<2, int>& cursor_pos) {
        maybe<input::input_adapter*> mouse_input_target;
        maybe<input::input_adapter*> keyboard_input_target;

        for(auto& view_pair : views) {
            if(view* v = std::get<1>(view_pair)) {
                if(input::input_adapter* input_adapter = v->get_input_adapter()) {
                    input_adapter->set_mouse_cursor_position(time, cursor_pos);

                    if(input_adapter->wants_mouse_focus()) {
                        mouse_input_target = input_adapter;
                    }

                    if(input_adapter->wants_keyboard_focus()) {
                        keyboard_input_target = input_adapter;
                    }
                }
            }
        }

        if(input::input_adapter* mouse_input_adapter = mouse_input_target) {
            window.setMouseCursorVisible(!mouse_input_adapter->hide_mouse_cursor());
            mouse_input_adapter->handle_mouse_input(time);
        }

        if(input::input_adapter* keyboard_input_adapter = keyboard_input_target) {
            keyboard_input_adapter->handle_keyboard_input(time);
        }
    }

    void handle_text(const time& time, char ch) {
        maybe<input::input_adapter*> keyboard_input_target;

        for(auto& view_pair : views) {
            if(view* v = std::get<1>(view_pair)) {
                if(input::input_adapter* input_adapter = v->get_input_adapter()) {
                    if(input_adapter->wants_keyboard_focus()) {
                        keyboard_input_target = input_adapter;
                    }
                }
            }
        }

        if(input::input_adapter* keyboard_input_adapter = keyboard_input_target) {
            keyboard_input_adapter->handle_text_input(time, ch);
        }
    }
};

}
