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

#include <artbasedeployartifactshandlers.h>

#include <artbaseconstants.h>
#include <cstring>
#include <logging.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtBaseDeployArtifactsHandlers,\
        true,             false,         true,           true,          false,          false);

namespace art { namespace lib {

ArtBaseDeployArtifactsHandlers::ArtBaseDeployArtifactsHandlers(const std::string& api_token)
    : ArtBaseApiHandlers(api_token)
    , tree_()
    , uploader_()
    , answer_()
    , url_()
    , repo_()
    , path_()
    , first_call_(true)

{
}

ArtBaseDeployArtifactsHandlers::ArtBaseDeployArtifactsHandlers(const std::string& api_token, const std::string& url, const std::string& repo, const std::string& path)
    : ArtBaseApiHandlers(api_token)
    , tree_()
    , uploader_()
    , answer_()
    , url_()
    , repo_()
    , path_()
    , first_call_(true)
{
    set_url(url);
    set_repo(repo);
    set_path(path);
}

void ArtBaseDeployArtifactsHandlers::push_input_stream(boost::shared_ptr<std::istream> is)
{
    uploader_.push_input_stream(is);
}

size_t ArtBaseDeployArtifactsHandlers::putto(char* ptr, size_t size)
{
    return uploader_.putto(ptr, size);
}

boost::shared_ptr<std::istream> ArtBaseDeployArtifactsHandlers::prepare_header()
{
    std::stringstream os;

    gen_additional_tree(tree_);

    pt::write_json(os, tree_, false);

    boost::shared_ptr<std::istream> is(new std::stringstream(os.str()));
    return is;
}

size_t ArtBaseDeployArtifactsHandlers::handle_input(char *ptr, size_t size)
{
    if (first_call_) {
        first_call_ = false;
        uploader_.push_input_stream(prepare_header());
    }
    return uploader_.putto(ptr, size);
}

size_t ArtBaseDeployArtifactsHandlers::handle_output(char *ptr, size_t size)
{
    std::stringstream read_os;
    pt::ptree tree;

    read_os.write(ptr, size);
    pt::read_json(read_os, tree);

    for(pt::ptree::const_iterator it = tree.begin(), end = tree.end(); it != end; ++it) {
        LOGT << "answer[" << (*it).first.c_str() << "]=" << (*it).second.data() << ELOG;

        answer_[(*it).first.c_str()] = (*it).second.data();
    }
    return size;
}

std::string ArtBaseDeployArtifactsHandlers::trim(const std::string &src)
{
    std::string dest = src;
    boost::algorithm::trim(dest);
    return dest;
}

void ArtBaseDeployArtifactsHandlers::update_attributes(const std::string& key, const std::string& value)
{
    tree_.insert(tree_.end(), std::make_pair(key, pt::ptree(value)));
}

void ArtBaseDeployArtifactsHandlers::update_attributes(const std::string& key, const char* value)
{
    tree_.insert(tree_.end(), std::make_pair(key, pt::ptree(value)));
}

void ArtBaseDeployArtifactsHandlers::set_url(const std::string& url)
{
    url_ = trim(url);
    std::string value = get_url();
    if (!value.empty()) {
        tree_.insert(tree_.end(), std::make_pair(
                         ArtBaseConstants::uri_attribute,
                         pt::ptree(value)));
    }
}

void ArtBaseDeployArtifactsHandlers::set_repo(const std::string& repo)
{
    repo_ = trim(repo);
    std::string value = get_repo();
    if (!value.empty()) {
        tree_.insert(tree_.end(), std::make_pair(
                         ArtBaseConstants::repo_attribute,
                         pt::ptree(value)));
    }
}

void ArtBaseDeployArtifactsHandlers::set_path(const std::string& path)
{
    path_ = trim(path);
    std::string value = get_path();
    if (!value.empty()) {
        tree_.insert(tree_.end(), std::make_pair(
                         ArtBaseConstants::path_attribute,
                         pt::ptree(value)));
    }
}

std::string ArtBaseDeployArtifactsHandlers::gen_uri()
{
    std::string ret_val;
    return ret_val.append(ArtBaseConstants::uri_delimiter).append(get_repo()).append(ArtBaseConstants::uri_delimiter).append(get_path());
}

} } // namespace art::lib
