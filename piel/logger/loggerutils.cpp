#include "loggerutils.h"
#include <boost/algorithm/string/replace.hpp>

namespace piel { namespace lib { namespace logger {

std::string const  LoggerUtils::DEFAULT_EMAIL_FROM = "scm_support@dev.zodiac.tv";
std::string const  LoggerUtils::DEFAULT_EMAIL_TO = "cm@maillist.dev.zodiac.tv";
std::string const  LoggerUtils::DEFAULT_EMAIL_MESSAGE = "";
std::string const  LoggerUtils::DEFAULT_EMAIL_SUBJECT = "";
std::string const  LoggerUtils::DEFAULT_EMAIL_HOSTNAME = "smtp.dev.zodiac.tv";


std::string LoggerUtils::replace(std::string str, std::string from, std::string to)
{
    return boost::replace_all_copy(str, from, to);
}

std::string LoggerUtils::normalize(std::string loggerName)
{
    return replace(replace(loggerName, "-", "_"), " ", "_");
}

void LoggerUtils::sendEmail(std::string to, std::string from, std::string subject, std::string message)
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
void LoggerUtils::sendEmail(std::string subject, std::string message)
{
    sendEmail(DEFAULT_EMAIL_TO, DEFAULT_EMAIL_FROM, subject, message);
}

} } } // namespace piel::lib::logger
