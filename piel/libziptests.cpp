#include "libziptests.h"

#include <zip.h>
#include <zipfile.h>
#include <checksumdigestbuilder.hpp>
#include <boost/log/trivial.hpp>

LibzipTests::LibzipTests()
{

}

void LibzipTests::test_read_archive_content(const std::string& afile)
{
    using namespace piel::lib;

    Sha256Context sha256_context;
//    ShaContext sha_context;
//    Md5Context md5_context;

    MultiChecksumsDigestBuilder digest_builder;

    ZipFile zip_file(afile);
    for (zip_int64_t i = 0; i < zip_file.num_entries(); i++) {
        boost::shared_ptr<ZipEntry> entry = zip_file.entry(i);

        ZipEntryAttributes attrs = entry->attributes();

        MultiChecksumsDigestBuilder::StrDigests checksums = digest_builder.str_digests_for(ZIP_ENTRY_ISTREAM(entry));

        std::string sha256 = checksums[sha256_context.name()];

        BOOST_LOG_TRIVIAL(trace) << std::string(entry->symlink() ? "s " +  entry->target() + " " : "f ")
                                 << entry->name()
                                 << " os:"
                                 << boost::format("%1$08x") % (int)attrs.opsys
                                 << " attributes: "
                                 << boost::format("%1$08x") % (int)attrs.attributes
                                 << " mode: "
                                 << boost::format("%1$08o") % (int)(attrs.mode() & 0777)
                                 << " sha256: "
                                 << sha256;

//        std::string sha = checksums[sha_context.name()];
//        std::string md5 = checksums[md5_context.name()];
//        BOOST_LOG_TRIVIAL(trace) << "sha: " << sha << " md5: " << md5;
    }
}
