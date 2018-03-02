#include "artbaseapideployartifacthandlers.h"
#include "artbaseconstants.h"
#include <cstring>
#include <logging.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace pt = boost::property_tree;

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtBaseApiDeployArtifactHandlers,\
        true,             false,         true,           true,          false,          false);

namespace art { namespace lib {

ArtBaseApiDeployArtifactHandlers::ArtBaseApiDeployArtifactHandlers(const std::string& api_token)
    : ArtBaseApiSendlerHandlers(api_token)
    , str_digests_()
{
    LOGT << ELOG;
}

ArtBaseApiDeployArtifactHandlers::ArtBaseApiDeployArtifactHandlers(const std::string& api_token,
                                                                   const std::string& url,
                                                                   const std::string& repo,
                                                                   const std::string& path,
                                                                   const std::string& fname)
    : ArtBaseApiSendlerHandlers(api_token, url, repo, path)
    , str_digests_()
{
    LOGT << ELOG;
    file(fname);
}

ArtBaseApiDeployArtifactHandlers::~ArtBaseApiDeployArtifactHandlers()
{
    LOGT << ELOG;
}

void ArtBaseApiDeployArtifactHandlers::generate_pom(const std::string& uri,
                                                    const std::string& repo,
                                                    const std::string& path,
                                                    const std::string& name,
                                                    const std::string& ver)
{
    set_url(uri);
    set_repo(repo);
    set_path(path);
    set_name(name);
    set_version(ver);
    generate_pom();
}

void ArtBaseApiDeployArtifactHandlers::generate_pom()
{
    set_classifier(ArtBaseConstants::pom_classifier);

    std::stringstream os;
    pt::ptree tree_project;
    pt::ptree tree;

    tree_project.put(ArtBaseConstants::pom_modelVersion, ArtBaseConstants::pom_modelVersion_ver);
    tree_project.put(ArtBaseConstants::pom_groupId, ArtBaseApiSendlerHandlers::get_path());
    tree_project.put(ArtBaseConstants::pom_artifactId, get_name());
    tree_project.put(ArtBaseConstants::pom_version, get_version());
    tree_project.put(ArtBaseConstants::pom_packaging, ArtBaseConstants::pom_packaging_pack);

    tree.add_child(ArtBaseConstants::pom_project, tree_project);

    pt::write_xml(os, tree);

    LOGT << "------------ +++ POM:XML +++ -------------" << ELOG;
    LOGT << os.str() << ELOG;
    LOGT << "------------ --- POM:XML --- -------------" << ELOG;

    boost::shared_ptr<std::istream> is(new std::stringstream(os.str()));
    push_input_stream(is);
}

void ArtBaseApiDeployArtifactHandlers::file(const std::string& fname)
{
    LOGT << ELOG;
    boost::shared_ptr<std::istream> file_ptr(new std::ifstream(fname));
    push_input_stream(file_ptr);

    std::ifstream in(fname, std::ifstream::ate | std::ifstream::binary);

    if (!in.is_open()) {
        LOGE << "Wrong to open file:" << fname << ELOG;
        return;
    }
    //file_size_ = boost::numeric_cast<size_t>(in.tellg());
    size_t file_size = in.tellg();

    in.seekg(0);

    piel::lib::ChecksumsDigestBuilder digest_builder;
    digest_builder.init();

    str_digests_ = digest_builder.str_digests_for(in);

    in.close();

    std::stringstream ss;

    ss << file_size;
    update_attributes(ArtBaseConstants::size, ss.str());
    update_attributes(ArtBaseConstants::mem_type, ArtBaseConstants::mem_type_text);
}

void ArtBaseApiDeployArtifactHandlers::gen_additional_tree(boost::property_tree::ptree& tree)
{
    LOGT << ELOG;
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

size_t ArtBaseApiDeployArtifactHandlers::handle_input(char *ptr, size_t size)
{
    LOGT << ELOG;

    return putto(ptr, size);
}

std::string ArtBaseApiDeployArtifactHandlers::get_path()
{
    std::string p = ArtBaseApiSendlerHandlers::get_path();
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
    LOGT <<  p << ELOG;
    return p;
}

} } // namespace art::lib
