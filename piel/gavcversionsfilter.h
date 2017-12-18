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

#ifndef GAVCVERSIONSFILTER_H_
#define GAVCVERSIONSFILTER_H_

#include <vector>
#include <gavcquery.h>

namespace art { namespace lib {

//! GAVC versions filter.
//! @sa MavenMetadata, GavcQuery, art::lib::gavc::OpType
class GavcVersionsFilter
{
public:
    //! Constructor.
    //!
    //! @param query_ops Query operations for creating filter.
    GavcVersionsFilter(const std::vector<gavc::OpType>& query_ops);

    //! Destructor.
    ~GavcVersionsFilter();

    //! Method will return filtered collection.
    //!
    //! @param versions Collection of versions to filter.
    //! @return Filtered versions vector.
    std::vector<std::string> filtered(const std::vector<std::string>& versions);

    //! Method will return vector with versions what was filtered.
    //!
    //! @param versions Collection of versions to filter.
    //! @return Vector of the filtered elements.
    std::vector<std::string> filtered_out(const std::vector<std::string>& versions);

protected:
    //! Method will return true if filtering is not needed. It will be true
    //! for query_ops_ what not contains any Op_const elements.
    bool is_trivial() const;

private:
    std::vector<gavc::OpType> query_ops_;   //!< Query operations for creating filter.

};

} } // namespace art::lib

#endif /* GAVCVERSIONSFILTER_H_ */
