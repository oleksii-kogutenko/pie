#include "streamssequencepartitionallyoutputhelper.h"
#include <cstring>
#include <logging.h>

#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace pt = boost::property_tree;


namespace art { namespace lib {

StreamsSequencePartitionallyOutputHelper::StreamsSequencePartitionallyOutputHelper()
    : is_queue_()
    , current_is_()
    , put_size_(0)
{
}

void StreamsSequencePartitionallyOutputHelper::push_input_stream(boost::shared_ptr<std::istream> is)
{
    is_queue_.push(is);
}

bool StreamsSequencePartitionallyOutputHelper::next()
{
    bool res = !is_queue_.empty();
    if (res) {
        current_is_ = is_queue_.front();
        is_queue_.pop();
    }
    return res;
}

size_t StreamsSequencePartitionallyOutputHelper::putto(char* ptr, size_t size)
{
    size_t filled_size = 0;
    while (filled_size < size) {
        size_t readed = 0;

        if (current_is_) {
            readed += boost::numeric_cast<size_t>(current_is_->read(ptr + filled_size,
                                        boost::numeric_cast<std::streamsize>(size - filled_size)).gcount());
        }

        if (!readed){
            if (next()) {
                continue;
            }
            else {
                return filled_size;
            }
        }

        filled_size += readed;
    }
    return filled_size;
}

} } // namespace art::lib
