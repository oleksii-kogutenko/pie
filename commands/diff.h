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

#ifndef COMMANDS_DIFF_H_
#define COMMANDS_DIFF_H_

#include <workingcopycommand.h>
#include <iostreamsholder.h>

namespace piel { namespace cmd {

namespace errors {
    struct can_not_resolve_non_empty_reference {
        std::string ref;
        can_not_resolve_non_empty_reference(const std::string& ref_) : ref(ref_) {}
    };
}

class Diff: public WorkingCopyCommand, public piel::lib::IOstreamsHolder
{
public:
    Diff(const piel::lib::WorkingCopy::Ptr& working_copy, const boost::optional<piel::lib::refs::Range>& range);
    virtual ~Diff();

    void operator()();

protected:
    piel::lib::TreeIndex::Ptr resolve_ref(const std::string& ref, const piel::lib::TreeIndex::Ptr& def_index);
    piel::lib::TreeIndex::Ptr working_dir_state();

private:
    boost::optional<piel::lib::refs::Range> range_;
    piel::lib::TreeIndex::Ptr working_dir_state_;

};

} } // namespace piel::cmd

#endif /* COMMANDS_DIFF_H_ */
