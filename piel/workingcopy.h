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

#ifndef PIEL_WORKINGCOPY_H_
#define PIEL_WORKINGCOPY_H_

#include <properties.h>
#include <indexesdiff.h>
#include <localdirectorystorage.h>
#include <boost/weak_ptr.hpp>
#include <treeindex.h>

/*
 * Abstract:
 *
 * "Asset"          - abstract any size data block (object).
 * "AssetId"        - unique hash of the asset data.
 * "Tree"           - structured set of the "Asset"s (FE filesystem directory).
 * "TreeIndex"      - data structure what contains information about particular "Tree" state.
 * "Pie"            - ordered, one way linked list of the "Tree index"es.
 *
 * "ObjectsStorage" - abstract "Asset"s storage. Contains "Assets"s itself and collection of the "Pie"s heads what available in.
 * "Working copy"   - directory used for the manipulation related to "Pie"s.
 *
 */

namespace piel { namespace lib {

namespace errors {
    struct init_existing_working_copy {};
    struct attach_to_non_working_copy {};
    struct unable_to_find_reference_file {};
};

class WorkingCopy: private boost::noncopyable
{
public:
    typedef std::vector<IObjectsStorage::Ptr> Storages;
    typedef boost::shared_ptr<WorkingCopy> Ptr;
    typedef boost::weak_ptr<WorkingCopy> Weak;
    static const int local_storage_index;

    ~WorkingCopy();

    bool is_valid() const;

    static Ptr init(const boost::filesystem::path& working_dir, const std::string reference);
    static Ptr attach(const boost::filesystem::path& working_dir);

    void set_config(const std::string& name, const std::string& value);
    std::string get_config(const std::string& name, const std::string& default_value = std::string());

    void init_local_storage();
    IObjectsStorage::Ptr local_storage() const;
    boost::filesystem::path working_dir() const;
    boost::filesystem::path metadata_dir() const;
    boost::filesystem::path storage_dir() const;
    boost::filesystem::path config_file() const;
    Properties& config();
    Storages& storages();

    std::string current_tree_name() const;
    const TreeIndex& current_tree_index() const;

    void setup_current_tree(const std::string& new_reference, const TreeIndex& new_reference_index);

    TreeIndex current_index() const;

protected:
    WorkingCopy();
    WorkingCopy(const boost::filesystem::path& working_dir);

    void set_current_tree(const std::string& new_tree_name);
    void set_current_tree_index(const TreeIndex& new_tree_index);

private:
    void init_filesystem(const std::string reference);
    void init_storages(const std::string reference);
    void attach_filesystem();
    void attach_storages();

private:
    boost::filesystem::path working_dir_;                       //!< Working copy filesystem directory.
    boost::filesystem::path metadata_dir_;                      //!< Metadata subdirectory.
    boost::filesystem::path storage_dir_;                       //!< Local "ObjectsStorage" directory.
    boost::filesystem::path current_tree_file_;                 //!< Metadata file what contains working copy current "Pie" name.
    boost::filesystem::path current_tree_index_file_;           //!< Metadata file what contains working copy current "Pie" tree index.
    boost::filesystem::path config_file_;                       //!< Working copy specific pie configuration parameters file.
    Properties              config_;                            //!< Working copy specific pie configuration parameters.
    Storages                storages_;                          //!< "ObjectStorage"s collection.
    std::string             current_tree_name_;                 //!< Working copy current "Tree" name.
    TreeIndex               current_tree_index_;                //!< working copy current "Tree" tree index.
};

struct PredefinedConfigs {
    static Properties::DefaultFromEnv author;
    static Properties::DefaultFromEnv email;
    static Properties::DefaultFromEnv commiter;
    static Properties::DefaultFromEnv commiter_email;
};

} } // namespace piel::lib

#endif /* PIEL_WORKINGCOPY_H_ */
