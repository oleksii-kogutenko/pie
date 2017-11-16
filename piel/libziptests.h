#ifndef LIBZIPTESTS_H
#define LIBZIPTESTS_H

#include <string>

class LibzipTests
{
public:
    LibzipTests();

    void test_read_archive_content(const std::string& afile);

};

#endif // LIBZIPTESTS_H
