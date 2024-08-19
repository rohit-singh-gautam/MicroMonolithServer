/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <mms/net/tcpserver.h>
#include <mms/ds/prefixmap.h>
#include <http/httpparser.h>
#include <unordered_map>

namespace MMS::server {

class httpwriter_t {
    listener::writer_t &connectionwriter;
public:
    httpwriter_t(listener::writer_t &connectionwriter) : connectionwriter { connectionwriter } { }
    void Write(const http::response &response);
};

class httphandler_t {
public:
    virtual ~httphandler_t() = default;
    virtual void ProcessRead(const http::request &request, listener::writer_t &writer) = 0;
};

class http_t : public net::protocol_t {
    prefixmap<std::string_view, std::unique_ptr<httphandler_t>> &handlermap;

public:
    http_t(prefixmap<std::string_view, std::unique_ptr<httphandler_t>> &handlermap) : handlermap { handlermap } { }
    void ProcessRead(const uint8_t *buffer, const size_t size, listener::writer_t &writer) override;

    
};

class httpcreator_t : public net::protocol_creator_t {
    prefixmap<std::string_view, std::unique_ptr<httphandler_t>> handlermap { };
public:
    net::protocol_t *create_protocol() override { return new http_t { handlermap }; }

    void AddHandler(const std::string_view &path, std::unique_ptr<httphandler_t> &&handler) {
        handlermap.insert(path, std::move(handler));
    }
};

} // namespace MMS::server