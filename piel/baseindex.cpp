#include "baseindex.h"
#include <boost/log/trivial.hpp>

namespace piel { namespace lib {

BaseIndex::BaseIndex()
    : _index()
{

}

BaseIndex::BaseIndex(const BaseIndex& src)
    : _index(src._index)
{

}

void BaseIndex::put(const std::string &name, const std::string &hash, const std::string &source)
{
    BOOST_LOG_TRIVIAL(trace) << "name: " << name << " hash: " << hash << " source: " << source;

    IndexItem item;
    item.name = name;
    item.hash = hash;
    item.source = source;

    _index.insert(std::make_pair<std::string, IndexItem>(name, item));
}

bool BaseIndex::empty() const
{
    return _index.empty();
}

} } // namespace piel::lib
