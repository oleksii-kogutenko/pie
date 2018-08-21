/*
 * Copyright (c) 2017-2018
 *
 *  Dmytro Iakovliev daemondzk@gmail.com
 *  Oleksii Kogutenko https://github.com/oleksii-kogutenko
 *
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
 * THIS SOFTWARE IS PROVIDED BY Dmytro Iakovliev daemondzk@gmail.com ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Dmytro Iakovliev daemondzk@gmail.com BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ARTBASEAPIDEPLOYARTIFACTCHECSUMHANDLERS_H
#define ARTBASEAPIDEPLOYARTIFACTCHECSUMHANDLERS_H

#include <checksumsdigestbuilder.hpp>
#include <artdeployartifacthandlers.h>

#include <fstream>
#include <artbasedeployartifactshandlers.h>
#include <datasequencecutter.h>

namespace art { namespace lib {

class ArtDeployArtifactCheckSumHandlers : public ArtDeployArtifactHandlers
{
public:
    ArtDeployArtifactCheckSumHandlers(ArtDeployArtifactHandlers *processed_handler, const std::string& digest);
    virtual ~ArtDeployArtifactCheckSumHandlers();

    virtual std::string gen_uri();

protected:
    std::string basic_digest_name(const std::string& digest) const;

private:
    std::string                 digest_;
    ArtDeployArtifactHandlers   *processed_handler_;
};

} } // namespace art::lib

#endif // ARTBASEAPIDEPLOYARTIFACTCHECSUMHANDLERS_H
