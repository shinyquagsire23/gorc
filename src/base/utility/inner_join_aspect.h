#pragma once

#include "aspect.h"
#include "entity_id.h"
#include "component_system.h"
#include <type_traits>

namespace gorc {
inline namespace utility {

template <typename HeadCompT, typename... CompT> class inner_join_aspect : public aspect {
public:
    virtual void update(gorc::time, entity_id, HeadCompT&, CompT&...) {
        return;
    }

    virtual void draw(gorc::time, entity_id, HeadCompT&, CompT&...) {
        return;
    }

protected:
    component_system& cs;

private:
    template <size_t n, typename HeadT, typename... TailT> struct nth_param {
    public:
        using type = typename nth_param<n - 1, TailT...>::type;
    };

    template <typename HeadT, typename... TailT> struct nth_param<0, HeadT, TailT...> {
    public:
        using type = HeadT;
    };

    template <typename... ArgT>
    void update_over_ranges(gorc::time time, typename std::enable_if<sizeof...(ArgT) == sizeof...(CompT) + 1, entity_id>::type id, ArgT&... args) {
        update(time, id, args...);
    }

    template <typename... ArgT>
    void update_over_ranges(gorc::time time, typename std::enable_if<sizeof...(ArgT) != sizeof...(CompT) + 1, entity_id>::type id, ArgT&... args) {
        auto rng = cs.find_component<typename nth_param<sizeof...(ArgT), HeadCompT, CompT...>::type>(id);
        for(auto it = rng.begin(); it != rng.end();) {
            auto jt = it;
            ++it;
            update_over_ranges(time, id, args..., jt->second);
        }
    }

    template <typename... ArgsT> struct register_component_types;

    template <typename HeadT, typename... TailT> struct register_component_types<HeadT, TailT...> {
    public:
        register_component_types(component_system& cs) {
            cs.register_component_type<HeadT>();
            register_component_types<TailT...> rct(cs);
        }
    };

    template <typename LastT> struct register_component_types<LastT> {
    public:
        register_component_types(component_system& cs) {
            cs.register_component_type<LastT>();
        }
    };

public:
    inner_join_aspect(component_system& cs) : cs(cs) {
        register_component_types<HeadCompT, CompT...> rct(cs);

        return;
    }

    virtual void update(gorc::time time) override {
        auto rng = cs.all_components<HeadCompT>();
        for(auto it = rng.begin(); it != rng.end();) {
            auto jt = it;
            ++it;
            update_over_ranges(time, jt->first, jt->second);
        }
    }
};

}
}
