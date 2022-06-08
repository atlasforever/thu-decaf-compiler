#include <iostream>
#include "error.h"

bool isIntegerTooLarge(const std::string &input)
{
    try {
        std::stoi(input, nullptr, 0);
    } catch (std::out_of_range e) {
        return true;
    }
    return false;
}