/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/net/tcpserverevent.h>
#include <mms/ds/prefixmap.h>
#include <http/httpparser.h>
#include <unordered_map>

namespace MMS::server {

class httpwriter_t {
    MMS::event::writer_t &connectionwriter;
public:
    httpwriter_t(MMS::event::writer_t &connectionwriter) : connectionwriter { connectionwriter } { }
    void Write(const http::response &response);
};

class httphandler_t {
public:
    virtual ~httphandler_t() = default;
    virtual void ProcessRead(const http::request &request, MMS::event::writer_t &writer) = 0;
};

class http_t : public MMS::event::protocol_implementation_t {
    prefixmap<std::string_view, std::unique_ptr<httphandler_t>> &handlermap;

public:
    http_t(prefixmap<std::string_view, std::unique_ptr<httphandler_t>> &handlermap) : handlermap { handlermap } { }
    void ProcessRead(const uint8_t *buffer, const size_t size, MMS::event::writer_t &writer) override;

    
};

class httpcreator_t : public MMS::event::protocol_implementation_creator_t {
    prefixmap<std::string_view, std::unique_ptr<httphandler_t>> handlermap { };
public:
    MMS::event::protocol_implementation_t *create_protocol_implementation() override { return new http_t { handlermap }; }

    void AddHandler(const std::string_view &path, std::unique_ptr<httphandler_t> &&handler) {
        handlermap.insert(path, std::move(handler));
    }
};

} // namespace MMS::server