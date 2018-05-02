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

#ifndef COMMANDS_COMMIT_H_
#define COMMANDS_COMMIT_H_

#include <workingcopycommand.h>
#include <setconfig.h>
#include <indexesdiff.h>

namespace piel { namespace cmd {

namespace errors {
    struct nothing_to_commit {};
};

class Commit: public WorkingCopyCommand
{
public:
    Commit(const piel::lib::WorkingCopy::Ptr& working_copy);
    virtual ~Commit();

    std::string operator()();

    const Commit* set_message(const std::string& message);

protected:
    piel::lib::IndexesDiff diff(const piel::lib::TreeIndex::Ptr& current_index) const;

private:
    std::string message_;
};

} } // namespace piel::cmd

#endif /* COMMANDS_COMMIT_H_ */
