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

#include "countrycode.h"
#include <iostream>
#include <algorithm>
#include <utility>

constexpr auto GetPhoneCode(const uint16_t alpha2) {
    const CountryCodeContainer container { };
    const auto countrycode =  container.GetCountryCodeByAlpha2(alpha2);
    return countrycode.GetPhoneCode();
}

int main(int, char *[]) {
    constexpr uint16_t MalasiaAlpha2 { 0x594d };
    auto MalasiaPhoneCode = GetPhoneCode(MalasiaAlpha2);

    std::cout << "Malasia Alpha2: " << MalasiaAlpha2 << "; " << MalasiaPhoneCode << std::endl;

    CountryCodeContainer container { };
    auto alpha2list = container.GetAlpha2CodeList();
    std::ranges::sort(alpha2list);
    for(auto alpha2: alpha2list){
        auto code = container.GetCountryCodeByAlpha2(alpha2);
        std::cout << code.GetShortName() << std::hex << " " << code.GetAlpha2Code() <<  " " << code.GetAlpha2CodeString() << " " << code.GetAlpha3CodeString() << " " << code.GetNumericCode() << " " << code.GetPhoneCode() << std::endl; 
    }

    std::cout << std::endl;
    return 0;
}
