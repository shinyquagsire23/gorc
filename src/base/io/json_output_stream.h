#pragma once

#include <cstdio>
#include <type_traits>
#include <iterator>
#include <stack>
#include "input_stream.h"
#include "output_stream.h"
#include "file.h"
#include "exception.h"

namespace gorc {
namespace io {

class json_output_stream {
    template <typename T> friend typename std::enable_if<std::is_fundamental<T>::value>::type json_serialize(json_output_stream&, const T&);
    friend void json_serialize(json_output_stream&, const std::string&);
    template <typename RngT, typename EmFmtFn> friend void json_serialize_array(json_output_stream& f, const RngT& rng, EmFmtFn efn);
    template <typename RngT> friend void json_serialize_array(json_output_stream& f, const RngT& rng);
    friend void json_serialize_null(json_output_stream&);
    template <typename EmFmtFn> friend void json_serialize_member(json_output_stream&, const char*, EmFmtFn);
    template <typename ObjT> friend void json_serialize_object(json_output_stream&, const ObjT&);

private:
    enum class state_mode {
        root,
        array,
        object
    };

    struct state {
        state_mode mode;
        bool needs_comma;
        bool awaiting_content;

        state(state_mode mode);
    };

    output_stream& f;
    std::stack<state> states;
    int tab_level = 0;

    inline void print_tabs() {
        for(int i = 0; i < tab_level; ++i) {
            f.write("  ", 2);
        }
    }

    inline void print_raw_string(const char* str) {
        const char* end = str;
        while(*end) ++end;

        f.write(str, end - str);
    }

    void print_json_escaped_string(const char* str);
    void print_integer(int64_t value);
    void print_double(double value);

    inline void begin_field() {
        // Called immediately before printing some element.

        if(states.top().awaiting_content) {
            // Field has already been started.
            return;
        }

        // Make sure this isn't a second root element.
        if(states.top().mode == state_mode::root && states.top().needs_comma) {
            throw json_illegal_element_exception();
        }

        if(states.top().needs_comma) {
            print_raw_string(",");
        }

        if(states.top().mode != state_mode::root) {
            print_raw_string("\n");
        }

        print_tabs();

        states.top().needs_comma = false;
        states.top().awaiting_content = true;
    }

    inline void begin_object_element(const char* name) {
        // Make sure this is object, and not awaiting content.
        if(states.top().awaiting_content || states.top().mode != state_mode::object) {
            throw json_illegal_element_exception();
        }

        begin_field();
        print_json_escaped_string(name);
        print_raw_string(" : ");
    }

    inline void end_field() {
        // Called immediately after printing some element.
        states.top().awaiting_content = false;
        states.top().needs_comma = true;
    }

    inline void assert_element_label_printed() {
        // Asserts that the obj element name has been printed
        // if this is an object state.
        if(!states.top().awaiting_content && states.top().mode == state_mode::object) {
            throw json_illegal_element_exception();
        }
    }

    inline void assert_not_root() {
        // Asserts that the current node is not root.
        // Constraining the types of nodes which can appear inside the root node.
        if(states.top().mode == state_mode::root) {
            throw json_illegal_element_exception();
        }
    }

    inline void begin_object() {
        assert_element_label_printed();
        begin_field();

        print_raw_string("{");
        ++tab_level;
        states.emplace(state_mode::object);
    }

    inline void end_object() {
        --tab_level;
        print_raw_string("\n");
        print_tabs();
        print_raw_string("}");

        states.pop();
        end_field();
    }

    inline void begin_array() {
        assert_element_label_printed();
        begin_field();

        print_raw_string("[");
        ++tab_level;
        states.emplace(state_mode::array);
    }

    inline void end_array() {
        --tab_level;
        print_raw_string("\n");
        print_tabs();
        print_raw_string("]");

        states.pop();
        end_field();
    }

    inline void print_null() {
        assert_not_root();
        assert_element_label_printed();
        begin_field();
        print_raw_string("null");
        end_field();
    }

    inline void print_value(bool value) {
        assert_not_root();
        assert_element_label_printed();
        begin_field();
        print_raw_string(value ? "true" : "false");
        end_field();
    }

    inline void print_value(const std::string& value) {
        assert_not_root();
        assert_element_label_printed();
        begin_field();
        print_json_escaped_string(value.c_str());
        end_field();
    }

    template <typename T> typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value>::type
    print_value(T value) {
        assert_not_root();
        assert_element_label_printed();
        begin_field();
        print_integer(value);
        end_field();
    }

    template <typename T> typename std::enable_if<std::is_floating_point<T>::value>::type print_value(T value) {
        assert_not_root();
        assert_element_label_printed();
        begin_field();
        print_double(value);
        end_field();
    }

public:
    explicit json_output_stream(output_stream& f);
};

template <typename T> typename std::enable_if<std::is_fundamental<T>::value>::type json_serialize(json_output_stream& f, const T& v) {
    f.print_value(v);
}

inline void json_serialize(json_output_stream& f, const std::string& v) {
    f.print_value(v);
}

inline void json_serialize_null(json_output_stream& f) {
    f.print_null();
}

template <typename RngT, typename EmFmtFn> void json_serialize_array(json_output_stream& f, const RngT& rng, EmFmtFn efn) {
    f.begin_array();

    for(const auto& em : rng) {
        efn(em);
    }

    f.end_array();
}

template <typename RngT> void json_serialize_array(json_output_stream& f, const RngT& rng) {
    f.begin_array();

    for(const auto& em : rng) {
        json_serialize(f, em);
    }

    f.end_array();
}

template <typename ObjT> void json_serialize_object(json_output_stream& f, const ObjT& obj) {
    f.begin_object();

    obj.json_serialize(f);

    f.end_object();
}

template <typename EmFmtFn> void json_serialize_member(json_output_stream& f, const char* name, EmFmtFn efn) {
    f.begin_object_element(name);
    efn();
}

}
}
