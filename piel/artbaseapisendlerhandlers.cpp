#include "artbaseapisendlerhandlers.h"
#include "artbaseconstants.h"
#include <cstring>
#include <logging.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtBaseApiSendlerHandlers,\
        true,             false,         true,           true,          false,          false);

namespace art { namespace lib {

ArtBaseApiSendlerHandlers::ArtBaseApiSendlerHandlers(const std::string& api_token)
    : ArtBaseApiHandlers(api_token)
    , attributes_()
    , send_size_(0)
    , os_()
    , answer_()
    , url_()
    , repo_()
    , path_()
    //, is_()

{
    LOGT << __PRETTY_FUNCTION__ << ELOG;
}

ArtBaseApiSendlerHandlers::ArtBaseApiSendlerHandlers(const std::string& api_token, const std::string& url, const std::string& repo, const std::string& path)
    : ArtBaseApiHandlers(api_token)
    , attributes_()
    , send_size_(0)
    , os_()
    , answer_()
    , url_()
    , repo_()
    , path_()
{
    set_url(url);
    set_repo(repo);
    set_path(path);
}

size_t ArtBaseApiSendlerHandlers::handle_input(char *ptr, size_t size)
{
    std::string str = os_.str();
    size_t os_size = str.length();
    size_t size_to_send = 0;

    LOGT <<  "size:" << size << " send_size_:" << send_size_ << " os_size:" << os_size << ELOG;

    if ( 0 == send_size_ ) {
        LOGT <<  " --1--" << ELOG;
        pt::ptree tree;

        for (Attributes::const_iterator i = attributes_.begin(), end = attributes_.end(); i != end; ++i)
        {
            tree.insert(tree.end(), std::make_pair(i->first, pt::ptree(i->second)));
        }

        gen_additional_tree(tree);

        pt::write_json(os_, tree, false);

        str = os_.str();
        os_size = str.length();

        send_size_ = str.copy(ptr, size, 0);

        size_to_send = send_size_;
    } else if (send_size_ == os_size) {
        LOGT  << __PRETTY_FUNCTION__ << " --2--" << ELOG;

        size_to_send = 0;
    } else {
        LOGT  << __PRETTY_FUNCTION__ << " --3--" << ELOG;

        size_to_send = str.copy(ptr, size, send_size_);
        send_size_ += size_to_send;

        LOGT << __PRETTY_FUNCTION__ << " --size_to_send:" << size_to_send << " --send_size_:" << send_size_ << ELOG;
    }

    return  size_to_send;
}

size_t ArtBaseApiSendlerHandlers::handle_output(char *ptr, size_t size)
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

std::string ArtBaseApiSendlerHandlers::trim(const std::string &src)
{
    std::string dest = src;
    boost::algorithm::trim(dest);
    return dest;
}

void ArtBaseApiSendlerHandlers::update_attributes(const std::string& key, const std::string& value)
{
    attributes_[key] = value;
}

void ArtBaseApiSendlerHandlers::update_attributes(const std::string& key, const char* value)
{
    attributes_[key] = value;
}

void ArtBaseApiSendlerHandlers::set_url(const std::string& url)
{
    LOGT << __PRETTY_FUNCTION__ << url << ELOG;
    url_ = trim(url);
    attributes_[ArtBaseConstants::uri_attribute] = get_url();

    LOGT << "get_url:" << get_url() << ELOG;
}

void ArtBaseApiSendlerHandlers::set_repo(const std::string& repo)
{
    LOGT << __PRETTY_FUNCTION__ << repo << ELOG;
    repo_ = trim(repo);
    attributes_[ArtBaseConstants::repo_attribute] = get_repo();

    LOGT << "get_repo:" << get_repo() << ELOG;
}

void ArtBaseApiSendlerHandlers::set_path(const std::string& path)
{
    LOGT << __PRETTY_FUNCTION__ << path << ELOG;
    path_ = trim(path);
    attributes_[ArtBaseConstants::path_attribute] = get_path();

    LOGT << "get_path:" << get_path() << ELOG;
}


std::string ArtBaseApiSendlerHandlers::gen_uri()
{
    std::string ret_val = get_url();

    ret_val.append(ArtBaseConstants::uri_delimiter).append(get_repo()).append(ArtBaseConstants::uri_delimiter).append(get_path());

    LOGT << "URI: " << ret_val << ELOG;

    return ret_val;
}

} } // namespace art::lib
