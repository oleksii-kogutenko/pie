#include "env.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace piel { namespace lib { namespace logger {

std::string Env::getEnv(std::string name, std::string def)
{
    const char *env = ::getenv(name.c_str());
    if (env) return env;
    return def;
}

bool Env::getEnv(std::string name, bool def)
{
    const char *env = ::getenv(name.c_str());
    if (env) {
        std::string senv = boost::to_lower_copy<std::string>(env);
        if (senv == "true") return true;
        return false;
    }
    return def;
}

int Env::getEnv(std::string name, int def)
{
    const char *env = ::getenv(name.c_str());
    if (env) {
        return std::stoi(env);
    }
    return def;
}

} } } // namespace piel::lib::logger
