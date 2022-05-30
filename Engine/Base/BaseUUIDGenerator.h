#pragma once
#include <random>
#include <sstream>

// https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library

namespace uuid {
    std::string generate_uuid();
}