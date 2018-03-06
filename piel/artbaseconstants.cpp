/*
 * Copyright (c) 2017-2018
 *
 *  Dmytro Iakovliev daemondzk@gmail.com
 *  Oleksii Kogutenko https://github.com/oleksii-kogutenko
 *
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
 * THIS SOFTWARE IS PROVIDED BY Dmytro Iakovliev daemondzk@gmail.com ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Dmytro Iakovliev daemondzk@gmail.com BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <artbaseconstants.h>

namespace art { namespace lib {

const std::string ArtBaseConstants::uri_attribute   = "uri";
const std::string ArtBaseConstants::repo_attribute  = "repo";
const std::string ArtBaseConstants::path_attribute  = "path";
const std::string ArtBaseConstants::created_date_attribute = "created";
const std::string ArtBaseConstants::uri_delimiter   = "/";
const std::string ArtBaseConstants::download_uri    = "downloadUri";
const std::string ArtBaseConstants::size            = "size";
const std::string ArtBaseConstants::checksums       = "checksums";
const std::string ArtBaseConstants::checksums_md5   = "md5";
const std::string ArtBaseConstants::checksums_sha1  = "sha1";
const std::string ArtBaseConstants::mime_type       = "mimeType";
const std::string ArtBaseConstants::mime_type_text  = "application/text";

const std::string ArtBaseConstants::pom_modelVersion        = "modelVersion";
const std::string ArtBaseConstants::pom_modelVersion_ver    = "4.0.0";
const std::string ArtBaseConstants::pom_groupId             = "groupId";
const std::string ArtBaseConstants::pom_artifactId          = "artifactId";
const std::string ArtBaseConstants::pom_version             = "version";
const std::string ArtBaseConstants::pom_packaging           = "packaging";
const std::string ArtBaseConstants::pom_packaging_pack      = "pom";
const std::string ArtBaseConstants::pom_project             = "project";
const std::string ArtBaseConstants::pom_classifier          = ".pom";

} } // namespace art::lib
