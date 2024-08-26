/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/server/http.h>
#include <filesystem>
#include <list>
#include <unordered_map>
#include <memory>

namespace MMS::server {
struct filecacheentry {
    std::filesystem::path path;
    size_t size;
    uint8_t *buffer;

    filecacheentry(const filecacheentry &cc) = delete;
    filecacheentry &operator=(const filecacheentry &) = delete;
    filecacheentry(filecacheentry &&cacheentry) : path { cacheentry.path }, size { cacheentry.size }, buffer { cacheentry.buffer } {
        cacheentry.buffer = nullptr;
        cacheentry.size = 0;
    }

    filecacheentry(const std::filesystem::path &path, size_t size, uint8_t *buffer) : path { path }, size { size }, buffer { buffer } { }

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

    std::pair<const char *, size_t> GetCache(const std::filesystem::path &path) {
        auto cacheitr = cachemap.find(path);
        if (cacheitr == std::end(cachemap)) {
            if (!std::filesystem::is_regular_file(path)) {
                return {nullptr, 0};
            }
            int fd = open(path.c_str(), O_RDONLY);
            if ( fd == -1 ) {
                perror("Unable to open file");
                return { nullptr, 0 };
            }

            struct stat bufstat;
            fstat(fd, &bufstat);

            size_t size = bufstat.st_size;
            auto buffer = malloc(size);
            auto read_size = read(fd, buffer, size);
            current_size += read_size;
            if (current_size > max_size) {
                auto &cacheback = cachelist.back();
                cachelist.pop_back();
                cachemap.erase(cacheback.path);
                current_size -= cacheback.size;
            }
            auto bufferentry = filecacheentry {path, size, reinterpret_cast<uint8_t *>(buffer)};
            cachelist.push_front(std::move(bufferentry));
            auto cachelistitr = std::begin(cachelist);
            cachemap.insert(std::pair(path, cachelistitr));
            return { reinterpret_cast<char *>(buffer), size };
        } else {
            auto bufferentry = cacheitr->second;
            return { reinterpret_cast<char *>(bufferentry->buffer), bufferentry->size };
        }
    }


};


class httpfilehandler : public http::handler_t {
    filecache &cache;
    const std::filesystem::path rootpath;
    const http::configuration_t &conf;

public:
    httpfilehandler(filecache &cache, const std::filesystem::path &rootpath, const http::configuration_t &conf)
        : cache { cache }, rootpath { std::filesystem::canonical(rootpath) }, conf { conf } { }

    auto GetFromfileCahce(const std::filesystem::path &fullpath) {
        if (std::filesystem::is_directory(fullpath)) {
            for(const auto &defaultfile: conf.defaultlist) {
                auto newpath = fullpath;
                newpath /= defaultfile;
                auto [buffer, size] = cache.GetCache(newpath);
                if (buffer) return std::make_tuple(buffer, size, newpath);
            }
            return std::tuple<const char *, size_t, std::filesystem::path>(nullptr, 0, {});
        } else {
            auto [buffer, size] = cache.GetCache(fullpath);
            return std::make_tuple(buffer, size, fullpath);
        }
    }

    void ProcessRead(const MMS::http::request &request, const std::string &relative_path, listener::writer_t &writer) override;
};

} // namespace MMS::server