#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <dirent.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>
#include <string>

template <typename T>
T _syserreq(T result, T comparison, char const * name)
{
    if (result == comparison) {
        throw std::runtime_error(std::string() + name + ": " + strerror(errno));
    }
    return result;
}
template <typename T>
T _syserrneq(T result, T comparison, char const * name)
{
    if (result != comparison) {
        throw std::runtime_error(std::string() + name + ": " + strerror(errno));
    }
    return result;
}
#define syserreq(func, comparison, ...) _syserreq(func(__VA_ARGS__), comparison, #func)
#define syserrneq(func, comparison, ...) _syserrneq(func(__VA_ARGS__), comparison, #func)

class Fts
{
public:
    Fts(char * path = 0)
    {
        if (path == 0) {
            path = (char*)calloc(1, 2);
            path[0] = '/';
        }
        char * const paths[] = { path, 0 };
        fts = syserreq(fts_open, (FTS*)0, paths, /*FTS_NOSTAT | */FTS_PHYSICAL | FTS_XDEV, NULL);
    }
    ~Fts()
    {
        syserrneq(fts_close, 0, fts);
    }
    struct entry
    {
        entry(FTS * fts, FTSENT * ftsent)
        : fts(fts), ftsent(ftsent)
        {
            init();
        }
        static entry begin(FTS * fts) {
            return entry(fts, fts_read(fts));
        }
        static entry end(FTS * fts) {
            return entry(fts, 0);
        }
        //void skip() {
        //    syserrneq(fts_set, 0, fts, ftsent, FTS_SKIP);
        //}
        FTSENT & operator*() {
            return *ftsent;
        }
        //FTSENT * operator->() {
        //    return ftsent;
        //}
        entry & operator++() {
            ftsent = fts_read(fts);
            init();
            return *this;
        }
        bool operator!=(entry const & other) {
            return other.ftsent != ftsent;
        }

        FTS * fts;
        FTSENT * ftsent;

    private:
        void init() {
            int error = 0;
            char const * name = "";
            if (ftsent != 0) {
                switch (ftsent->fts_info) {
                case FTS_DNR: // directory cannot be read
                case FTS_ERR: // error
                case FTS_NS: // no stat information available
                    name = ftsent->fts_path;
                    error = ftsent->fts_errno;
                //case FTS_NSOK: // stat disabled
                //    throw std::runtime_error(std::string() + ftsent->fts_path + ": no stat requested");
                }
            } else if (errno != 0) {
                error = errno;
            }
            if (error) {
                throw std::runtime_error(std::string() + name + ": " + strerror(error));
            }
        }
    };
    entry begin() { return entry(fts, fts_read(fts)); }
    entry end() { return entry(fts, 0); }
private:
    FTS * fts;
};

int main()
{
    /*
        size name id depth parentid
    */
    uintptr_t guid = 0;
    unsigned int max_depth = ~0;
    for (auto & entry : Fts()) {
        ++ guid;
        switch (entry.fts_info) {
        case FTS_NSOK:
            throw std::runtime_error(std::string() + entry.fts_path + ": no stat requested");
        case FTS_D:
            entry.fts_pointer = (void*)guid;
            if (guid < 65536) {
                guid <<= 4;
                max_depth = entry.fts_level + 1;
            } else {
                max_depth = entry.fts_level;
            }
            break;
        case FTS_F:
            entry.fts_number = entry.fts_statp->st_size;
        case FTS_DP:
            entry.fts_parent->fts_number += entry.fts_number;
            guid = (uintptr_t)entry.fts_pointer;
            if (entry.fts_level <= max_depth) {
                printf("%lu %s %tu %hu %tu\n", entry.fts_number, entry.fts_name, entry.fts_pointer, entry.fts_level, entry.fts_parent->fts_pointer);
            }
            break;
        }
    }
}
