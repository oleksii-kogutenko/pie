#ifndef ARTBASEAPICREATEDIRECTORYHANDLERS_H
#define ARTBASEAPICREATEDIRECTORYHANDLERS_H
#include <artbaseapisendlerhandlers.h>

namespace art { namespace lib {

class ArtBaseApiCreateDirectoryHandlers : public ArtBaseApiSendlerHandlers
{
public:
    ArtBaseApiCreateDirectoryHandlers(const std::string& api_token);
    ArtBaseApiCreateDirectoryHandlers(const std::string& api_token, const std::string& uri, const std::string& repo, const std::string& path);
    virtual ~ArtBaseApiCreateDirectoryHandlers() {}

    virtual std::string get_path();
};

} } // namespace art::lib

#endif // ARTBASEAPICREATEDIRECTORYHANDLERS_H
