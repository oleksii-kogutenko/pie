#pragma once
#include <string>
#include <ctime>

namespace piel { namespace lib {

class Utils {
public:
    static std::string format(char const * fmt, ...);
    static std::string timeToStr(char const * fmt, std::time_t t, std::string &str);
    static std::string timeToStr(char const * fmt, std::time_t t);
    static std::string timeToStr(char const * fmt);
};

} } // namespace piel::lib
