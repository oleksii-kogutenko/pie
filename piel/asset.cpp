/*
 * Copyright (c) 2017, diakovliev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY diakovliev ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL diakovliev BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <asset.h>
#include <logging.h>

#include <fstream>
#include <sstream>

#include <iobjectsstorage.h>

namespace piel { namespace lib {

class AssetImpl {
public:
    AssetImpl()
    {
    }

    AssetImpl(const AssetId& id)
        : id_(id)
    {
    }

    virtual ~AssetImpl()
    {
    }

    virtual const AssetId& id()
    {
        if (id_ != AssetId::base)
            return id_;

        // Calculate id;
        boost::shared_ptr<std::istream> pis = istream();
        if (pis)
        {
            id_ = AssetId::create_for(*pis.get());
            LOG_T << "Calculated asset id: " << id_.presentation();
        }

        return id_;
    }

    virtual boost::shared_ptr<std::istream> istream() const = 0;

    virtual AssetImpl *clone() const = 0;

protected:
    mutable boost::shared_ptr<std::istream>  stream_;
    AssetId                                  id_;
};

// Non readable asset.
class IdImpl: public AssetImpl {
public:
    IdImpl(const IdImpl& src)
        : AssetImpl(src.id_)
    {
    }

    IdImpl(const AssetId& id)
        : AssetImpl(id)
    {
    }

    boost::shared_ptr<std::istream> istream() const
    {
        return stream_;
    }

    AssetImpl *clone() const
    {
        return new IdImpl(*this);
    }
};

// Readable asset with data in string buffer. Currently used for Index.
class StringImpl: public AssetImpl {
public:
    StringImpl(const StringImpl& src)
        : AssetImpl(src.id_)
        , str_(src.str_)
    {
    }

    StringImpl(const std::string& str)
        : str_(str)
    {
    }

    boost::shared_ptr<std::istream> istream() const
    {
        stream_.reset(new std::istringstream(str_));
        return stream_;
    }

    AssetImpl *clone() const
    {
        return new StringImpl(*this);
    }

private:
    std::string str_;

};

// Readable asset what points to file on local filesystem.
class FileImpl: public AssetImpl {
public:
    FileImpl(const FileImpl& src)
        : AssetImpl(src.id_)
        , file_path_(src.file_path_)
    {
    }

    FileImpl(const boost::filesystem::path& file_path)
        : file_path_(file_path)
    {
    }

    boost::shared_ptr<std::istream> istream() const
    {
        stream_.reset(new std::ifstream(file_path_.c_str(), std::ifstream::in|std::ifstream::binary));
        return stream_;
    }

    AssetImpl *clone() const
    {
        return new FileImpl(*this);
    }

private:
    boost::filesystem::path file_path_;

};

// Readable asset what points to asset in objects storage.
class StorageImpl: public AssetImpl {
public:
    StorageImpl(const StorageImpl& src)
        : AssetImpl(src.id_)
        , storage_(src.storage_)
    {
    }

    StorageImpl(const IObjectsStorage *storage, const AssetId& id)
        : AssetImpl(id)
        , storage_(storage)
    {
    }

    boost::shared_ptr<std::istream> istream() const
    {
        return storage_->istream_for(id_);
    }

    AssetImpl *clone() const
    {
        return new StorageImpl(*this);
    }

private:
    const IObjectsStorage *storage_;

};


Asset::Asset()
    : impl_(new IdImpl(AssetId::base))
{
}

Asset::Asset(AssetImpl *impl)
    : impl_(impl)
{
}

Asset::Asset(const Asset& src)
    : impl_(src.impl_->clone())
{
}

Asset::~Asset()
{
    delete impl_;
}

void Asset::operator=(const Asset& src)
{
    AssetImpl *prt_to_delete_ = impl_;
    impl_ = src.impl_->clone();
    delete prt_to_delete_;
}

bool Asset::operator<(const Asset& src) const
{
    return id() < src.id();
}

Asset::operator AssetId() const
{
    return id();
}

const AssetId& Asset::id() const
{
    return impl_->id();
}

boost::shared_ptr<std::istream> Asset::istream() const
{
    return impl_->istream();
}

/*static*/ Asset Asset::create_id(const AssetId& id)
{
    return Asset(new IdImpl(id));
}

/*static*/ Asset Asset::create_for(const IObjectsStorage *storage, const AssetId& id)
{
    return Asset(new StorageImpl(storage, id));
}

/*static*/ Asset Asset::create_for(const std::string& str_data)
{
    return Asset(new StringImpl(str_data));
}

/*static*/ Asset Asset::create_for(const boost::filesystem::path& file_path)
{
    return Asset(new FileImpl(file_path));
}

struct SerializationConstants {
    static const std::string id;
};

const std::string SerializationConstants::id = "id";

/*static*/ void Asset::store(boost::property_tree::ptree& tree, const Asset& asset)
{
    tree.add(SerializationConstants::id, asset.id().presentation());
}

/*static*/ Asset Asset::load(const boost::property_tree::ptree& tree)
{
    std::string id = tree.get<std::string>(SerializationConstants::id);
    return Asset::create_id(AssetId::create(id));
}

} } // namespace piel::lib
