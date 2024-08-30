/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http2.h>
#include <format>

namespace MMS::server::http::v2 {

void protocol_t::WriteError(const CODE code, const std::string &errortext) {
    constexpr uint64_t max_date_string_size = 92;
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    uint8_t date_str[max_date_string_size];
    size_t date_str_size = strftime((char *)date_str, max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm) + 1;
    if (current_request && header_request) {
        rohit::http::v2::frame *pframe = (rohit::http::v2::frame *)response_itr;
        response_itr += sizeof(rohit::http::v2::frame);
        response_itr = current_request.copy_http_header_response(response_itr, );
        response_itr = http2_add_404_Not_Found(response_itr, current_request, header_request, MMS::net::get_local_ipv6_addr(GetFD()), date_str, date_str_size);
    } else {
        auto res = response::CreateErrorResponse(code, errortext, configuration->ServerName);
        Write(res.to_string());
    }
}

void protocol_t::ProcessRead(const ConstStream &stream) {
    response_itr = response_buffer.get();
    try {
        // TODO: Optimize memory utilized by this.
        // Currently we are allocating huge memory
        http::v2::request request { dynamic_table, peer_settings };
        auto ret = request.parse(buffer, buffer + size);
        if (ret != err_t::HTTP2_INITIATE_GOAWAY) {
            std::string newpath { };
            auto &handler = configuration->handlermap.search(request.GetPath(), newpath);
            if (handler == nullptr) {
                WriteError(CODE::_404,  std::format("Path {} not found", request.GetPath()));
            }
            else {
                current_request = &request;
                handler->ProcessRead(request, newpath, this);
            }
        }
    }
    catch(http_parser_failed_t &parser_failed) {
        WriteError(CODE::_400, parser_failed.to_string(reinterpret_cast<const char *>(buffer)));
    }

    response_itr = nullptr;
    body_buffer = nullptr;
    current_request = nullptr;
    FinalizeWrite();
}


void protocol_t::Write(const CODE code, const char *bodybuffer, size_t bodysize, const std::vector<std::pair<FIELD, std::string>> &fields) {
    auto response = response::CreateBasicResponse(code);
    std::ranges::for_each(fields, [&response](const std::pair<FIELD, std::string> &field) { response.add_field(field); });
    response.add_field(MMS::http::FIELD::Server, configuration->ServerName);
    response.add_field(FIELD::Content_Length, bodysize);
    Write(
        listener::write_entry_const { response.to_string()},
        listener::write_entry_const { bodybuffer, bodysize });
}

} // namespace MMS::server::http