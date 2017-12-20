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

namespace piel { namespace lib {

class AssetImpl {
public:
    AssetImpl()
        : stream_(0)
    {
    }

    AssetImpl(const AssetId& id)
        : id_(id)
        , stream_(0)
    {
    }

    virtual ~AssetImpl() {}

    const AssetId& id()
    {
        if (id_ != AssetId::base)
            return id_;

        // Calculate id;
        id_ = AssetId::create_for(*istream());
        LOG_T << "Calculated asset id: " << id_.presentation();

        return id_;
    }

    virtual std::istream *istream() = 0;

    virtual AssetImpl *clone() const = 0;

protected:
    std::auto_ptr<std::istream> stream_;
    AssetId id_;
};

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

    std::istream *istream()
    {
        stream_.reset(new std::istringstream(str_));
        return stream_.get();
    }

    virtual AssetImpl *clone() const
    {
        return new StringImpl(*this);
    }

private:
    std::string str_;

};

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

    std::istream *istream()
    {
        stream_.reset(new std::ifstream(file_path_.c_str(), std::ifstream::in|std::ifstream::binary));
        return stream_.get();
    }

    virtual AssetImpl *clone() const
    {
        return new FileImpl(*this);
    }

private:
    boost::filesystem::path file_path_;

};

// TODO: StorageImpl
Asset::Asset()
    : impl_(new StringImpl(std::string()))
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

const AssetId& Asset::id() const
{
    return impl_->id();
}

std::istream *Asset::istream()
{
    return impl_->istream();
}

Asset Asset::create_for(const std::string& str_data)
{
    return new StringImpl(str_data);
}

Asset Asset::create_for(const boost::filesystem::path& file_path)
{
    return new FileImpl(file_path);
}

} } // namespace piel::lib
