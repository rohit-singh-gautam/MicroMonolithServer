/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <http/hpack.h>
#include <mms/base/maths.h>
#include <iostream>
#include <mms/listener.h>

namespace MMS::http::v2 {

namespace constant {
    constexpr uint32_t SETTINGS_HEADER_TABLE_SIZE = 4096;
    constexpr bool SETTINGS_ENABLE_PUSH = false;
    constexpr uint32_t SETTINGS_MAX_CONCURRENT_STREAMS = 100;
    constexpr uint32_t SETTINGS_INITIAL_WINDOW_SIZE = 65535;
    constexpr uint32_t SETTINGS_MAX_FRAME_SIZE = 16777215;
    constexpr uint32_t SETTINGS_MAX_HEADER_LIST_SIZE = 1024;

} // namespace constant

constexpr char connection_preface[] = {
        'P', 'R', 'I', ' ', '*', ' ', 
        'H', 'T', 'T', 'P', '/', '2', '.', '0',
        '\r', '\n', '\r', '\n', 'S', 'M', '\r', '\n', '\r', '\n'};

constexpr size_t connection_preface_size = std::size(connection_preface);


// This is definition of frame
// Can be called as frame header
struct frame {
    enum class type_t : uint8_t {
        DATA            = 0x00,
        HEADERS         = 0x01,
        PRIORITY        = 0x02,
        RST_STREAM      = 0x03,
        SETTINGS        = 0x04,
        PUSH_PROMISE    = 0x05,
        PING            = 0x06,
        GOAWAY          = 0x07,
        WINDOW_UPDATE   = 0x08,
        CONTINUATION    = 0x09,
    };

    static constexpr auto get_string(const type_t type) {
        switch(type) {
            case type_t::DATA: return "DATA";
            case type_t::HEADERS: return "HEADERS";
            case type_t::PRIORITY: return "PRIORITY";
            case type_t::RST_STREAM: return "RST STREAM";
            case type_t::SETTINGS: return "SETTINGS";
            case type_t::PUSH_PROMISE: return "PUSH PROMISE";
            case type_t::PING: return "PING";
            case type_t::GOAWAY: return "GOAWAY";
            case type_t::WINDOW_UPDATE: return "WINDOW UPDATE";
            case type_t::CONTINUATION: return "CONTINUATION";
            default: return "BAD TYPE";
        }
    }

    enum class flags_t : uint8_t {
        NONE        = 0x00,
        END_STREAM  = 0x01,
        END_HEADERS = 0x04,
        PADDED      = 0x08,
        PRIORITY    = 0x20,

        // Flags are different for ping
        REQUEST = 0x00,
        ACK     = 0x01,
    };

    static constexpr auto get_string(const flags_t flag) {
        switch(flag) {
            case flags_t::NONE: return "NONE";
            case flags_t::END_STREAM: return "END STREAM";
            case flags_t::END_HEADERS: return "END HEADERS";
            case flags_t::PADDED: return "PADDED";
            case flags_t::PRIORITY: return "PRIORITY";
            default: return "BAD FLAG";
        }
    }

    enum class error_t : uint32_t {
        NO_ERROR            = 0x00,
        PROTOCOL_ERROR      = 0x01,
        INTERNAL_ERROR      = 0x02,
        FLOW_CONTROL_ERROR  = 0x03,
        SETTINGS_TIMEOUT    = 0x04,
        STREAM_CLOSED       = 0x05,
        FRAME_SIZE_ERROR    = 0x06,
        REFUSED_STREAM      = 0x07,
        CANCEL              = 0x08,
        COMPRESSION_ERROR   = 0x09,
        CONNECT_ERROR       = 0x0a,
        ENHANCE_YOUR_CALM   = 0x0b,
        INADEQUATE_SECURITY = 0x0c,
        HTTP_1_1_REQUIRED   = 0x0d,
    };

    static constexpr auto get_string(const error_t err) {
        switch(err) {
            case error_t::NO_ERROR: return "NO ERROR";
            case error_t::PROTOCOL_ERROR: return "PROTOCOL ERROR";
            case error_t::INTERNAL_ERROR: return "INTERNAL ERROR";
            case error_t::FLOW_CONTROL_ERROR: return "FLOW CONTROL ERROR";
            case error_t::SETTINGS_TIMEOUT: return "SETTINGS TIMEOUT";
            case error_t::STREAM_CLOSED: return "STREAM CLOSED";
            case error_t::FRAME_SIZE_ERROR: return "FRAME SIZE ERROR";
            case error_t::REFUSED_STREAM: return "REFUSED STREAM";
            case error_t::CANCEL: return "CANCEL";
            case error_t::COMPRESSION_ERROR: return "COMPRESSION ERROR";
            case error_t::CONNECT_ERROR: return "CONNECT ERROR";
            case error_t::ENHANCE_YOUR_CALM: return "ENHANCE YOUR CALM";
            case error_t::INADEQUATE_SECURITY: return "INADEQUATE SECURITY";
            case error_t::HTTP_1_1_REQUIRED: return "HTTP 1.1 REQUIRED";
            default: return "UNKNOWN ERROR";
        }
    }

private:
    /* big endian format
    uint32_t    length:24;
    type_t      type;
    flags_t     flags;
    uint32_t    reserved:1;
    uint32_t    stream_identifier:31; */

    // Little endian format
    uint32_t    length:24 { };
    type_t      type { };
    flags_t     flags { };
    uint32_t    stream_identifier:31 { };
    uint32_t    reserved:1 { };

public:

    constexpr const char * get_string() const { return get_string(type); }

    constexpr uint32_t get_length() const { return changeEndian<std::endian::big, std::endian::native>(length << 8); }
    constexpr void set_length(const uint32_t length) { this->length = changeEndian<std::endian::native, std::endian::big>(length) >> 8;}
    constexpr type_t get_type() const { return type; }
    constexpr void set_type(const type_t type) { this->type = type; }
    constexpr flags_t get_flags() const { return flags; }
    constexpr void clean_flag() { flags = flags_t::NONE; }
    constexpr void set_flag(const flags_t flag) { flags = (flags_t)((uint8_t)flags | (uint8_t)flag); }
    constexpr bool contains(const flags_t flag) const { return (flags_t)((uint8_t)flags & (uint8_t)flag) == flag; }
    constexpr uint32_t get_stream_identifier() const { return changeEndian<std::endian::big, std::endian::native>(stream_identifier); }
    constexpr void set_stream_identifier(const uint32_t stream_identifier) {
        this->stream_identifier = changeEndian<std::endian::native, std::endian::big>(stream_identifier);
    }

    constexpr void init_frame(uint32_t length, type_t type, flags_t flag, uint32_t stream_identifier) {
        set_length(length);
        set_type(type);
        flags = flag; // This will also clean flag
        reserved = 0;
        set_stream_identifier(stream_identifier); 
    }

    constexpr void init_frame(uint32_t length, type_t type, flags_t flag, flags_t flag1, uint32_t stream_identifier) {
        set_length(length);
        set_type(type);
        flags = (flags_t)((uint8_t)flag | (uint8_t)flag1); // This will also clean flag
        reserved = 0;
        set_stream_identifier(stream_identifier); 
    }

    constexpr void init_frame(uint32_t length, type_t type, flags_t flag, flags_t flag1, flags_t flag2, uint32_t stream_identifier) {
        set_length(length);
        set_type(type);
        flags = (flags_t)((uint8_t)flag | (uint8_t)flag1 | (uint8_t)flag2); // This will also clean flag
        reserved = 0;
        set_stream_identifier(stream_identifier); 
    }


    constexpr frame() {}

    constexpr frame(
                uint32_t    length,
                type_t      type,
                flags_t     flags,
                uint32_t    stream_identifier)
            : length(changeEndian<std::endian::native, std::endian::big>(length) >> 8),
              type(type),
              flags(flags),
              stream_identifier(changeEndian<std::endian::native, std::endian::big>(stream_identifier)),
              reserved(0) {}
} __attribute__((packed)); // struct frame

inline std::ostream& operator<<(std::ostream& os, const frame::type_t &http2frametype) {
    return os << (uint16_t)http2frametype << ':' << frame::get_string(http2frametype);
}

inline std::ostream& operator<<(std::ostream& os, const frame::flags_t &http2frameflag) {
    os << (uint16_t)http2frameflag;
    uint8_t flags = (uint8_t)http2frameflag;
    
    if (flags) os << ':';

    while(flags) {
        uint8_t next_flags = flags & (flags - 1);
        const frame::flags_t flg = (frame::flags_t)(flags^next_flags);
        os << '(' << (uint16_t)flg << ')';
        os << frame::get_string(flg);
        if (next_flags) os << ",";
        flags = next_flags;
    }

    return os;
}

inline std::ostream& operator<<(std::ostream& os, const frame::error_t &http2frameerror) {
    return os << (uint32_t)http2frameerror << ':' << frame::get_string(http2frameerror);
}

inline std::ostream& operator<<(std::ostream& os, const frame &http2frame) {
    return os << "{L:" << http2frame.get_length() << ", T:" << http2frame.get_type()
        << ", F:" << http2frame.get_flags() << ", ID:" << http2frame.get_stream_identifier() << '}';
}

struct header {
private:
    /* Big endian
    uint32_t    E:1;
    uint32_t    stream_dependency:31; */

    // Little endian
    uint32_t    stream_dependency:31;
    uint32_t    E:1;
    //uint8_t     weight;
public:
    constexpr bool get_execlusive() const { return E; }
    constexpr const char *get_exclusive_string() const { return E ? "Exclusive" : ""; }
    constexpr uint32_t get_stream_dependency() const { return changeEndian<std::endian::big, std::endian::native>(stream_dependency); }
    //constexpr uint8_t get_weight() const { return weight; }
} __attribute__((packed)); // struct header

inline std::ostream& operator<<(std::ostream& os, const header &header) {
    return os << '[' << (header.get_execlusive() ? "Exclusive," : "") << "S:" << header.get_stream_dependency()
              << ']';

}

struct settings {
    enum class identifier_t : uint16_t {
        SETTINGS_HEADER_TABLE_SIZE          = 0x01,
        SETTINGS_ENABLE_PUSH                = 0x02,
        SETTINGS_MAX_CONCURRENT_STREAMS     = 0x03,
        SETTINGS_INITIAL_WINDOW_SIZE        = 0x04,
        SETTINGS_MAX_FRAME_SIZE             = 0x05,
        SETTINGS_MAX_HEADER_LIST_SIZE       = 0x06,
    };

    static constexpr auto get_string(const identifier_t id) {
        switch(id) {
            case identifier_t::SETTINGS_HEADER_TABLE_SIZE: return "HEADER TABLE SIZE";
            case identifier_t::SETTINGS_ENABLE_PUSH: return "ENABLE PUSH";
            case identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS: return "MAX CONCURRENT STREAMS";
            case identifier_t::SETTINGS_INITIAL_WINDOW_SIZE: return "INITIAL WINDOW SIZE";
            case identifier_t::SETTINGS_MAX_FRAME_SIZE: return "MAX FRAME SIZE";
            case identifier_t::SETTINGS_MAX_HEADER_LIST_SIZE: return "MAX HEADER LIST SIZE";
            default: return "UNKNOWN SETTINGS";
        }
    }

private:
    uint16_t identifier;
    uint32_t value;

    template <typename... ARGS>
    static constexpr auto add(uint8_t *buffer, const identifier_t identifier, uint32_t value, const ARGS&... args) {
        buffer = add(buffer, identifier, value);
        return add(buffer, args...);
    }

public:
    constexpr auto get_identifier() const { return static_cast<identifier_t>(changeEndian<std::endian::big, std::endian::native>(identifier)); }
    constexpr auto get_value() const { return changeEndian<std::endian::big, std::endian::native>(value); }

    constexpr void init(const identifier_t identifier, const uint32_t value) {
        this->identifier = changeEndian<std::endian::native, std::endian::big>((uint16_t)identifier);
        this->value = changeEndian<std::endian::native, std::endian::big>(value);
    }

    static inline auto add(uint8_t *buffer, const identifier_t identifier, const uint32_t value) {
        settings *psettings = reinterpret_cast<settings *>(buffer);
        psettings->init(identifier, value);
        return buffer + sizeof(settings);
    }

    static constexpr size_t get_frame_len(size_t count) {
        return sizeof(frame) + count * 6;
    }

    static inline auto add_ack_frame(Stream &stream) {
        frame *pframe = reinterpret_cast<frame *>(stream.GetCurrAndIncrease(sizeof(frame)));
        pframe->init_frame(0x00, frame::type_t::SETTINGS, frame::flags_t::ACK, 0x00);
    }

    template <typename... ARGS>
    static inline auto add_frame(uint8_t *buffer, const ARGS&... args) {
        constexpr uint32_t length = sizeof...(ARGS) * 3;
        frame *pframe = reinterpret_cast<frame *>(buffer);
        buffer += sizeof(frame);
        pframe->init_frame(length, frame::type_t::SETTINGS, frame::flags_t::NONE, 0x00);
        return add(buffer, args...);
    }

} __attribute__((packed)); // struct settings

class settings_store {
public:
    uint32_t SETTINGS_HEADER_TABLE_SIZE;
    bool SETTINGS_ENABLE_PUSH;
    uint32_t SETTINGS_MAX_CONCURRENT_STREAMS;
    uint32_t SETTINGS_INITIAL_WINDOW_SIZE;
    uint32_t SETTINGS_MAX_FRAME_SIZE;
    uint32_t SETTINGS_MAX_HEADER_LIST_SIZE;

    constexpr settings_store() :
                SETTINGS_HEADER_TABLE_SIZE(constant::SETTINGS_HEADER_TABLE_SIZE),
                SETTINGS_ENABLE_PUSH(constant::SETTINGS_ENABLE_PUSH),
                SETTINGS_MAX_CONCURRENT_STREAMS(constant::SETTINGS_MAX_CONCURRENT_STREAMS),
                SETTINGS_INITIAL_WINDOW_SIZE(constant::SETTINGS_INITIAL_WINDOW_SIZE),
                SETTINGS_MAX_FRAME_SIZE(constant::SETTINGS_MAX_FRAME_SIZE),
                SETTINGS_MAX_HEADER_LIST_SIZE(constant::SETTINGS_MAX_HEADER_LIST_SIZE)
    {}

    inline const auto parse_one(const ConstStream &stream) {
        auto psettings = reinterpret_cast<const settings *>(stream.GetCurrAndIncrease(sizeof(settings)));
        switch(psettings->get_identifier()) {
        case settings::identifier_t::SETTINGS_HEADER_TABLE_SIZE:
            SETTINGS_HEADER_TABLE_SIZE = psettings->get_value();
            break;
        case settings::identifier_t::SETTINGS_ENABLE_PUSH:
            SETTINGS_ENABLE_PUSH = psettings->get_value() != 0;
            break;
        case settings::identifier_t::SETTINGS_MAX_CONCURRENT_STREAMS:
            SETTINGS_MAX_CONCURRENT_STREAMS = psettings->get_value();
            break;
        case settings::identifier_t::SETTINGS_INITIAL_WINDOW_SIZE:
            SETTINGS_INITIAL_WINDOW_SIZE = psettings->get_value();
            break;
        case settings::identifier_t::SETTINGS_MAX_FRAME_SIZE:
            SETTINGS_MAX_FRAME_SIZE = psettings->get_value();
            break;
        case settings::identifier_t::SETTINGS_MAX_HEADER_LIST_SIZE:
            SETTINGS_MAX_HEADER_LIST_SIZE = psettings->get_value();
            break;
        default:
            // Ignore this settings
            break;
        }
    }

    constexpr void parse_frame(const ConstStream &stream) {
        while(stream.remaining_buffer()) {
            parse_one(stream);
        }
    }

    inline void parse_base64_frame(const ConstStream &stream) {
        const size_t decode_len = base64_decode_len(stream.curr(), stream.remaining_buffer());
        auto decoded_buffer = std::make_unique<uint8_t[]>(decode_len);
        base64_decode(stream.curr(), stream.remaining_buffer(), decoded_buffer.get());

        ConstStream decoded_stream { decoded_buffer.get(), decode_len };
        const frame *pframe = reinterpret_cast<const frame *>(decoded_stream.GetCurrAndIncrease(sizeof(frame)));
        const uint8_t padded_bytes = pframe->contains(frame::flags_t::PADDED) ? *decoded_stream++ : 0;
        auto frameStreamBuffer = decoded_stream.GetCurrAndIncrease(pframe->get_length());
        ConstStream frameStream { frameStreamBuffer, frameStreamBuffer +  pframe->get_length() - padded_bytes};
        parse_frame( frameStream );
    }
};

inline std::ostream& operator<<(std::ostream& os, const settings::identifier_t &id) {
    return os << settings::get_string(id);
}

inline std::ostream& operator<<(std::ostream& os, const settings &settings) {
    return os << "[" << settings.get_identifier() << "," << settings.get_value() << "]";
}

inline void copy_http_header_response(
            dynamic_table_t &dynamic_table,
            Stream &stream,
            const std::pair<FIELD, std::string> &header_line,
            bool add_index) {
    // 6.1.  Indexed Header Field Representation
    // Case 1: found entry in static table
    if (static_table.contains(header_line)) {
        encode_integer<7>(stream, (uint8_t)0x80, static_table[header_line]);
        return;
    }

    // Case 2: found entry in dynamic table
    if (dynamic_table.contains(header_line)) {
        encode_integer<7>(stream, (uint8_t)0x80, dynamic_table[header_line]);
        return;
    }

    // TODO: Cover never indexed case
    if (add_index) {
        // 6.2.1.  Literal Header Field with Incremental Indexing
        // Result will go in dynamic table
        // Case 1: Field indexed
        if (static_table.contains(header_line.first)) {
            // Subcase 1: Static table
            encode_integer<6>(stream, (uint8_t)0x40, static_table[header_line.first]);
            add_header_string(stream, header_line.second);
        } else
        if (dynamic_table.contains(header_line.first)) {
            // Subcase 2: Dynamic table
            encode_integer<6>(stream, (uint8_t)0x40, dynamic_table[header_line.first]);
            add_header_string(stream, header_line.second);
            return;
        } else {
            // Case 2: No index
            *stream++ = 0x40;
            add_header_string(stream, to_string(header_line.first));
            add_header_string(stream, header_line.second);
        }

        dynamic_table.insert(header_line);
        return;
    } else {
        // 6.2.2.  Literal Header Field without Indexing
        // Case 1: Field indexed
        if (static_table.contains(header_line.first)) {
            // Subcase 1: Static table
            encode_integer<4>(stream, (uint8_t)0x00, static_table[header_line.first]);
            add_header_string(stream, header_line.second);
        } else
        if (dynamic_table.contains(header_line.first)) {
            // Subcase 2: Dynamic table
            encode_integer<4>(stream, (uint8_t)0x00, dynamic_table[header_line.first]);
            add_header_string(stream, header_line.second);
            return;
        } else {
            // Case 2: No index
            *stream++ = 0x00;
            add_header_string(stream, to_string(header_line.first));
            add_header_string(stream, header_line.second);
        }
        return;
    }
    
}

struct goaway {
private:
    // GCC endianness
    uint32_t        last_stream_id:31;
    uint32_t        reserved:1;
    uint32_t        error_code;

public:
    constexpr goaway(
                const uint32_t &last_stream_id,
                const frame::error_t &error_code)
            :   last_stream_id(changeEndian<std::endian::native, std::endian::big>(last_stream_id)),
                reserved(0),
                error_code(changeEndian<std::endian::native, std::endian::big>((uint32_t)error_code)) {}
    constexpr uint32_t get_last_stream_id() const { return changeEndian<std::endian::big, std::endian::native>(last_stream_id);}
    constexpr frame::error_t get_error_code() const { return (frame::error_t)changeEndian<std::endian::big, std::endian::native>(error_code); }

    template <size_t debug_data_size>
    static constexpr size_t add_frame_size(const char (&debug_data)[debug_data_size]) {
        return sizeof(frame) + sizeof(goaway) + debug_data_size;
    }

    template <size_t debug_data_size>
    static constexpr void add_frame(
                Stream &stream,
                uint32_t max_stream,
                frame::error_t error_code,
                const char (&debug_data)[debug_data_size]) {
        const uint32_t length = sizeof(frame) + sizeof(goaway) + debug_data_size;
        frame *pframe = reinterpret_cast<frame *>(stream.GetCurrAndIncrease(sizeof(frame)));
        pframe->init_frame(length, frame::type_t::SETTINGS, frame::flags_t::ACK, 0x00);
        
        auto goaway_buffer = stream.GetCurrAndIncrease(sizeof(goaway));
        new (goaway_buffer) goaway {max_stream, error_code};
        
        auto debug_data_writer = stream.GetCurrAndIncrease(debug_data_size);
        std::copy(debug_data, debug_data + debug_data_size, debug_data_writer);
    }
} __attribute__((packed));

inline std::ostream& operator<<(std::ostream& os, const goaway &goaway) {
    return os << "[" << goaway.get_last_stream_id() << "," << goaway.get_error_code() << "]";
}

struct window_update {
private:
    /* Big endian
    uint32_t    reserved:1;
    uint32_t    window_size_increment:31; */

    // Little endian
    uint32_t    window_size_increment:31;
    uint32_t    reserved:1;

public:
    uint32_t get_window_size_increment() const { return changeEndian<std::endian::big, std::endian::native>(window_size_increment); }
} __attribute__((packed));

inline std::ostream& operator<<(std::ostream& os, const window_update &windowupdate) {
    return os << '[' << windowupdate.get_window_size_increment() << ']';
}

inline void displaymem(std::ostream& os, const uint8_t *pstart, const uint8_t *const pend) {
    os << '[';
    for(;pstart < pend; ++pstart) {
        os << MMS::constant::lower_case_numbers[*pstart / 16] << MMS::constant::lower_case_numbers[*pstart % 16];
    }
    os << ']' << std::endl;
}

class header_request : public request_header {
public:
    uint32_t stream_identifier;
    uint8_t weight;
    frame::error_t error;

    // Doubly linked list
    header_request *next;
    header_request *previous;

    inline header_request(uint32_t stream_identifier = 0, uint8_t weight = 16)
                :   request_header(VERSION::VER_2),
                    stream_identifier(stream_identifier),
                    weight(weight),
                    error(frame::error_t::NO_ERROR),
                    next(nullptr), previous(nullptr) {}

    // Upgrade from http 1.1
    inline header_request(request_header &&header)
                :   request_header(std::move(header)),
                    stream_identifier(1),   // Upgrade stream is 1
                    weight(16),             // Setting up default
                    error(frame::error_t::NO_ERROR),
                    next(nullptr), previous(nullptr) {}

    header_request(const header_request &) = delete;
    header_request &operator=(const header_request &) = delete;

    void parse_header(const ConstStream &stream, const uint32_t stream_identifier, dynamic_table_t &dynamic_table) {
        this->stream_identifier = stream_identifier;
        while(stream.remaining_buffer()) {
            if ((*stream & 0x80) == 0x80) {
                // rfc7541 # 6.1 Indexed Header Field Representation
                uint32_t index = decode_integer<7>(stream);
                // Dynamic table check is internal
                auto header = index < 62 ? static_table[index] : dynamic_table[index - 62];
                add_field(header);
            } else if (*stream == 0x40) {
                ++stream;
                auto field = get_header_field(stream);
                auto value = get_header_string(stream);
                auto header = std::make_pair(field, value);
                dynamic_table.insert(header);
                add_field(header);
            } else if ((*stream & 0xc0) == 0x40) {
                // rfc7541 # 6.2.1 Literal Header Field with Incremental Indexing
                uint32_t index = decode_integer<6>(stream);
                auto header_for_field = index < 62 ? static_table[index] : dynamic_table[index - 62];
                auto value = get_header_string(stream);
                auto header = std::make_pair(header_for_field.first, value);
                dynamic_table.insert(header);
                add_field(header);
            } else if (*stream == 0x00) {
                ++stream;
                auto field = get_header_field(stream);
                auto value = get_header_string(stream);
                auto header = std::make_pair(field, value);
                add_field(header);
            } else if ((*stream & 0xf0) == 0x00) {
                uint32_t index = decode_integer<4>(stream);
                auto header_for_field = index < 62 ? static_table[index] : dynamic_table[index - 62];
                auto value = get_header_string(stream);
                auto header = std::make_pair(header_for_field.first, value);
                add_field(header);
            } else if (*stream == 0x10) {
                ++stream;
                auto field = get_header_field(stream);
                auto value = get_header_string(stream);
                auto header = std::make_pair(field, value);
                add_field(header);
            } else if ((*stream & 0xf0) == 0x10) {
                uint32_t index = decode_integer<4>(stream);
                auto header_for_field = index < 62 ? static_table[index] : dynamic_table[index - 62];
                auto value = get_header_string(stream);
                auto header = std::make_pair(header_for_field.first, value);
                add_field(header);
            } else if ((*stream &0xe0) == 0x20) {
                uint32_t index = decode_integer<5>(stream);;
                dynamic_table.update_size(index);
            }
        }

        // TODO: Update Method
        // Update version
        version = VERSION::VER_2;
        auto method_itr = fields.find(FIELD::Method);
        if (method_itr != fields.end()) {
            method = to_method(method_itr->second);
        } else {
            method = METHOD::IGNORE_THIS;
        }
    }

    void set_error(const frame::error_t error) {
        this->error = error;
    }

    constexpr header_request *get_next() { return next; }
    constexpr header_request *get_previous() { return previous; }
    constexpr const header_request *get_next() const { return next; }
    constexpr const header_request *get_previous() const { return previous; }

}; // class header_request

class request {
    //Stream_count
    dynamic_table_t &dynamic_table; // This will be share by the multiple request in a connection
    header_request *first;
    header_request *last;

    // <stream identifier> <request header> map
    std::unordered_map<uint32_t, header_request *> header_map;
    MMS::http::v2::settings_store &peer_settings;

    uint32_t max_stream;

    friend std::ostream& operator<<(std::ostream& os, const request &request);

public:
    inline request(
            dynamic_table_t &dynamic_table,
            MMS::http::v2::settings_store &peer_settings)
                :   dynamic_table(dynamic_table),
                    first(nullptr), last(nullptr), header_map(),
                    peer_settings(peer_settings), max_stream(0) {}

    inline request(
                dynamic_table_t &dynamic_table,
                MMS::http::v2::settings_store &peer_settings,
                request_header &&header)
        :   dynamic_table(dynamic_table),
                    first(nullptr), last(nullptr), header_map(),
                    peer_settings(peer_settings), max_stream(0)
    {
        header_request *pheader = new header_request(std::move(header));
        insert(pheader, 0x00);
    }

    request(const request &) = delete;
    request &operator=(const request &) = delete;

    inline ~request() {
        // This class allocate memory, hence will be freeing it
        while(first) {
            header_request *next = first->next;
            delete first;
            first = next;
        }
    }

    inline void insert(header_request *pheader, uint32_t stream_dependency) {
        if (stream_dependency != 0) {
            auto header_itr = header_map.find(stream_dependency);
            header_request *request;
            if (header_itr == header_map.end()) {
                // Dependency stream is not present
                // We are creating dummy header and appending it at the end
                request = new header_request(stream_dependency);
                header_map.insert(std::make_pair(stream_dependency, request));
                request->next = pheader;
                if (last != nullptr)
                    last->next = request;
                else {
                    first = request;
                }
                last = pheader;
            } else {
                request = header_itr->second;
                pheader->next = request->next;
                request->next = pheader;
                if (request == last) {
                    last = pheader;
                }
            }
        } else {
            // Just appending at the end
            if (last != nullptr) {
                last->next = pheader;
            } else {
                first = pheader;
            }
            last = pheader;
        }

        header_map.insert(std::make_pair(pheader->stream_identifier, pheader));
    }

    err_t parse(const ConstStream &stream, Stream &writestream) {
        while(stream.CheckCapacity(sizeof(frame))) {
            const frame *pframe = reinterpret_cast<const frame *>(stream.GetCurrAndIncrease(sizeof(frame)));
            const auto stream_identifier = pframe->get_stream_identifier();
            if (stream_identifier > max_stream) max_stream = stream_identifier;
            const uint8_t padded_bytes = pframe->contains(frame::flags_t::PADDED) ? *stream++ : 0;

            auto frameStreamBuffer = stream.GetCurrAndIncrease(pframe->get_length());
            ConstStream frameStream { frameStreamBuffer, frameStreamBuffer +  pframe->get_length() - padded_bytes};

            switch(pframe->get_type()) {
            case frame::type_t::HEADERS: {
                // rfc7540 - 6.2.  HEADERS
                auto frameStream = stream.GetSimpleConstStream();
                uint32_t stream_dependency;
                if (pframe->contains(frame::flags_t::PRIORITY)) {
                    stream_dependency = changeEndian<std::endian::big, std::endian::native>(*reinterpret_cast<const uint32_t *>(frameStream.curr()));
                    stream_dependency &= 0x7fffffff;
                    frameStream += sizeof(uint32_t);
                    frameStream++;
                } else {
                    stream_dependency = 0x00;
                }

                if (stream_identifier == 0x00) {
                    // PROTOCOL_ERROR we will stop parsing
                    goaway::add_frame(
                                writestream,
                                max_stream,
                                frame::error_t::PROTOCOL_ERROR,
                                "HEADERS frame requires stream ID to be set");
                    return err_t::HTTP2_INITIATE_GOAWAY;
                }

                auto header_itr = header_map.find(stream_identifier);
                header_request *request;
                if (header_itr == header_map.end()) {
                    request = new header_request();
                    insert(request, stream_dependency);
                } else {
                    request = header_itr->second;
                }

                request->parse_header(frameStream, stream_identifier, dynamic_table);
                break;
            }
            case frame::type_t::PRIORITY: {
                // rfc7540 - 6.3.  PRIORITY
                // We will just ignore this

                //uint32_t stream_dependency = changeEndian<std::endian::big, std::endian::native>(*(uint32_t *)pstart_);
                //stream_dependency &= 0x7fffffff;
                break;
            }
            case frame::type_t::RST_STREAM: {
                const uint32_t error_code = *reinterpret_cast<const uint32_t *>(frameStream.curr());
                if (stream_identifier == 0x00) {
                    // PROTOCOL_ERROR we will stop parsing
                    goaway::add_frame(
                                writestream,
                                max_stream,
                                frame::error_t::PROTOCOL_ERROR,
                                "RST_STREAM requires stream ID to be set");
                    return err_t::HTTP2_INITIATE_GOAWAY;
                }

                auto header_itr = header_map.find(stream_identifier);
                header_request *request;
                if (header_itr == header_map.end()) {
                    request = new header_request();
                    insert(request, 0x00);
                } else {
                    request = header_itr->second;
                }

                request->set_error((frame::error_t)error_code);
                break;
            }
            case frame::type_t::SETTINGS: {
                if (!pframe->contains(frame::flags_t::ACK)) {
                    peer_settings.parse_frame(frameStream);
                    dynamic_table.update_size(peer_settings.SETTINGS_HEADER_TABLE_SIZE);
                    settings::add_ack_frame(writestream);
                }
                break;
            }
            case frame::type_t::PUSH_PROMISE: {
                // Push promise is not supported we will ignore this
                goaway::add_frame(
                                writestream,
                                max_stream,
                                frame::error_t::PROTOCOL_ERROR,
                                "PUSH PROMISE not supported");
                return err_t::HTTP2_INITIATE_GOAWAY;
            }
            case frame::type_t::PING: {
                // Ping contains 64 octet
                constexpr size_t ping_payload_size = 64;

                if (stream_identifier != 0x00) {
                    // PROTOCOL_ERROR we will stop parsing
                    goaway::add_frame(
                                writestream,
                                max_stream,
                                frame::error_t::PROTOCOL_ERROR,
                                "PING can have have stream ID zero(0)");
                    return err_t::HTTP2_INITIATE_GOAWAY;
                }
                if (pframe->get_length() != ping_payload_size) {
                    // PROTOCOL_ERROR we will stop parsing
                    goaway::add_frame(
                                writestream,
                                max_stream,
                                frame::error_t::PROTOCOL_ERROR,
                                "PING frame length can only be 64");
                    return err_t::HTTP2_INITIATE_GOAWAY;
                }
                frame *response_frame = reinterpret_cast<frame *>(writestream.GetCurrAndIncrease(sizeof(frame)));
                response_frame->init_frame(ping_payload_size, frame::type_t::PING, frame::flags_t::ACK, 0x00);
                std::copy(frameStream.curr(), frameStream.curr() + ping_payload_size, writestream.GetCurrAndIncrease(ping_payload_size));
                break;
            }
            case frame::type_t::GOAWAY: {
                return err_t::HTTP2_INITIATE_GOAWAY;
            }
            case frame::type_t::WINDOW_UPDATE: {
                // TODO: Implement Windows Update
                break;
            }
            case frame::type_t::CONTINUATION: {
                // TODO: Implement Continuation, this is required only for POST call
                // We may not choose to support ignoring this call
                // Best implementation would be RST_STREAM
                break;
            }
            default:
                break;
            }
        }
        return err_t::SUCCESS;
    }

    inline void copy_http_header_response(Stream &stream, const std::pair<FIELD, std::string> &header_line, bool add_index){
        MMS::http::v2::copy_http_header_response(dynamic_table, stream, header_line, add_index);
    }

    inline void copy_http_header_response(Stream &stream, FIELD field, const std::string &value, bool add_index) {
        copy_http_header_response(stream, std::make_pair(field, value), add_index);
    }

    // N includes null character
    inline void copy_http_header_response(Stream &stream, FIELD field, const uint8_t *value, size_t N, bool add_index) {
        std::string _value((char *)value, N);
        copy_http_header_response(stream, std::make_pair(field, _value), add_index);
    }

    // N includes null character
    template <typename CHAR_TYPE, size_t N>
    inline void copy_http_header_response(
                Stream &stream,
                FIELD field,
                const CHAR_TYPE (&value)[N],
                bool add_index) {
        //
        std::string _value((char *)value, N);
        copy_http_header_response(stream, std::make_pair(field, _value), add_index);
    }

    template <typename VALUE_TYPE>
    inline void copy_http_header_response(
                Stream &stream,
                FIELD field,
                const VALUE_TYPE &value,
                bool add_index) {
        if constexpr (std::is_enum_v<VALUE_TYPE>) {
            auto __value = static_cast<typename std::underlying_type_t<typename std::decay_t<VALUE_TYPE>>>(value);
            const std::string _value = std::to_string(__value);
            copy_http_header_response(
                stream,
                std::make_pair(field, _value),
                add_index
            );
        } else {
            auto __value = static_cast<typename std::decay_t<VALUE_TYPE>>(value);
            const std::string _value = std::to_string(__value);
            copy_http_header_response(
                stream,
                std::make_pair(field, _value),
                add_index
            );
        }
    }


    // Status is always cached
    template <FIELD field, CODE code>
    void copy_http_header_response(Stream &stream) { return copy_http_header_response(stream, field, code, true); }

    constexpr header_request *get_first_header() { return first; }
    constexpr const header_request *get_first_header() const { return first; }
}; // class request

template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_200>(Stream &stream) { *stream++ = 0x80 + 8; }
template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_204>(Stream &stream) { *stream++ = 0x80 + 9;}
template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_206>(Stream &stream) { *stream++ = 0x80 + 10; }
template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_304>(Stream &stream) { *stream++ = 0x80 + 11; }
template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_400>(Stream &stream) { *stream++ = 0x80 + 12; }
template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_404>(Stream &stream) { *stream++ = 0x80 + 13; }
template <> constexpr void request::copy_http_header_response<FIELD::Status, CODE::_500>(Stream &stream) { *stream++ = 0x80 + 14; }

} // namespace MMS::http::v2