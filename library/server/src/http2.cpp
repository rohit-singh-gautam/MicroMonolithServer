/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/server/http2.h>
#include <http/http2.h>
#include <format>

namespace MMS::server::http::v2 {

void protocol_t::WriteError(const CODE code, const std::string &errortext) {
    std::vector<std::pair<FIELD, std::string>> fields {
        { FIELD::Content_Type, std::string { "text/html" } }
    };

    auto body = std::format(
        "<html>"
        "<head><title>{0}</title></head>"
        "<h1>{2}</h1>"
        "<h2>{0}</h2>"
        "<pre>{1}</pre>"
        "</html>",
        MMS::http::to_string(code), errortext, configuration->ServerName
    );

    auto icode = static_cast<uint16_t>(code);
    if (icode >= 400 && icode <= 599) fields.emplace_back( FIELD::Connection, std::string { "close" } );
    if (icode == 405) fields.emplace_back( FIELD::Connection, std::string { "GET, PRI" } );

    Write(code, ConstStream { body }, fields);
}

void protocol_t::Write(const CODE code, const ConstStream &bodystream, std::vector<std::pair<FIELD, std::string>> &fields) {
    fields.emplace_back(FIELD::Server, configuration->ServerName);
    fields.emplace_back(FIELD::Content_Length, std::to_string(bodystream.remaining_buffer()));
    MMS::http::v2::CreateHeaderFrame(dynamic_table, response_buffer, header_request->stream_identifier, code, fields);
    auto bodysize = bodystream.remaining_buffer();
    response_buffer.Reserve(bodysize + (bodysize / (configuration->max_frame_size - sizeof(MMS::http::v2::frame)) * sizeof(MMS::http::v2::frame)));
    MMS::http::v2::CreateBodyFrame(response_buffer, configuration->max_frame_size, bodystream, header_request->stream_identifier);
}

void protocol_t::FinalizeWrite() {
    if (!response_buffer.empty()) {
        auto writestream = response_buffer.ReturnOldAndAlloc(response_buffe_initial_size);
        Write<false>(writestream.begin(), writestream.index());
    }
}

void protocol_t::ProcessRead(const ConstStream &stream) {
    response_buffer.Reset();
    try {
        if (first_frame) {
            // TODO: Pull setting range from configurations
            MMS::http::v2::settings::add_frame(response_buffer,
                MMS::http::v2::settings::identifier_t::SETTINGS_ENABLE_PUSH, 0,
                MMS::http::v2::settings::identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS, 10,
                MMS::http::v2::settings::identifier_t::SETTINGS_INITIAL_WINDOW_SIZE, 1048576,
                MMS::http::v2::settings::identifier_t::SETTINGS_HEADER_TABLE_SIZE, 2048
            );
            first_frame = false;
        }
        MMS::http::v2::request request { dynamic_table, peer_settings };
        auto ret = request.parse(stream, response_buffer);
        current_request = &request;
        if (ret != err_t::HTTP2_INITIATE_GOAWAY) {
            auto header = request.get_first_header();
            while(header) {
                header_request = header;
                std::string newpath { };
                auto &handler = configuration->handlermap.search(header->GetPath(), newpath);
                if (handler == nullptr) {
                    WriteError(CODE::_404,  std::format("Path {} not found", header->GetPath()));
                }
                else {
                    handler->ProcessRead(*header, newpath, this);
                }
                header = header->get_next();
            }
        }
    }
    catch(http_parser_failed_t &parser_failed) {
        WriteError(CODE::_400, parser_failed.to_string());
    }

    current_request = nullptr;
    header_request = nullptr;

    FinalizeWrite();
}


} // namespace MMS::server::http