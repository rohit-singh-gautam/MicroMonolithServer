/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/httpdef.h>
#include <gtest/gtest.h>

bool http_request_parse(const std::string_view &text) {
    try {
        MMS::http::request req { text };
    } catch(MMS::http_parser_failed_t &e) {
        return false;
    }
    return true;
}

TEST(HttpParserTest, RequestLineTest) {
    EXPECT_FALSE(http_request_parse({"FAIL http://localhost/echotest/regional HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"GET /echotest HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"POST http://localhost/echotest/regional HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"HEAD http://localhost/echotest/regional HTTP/1.1"}));
    EXPECT_TRUE(http_request_parse({"OPTIONS junkyard HTTP/1.1"}));
}

TEST(HttpParserTest, RequestTest) {
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

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}