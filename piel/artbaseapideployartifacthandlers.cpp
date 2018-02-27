#include "artbaseapideployartifacthandlers.h"
#include "artbaseconstants.h"
#include <cstring>
#include <logging.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtBaseApiDeployArtifactHandlers,\
        true,             false,         true,           true,          false,          false);

namespace art { namespace lib {

ArtBaseApiDeployArtifactHandlers::ArtBaseApiDeployArtifactHandlers(const std::string& api_token)
    : ArtBaseApiSendlerHandlers(api_token)
    , file_()
    , file_send_size_(0)
{
    LOGT << ELOG;
}

ArtBaseApiDeployArtifactHandlers::ArtBaseApiDeployArtifactHandlers(const std::string& api_token,
                                                                   const std::string& url,
                                                                   const std::string& repo,
                                                                   const std::string& path,
                                                                   const std::string& fname)
    : ArtBaseApiSendlerHandlers(api_token, url, repo, path)
    , file_()
    , file_send_size_(0)
{
    LOGT << ELOG;
    file(fname);
}

ArtBaseApiDeployArtifactHandlers::~ArtBaseApiDeployArtifactHandlers()
{
    LOGT << ELOG;
    if (file_.is_open())
    {
        file_.close();
    }
}

void ArtBaseApiDeployArtifactHandlers::file(const std::string& fname)
{
    LOGT << ELOG;
    file_.open(fname);
    if (!file_.is_open())
    {
        LOGE << "Wrong to open file:" << fname << ELOG;
    } else {
        std::stringstream ss;
        std::ifstream in(fname, std::ifstream::ate | std::ifstream::binary);
        file_size_ = in.tellg();
        in.close();
        ss << file_size_;
        LOGT  << __PRETTY_FUNCTION__ << fname << " file_size_:" << file_size_ << ELOG;
        update_attributes(ArtBaseConstants::size, ss.str());
        update_attributes(ArtBaseConstants::mem_type, "application/text");
    }
}

void ArtBaseApiDeployArtifactHandlers::sha1(const std::string& sha1)
{
    LOGT << ELOG;
    sha1_ = sha1;
}

void ArtBaseApiDeployArtifactHandlers::md5(const std::string& md5)
{
    LOGT << ELOG;
    md5_ = md5;
}

void ArtBaseApiDeployArtifactHandlers::gen_additional_tree(boost::property_tree::ptree& tree)
{
    LOGT << ELOG;
    pt::ptree checksum;
    checksum.insert(checksum.end(), std::make_pair(ArtBaseConstants::checksums_md5, pt::ptree(md5_)));
    checksum.insert(checksum.end(), std::make_pair(ArtBaseConstants::checksums_sha1, pt::ptree(sha1_)));
    tree.add_child(ArtBaseConstants::checksums, checksum);
}

size_t ArtBaseApiDeployArtifactHandlers::handle_input(char *ptr, size_t size)
{
    LOGT << ELOG;
    size_t ret_val = 0;//ArtBaseApiSendlerHandlers::handle_input(ptr, size);
    if (!ret_val) {
        if (0 == file_send_size_) {
            LOGT  << __PRETTY_FUNCTION__ << " --1--" << ELOG;
            file_send_size_ = file_.read(ptr, size).gcount();
            ret_val = file_send_size_;
            LOGT  << __PRETTY_FUNCTION__ << " --1-- ret_val:" << ret_val << ELOG;
        }else if (file_send_size_ == file_size_) {
            LOGT  << __PRETTY_FUNCTION__ << " --2--" << ELOG;

            ret_val = 0;
        } else {
            LOGT  << __PRETTY_FUNCTION__ << " --3--" << ELOG;

            ret_val = file_.readsome(ptr, size);
            file_send_size_ += ret_val;

            LOGT << __PRETTY_FUNCTION__ << " --size_to_send:" << ret_val << " --send_size_:" << file_send_size_ << ELOG;
        }
    }
    return ret_val;
}


} } // namespace art::lib
