/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/httpfilehandler.h>
#include <format>

namespace MMS::server {

const filecacheentry filecache::empty { };

void httpfilehandler::ProcessRead(const MMS::http::request &request, const std::string &relative_path, http::protocol_t *writer) {
    auto method = request.GetMethod();

#if defined(DEBUG) || defined(NDEBUG)
// This check is already present at caller of this function
// Additional check will be only in debug mode
    if (!IsSupported(method)) {
        writer->WriteError(http::CODE::_405, std::format("Method {} not allowed", to_string(method)));
        return;
    }
#endif
    std::filesystem::path fullpath = rootpath;
    fullpath /= relative_path;

    auto is_subfolder = std::mismatch(std::begin(rootpath), std::end(rootpath), std::begin(fullpath), std::end(fullpath)).first == std::end(rootpath);
    
    if (!is_subfolder) {
        std::string errortext { "File/Path: "};
        errortext += request.GetPath();
        errortext += " forbidden.";
        writer->WriteError(http::CODE::_403, errortext);
        return;
    }

    auto &filecacheentry = GetFromfileCahce(fullpath);
    auto &newpath = filecacheentry.path;

    if (filecacheentry.buffer == nullptr) {
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not found";
        writer->WriteError(http::CODE::_404, errortext);
        return;
    }

    auto etag_match_list = request.GetField(MMS::http::FIELD::If_None_Match);
    char etag_str[etag_size];
    to_string64_hash(filecacheentry.etag, etag_str);
    if (!etag_match_list.empty()) {
        bool matchetag = match_etag(etag_match_list, etag_str);
        if (matchetag) {
            writer->Write(http::CODE::_304, std::pair<http::FIELD, std::string> { MMS::http::FIELD::ETag, { etag_str } });
            return;
        }
    }

    const auto extension = newpath.extension().string();
    auto contenttype = conf.mimemap.find(extension);
    if (contenttype == std::end(conf.mimemap)) {
        log<log_t::HTTP_UNKNOWN_EXTENSION>(writer->GetFD());
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not found";
        writer->WriteError(http::CODE::_404, errortext);
        return;
    }
    if (method == http::METHOD::GET) {
        ConstStream stream { filecacheentry.buffer, filecacheentry.size };
        writer->Write(http::CODE::_200, stream, 
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Cache_Control, { "private, max-age=2592000" } },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Content_Type, contenttype->second },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::ETag, { etag_str } }
        );
    } else { // if (method == http::METHOD::HEAD)
        writer->Write(http::CODE::_200, 
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Cache_Control, { "private, max-age=2592000" } },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Content_Type, contenttype->second },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::ETag, { etag_str } }
        );
    }

}

} // namespace MMS::server