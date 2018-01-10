#ifndef PIEL_TEST_UTILS_HPP
#define PIEL_TEST_UTILS_HPP

#include <boost_filesystem_ext.hpp>

#include <map>
#include <queue>
#include <sstream>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace piel { namespace lib { namespace test_utils {

// Test utils
inline std::string generate_random_string()
{
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(5, 256);
    int count = index_dist(rng);
    std::ostringstream oss;
    for(int i = 0; i < count; ++i) {
        std::string::value_type ch = (std::string::value_type)index_dist(rng);
        oss << ch;
    }
    return oss.str();
}

inline std::string generate_random_printable_string(unsigned min_size = 5, unsigned max_size = 256)
{
    boost::random::random_device rng;
    std::string ch_vector = "123456789qwertyuiopasdfghjklzxcvbnm";
    boost::random::uniform_int_distribution<> index_dist(min_size, max_size);
    boost::random::uniform_int_distribution<> vector_dist(0, ch_vector.size() -1);
    int count = index_dist(rng);
    std::ostringstream oss;
    for(int i = 0; i < count; ++i) {
        std::string::value_type ch = ch_vector[vector_dist(rng)];
        oss << ch;
    }
    return oss.str();
}

struct TempFileHolder: public std::pair<boost::filesystem::path,std::string> {
    typedef boost::shared_ptr<TempFileHolder> Ptr;
    TempFileHolder(const boost::filesystem::path& path, const std::string& content)
        : std::pair<boost::filesystem::path,std::string>(std::make_pair(path, content))
    {
    }
    ~TempFileHolder()
    {
        if (boost::filesystem::exists(first)) boost::filesystem::remove_all(first);
    }
};

inline void update_temp_dir(boost::filesystem::path temp_dir, boost::filesystem::path exclude, unsigned min_size = 5, unsigned max_size = 256)
{
    boost::random::random_device rng;

    boost::random::uniform_int_distribution<> rm_add_modify(1, 100);
    boost::random::uniform_int_distribution<> index_dist(min_size, max_size);

    bool modified_once = false;
    bool removed_once = false;

    // I need at least one modified and one removed item
    while (!removed_once && !modified_once)
    {
        for (boost::filesystem::directory_iterator i = boost::filesystem::directory_iterator(temp_dir), end = boost::filesystem::directory_iterator(); i != end; i++)
        {
            boost::filesystem::directory_entry e = *i;
            if (e.path() != exclude)
            {
                if (!removed_once || rm_add_modify(rng) < 50)
                {
                    boost::filesystem::remove_all(e.path());
                    removed_once = true;
                }
                else
                {
                    boost::filesystem::path temp_file = temp_dir / boost::filesystem::unique_path();
                    if (!modified_once || rm_add_modify(rng) < 50)
                    {
                        temp_file = e.path();
                        modified_once = true;
                    }
                    int count = index_dist(rng);
                    boost::shared_ptr<std::ostream> ofs = boost::filesystem::ostream(temp_file);
                    std::ostringstream oss;
                    for(int i = 0; i < count; ++i) {
                        std::string::value_type ch = (std::string::value_type)index_dist(rng);
                        (*ofs) << ch;
                        oss << ch;
                    }
                }
            }
        }
    }
}

inline TempFileHolder::Ptr create_temp_dir(unsigned files_count = 0, unsigned min_size = 5, unsigned max_size = 256)
{
    boost::filesystem::path temp_dir = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    boost::filesystem::create_directory(temp_dir);

    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(min_size, max_size);

    for (unsigned i = 0; i < files_count; ++i)
    {
        boost::filesystem::path temp_file = temp_dir / boost::filesystem::unique_path();
        int count = index_dist(rng);
        boost::shared_ptr<std::ostream> ofs = boost::filesystem::ostream(temp_file);
        std::ostringstream oss;
        for(int i = 0; i < count; ++i) {
            std::string::value_type ch = (std::string::value_type)index_dist(rng);
            (*ofs) << ch;
            oss << ch;
        }
    }

    return TempFileHolder::Ptr(new TempFileHolder(temp_dir, ""));
}

inline TempFileHolder::Ptr create_random_temp_file(unsigned min_size = 5, unsigned max_size = 256)
{
    boost::filesystem::path temp_file = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(min_size, max_size);
    int count = index_dist(rng);
    boost::shared_ptr<std::ostream> ofs = boost::filesystem::ostream(temp_file);
    std::ostringstream oss;
    for(int i = 0; i < count; ++i) {
        std::string::value_type ch = (std::string::value_type)index_dist(rng);
        (*ofs) << ch;
        oss << ch;
    }
    return TempFileHolder::Ptr(new TempFileHolder(temp_file, oss.str()));
}

typedef std::map<std::string, std::string> DirState;
typedef DirState::value_type DirStateElement;

inline void make_directory_state(const boost::filesystem::path& dir, const boost::filesystem::path& exclude, const DirState& needed_state)
{
    boost::filesystem::remove_directory_content(dir, exclude);

    for (DirState::const_iterator i = needed_state.begin(), end = needed_state.end(); i != end; ++i)
    {
        boost::filesystem::path file = dir / i->first;

        boost::filesystem::create_directories(file.parent_path());

        boost::shared_ptr<std::ostream> ofs = boost::filesystem::ostream(file);
        (*ofs) << i->second;
    }
}

inline DirState get_directory_state(const boost::filesystem::path& dir, const boost::filesystem::path& exclude)
{
    namespace fs = boost::filesystem;

    DirState                result;
    std::queue<fs::path>    directories;

    directories.push( dir );

    while( !directories.empty() )
    {
        fs::path p = directories.front();
        directories.pop();

        for (fs::directory_iterator i = fs::directory_iterator(p), end = fs::directory_iterator(); i != end; i++)
        {
            fs::directory_entry e           = *i;
            fs::path            relative    = fs::make_relative( dir, e.path() );
            std::string         name        = relative.generic_string();

            if ( fs::is_regular_file( e.path() ) )
            {
                boost::shared_ptr<std::istream> ifsp = fs::istream(e.path());
                std::ostringstream *oss = new std::ostringstream();
                boost::shared_ptr<std::ostream> ossp(oss);
                fs::copy_into(ossp, ifsp);
                result.insert(DirStateElement(name, oss->str()));
            }
            else if ( fs::is_directory(e.path()) )
            {
                if ( exclude.empty() || e.path() != exclude )
                {
                    directories.push( e.path() );
                }
            }
        }
    }

    return result;
}

} } } // namespace piel::lib::test_utils

#endif // PIEL_TEST_UTILS_HPP
