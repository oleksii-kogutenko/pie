#pragma once

#include <string>

namespace piel { namespace lib { namespace logger {

class Env {
public:
    static std::string getEnv(std::string name, std::string def);
    static bool getEnv(std::string name, bool def);
    static int getEnv(std::string name, int def);
};

} } } // namespace piel::lib::logger
