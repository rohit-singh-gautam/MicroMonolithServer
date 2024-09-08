/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/server/http.h>
#include <mms/base/maths.h>
#include <filesystem>
#include <list>
#include <unordered_map>
#include <memory>

namespace MMS::server {

constexpr size_t etag_size = to_string64_hash<uint64_t, false>();

inline uint64_t get_etag(int fd) {
    struct stat statbuf;
    auto ret = fstat(fd, &statbuf);
    if (ret == -1) throw exception_t(err_t::BAD_FILE_DESCRIPTOR);

    uint64_t nanos = (uint64_t)statbuf.st_mtim.tv_sec * 1000000000 + (uint64_t)statbuf.st_mtim.tv_nsec;
    nanos = nanos & (~0xfffff);
    nanos += statbuf.st_size;

    return nanos;
}

struct filecacheentry {
    std::filesystem::path path { };
    size_t size { 0 };
    uint8_t *buffer { nullptr };
    uint64_t etag { 0 };

    filecacheentry() { }
    filecacheentry(const filecacheentry &cc) = delete;
    filecacheentry &operator=(const filecacheentry &) = delete;
    filecacheentry(filecacheentry &&cacheentry) : path { std::move(cacheentry.path) }, size { cacheentry.size }, buffer { cacheentry.buffer }, etag { cacheentry.etag } {
        cacheentry.buffer = nullptr;
        cacheentry.size = 0;
        cacheentry.etag = 0;
    }

    filecacheentry(const std::filesystem::path &path, size_t size, uint8_t *buffer, uint64_t etag) : path { path }, size { size }, buffer { buffer }, etag { etag } { }

    ~filecacheentry() {
        if (buffer) free(buffer);
    }
};

using filepairlist = std::list<filecacheentry>;

class filecache {
    const size_t max_size;
    filepairlist cachelist { };
    std::unordered_map<std::filesystem::path, filepairlist::iterator> cachemap { };
    size_t current_size { 0 };

public:
    filecache(const size_t max_memory_size = std::numeric_limits<size_t>::max()) : max_size { max_memory_size } { }

    const filecacheentry &GetCache(const std::filesystem::path &path) {
        auto cacheitr = cachemap.find(path);
        if (cacheitr == std::end(cachemap)) {
            if (!std::filesystem::is_regular_file(path)) {
                return empty;
            }
            int fd = open(path.c_str(), O_RDONLY);
            if ( fd == -1 ) {
                perror("Unable to open file");
                return empty;
            }

            struct stat bufstat;
            fstat(fd, &bufstat);

            size_t size = bufstat.st_size;
            auto buffer = malloc(size);
            const auto read_size = read(fd, buffer, size);
            const auto etag = get_etag(fd);
            close(fd);
            current_size += read_size;
            if (current_size > max_size) {
                auto &cacheback = cachelist.back();
                cachelist.pop_back();
                cachemap.erase(cacheback.path);
                current_size -= cacheback.size;
            }
            auto bufferentry = filecacheentry {path, size, reinterpret_cast<uint8_t *>(buffer), etag};
            cachelist.push_front(std::move(bufferentry));
            auto cachelistitr = std::begin(cachelist);
            cachemap.insert(std::pair(path, cachelistitr));
            return *cachelistitr;
        } else {
            auto bufferentry = cacheitr->second;
            return *bufferentry;
        }
    }

    static const filecacheentry empty;
};


class httpfilehandler : public http::handler_t {
    filecache &cache;
    const std::filesystem::path rootpath;
    const http::configuration_t &conf;

public:
    httpfilehandler(filecache &cache, const std::filesystem::path &rootpath, const http::configuration_t &conf)
        : cache { cache }, rootpath { std::filesystem::canonical(rootpath) }, conf { conf } { }

    const filecacheentry &GetFromfileCahce(const std::filesystem::path &fullpath) {
        if (std::filesystem::is_directory(fullpath)) {
            for(const auto &defaultfile: conf.defaultlist) {
                auto newpath = fullpath;
                newpath /= defaultfile;
                return cache.GetCache(newpath);
            }
            return filecache::empty;
        } else {
            return cache.GetCache(fullpath);
        }
    }

    static constexpr inline bool match_etag(const std::string &etag_match_list, const char *etag) {
        auto listitr = std::begin(etag_match_list);
        const auto listend = std::end(etag_match_list);
        if (listitr == listend) return false;

        for(;;) {
            auto ch = *listitr;
            while(ch == ' ' || ch == '\t' || ch == ',' || ch == '"') {
                listitr = std::next(listitr);
                if (listitr == listend) return false;
                ch = *listitr;
            }

            auto remaining_size = listend - listitr;
            if (static_cast<size_t>(remaining_size) < etag_size) return false;

            size_t index { 0 };
            for(; index < etag_size; ++index) {
                if (*listitr != etag[index]) break;
                listitr = std::next(listitr);
            }

            if (index == etag_size) {
                if (listitr == listend) return true;
                ch = *listitr;
                if (ch == ' ' || ch == '\t' || ch == ',' || ch == '"') return true;
            }

            while(ch != ' ' && ch != '\t' && ch != ',' && ch != '"') {
                listitr = std::next(listitr);
                if (listitr == listend) return false;
                ch = *listitr;
            }
        }

        // Non rechable code
        return false;
    }

    void ProcessRead(const MMS::http::request &request, const std::string &relative_path, http::protocol_t *writer) override;

    constexpr const std::vector<http::METHOD> &GetSupportedMethod() override {
        static const std::vector<http::METHOD> supported_methods {
            http::METHOD::GET,
            http::METHOD::HEAD
        };
        return supported_methods;
    }
    constexpr bool IsSupported(const http::METHOD method) override {
        return method == http::METHOD::GET || method == http::METHOD::HEAD;
    }
};

} // namespace MMS::server