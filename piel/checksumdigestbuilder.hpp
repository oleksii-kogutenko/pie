/*
 * Copyright (c) 2017, <copyright holder> <email>
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
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef CHECKSUM_DIGEST_BUILDER_HPP
#define CHECKSUM_DIGEST_BUILDER_HPP

#include <openssl/sha.h>
#include <openssl/md5.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/format.hpp>

namespace piel { namespace lib {

template<typename value_type> class DigestFormatter
{
public:
    DigestFormatter(std::string& str)
        : _str(str)
    {}

    void operator()(const value_type& v)
    {
        _str.append((boost::format("%1$02x") % (int)v).str());
    }

private:
    std::string& _str;
};

template<typename CTX, size_t digestSize> class DigestContext
{
public:
    typedef unsigned char uchar;

    typedef std::vector<uchar> Digest;
    typedef DigestFormatter<Digest::value_type> Formatter;

    DigestContext()
        : _ctx()
        , _digest(digestSize)
    {}

    void init();
    void update(const void *data, size_t size);
    Digest& finalize();

private:
    Digest _digest;
    CTX _ctx;
};

// SHA-256
template<> void DigestContext<SHA256_CTX,SHA256_DIGEST_LENGTH>::init()
{
    SHA256_Init(&_ctx);
}
template<> void DigestContext<SHA256_CTX,SHA256_DIGEST_LENGTH>::update(const void *data, size_t size)
{
    SHA256_Update(&_ctx, data, size);
}
template<> DigestContext<SHA256_CTX,SHA256_DIGEST_LENGTH>::Digest& DigestContext<SHA256_CTX,SHA256_DIGEST_LENGTH>::finalize()
{
    SHA256_Final(_digest.data(), &_ctx);
    return _digest;
}

typedef DigestContext<SHA256_CTX,SHA256_DIGEST_LENGTH> Sha256Context;

// SHA-1
template<> void DigestContext<SHA_CTX,SHA_DIGEST_LENGTH>::init()
{
    SHA1_Init(&_ctx);
}
template<> void DigestContext<SHA_CTX,SHA_DIGEST_LENGTH>::update(const void *data, size_t size)
{
    SHA1_Update(&_ctx, data, size);
}
template<> DigestContext<SHA_CTX,SHA_DIGEST_LENGTH>::Digest& DigestContext<SHA_CTX,SHA_DIGEST_LENGTH>::finalize()
{
    SHA1_Final(_digest.data(), &_ctx);
    return _digest;
}
typedef DigestContext<SHA_CTX,SHA_DIGEST_LENGTH> ShaContext;

// MD5
template<> void DigestContext<MD5_CTX,MD5_DIGEST_LENGTH>::init()
{
    MD5_Init(&_ctx);
}
template<> void DigestContext<MD5_CTX,MD5_DIGEST_LENGTH>::update(const void *data, size_t size)
{
    MD5_Update(&_ctx, data, size);
}
template<> DigestContext<MD5_CTX,MD5_DIGEST_LENGTH>::Digest& DigestContext<MD5_CTX,MD5_DIGEST_LENGTH>::finalize()
{
    MD5_Final(_digest.data(), &_ctx);
    return _digest;
}
typedef DigestContext<MD5_CTX,MD5_DIGEST_LENGTH> Md5Context;

template<class DigestCtx> class ChecksumDigestBuilder
{
public:
    ChecksumDigestBuilder() 
      : _buf(_buf_size)
      , _bad(false)
    {}

    typedef typename DigestCtx::Digest Digest;
    typedef typename DigestCtx::Formatter Formatter;

    Digest digest_for(std::istream& is)
    {
        reset();
        DigestCtx context;
        context.init();
        std::streamsize readed = 0;
        do {
            readed = is.read(_buf.data(), _buf.size()).gcount();
            if (readed != 0) {
                context.update(_buf.data(), readed);
            }
        } while(!is.eof() & !is.fail() & !is.bad());
	_bad = is.bad();
        return context.finalize();
    }

    Digest digest_for(const std::string& string)
    {
        reset();
        DigestCtx context;
        context.init();
        context.update(string.c_str(), string.size());
        return context.finalize();
    }

    std::string str_digest_for(const std::string& string)
    {
        std::string result;
        Formatter resultFormatter(result);
        Digest digest = digest_for(string);
        std::for_each(digest.begin(), digest.end(), resultFormatter);
        return result;
    }

    std::string str_digest_for(std::istream& is)
    {
        std::string result;
        Formatter resultFormatter(result);
        Digest digest = digest_for(is);
        std::for_each(digest.begin(), digest.end(), resultFormatter);
        return result;
    }    
    
    bool bad() const
    {
        return _bad;
    }

private:
    
    void reset() {
	_bad = false;
    }
  
    static const size_t _buf_size = 640*1024; // 640K buffer
    std::vector<std::istream::char_type> _buf;
    bool _bad;
};

typedef ChecksumDigestBuilder<Sha256Context> Sha256DigestBuilder;
typedef ChecksumDigestBuilder<ShaContext> ShaDigestBuilder;
typedef ChecksumDigestBuilder<Md5Context> Md5DigestBuilder;

} } // namespace piel::lib

#endif // CHECKSUM_DIGEST_BUILDER_HPP
