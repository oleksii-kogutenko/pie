#include <cstdarg>
#include <cstdio>
#include <locale>
#include <time.h>
#include <ctime>
#include <iostream>

#include "utils.h"

namespace piel { namespace lib {

std::string Utils::format(char const * fmt, ...)
{
    va_list vl;
    int size;

    enum { INITIAL_BUFFER_SIZE = 512 };

    {
        char buffer[INITIAL_BUFFER_SIZE];

        va_start(vl, fmt);
        size = vsnprintf(buffer, INITIAL_BUFFER_SIZE, fmt, vl) + sizeof('\0');
        va_end(vl);

        if (size < INITIAL_BUFFER_SIZE)
            return std::string(buffer, size);
    }

    //size += sizeof('\0');

    char buffer[size];

    va_start(vl, fmt);
    size = vsnprintf(buffer, size, fmt, vl);
    va_end(vl);

    return std::string(buffer, size);
}

std::string Utils::timeToStr(char const * fmt, std::time_t t, std::string& str)
{
    char time_str[80];
    struct tm* ptm;
    ptm = std::localtime(&t);
    std::strftime(time_str, sizeof(time_str), fmt, ptm);
    str = time_str;
    return str;
}

std::string Utils::timeToStr(char const * fmt, std::time_t t)
{
    std::string str;
    return timeToStr(fmt, t, str);
}

std::string Utils::timeToStr(char const * fmt)
{
    std::string str;
    return timeToStr(fmt, std::time(nullptr), str);
}


} } // namespace piel::lib

