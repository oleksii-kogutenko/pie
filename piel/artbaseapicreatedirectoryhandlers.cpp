#include "artbaseapicreatedirectoryhandlers.h"
#include "artbaseconstants.h"
#include <cstring>
#include <logging.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtBaseApiCreateDirectoryHandlers,\
        true,             false,         true,           true,          false,          false);

namespace art { namespace lib {

ArtBaseApiCreateDirectoryHandlers::ArtBaseApiCreateDirectoryHandlers(const std::string& api_token)
    : ArtBaseApiSendlerHandlers(api_token)
{
    LOGT << ELOG;
}

ArtBaseApiCreateDirectoryHandlers::ArtBaseApiCreateDirectoryHandlers(const std::string& api_token, const std::string& url, const std::string& repo, const std::string& path)
    : ArtBaseApiSendlerHandlers(api_token, url, repo, path)
{
    LOGT << ELOG;
}

std::string ArtBaseApiCreateDirectoryHandlers::get_path()
{
    std::string path = ArtBaseApiSendlerHandlers::get_path();
    if (path.compare(path.length() - ArtBaseConstants::uri_delimiter.length(), ArtBaseConstants::uri_delimiter.length(), ArtBaseConstants::uri_delimiter) != 0) {
        path.append(ArtBaseConstants::uri_delimiter);
    }
    return path;
}


} } // namespace art::lib
