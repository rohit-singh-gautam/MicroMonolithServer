/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                         //
// HPack is based on https://www.rfc-editor.org/rfc/rfc9000.html                           //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/net/quic.h>
#include <gtest/gtest.h>

auto EncodeAndDecode(uint64_t value, MMS::FullStream &stream) {
    stream.Reset();
    MMS::net::quic::EncodeUnsignedInteger(value, stream);
    size_t size = stream.index();
    stream.Reset();
    uint64_t decoded = MMS::net::quic::DecodeUnsignedInteger(stream);
    return std::make_pair(decoded, size);
}

TEST(QUICTest, IntegerTestByte) {
    MMS::FullStreamAutoAlloc stream { 9 };
    for(uint64_t value = 0; value <= 63; ++value) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 1);
        EXPECT_EQ(value, decoded);
    }
}

TEST(QUICTest, IntegerTestShort) {
    std::vector<uint64_t> values { 16383, 64,  65, 88, 330, 2345, 2034, 3934, 12445 };
    MMS::FullStreamAutoAlloc stream { 17 };
    for(auto value: values) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 2);
        EXPECT_EQ(value, decoded);
    }
}

TEST(QUICTest, IntegerTestU32) {
    std::vector<uint64_t> values { 16384, 1073741823ULL, 20000, 8000000ULL, 330345677ULL, 43455555ULL, 203432423ULL, 39342344ULL, 124544343ULL };
    MMS::FullStreamAutoAlloc stream { 33 };
    for(auto value: values) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 4);
        EXPECT_EQ(value, decoded);
    }
}

TEST(QUICTest, IntegerTestU64) {
    std::vector<uint64_t> values { 4611686018427387903ULL, 1073741824ULL, 4611686018427387ULL, 80000000000ULL, 330345643577ULL, 4345553334455ULL, 2034324255553ULL, 39342355553344ULL, 1245433334343ULL };
    MMS::FullStreamAutoAlloc stream { 65 };
    for(auto value: values) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 8);
        EXPECT_EQ(value, decoded);
    }
}