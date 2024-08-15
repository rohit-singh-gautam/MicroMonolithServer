/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpparser.h>
#include <gtest/gtest.h>

bool http_request_parse(const std::string_view &text) {
    try {
        MMS::http::request req { text };
    } catch(MMS::http_parser_failed_t &e) {
        return false;
    }
    return true;
}

bool http_response_parse(const std::string_view &text) {
    try {
        MMS::http::response response { text };
    } catch(MMS::http_parser_failed_t &e) {
        return false;
    }
    return true;
}

TEST(HttpRequestParserTest, RequestLineTest) {
    EXPECT_FALSE(http_request_parse({"FAIL http://localhost/echotest/regional HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"GET /echotest HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"POST http://localhost/echotest/regional HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"HEAD http://localhost/echotest/regional HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"OPTIONS junkyard HTTP/1.1"}));
}

TEST(HttpRequestParserTest, RequestTest) {
    EXPECT_TRUE(http_request_parse(
        "GET /example.txt HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
        "Accept-Language: en, mi"
    ));

    EXPECT_TRUE(http_request_parse(
        "POST /createaccount HTTP/1.1\n"
        "Host: www.example.com\n"
        "Content-Type: application/x-www-form-urlencoded\n"
        "Content-Length: 30\n\n"
        "name=Rohit+Singh&age=47"
    ));
}

TEST(HttpResponseParserTest, ResponseLineTest) {
    EXPECT_TRUE(http_response_parse({"HTTP/1.1 200 OK"}));
    EXPECT_FALSE(http_response_parse({"GET /echotest HTTP/1.1"}));
    EXPECT_TRUE(http_response_parse({"HTTP/1.1 206 Partial content"}));
}

TEST(HttpResponseParserTest, ResponseTest) {
    EXPECT_TRUE(http_response_parse({
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
    }));
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}