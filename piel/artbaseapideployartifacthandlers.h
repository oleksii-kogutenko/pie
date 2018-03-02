#ifndef ARTBASEAPIDEPLOYARTIFACTHANDLERS_H
#define ARTBASEAPIDEPLOYARTIFACTHANDLERS_H
#include <artbaseapisendlerhandlers.h>
#include "streamssequencepartitionallyoutputhelper.h"
#include "checksumsdigestbuilder.hpp"

#include <fstream>

namespace art { namespace lib {

class ArtBaseApiDeployArtifactHandlers : public ArtBaseApiSendlerHandlers
{
    //typedef std::map<std::string, std::string> Attributes;
public:
    ArtBaseApiDeployArtifactHandlers(const std::string& api_token);
    ArtBaseApiDeployArtifactHandlers(const std::string& api_token, const std::string& uri, const std::string& repo, const std::string& path, const std::string& fname);
    virtual ~ArtBaseApiDeployArtifactHandlers();

    virtual size_t handle_input(char *ptr, size_t size);
    virtual void gen_additional_tree(boost::property_tree::ptree &);

    virtual std::string get_path();

    void file(const std::string& fname);
    void generate_pom();

    void set_name(const std::string& s) { name_ = s; }
    void set_version(const std::string& s) { version_ = s; }
    void set_classifier(const std::string& s) { classifier_ = s; }

    std::string get_name() { return name_; }
    std::string get_version() { return version_; }
    std::string get_classifier() { return classifier_; }
private:
    piel::lib::ChecksumsDigestBuilder::StrDigests str_digests_;

    std::string   name_;
    std::string   version_;
    std::string   classifier_;
};

} } // namespace art::lib

#endif // ARTBASEAPIDEPLOYARTIFACTHANDLERS_H
