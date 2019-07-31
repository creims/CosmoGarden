#ifndef EM_WRAPPER_H
#define EM_WRAPPER_H

#include <functional>
#include <emscripten/bind.h>

template <typename T>
class em_wrapper {
public:
    em_wrapper() = default;
    void set(T _t) { t = _t; }
    T get() { return t; }

    void setUpdate(emscripten::val newUpdate) {
        update = newUpdate;
    }

    std::function<T()> getCallback() {
        return [this] {
            this->update();
            return this->get();
        };
    }

private:
    T t{};
    emscripten::val update{emscripten::val::undefined()};
};


#endif //EM_WRAPPER_H
