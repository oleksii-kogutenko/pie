/*
 * Copyright (c) 2018, diakovliev
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

#ifndef GAVC_H_
#define GAVC_H_

#include <gavcquery.h>
#include <iostreamsholder.h>
#include <properties.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <list>

namespace piel { namespace cmd {

namespace errors {
    struct unable_to_parse_maven_metadata {};
    struct no_server_maven_metadata {
        no_server_maven_metadata(const std::string& e)
            : error(e)
        {}
        std::string error;
    };
    struct error_processing_version {
        error_processing_version(const std::string& e, const std::string& v)
            : error(e)
            , ver(v)
        {}
        std::string error;
        std::string ver;
    };
    struct cant_get_maven_metadata {};
    struct cant_find_version_for_query {};
    struct gavc_download_file_error {};
};

class GAVC: public piel::lib::IOstreamsHolder
{
public:
    typedef std::list<boost::filesystem::path> paths_list;
    typedef std::map<boost::filesystem::path,std::pair<std::string,std::string> > query_results;

    GAVC(  const std::string& server_api_access_token
         , const std::string& server_url
         , const std::string& server_repository
         , const art::lib::GavcQuery& query
         , const bool have_to_download_results
         , const std::string& output_file = std::string());

    virtual ~GAVC();

    void operator()();

    void set_path_to_download(const boost::filesystem::path& path);
    boost::filesystem::path get_path_to_download() const;
    paths_list get_list_of_actual_files() const;
    query_results get_query_results() const;

    std::vector<std::string> get_versions_to_process() const;
    void process_versions(const std::vector<std::string>& );
    void process_version(const std::string&);
    std::string get_maven_metadata_path() const;

    static bool validate_local_file(const boost::filesystem::path& object_path, const piel::lib::Properties& server_checksums);

    static piel::lib::Properties load_object_properties(const boost::filesystem::path& object_path);
    static void store_object_properties(const boost::filesystem::path& object_path, const piel::lib::Properties& properties);

    void set_cache_mode(bool value);
    static std::string get_classifier_file_name(const std::string& query_name, const std::string& ver, const std::string& classifier);

protected:
    std::string create_url(const std::string& version_to_query, const std::string& classifier) const;
    void on_object(const boost::property_tree::ptree::value_type& obj, const std::string& version, const std::string& query_classifier);
    std::map<std::string,std::string> get_server_checksums(const boost::property_tree::ptree& obj_tree, const std::string& section) const;
    void download_file(const boost::filesystem::path& object_path, const std::string& object_id, const std::string& download_uri) const;

private:
    std::string server_url_;
    std::string server_api_access_token_;
    std::string server_repository_;
    art::lib::GavcQuery query_;
    boost::filesystem::path path_to_download_;
    bool have_to_download_results_;
    bool cache_mode_;
    paths_list list_of_actual_files_;
    query_results query_results_;
    std::string output_file_;
};

} } // namespace piel::cmd

#endif /* GAVC_H_ */
