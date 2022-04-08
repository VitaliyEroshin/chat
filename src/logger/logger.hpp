#pragma once
#include <vector>
#include <iostream>

class Logger {
public:
    Logger(std::initializer_list<std::ostream*> streams);

    template<typename T>
    Logger& operator<<(T object);

private:
    std::vector<std::ostream*> s;
};

template<typename T>
Logger& Logger::operator<<(T object) {
    for (auto &out : s) {
        *out << object;
    }
    return *this;
}