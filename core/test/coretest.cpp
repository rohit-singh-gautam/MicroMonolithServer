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
        std::string newpath { };
        const auto &value = match.search(test.first, newpath);
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