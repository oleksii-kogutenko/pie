#include <cstdarg>
#include <cstdio>
#include <locale>
#include <time.h>
#include <ctime>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <iostream>

#include "utils.h"

namespace piel { namespace lib { namespace utils {

//std::string const  DEFAULT_EMAIL_FROM = "scm_support@dev.zodiac.tv";
//std::string const  DEFAULT_EMAIL_TO = "cm@maillist.dev.zodiac.tv";
//std::string const  DEFAULT_EMAIL_MESSAGE = "";
//std::string const  DEFAULT_EMAIL_SUBJECT = "";
//std::string const  DEFAULT_EMAIL_HOSTNAME = "smtp.dev.zodiac.tv";

std::string timeToStr(char const * fmt, std::time_t t, std::string& str)
{
    char time_str[80];
    struct tm* ptm;
    ptm = std::localtime(&t);
    std::strftime(time_str, sizeof(time_str), fmt, ptm);
    str = time_str;
    return str;
}

std::string timeToStr(char const * fmt, std::time_t t)
{
    std::string str;
    return timeToStr(fmt, t, str);
}

std::string timeToStr(char const * fmt)
{
    std::string str;
    return timeToStr(fmt, std::time(nullptr), str);
}

std::string getEnv(std::string name, std::string def)
{
    const char *env = ::getenv(name.c_str());
    if (env) return env;
    return def;
}

bool getEnv(std::string name, bool def)
{
    const char *env = ::getenv(name.c_str());
    if (env) {
        std::string senv = boost::to_lower_copy<std::string>(env);
        if (senv == "true") return true;
        return false;
    }
    return def;
}

int getEnv(std::string name, int def)
{
    const char *env = ::getenv(name.c_str());
    if (env) {
        return std::stoi(env);
    }
    return def;
}

std::string replace(std::string str, std::string from, std::string to)
{
    return boost::replace_all_copy(str, from, to);
}

std::string normalize(std::string loggerName)
{
    return replace(replace(loggerName, "-", "_"), " ", "_");
}

void sendEmail(std::string to, std::string from, std::string subject, std::string message)
{
/*
        Email email = new SimpleEmail();
        email.setHostName(DEFAULT_EMAIL_HOSTNAME);
        email.setFrom(from == null ? DEFAULT_EMAIL_FROM : from);
        email.setSubject(subject == null ? DEFAULT_EMAIL_SUBJECT : subject);
        email.setMsg(message == null ? DEFAULT_EMAIL_MESSAGE : message);

        if (to == null) {
            to = DEFAULT_EMAIL_TO;
        }
        String[] toArray = to.split(";");
        for (String toItem : toArray) {
            email.addTo(toItem);
        }

        email.send();
*/
}
void sendEmail(std::string subject, std::string message)
{
    //sendEmail(DEFAULT_EMAIL_TO, DEFAULT_EMAIL_FROM, subject, message);
}

} } } // namespace piel::lib:utils

