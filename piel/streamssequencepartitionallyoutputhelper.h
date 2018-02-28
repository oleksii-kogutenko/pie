#ifndef STREAMSSEQUENCEPARTITIONALLYOUTPUTHELPER_H
#define STREAMSSEQUENCEPARTITIONALLYOUTPUTHELPER_H
#include <boost/shared_ptr.hpp>
#include <queue>

namespace art { namespace lib {

class StreamsSequencePartitionallyOutputHelper
{
    typedef boost::shared_ptr<std::istream> ISPtr;
    typedef std::queue<ISPtr> ISPtrQueue;
public:
    StreamsSequencePartitionallyOutputHelper();
    ~StreamsSequencePartitionallyOutputHelper(){}

    /*boost::shared_ptr<std::istream> istream() const
    {
        return is_;
    }*/

    void push_input_stream(boost::shared_ptr<std::istream> is);

    size_t putto(char* ptr, size_t size);
private:
    bool next();
private:
    ISPtrQueue   is_queue_;
    ISPtr       current_is_;
    size_t      put_size_;
};

} } // namespace art::lib

#endif // STREAMSSEQUENCEPARTITIONALLYOUTPUTHELPER_H
