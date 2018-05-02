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

#ifndef COMMANDS_SETCONFIG_H_
#define COMMANDS_SETCONFIG_H_

#include <workingcopycommand.h>

namespace piel { namespace cmd {

namespace errors {
    struct attempt_to_set_unsupported_config {};
};

class SetConfig: public WorkingCopyCommand
{
public:
    SetConfig(const piel::lib::WorkingCopy::Ptr& working_copy);
    virtual ~SetConfig();

    /*static*/ std::map<std::string,std::string> supported() const;

    void operator()();

    void set_global(bool global);
    void set_name(const std::string& prop);
    void set_value(const std::string& value);

    /* Supported configuration parameters */
    static piel::lib::Properties::DefaultFromEnv author;
    static piel::lib::Properties::DefaultFromEnv email;
    static piel::lib::Properties::DefaultFromEnv commiter;
    static piel::lib::Properties::DefaultFromEnv commiter_email;

private:
    bool        global_;
    std::string prop_to_set_;
    std::string value_to_set_;

};

} } // namespace piel::cmd

#endif /* COMMANDS_SETCONFIG_H_ */
