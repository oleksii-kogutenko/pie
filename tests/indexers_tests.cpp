#define BOOST_TEST_MODULE IndexersTests
#include <boost/test/unit_test.hpp>

#include <fsindexer.h>
#include <zipindexer.h>

using namespace piel::lib;
namespace fs = boost::filesystem;

BOOST_AUTO_TEST_CASE(FsIndexer_BuildDirectoryIndex)
{
    FsIndexer fs_indexer;

    Index index;
    //index = fs_indexer.build(fs::path("/home/diakovliev/Gerrit/cache"));

    std::ostringstream test_os;
    index.store(test_os);
    std::string serialized_index = test_os.str();

    std::cout << serialized_index;

//    std::map<std::string,std::string> test_map;
//    test_map.insert(std::make_pair("test","test1"));
//    std::pair<std::map<std::string,std::string>::iterator, bool> res = test_map.insert(std::make_pair("test","test2"));
//
//    BOOST_CHECK(res.second);
//    BOOST_CHECK_EQUAL("test2", test_map["test"]);
}

BOOST_AUTO_TEST_CASE(ZipIndexer_BuildDirectoryIndex)
{
    ZipIndexer zip_indexer;

    Index index;
    //index = zip_indexer.build(fs::path("/home/diakovliev/Gerrit/cache/bin-release-local/adk/trunk/adk/67/ADK32419p_Explorer3040hd_OCAP_ATSC_SA_pKey/73ffa846b91bf995384b25afe155e879/ADK32419p_Explorer3040hd_OCAP_ATSC_SA_pKey"));

    std::ostringstream test_os;
    index.store(test_os);
    std::string serialized_index = test_os.str();

    std::cout << serialized_index;
}
