#pragma once

template <typename T>
class RefStateGuard {
public:
    RefStateGuard(T*& state, T* new_state)
        : state(state), old(state)
    {
        state = new_state;
    }

    ~RefStateGuard() {
        state = old;
    }

private:
    T*& state;
    T* old;
};
