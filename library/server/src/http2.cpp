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

void protocol_t::Write(const CODE code, std::vector<std::pair<FIELD, std::string>> &fields) {
    fields.emplace_back(FIELD::Server, configuration->ServerName);
    MMS::http::v2::CreateHeaderFrame(dynamic_table, response_buffer, header_request->stream_identifier, code, fields);
}

void protocol_t::FinalizeWrite() {
    if (!response_buffer.empty()) {
        auto writestream = response_buffer.ReturnOldAndAlloc(response_buffe_initial_size);
        FixedBuffer writebuffer { std::move(writestream) };
        WriteNoCopy(std::move(writebuffer));
    }
}

void protocol_t::AddBase64Settings(const std::string &settings) {
    peer_settings.parse_base64( ConstStream { settings.c_str(), settings.size() }, &configuration->limits);
}

void protocol_t::AddSettingResponse() {
    if (!settings_responded) {
        MMS::http::v2::settings::add_frame(response_buffer,
            MMS::http::v2::settings::identifier_t::SETTINGS_ENABLE_PUSH, peer_settings.SETTINGS_ENABLE_PUSH,
            MMS::http::v2::settings::identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS, peer_settings.SETTINGS_MAX_CONCURRENT_STREAMS,
            MMS::http::v2::settings::identifier_t::SETTINGS_INITIAL_WINDOW_SIZE, peer_settings.SETTINGS_INITIAL_WINDOW_SIZE,
            MMS::http::v2::settings::identifier_t::SETTINGS_HEADER_TABLE_SIZE, peer_settings.SETTINGS_HEADER_TABLE_SIZE,
            MMS::http::v2::settings::identifier_t::SETTINGS_MAX_HEADER_LIST_SIZE, peer_settings.SETTINGS_MAX_HEADER_LIST_SIZE,
            MMS::http::v2::settings::identifier_t::SETTINGS_MAX_FRAME_SIZE, peer_settings.SETTINGS_MAX_FRAME_SIZE
        );
        settings_responded = true;
    }
}

void protocol_t::Upgrade(MMS::http::request &&request) {
    try {
        response_buffer.Copy(MMS::http::v2::connection_upgrade, MMS::http::v2::connection_upgrade_size);
        AddSettingResponse();
        MMS::http::v2::settings::add_ack_frame(response_buffer);
        MMS::http::v2::header_request newreq { std::move(request) };
        header_request = &newreq;
        ProcessRequest();
        header_request = nullptr;
    }
    catch(http_parser_failed_t &parser_failed) {
        WriteError(CODE::_400, parser_failed.to_string());
    }
    FinalizeWrite();
}

void protocol_t::ProcessRequest() {
    std::string newpath { };
    auto &handler = configuration->handlermap.search(header_request->GetPath(), newpath);
    if (handler == nullptr) {
        WriteError(CODE::_404,  std::format("Path {} not found", header_request->GetPath()));
    }
    else {
        auto method = header_request->GetMethod();
        if (handler->IsSupported(method)) {
            handler->ProcessRead(*header_request, newpath, this);
        }
        else if (method == METHOD::OPTIONS) {
            std::string optionsstr {};
            CreateSupportedMethodString(optionsstr, handler->GetSupportedMethod(), http::METHOD::PRI, http::METHOD::OPTIONS);
            std::vector<std::pair<FIELD, std::string>> fields {
                std::pair<FIELD, std::string> {http::FIELD::Allow, optionsstr}
            };
            Write(http::CODE::_204, fields);
        } else {
            const std::string errstr = CreateSupportedMethodErrorString(method, handler->GetSupportedMethod(), http::METHOD::PRI, http::METHOD::OPTIONS);
            WriteError(http::CODE::_405, errstr);
        }
    }
}

void protocol_t::ProcessRead(const ConstStream &stream) {
    response_buffer.Reset();
    try {
        MMS::http::v2::request request { dynamic_table, peer_settings };
        if (first_frame) {
            if (stream == std::string_view { MMS::http::v2::connection_preface, MMS::http::v2::connection_preface_size }) {
                stream += MMS::http::v2::connection_preface_size;
                auto acksettings = request.CheckAndParseSetting(stream, &configuration->limits);
                AddSettingResponse();
                if (acksettings) MMS::http::v2::settings::add_ack_frame(response_buffer);
            }
            first_frame = false;
        }
        auto ret = request.parse(stream, response_buffer);
        if (ret != err_t::HTTP2_INITIATE_GOAWAY) {
            header_request = request.get_first_header();
            while(header_request) {
                ProcessRequest();
                header_request = header_request->get_next();
            }
        }
    }
    catch(http_parser_failed_t &parser_failed) {
        WriteError(CODE::_400, parser_failed.to_string());
    }

    header_request = nullptr;

    FinalizeWrite();
}


} // namespace MMS::server::http