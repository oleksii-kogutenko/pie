#include <iostream>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

void enumerate_directory(const path& p) {
    if (!is_directory(p))
        return;

    for (directory_iterator i = directory_iterator(p); i != end(i); i++) 
    {
        directory_entry e = *i;

        if (is_symlink(e.path()))
        {
            std::cout << e.path() << " is symlink to " << read_symlink(e.path()) << std::endl;
        }
        else if (is_regular_file(e.path()))
        {
            std::cout << e.path() << " is regular file" << std::endl;
        }
        else if (is_directory(e.path()))
        {
            std::cout << e.path() << " is directory" << std::endl;
        }
    }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "No arguments!" << std::endl;
        return -1;
    }

    path p(argv[1]);
    if (is_symlink(p))
    {
        std::cout << p << " is symlink to " << read_symlink(p) << std::endl;
    }
    else if (is_regular_file(p))
    {
        std::cout << p << " is regular file" << std::endl;
    }
    else if (is_directory(p))
    {
        std::cout << p << " is directory" << std::endl;
        enumerate_directory(p);
    }

    return 0;
}

