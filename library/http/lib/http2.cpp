/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/http2.h>
#include <deque>

namespace MMS::http::v2 {

void CreateHeaderFrame(dynamic_table_t &dynamic_table, FullStream &stream, uint32_t stream_identifier, CODE code, const std::deque<std::pair<FIELD, std::string>> &fields) {
    constexpr uint64_t max_date_string_size = 92;
    std::time_t now_time = std::time(0);   // get time now
    std::tm* now_tm = std::gmtime(&now_time);
    char date_str[max_date_string_size];
    size_t date_str_size = strftime(date_str, max_date_string_size, "%a, %d %b %Y %H:%M:%S %Z", now_tm);

    auto frame_buffer = stream.GetCurrAndIncrease(sizeof(MMS::http::v2::frame));
    auto start_frame_body = stream.curr();
    copy_http_header_response(dynamic_table, stream, std::make_pair(FIELD::Status, std::to_string(static_cast<uint16_t>(code)) ), true);
    copy_http_header_response(dynamic_table, stream, std::make_pair(FIELD::Date, std::string { date_str, date_str_size } ), false);
    std::ranges::for_each(fields, [&](auto field) {copy_http_header_response(dynamic_table, stream, field, true);});
    new (frame_buffer) frame{ static_cast<uint32_t>(stream.GetSizeFrom(start_frame_body)), frame::type_t::HEADERS, frame::flags_t::END_HEADERS, stream_identifier };
}

void CreateBodyFrame(FullStream &stream, uint32_t max_frame_size, const ConstStream &bodystream, uint32_t stream_identifier) {
    const uint32_t max_body_size = max_frame_size - sizeof(frame);
    const uint32_t first_body_size = std::min(max_body_size, static_cast<uint32_t>(bodystream.remaining_buffer()));
    {
        auto frame_buffer = stream.GetCurrAndIncrease(sizeof(frame));
        auto start_frame_body = stream.GetCurrAndIncrease(first_body_size);
        auto bodystreamstart = bodystream.GetCurrAndIncrease(first_body_size);
        std::copy(bodystreamstart, bodystream.curr(), start_frame_body);
        if (bodystream.remaining_buffer()) {
            new (frame_buffer) frame{ first_body_size, frame::type_t::DATA, frame::flags_t::NONE, stream_identifier };
        } else {
            new (frame_buffer) frame{ first_body_size, frame::type_t::DATA, frame::flags_t::END_STREAM, stream_identifier };
            return;
        }
    }

    while(bodystream.remaining_buffer() > max_body_size) {
        auto frame_buffer = stream.GetCurrAndIncrease(sizeof(frame));
        auto start_frame_body = stream.GetCurrAndIncrease(max_body_size);
        auto bodystreamstart = bodystream.GetCurrAndIncrease(first_body_size);
        std::copy(bodystreamstart, bodystream.curr(), start_frame_body);
        new (frame_buffer) frame{ max_body_size, frame::type_t::CONTINUATION, frame::flags_t::NONE, stream_identifier };
    }

    const uint32_t last_body_size = bodystream.remaining_buffer();
    auto frame_buffer = stream.GetCurrAndIncrease(sizeof(MMS::http::v2::frame));
    auto start_frame_body = stream.GetCurrAndIncrease(last_body_size);
    auto bodystreamstart = bodystream.GetCurrAndIncrease(first_body_size);
    std::copy(bodystreamstart, bodystream.end(), start_frame_body);
    new (frame_buffer) frame{ last_body_size, frame::type_t::CONTINUATION, frame::flags_t::END_STREAM, stream_identifier };
}

} // namespace MMS::http::v2