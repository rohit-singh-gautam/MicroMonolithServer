/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <mms/base/stream.h>
#include <gtest/gtest.h>


TEST(StreamTest, Stream) {
    std::string text { "This is a test for Stream" };
    MMS::Stream stream { text };

    EXPECT_TRUE(stream.remaining_buffer() == text.size());
    stream += 5;
    EXPECT_TRUE(reinterpret_cast<uint8_t *>(text.data()) + 5 == stream.curr());
    
}