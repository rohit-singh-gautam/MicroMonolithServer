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

#include <mms/base/stream.h>
#include <gtest/gtest.h>


TEST(StreamTest, Stream) {
    std::string text { "This is a test for Stream" };
    MMS::Stream stream { text };

    EXPECT_TRUE(stream.remaining_buffer() == text.size());
    stream += 5;
    EXPECT_TRUE(reinterpret_cast<uint8_t *>(text.data()) + 5 == stream.curr());
    
}

TEST(StreamTest, ConstStream) {
    const char *text { "This is a test" };
    auto textsize = strlen(text);
    auto stream = MMS::make_const_stream(text, textsize);

    EXPECT_TRUE(stream.remaining_buffer() == textsize);
}