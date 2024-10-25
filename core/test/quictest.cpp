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
//                                                                                         //
// HPack is based on https://www.rfc-editor.org/rfc/rfc9000.html                           //
//                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/net/quic.h>
#include <gtest/gtest.h>

std::vector<uint64_t> values16 { 16383, 64, 65, 88, 330, 2345, 2034, 3934, 12445 };
std::vector<uint64_t> values32 { 16384, 1073741823ULL, 20000, 8000000ULL, 330345677ULL, 43455555ULL, 203432423ULL, 39342344ULL, 124544343ULL };
std::vector<uint64_t> values64 { 4611686018427387903ULL, 1073741824ULL, 4611686018427387ULL, 80000000000ULL, 330345643577ULL, 4345553334455ULL, 2034324255553ULL, 39342355553344ULL, 1245433334343ULL };

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
    MMS::FullStreamAutoAlloc stream { 17 };
    for(auto value: values16) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 2);
        EXPECT_EQ(value, decoded);
    }
}

TEST(QUICTest, IntegerTestU32) {
    MMS::FullStreamAutoAlloc stream { 33 };
    for(auto value: values32) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 4);
        EXPECT_EQ(value, decoded);
    }
}

TEST(QUICTest, IntegerTestU64) {
    MMS::FullStreamAutoAlloc stream { 65 };
    for(auto value: values64) {
        auto [decoded, size] = EncodeAndDecode(value, stream);
        EXPECT_EQ(size, 8);
        EXPECT_EQ(value, decoded);
    }
}