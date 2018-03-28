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

#include <logging.h>
#include <artdeployartifactchecksumhandlers.h>
#include <artbaseconstants.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost_property_tree_ext.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtDeployArtifactCheckSumHandlers,\
        true,             false,         true,            true,           false,          false);

namespace art { namespace lib {

ArtDeployArtifactCheckSumHandlers::ArtDeployArtifactCheckSumHandlers(const ArtDeployArtifactHandlers &handler, std::string digest)
    : ArtDeployArtifactHandlers(handler)
    , digest_(digest)
{
}

ArtDeployArtifactCheckSumHandlers::~ArtDeployArtifactCheckSumHandlers()
{
}
/*
void ArtDeployArtifactCheckSumHandlers::gen_additional_tree(boost::property_tree::ptree& tree)
{
    pt::ptree checksum;
    checksum.insert(checksum.end(),
                    std::make_pair(
                        ArtBaseConstants::checksums_md5,
                        pt::ptree(str_digests_[piel::lib::Md5::t::name()])));
    checksum.insert(checksum.end(),
                    std::make_pair(
                        ArtBaseConstants::checksums_sha1,
                        pt::ptree(str_digests_[piel::lib::Sha::t::name()])));
    tree.add_child(ArtBaseConstants::checksums, checksum);
}
*/
boost::shared_ptr<std::istream> ArtDeployArtifactCheckSumHandlers::prepare_header()
{
    std::stringstream os;

    std::string check_name = (digest_ == art::lib::ArtBaseConstants::checksums_md5) ? piel::lib::Md5::t::name() : piel::lib::Sha::t::name();
    LOGI << "----- " << piel::lib::Md5::t::name() << "===" << digest_ << "-->" << check_name << ELOG;

    os << str_digests_[/*piel::lib::Md5::t::name()*/check_name];
    boost::shared_ptr<std::istream> is(new std::stringstream(os.str()));

    LOGI << __PRETTY_FUNCTION__ << "[" << digest_<< "]:" << os.str() << ELOG;
    return is;
}

size_t ArtDeployArtifactCheckSumHandlers::handle_input(char *ptr, size_t size)
{
    return ArtBaseDeployArtifactsHandlers::handle_input(ptr, size);
}

/*virtual*/ std::string ArtDeployArtifactCheckSumHandlers::gen_uri()
{
    std::string ret_val = ArtDeployArtifactHandlers::gen_uri().append(".").append(digest_);
    return ret_val;
}

} } // namespace art::lib
