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
    , tree_()
    , uploader_()
    //, attributes_()
    , send_size_(0)
    , answer_()
    , url_()
    , repo_()
    , path_()
    , first_call_(true)

{
    LOGT << __PRETTY_FUNCTION__ << ELOG;
}

ArtBaseApiSendlerHandlers::ArtBaseApiSendlerHandlers(const std::string& api_token, const std::string& url, const std::string& repo, const std::string& path)
    : ArtBaseApiHandlers(api_token)
    , tree_()
    , uploader_()
    //, attributes_()
    , send_size_(0)
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

void ArtBaseApiSendlerHandlers::push_input_stream(boost::shared_ptr<std::istream> is)
{
    uploader_.push_input_stream(is);
}

size_t ArtBaseApiSendlerHandlers::putto(char* ptr, size_t size)
{
    return uploader_.putto(ptr, size);
}

boost::shared_ptr<std::istream> ArtBaseApiSendlerHandlers::prepare_header()
{
    std::stringstream os;

    gen_additional_tree(tree_);

    pt::write_json(os, tree_, false);

    boost::shared_ptr<std::istream> is(new std::stringstream(os.str()));
    return is;
}

size_t ArtBaseApiSendlerHandlers::handle_input(char *ptr, size_t size)
{
    if (first_call_) {
        first_call_ = false;
        uploader_.push_input_stream(prepare_header());
    }
    return  uploader_.putto(ptr, size);
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
    tree_.insert(tree_.end(), std::make_pair(key, pt::ptree(value)));
}

void ArtBaseApiSendlerHandlers::update_attributes(const std::string& key, const char* value)
{
    tree_.insert(tree_.end(), std::make_pair(key, pt::ptree(value)));
}

void ArtBaseApiSendlerHandlers::set_url(const std::string& url)
{
    LOGT << __PRETTY_FUNCTION__ << url << ELOG;
    url_ = trim(url);
    std::string value = get_url();
    if (!value.empty()) {
        tree_.insert(tree_.end(), std::make_pair(
                         ArtBaseConstants::uri_attribute,
                         pt::ptree(value)));
    }

    LOGT << "set_url:" << get_url() << ELOG;
}

void ArtBaseApiSendlerHandlers::set_repo(const std::string& repo)
{
    LOGT << __PRETTY_FUNCTION__ << repo << ELOG;
    repo_ = trim(repo);
    std::string value = get_repo();
    if (!value.empty()) {
        tree_.insert(tree_.end(), std::make_pair(
                         ArtBaseConstants::repo_attribute,
                         pt::ptree(value)));
    }

    LOGT << "set_repo:" << get_repo() << ELOG;
}

void ArtBaseApiSendlerHandlers::set_path(const std::string& path)
{
    LOGT << __PRETTY_FUNCTION__ << path << ELOG;
    path_ = trim(path);
    std::string value = get_path();
    if (!value.empty()) {
        tree_.insert(tree_.end(), std::make_pair(
                         ArtBaseConstants::path_attribute,
                         pt::ptree(value)));
    }

    LOGT << "set_path:" << get_path() << ELOG;
}


std::string ArtBaseApiSendlerHandlers::gen_uri()
{
    std::string ret_val = get_url();

    ret_val.append(ArtBaseConstants::uri_delimiter).append(get_repo()).append(ArtBaseConstants::uri_delimiter).append(get_path());

    LOGT << "URI: " << ret_val << ELOG;

    return ret_val;
}

} } // namespace art::lib
