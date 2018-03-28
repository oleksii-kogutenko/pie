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
#include <artdeployartifacthandlers.h>
#include <artbaseconstants.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost_property_tree_ext.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtDeployArtifactHandlers,\
        true,             false,         true,           true,          false,          false);

namespace art { namespace lib {

ArtDeployArtifactHandlers::ArtDeployArtifactHandlers(const ArtDeployArtifactHandlers& handler)
    : ArtBaseDeployArtifactsHandlers(handler)
    , str_digests_(handler.str_digests_)
    , name_(handler.name_)
    , version_(handler.version_)
    , classifier_(handler.classifier_)
{
}

ArtDeployArtifactHandlers::ArtDeployArtifactHandlers(const std::string& api_token)
    : ArtBaseDeployArtifactsHandlers(api_token)
    , str_digests_()
{
}

ArtDeployArtifactHandlers::ArtDeployArtifactHandlers(const std::string& api_token,
                                                     const std::string& url,
                                                     const std::string& repo,
                                                     const std::string& path,
                                                     const std::string& fname)
    : ArtBaseDeployArtifactsHandlers(api_token, url, repo, path)
    , str_digests_()
{
    file(fname);
}

ArtDeployArtifactHandlers::~ArtDeployArtifactHandlers()
{
}

void ArtDeployArtifactHandlers::file(const std::string& fname)
{
    LOGI << __PRETTY_FUNCTION__ << ":" << fname << ELOG;
    boost::shared_ptr<std::istream> file_ptr(new std::ifstream(fname));
    push_input_stream(file_ptr);

    std::ifstream in(fname, std::ifstream::ate | std::ifstream::binary);

    if (!in.is_open()) {
        LOGE << "Wrong to open file:" << fname << ELOG;
        return;
    }

    size_t file_size = in.tellg();

    in.seekg(0);

    piel::lib::ChecksumsDigestBuilder digest_builder;
    digest_builder.init();

    str_digests_ = digest_builder.str_digests_for(in);

    in.close();

    std::stringstream ss;

    ss << file_size;
    update_attributes(ArtBaseConstants::size, ss.str());
    update_attributes(ArtBaseConstants::mime_type, ArtBaseConstants::mime_type_text);
}

void ArtDeployArtifactHandlers::gen_additional_tree(boost::property_tree::ptree& tree)
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

size_t ArtDeployArtifactHandlers::handle_input(char *ptr, size_t size)
{
    return putto(ptr, size);
}

std::string ArtDeployArtifactHandlers::get_path()
{
    std::string p = ArtBaseDeployArtifactsHandlers::get_path();

    p.append(ArtBaseConstants::uri_delimiter)
     .append(get_name()).append(ArtBaseConstants::uri_delimiter)
     .append(get_version()).append(ArtBaseConstants::uri_delimiter)
     .append(get_name()).append("-")
     .append(get_version());

    std::string classifier = get_classifier();
    std::vector<std::string> name_ext;
    boost::split(name_ext, classifier, boost::is_any_of("."));

    if (name_ext[0].size()) p.append("-");

    p.append(classifier);

    LOGT << p << ELOG;
    return p;
}

} } // namespace art::lib
