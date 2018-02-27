#ifndef ARTBASEAPIDEPLOYARTIFACTHANDLERS_H
#define ARTBASEAPIDEPLOYARTIFACTHANDLERS_H
#include <artbaseapisendlerhandlers.h>
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

    void file(const std::string& fname);

    void sha1(const std::string& sha1);
    void md5(const std::string& md5);
private:
    std::ifstream file_;
    size_t        file_size_;
    size_t        file_send_size_;
    std::string   sha1_;
    std::string   md5_;

};

} } // namespace art::lib

#endif // ARTBASEAPIDEPLOYARTIFACTHANDLERS_H
