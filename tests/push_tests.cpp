#define BOOST_TEST_MODULE PushTests
#include <boost/test/unit_test.hpp>

//#include <fsindexer.h>
//#include <zipindexer.h>
#include <zip.h>
#include <map>
#include <vector>
#include <zipfile.h>
#include "fstream"
#include "logging.h"

#include <boost/format.hpp>
#include <boost/optional.hpp>

#include "test_utils.hpp"

#include <workingcopy.h>
#include <assetsextractor.h>

#include <commit.h>
#include <create.h>
#include <clean.h>
#include <checkout.h>
#include <reset.h>

#include <treeenumerator.h>
#include <treeindexenumerator.h>
#include "test_utils.hpp"
#include <artdeployartifacthandlers.h>
#include <gavcquery.h>

#define DBOOST_AUTO_TEST_CASE(x) void x(void)

namespace cmd=piel::cmd;
namespace lib=piel::lib;
namespace fs=boost::filesystem;
namespace tst=lib::test_utils;

static std::string artifact_name_1 = "artifact_1";
static std::string artifact_name_2 = "artifact_2";

static const char* server_api_access_token_ = "";
static const char* server_url_ = "";
static const char* server_repository_ = "";

static const std::string query_str = "test_dir:push_test:1";

void deploy_pom(const art::lib::GavcQuery& query_)
{
    art::lib::ArtDeployArtifactHandlers deploy_handlers(server_api_access_token_);
    deploy_handlers.generate_pom(server_url_, server_repository_, query_.group(), query_.name(), query_.version());

    piel::lib::CurlEasyClient<art::lib::ArtDeployArtifactHandlers> upload_client(deploy_handlers.gen_uri(), &deploy_handlers);

    LOGD << "Upload pom to: " << deploy_handlers.gen_uri() << ELOG;

    if (!upload_client.perform())
    {
        LOGE << "Error on upload pom!"                      << ELOG;
        LOGE << upload_client.curl_error().presentation()   << ELOG;
    }
}

void upload(const std::string& query_str, const std::string& classifier, const std::string& file_name)
{
    bool no_errors = true;

    art::lib::GavcQuery query_;
    boost::optional<art::lib::GavcQuery> query_opt = art::lib::GavcQuery::parse(query_str);
    if (!query_opt) {
        LOGE << "Wrong query " << query_str << ELOG;
        return;
    }
    query_ = query_opt.get();

    LOGI << query_.to_string() << ELOG;
    {
        art::lib::ArtDeployArtifactHandlers deploy_handlers(server_api_access_token_);

        deploy_handlers.set_url(server_url_);
        deploy_handlers.set_repo(server_repository_);
        deploy_handlers.set_path(query_.group());
        deploy_handlers.set_name(query_.name());
        deploy_handlers.set_version(query_.version());
        deploy_handlers.set_classifier(classifier);
        deploy_handlers.file(file_name);

        piel::lib::CurlEasyClient<art::lib::ArtDeployArtifactHandlers> upload_client(deploy_handlers.gen_uri(), &deploy_handlers);

        LOGD << "Upload: " << file_name << " as " << classifier << " to: " << deploy_handlers.gen_uri() << ELOG;

        if (!(no_errors &= upload_client.perform()))
        {
            LOGE << "Error on upload file!"                     << ELOG;
            LOGE << upload_client.curl_error().presentation()   << ELOG;
        }
    }

    if (no_errors)
    {
        deploy_pom(query_);
    }
}

bool create_reference(lib::WorkingCopy::Ptr working_copy, const std::string& new_ref_)
{
    if (piel::lib::AssetId::empty != working_copy->local_storage()->resolve(new_ref_))
    {
        LOGE << "" << ELOG;
        return false;
    }

    piel::lib::TreeIndex::Ptr initial_tree_index(new piel::lib::TreeIndex());

    initial_tree_index->initial_for(new_ref_);
    working_copy->local_storage()->put(initial_tree_index->assets());

    piel::lib::AssetId new_tree_id = initial_tree_index->self().id();
    working_copy->local_storage()->create_reference(piel::lib::refs::Ref(new_ref_, new_tree_id));
    working_copy->setup_current_tree(new_ref_, initial_tree_index);

    LOGI << "Created new tree: " << new_ref_  << ":" << new_tree_id.string() << ELOG;

    return true;
}

BOOST_AUTO_TEST_CASE(Push_test_1)
{
    LOGI  << "---start Push_test_1---" << ELOG;

    // Generate zip file
    tst::TempFileHolder::Ptr wc_path = tst::create_temp_dir();
    //tst::TempFileHolder::Ptr zip_path = tst::create_temp_dir(0);

    //*********************************
    // Artifact 1
    LOGI << " generate state for " << artifact_name_1 << ELOG;

    tst::DirState state_artifact_1;
    state_artifact_1["artifact_1_file_1"] = "artifact_1 file 1 content 1";
    state_artifact_1["artifact_1_file_2"] = "artifact_1 file 2 content 2";
    state_artifact_1["artifact_1_file_3"] = "artifact_1 file 3 content 3";
    state_artifact_1["artifact_1_file_4"] = "artifact_1 file 4 content 4";
    state_artifact_1["artifact_1_1/artifact_1_file_5"] = "artifact_1_1/artifact_1 file 5 content 5";
    state_artifact_1["artifact_1_2/artifact_1_file_6"] = "artifact_1_2/artifact_1 file 6 content 6";
    state_artifact_1["artifact_1_3/artifact_1_file_7"] = "artifact_1_3/artifact_1 file 7 content 7";

    // Init workspace
    lib::WorkingCopy::Ptr wc = lib::WorkingCopy::init(wc_path->first, artifact_name_1);
    tst::make_directory_state(wc->working_dir(), wc->metadata_dir(), state_artifact_1);

    cmd::Commit commit_1(wc);
    commit_1.set_message("Commit for " + artifact_name_1);
    std::string artifact_1_state_id = commit_1();

    LOGI << " artifact_1_state_id:" << artifact_1_state_id << ELOG;
    //*********************************
    // Artifact 2
    LOGI << " generate state for " << artifact_name_2 << ELOG;

    create_reference(wc, artifact_name_2);

    tst::DirState state_artifact_2;
    state_artifact_2["artifact_2_file_1"] = "artifact_2 file 1 content 1";
    state_artifact_2["artifact_2_file_2"] = "artifact_2 file 2 content 2";
    state_artifact_2["artifact_2_file_3"] = "artifact_2 file 3 content 3";
    state_artifact_2["artifact_2_file_4"] = "artifact_2 file 4 content 4";
    state_artifact_2["artifact_2_1/artifact_2_file_5"] = "artifact_2_1/artifact_2 file 5 content 5";
    state_artifact_2["artifact_2_2/artifact_2_file_6"] = "artifact_2_2/artifact_2 file 6 content 6";
    state_artifact_2["artifact_2_3/artifact_2_file_7"] = "artifact_2_3/artifact_2 file 7 content 7";
    tst::make_directory_state(wc->working_dir(), wc->metadata_dir(), state_artifact_2);

    cmd::Commit commit_2(wc);
    commit_2.set_message("Commit for " + artifact_name_2);
    std::string artifact_2_state_id = commit_2();
    LOGI << " artifact_2_state_id:" << artifact_2_state_id << ELOG;

    LOGI << "\n\n\n\n" << ELOG;

    //*********************************
    // Generate zip files for all artifacts
    LOGI << " Make ZIP::" << ELOG;

    LOGI << "Trees:::" << ELOG;
    std::set<piel::lib::refs::Ref> all_refs = wc->local_storage()->references();

    for(std::set<piel::lib::refs::Ref>::const_iterator i = all_refs.begin(), end = all_refs.end(); i != end; ++i)
    {
        std::string log_str;
        if (wc->current_tree_name() == i->first)
            log_str  = "*" + i->first;
        else
            log_str  = " " + i->first;

        LOGI << log_str << ":" << i->second.string();

        LOGI << ELOG;

        piel::lib::TreeIndex::Ptr reference_index = piel::lib::TreeIndex::from_ref(wc->local_storage(), i->first);
        lib::TreeIndexEnumerator enumerator(reference_index);

        LOGI << "reference_index->self().id().string():" << reference_index->self().id().string() << ELOG;

        boost::filesystem::path zip_path_fs = wc_path->first / (i->first + ".zip");
        std::string zip_path = zip_path_fs.string();
        lib::ZipFile::FilePtr zip = lib::ZipFile::create(zip_path);

        while (enumerator.next())
        {
            LOGI << "\t" << enumerator.path << ":"
                    << enumerator.asset.id().string() << ELOG;

            LOGI << "\"" << lib::test_utils::istream_content(enumerator.asset.istream()) << "\"" << ELOG;

            zip->add_istream(enumerator.path, enumerator.asset.istream());
        }
    }

    //*********************************
    // Publishing to the artifactory server
    LOGI << " Publishing::" << ELOG;

    for(std::set<piel::lib::refs::Ref>::const_iterator i = all_refs.begin(), end = all_refs.end(); i != end; ++i)
    {
        boost::filesystem::path zip_path_fs = wc_path->first / (i->first + ".zip");
        std::string zip_path = zip_path_fs.string();
        LOGI << " Publishing " << zip_path <<  " as " << i->first << ELOG;
        upload(query_str, i->first, zip_path);
    }

    //*********************************
    LOGI  << "---FINISH Push_test_1---" << ELOG;
}
