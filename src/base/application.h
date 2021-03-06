#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include "base/utility/event_bus.h"
#include "base/utility/time.h"
#include "base/io/file_system.h"
#include "base/content/content_manager.h"
#include "base/place/place_controller.h"
#include "base/view_stack.h"
#include "base/content/assets/shader.h"
#include "content/assets/colormap.h"
#include "base/graphics/default_render_target.h"
#include "base/graphics/texture_render_target.h"
#include "base/diagnostics/report.h"

#include "base/events/exit.h"
#include "base/events/exiting.h"
#include "base/events/terminated.h"
#include "base/events/resized.h"
#include "base/events/window_focus.h"

namespace gorc {

template <typename LayerIdT, typename PresenterT, typename MapperT> class application {
public:
    event_bus eventbus;

protected:
    diagnostics::report& report;
    const io::file_system& filesystem;

private:
    content::content_manager master_content_manager;
    const uint32_t gameplay_timestep_ms;
    std::unique_ptr<graphics::default_render_target> default_target;
    std::unique_ptr<graphics::texture_render_target> texture_target;

    const content::assets::shader* screen_shader = nullptr;
    GLint screen_shader_diffuse_ul = 0;

public:
    const double gameplay_timestep;
    sf::Window window;
    view_stack<LayerIdT> views;
    place::place_controller<PresenterT, MapperT> place_controller;

    application(const std::string& title, const MapperT& place_mapper, diagnostics::report& report, const io::file_system& filesystem,
            const box<2, int>& window_size = make_box(make_vector(0, 0), make_vector(1280, 720)), bool windowed = true,
            double gameplay_timestep = (1.0 / 60.0))
        : report(report), filesystem(filesystem), master_content_manager(report, filesystem),
          gameplay_timestep_ms(static_cast<uint32_t>(gameplay_timestep * 1000.0)),
          gameplay_timestep(gameplay_timestep),
          window(sf::VideoMode(get_size<0>(window_size), get_size<1>(window_size), 32), title, windowed ? sf::Style::Default : sf::Style::Fullscreen,
                  sf::ContextSettings(24, 8, 0)),
          views(window), place_controller(eventbus, place_mapper) {
        window.setActive(true);
        window.setVerticalSyncEnabled(true);

        // Initialize GLEW
        GLenum err = glewInit();
        if(err != GLEW_OK) {
            // TODO: Print error
            throw std::exception();
        }

        screen_shader = &master_content_manager.load<content::assets::shader>("screen.glsl");
        screen_shader_diffuse_ul = glGetUniformLocation(screen_shader->program, "diffuse");

        texture_target = make_unique<graphics::texture_render_target>(get_view_size());
        default_target = make_unique<graphics::default_render_target>(get_screen_size());

        return;
    }

    virtual ~application() {
        return;
    }

    virtual void startup(event_bus& eventbus, content::content_manager& content) = 0;
    virtual void shutdown() = 0;

    virtual void update(const time&, const box<2, int>&) {
        return;
    }

    box<2, int> get_screen_size() const {
        return make_box(make_vector(0, 0), make_vector<int>(window.getSize().x, window.getSize().y));
    }

    virtual box<2, int> get_view_size() const {
        return get_screen_size();
    }

private:
    void draw(const time& time, const box<2, int>& view_size) {
        window.setActive(true);

        // Draw views using texture render target.
        graphics::set_render_target(*texture_target);
        views.draw(time, view_size, *texture_target);

        // Draw texture
        graphics::set_render_target(*default_target);

        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);

        glUseProgram(screen_shader->program);
        glUniform1i(screen_shader_diffuse_ul, 0);

        glActiveTexture(GL_TEXTURE0);
        graphics::bind_texture(*texture_target);

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 0.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 0.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);

        glEnd();

        window.display();
    }

    void internal_update(const time& time, const box<2, int>& view_size) {
        auto sf_mouse_pos = sf::Mouse::getPosition(window);
        auto screen_size = get_screen_size();

        double width_fac = static_cast<double>(get_size<0>(view_size)) / static_cast<double>(get_size<0>(screen_size));
        double height_fac = static_cast<double>(get_size<1>(view_size)) / static_cast<double>(get_size<1>(screen_size));

        auto mouse_pos = make_vector(static_cast<int>(static_cast<double>(sf_mouse_pos.x) * width_fac),
                static_cast<int>(static_cast<double>(sf_mouse_pos.y) * height_fac));

        views.handle_input(time, mouse_pos);

        if(PresenterT* curr_presenter = place_controller.current_presenter()) {
            curr_presenter->update(time);
        }

        update(time, view_size);
    }

public:
    void run() {
        bool running = true;

        // Register core event handlers
        eventbus.add_handler<events::exit>([this, &running](events::exit&) {
            // A component has requested application exit.
            // Check if components can quit.
            events::exiting exiting_event;
            eventbus.fire_event(exiting_event);

            if(exiting_event.cancel) {
                return;
            }

            // All components can quit. Request shutdown.
            eventbus.fire_event(events::terminated());
            running = false;
        });

        // Run user startup
        startup(eventbus, master_content_manager);

        // game loop
        sf::Clock elapsed_clock;
        sf::Clock total_clock;
        uint32_t gameplay_accumulator = 0;

        auto view_size = get_view_size();
        views.resize(view_size);
        *texture_target = graphics::texture_render_target(view_size);
        *default_target = graphics::default_render_target(get_screen_size());

        sf::Event event;
        while(running) {
            while(window.pollEvent(event)) {
                switch(event.type) {
                case sf::Event::Closed:
                    eventbus.fire_event(events::exit());
                    break;

                case sf::Event::Resized: {
                        auto new_view_size = get_view_size();
                        if(new_view_size != view_size) {
                            view_size = new_view_size;
                            eventbus.fire_event(events::resized(view_size));
                            *texture_target = graphics::texture_render_target(view_size);
                            *default_target = graphics::default_render_target(get_screen_size());
                            views.resize(view_size);
                        }
                    }
                    break;

                case sf::Event::TextEntered: {
                        char ch = static_cast<char>(event.text.unicode);
                        if(ch >= 0x20 && ch < 0x7F) {
                            views.handle_text(time(timestamp(total_clock.getElapsedTime().asMilliseconds()), timestamp(elapsed_clock.getElapsedTime().asMilliseconds())), ch);
                        }
                    }
                    break;

                case sf::Event::GainedFocus:
                    eventbus.fire_event(events::window_focus(true));
                    break;

                case sf::Event::LostFocus:
                    eventbus.fire_event(events::window_focus(false));
                    break;

                default:
                    // Silently consume unhandled events.
                    break;
                }
            }

            draw(time(timestamp(total_clock.getElapsedTime().asMilliseconds()), timestamp(elapsed_clock.getElapsedTime().asMilliseconds())), view_size);

            gameplay_accumulator += elapsed_clock.restart().asMilliseconds();
            while(gameplay_accumulator >= gameplay_timestep_ms) {
                int begin_ms = total_clock.getElapsedTime().asMilliseconds();
                int end_ms = begin_ms + gameplay_timestep_ms;
                internal_update(time(timestamp(end_ms), timestamp(begin_ms)), view_size);
                gameplay_accumulator -= gameplay_timestep_ms;
            }
        }
    }
};

}
