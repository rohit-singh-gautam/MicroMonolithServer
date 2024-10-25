//////////////////////////////////////////////////////////////////////////
// Copyright (C) 2024  Rohit Jairaj Singh (rohit@singh.org.in)          //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program.  If not, see <https://www.gnu.org/licenses/>//
//////////////////////////////////////////////////////////////////////////

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
        writer->WriteError(http::CODE::Method_Not_Allowed, std::format("Method {} not allowed", to_string(method)));
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
        writer->WriteError(http::CODE::Forbidden, errortext);
        return;
    }

    auto &filecacheentry = GetFromfileCahce(fullpath);
    auto &newpath = filecacheentry.path;

    if (filecacheentry.buffer == nullptr) {
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not found";
        writer->WriteError(http::CODE::Not_Found, errortext);
        return;
    }

    auto etag_match_list = request.GetField(MMS::http::FIELD::If_None_Match);
    char etag_str[etag_size];
    to_string64_hash(filecacheentry.etag, etag_str);
    if (!etag_match_list.empty()) {
        bool matchetag = match_etag(etag_match_list, etag_str);
        if (matchetag) {
            writer->Write(http::CODE::Not_Modified, std::pair<http::FIELD, std::string> { MMS::http::FIELD::ETag, { etag_str } });
            return;
        }
    }

    const auto extension = newpath.extension().string();
    auto contenttype = mimemap.find(extension);
    if (contenttype == std::end(mimemap)) {
        log<log_t::HTTP_UNKNOWN_EXTENSION>(writer->GetFD());
        std::string errortext { "File: "};
        errortext += request.GetPath();
        errortext += " not found";
        writer->WriteError(http::CODE::Not_Found, errortext);
        return;
    }
    if (method == http::METHOD::GET) {
        auto stream = make_const_stream(filecacheentry.buffer, filecacheentry.size);
        writer->Write(http::CODE::OK, stream, 
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Cache_Control, { "private, max-age=2592000" } },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Content_Type, contenttype->second },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::ETag, { etag_str } }
        );
    } else { // if (method == http::METHOD::HEAD)
        writer->Write(http::CODE::OK, 
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Cache_Control, { "private, max-age=2592000" } },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Content_Type, contenttype->second },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::Content_Length, std::to_string(filecacheentry.size) },
            std::pair<http::FIELD, std::string> { MMS::http::FIELD::ETag, { etag_str } }
        );
    }

}

} // namespace MMS::server