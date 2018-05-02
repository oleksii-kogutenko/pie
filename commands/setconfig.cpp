/*
 * Copyright (c) 2018, diakovliev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY diakovliev ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL diakovliev BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <setconfig.h>

namespace piel { namespace cmd {

/*static*/ piel::lib::Properties::DefaultFromEnv SetConfig::author =
        piel::lib::Properties::Property("author",         "unknown", "Commit author.").default_from_env("PIE_AUTHOR");

/*static*/ piel::lib::Properties::DefaultFromEnv SetConfig::email =
        piel::lib::Properties::Property("email",          "unknown", "Commit author email.").default_from_env("PIE_EMAIL");

/*static*/ piel::lib::Properties::DefaultFromEnv SetConfig::commiter =
        piel::lib::Properties::Property("commiter",       "unknown", "Commiter.").default_from_env("PIE_COMMITER");

/*static*/ piel::lib::Properties::DefaultFromEnv SetConfig::commiter_email =
        piel::lib::Properties::Property("commiter_email", "unknown", "Commiter email.").default_from_env("PIE_COMMITER_EMAIL");

SetConfig::SetConfig(const piel::lib::WorkingCopy::Ptr& working_copy)
    : WorkingCopyCommand(working_copy)
    , global_(false)
{
}

SetConfig::~SetConfig()
{
}

/*static*/ std::map<std::string,std::string>& SetConfig::supported()
{
    static std::map<std::string,std::string> result;
    if (result.empty()) {
        result.insert(std::make_pair(author.name(), author.description()));
        result.insert(std::make_pair(email.name(), email.description()));
        result.insert(std::make_pair(commiter.name(), commiter.description()));
        result.insert(std::make_pair(commiter_email.name(), commiter_email.description()));
    }
    return result;
}

void SetConfig::set_global(bool global)
{
    global_ = global;
}

void SetConfig::set_name(const std::string& prop)
{
    prop_to_set_ = prop;
}

void SetConfig::set_value(const std::string& value)
{
    value_to_set_ = value;
}

void SetConfig::operator()()
{
    if (supported().end() == supported().find(prop_to_set_)) {
        throw errors::attempt_to_set_unsupported_config();
    }

    working_copy()->set_config(prop_to_set_, value_to_set_);
}

} } // namespace piel::cmd
