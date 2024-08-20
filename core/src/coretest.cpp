/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////


#include <mms/ds/prefixmap.h>
#include <gtest/gtest.h>

bool InsertAndFind() {
    std::vector<std::pair<std::string, std::string>> allinserttest {
        { "http://example.com/test/value", "Testvalue" },
        { "http://example.com/test", "Test" },
        { "http://example.com/values", "values" }
    };

    std::vector<std::pair<std::string, std::string>> allsearchtest {
        {"http://example.com/test/key", "Test"},
        {"http://example.com/test/value", "Testvalue"},
        {"http://example.com/test", "Test"},
        {"http://example.com/values", "values"},
        {"http://example.com/values/test", "values"},
        {"http://example.com/tts", ""},
        {"http://example.com", ""},
        {"http://example.com/", ""}
    };

    MMS::prefixmap<std::string, std::string> match { };

    for(auto &test: allinserttest) {
        match.insert(test.first, test.second);
    }

    for(auto &test: allsearchtest) {
        auto value = match.search(test.first);
        if (value != test.second) {
            std::cout << "Mismatch key: " << test.first << ", expected: " << test.second << ", got: " << value << std::endl;
            return false;
        }
    }
    return true;
}

TEST(PrefixMapTest, PrefixMapTest_Insert_Search) {
    EXPECT_TRUE(InsertAndFind());
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}