/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/httpfilehandler.h>

namespace MMS::server {
void httpfilehandler::ProcessRead(const MMS::http::request &request, const std::string &relative_path, http::protocol_t *writer) {
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

    auto [bodybuffer, bodysize, newpath] = GetFromfileCahce(fullpath);

    if (bodybuffer == nullptr) {
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not found";
        writer->WriteError(http::CODE::_404, errortext);
    } else {
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
        writer->Write(http::CODE::_200, bodybuffer, bodysize, 
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Cache_Control, { "private, max-age=2592000" } },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Content_Type, contenttype->second });
    }
}

} // namespace MMS::server