#pragma once
#include <stdexcept>
#include <string>


class NotImplemented : public std::logic_error {
public:
    explicit NotImplemented(const std::string& message = "Not Implemented") : std::logic_error(message) {}
};
