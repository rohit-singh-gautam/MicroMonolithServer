/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/httpfilehandler.h>

namespace MMS::server {
void httpfilehandler::ProcessRead(const MMS::http::request &request, const std::string &relative_path, listener::writer_t &writer) {
    std::filesystem::path fullpath = rootpath;
    fullpath /= relative_path;
    auto absolutepath = std::filesystem::canonical(fullpath);

    auto is_subfolder = std::mismatch(std::begin(rootpath), std::end(rootpath), std::begin(absolutepath), std::end(absolutepath)).first == std::end(rootpath);
    
    if (!is_subfolder) {
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not allowed.";
        auto response = request.CreateErrorResponse(MMS::http::CODE::_403, errortext);
        response.add_field(MMS::http::FIELD::Server, conf.ServerName);
        writer.Write(response.to_string());
        return;
    }

    auto [bodybuffer, bodysize, newpath] = GetFromfileCahce(fullpath);

    if (bodybuffer == nullptr) {
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not found";
        auto response = request.CreateErrorResponse(MMS::http::CODE::_404, errortext);
        response.add_field(MMS::http::FIELD::Server, conf.ServerName);
        writer.Write(response.to_string());
    } else {
        auto response = MMS::http::response::CreateBasicResponse(MMS::http::CODE::_200);
        response.add_field(MMS::http::FIELD::Server, conf.ServerName);
        response.add_field(MMS::http::FIELD::Cache_Control, { "private, max-age=2592000" });
        const auto extension = newpath.extension().string();
        auto contenttype = conf.mimemap.find(extension);
        if (contenttype == std::end(conf.mimemap)) {
            log<log_t::HTTP_UNKNOWN_EXTENSION>();
            response.add_field(MMS::http::FIELD::Content_Type, { "text/plain" });    
        } else {
            response.add_field(MMS::http::FIELD::Content_Type, contenttype->second);
        }
        auto contentlength { std::to_string(bodysize) };
        response.add_field(MMS::http::FIELD::Content_Length, contentlength );
        auto headerstring = response.to_string();
        writer.Write(
            listener::write_entry_const {headerstring.c_str(), headerstring.size(), 0}, 
            listener::write_entry_const {bodybuffer, bodysize, 0});
    }
}

} // namespace MMS::server