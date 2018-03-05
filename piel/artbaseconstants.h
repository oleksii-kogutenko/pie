#ifndef ARTBASECONSTANTS_H
#define ARTBASECONSTANTS_H
#include <string>
namespace art { namespace lib {

struct ArtBaseConstants {
    static const std::string uri_attribute;
    static const std::string repo_attribute;
    static const std::string path_attribute;
    static const std::string created_date_attribute;
    static const std::string uri_delimiter;

    static const std::string download_uri;
    static const std::string size;
    static const std::string checksums;
    static const std::string checksums_md5;
    static const std::string checksums_sha1;

    static const std::string mem_type;
    static const std::string mem_type_text;

    static const std::string pom_modelVersion;
    static const std::string pom_modelVersion_ver;
    static const std::string pom_groupId;
    static const std::string pom_artifactId;
    static const std::string pom_version;
    static const std::string pom_packaging;
    static const std::string pom_packaging_pack;
    static const std::string pom_project;

    static const std::string pom_classifier;
};

} } // namespace art::lib

#endif // ARTBASECONSTANTS_H
