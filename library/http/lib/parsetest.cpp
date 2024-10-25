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

#include <http/httpparser.h>
#include <gtest/gtest.h>
#include <mms/base/stream.h>

bool http_request_parse(const MMS::FullStream &stream) {
    try {
        MMS::http::request req { stream };
    } catch(MMS::http_parser_failed_t &e) {
        return false;
    }
    return true;
}



bool http_response_parse(const MMS::FullStream &stream) {
    try {
        MMS::http::response response { stream };
    } catch(MMS::http_parser_failed_t &e) {
        return false;
    }
    return true;
}

TEST(HttpRequestParserTest, RequestLineTest) {
    EXPECT_FALSE(http_request_parse(MMS::make_const_fullstream("FAIL http://localhost/echotest/regional HTTP/1.1")));
    EXPECT_TRUE(http_request_parse(MMS::make_const_fullstream("GET /echotest HTTP/1.1")));
    EXPECT_TRUE(http_request_parse(MMS::make_const_fullstream("POST http://localhost/echotest/regional HTTP/1.1")));
    EXPECT_TRUE(http_request_parse(MMS::make_const_fullstream("HEAD http://localhost/echotest/regional HTTP/1.1")));
    EXPECT_TRUE(http_request_parse(MMS::make_const_fullstream("OPTIONS junkyard HTTP/1.1")));
}

TEST(HttpRequestParserTest, RequestTest) {
    EXPECT_TRUE(http_request_parse( MMS::make_const_fullstream(
        "GET /example.txt HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Accept-Language: en, mi" )
    ));

    EXPECT_TRUE(http_request_parse( MMS::make_const_fullstream(
        "POST /createaccount HTTP/1.1\n"
        "Host: www.example.com\n"
        "Content-Type: application/x-www-form-urlencoded\n"
        "Content-Length: 30\n\n"
        "name=Rohit+Singh&age=47" )
    ));
}

TEST(HttpResponseParserTest, ResponseLineTest) {
    EXPECT_TRUE(http_response_parse(MMS::make_const_fullstream("HTTP/1.1 200 OK")));
    EXPECT_FALSE(http_response_parse(MMS::make_const_fullstream("GET /echotest HTTP/1.1")));
    EXPECT_TRUE(http_response_parse(MMS::make_const_fullstream("HTTP/1.1 206 Partial content")));
}

TEST(HttpResponseParserTest, ResponseTest) {
    EXPECT_TRUE(http_response_parse(MMS::make_const_fullstream(
        "HTTP/1.1 200 OK\r\n"
        "Date: Thu, 15 Aug 2024 15:46:04 GMT\r\n"
        "Server: Apache/2.4.41 (Ubuntu)\r\n"
        "Last-Modified: Tue, 13 Aug 2024 18:30:00 GMT\r\n"
        "Content-Length: 123\r\n"
        "Content-Type: text/html\r\n"
        "Connection: Closed\r\n"
        "\r\n"
        "<html>\r\n"
        "  <body>\r\n"
        "    <h1>Hello, World!</h1>\r\n"
        "  </body>\r\n"
        "</html>"
    )));

    EXPECT_TRUE(http_response_parse(MMS::make_const_fullstream(
        "HTTP/1.1 206 Partial content\r\n"
        "Date: Wed, 15 Nov 1995 06:25:24 GMT\r\n"
        "Last-Modified: Wed, 15 Nov 1995 04:58:08 GMT\r\n"
        "Content-Range: bytes 21010-47021/47022\r\n"
        "Content-Length: 26012\r\n"
        "Content-Type: image/gif\r\n\r\n"
    )));
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}