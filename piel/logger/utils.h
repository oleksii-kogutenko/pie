#pragma once
#include <string>
#include <ctime>

namespace piel { namespace lib { namespace utils {

std::string timeToStr(char const * fmt, std::time_t t, std::string &str);
std::string timeToStr(char const * fmt, std::time_t t);
std::string timeToStr(char const * fmt);

std::string getEnv(std::string name, std::string def);
bool getEnv(std::string name, bool def);
int getEnv(std::string name, int def);

std::string replace(std::string str, std::string from, std::string to);
std::string normalize(std::string loggerName);
void sendEmail(std::string to, std::string from, std::string subject, std::string message);
void sendEmail(std::string subject, std::string message);

} } } // namespace piel::lib::utils
