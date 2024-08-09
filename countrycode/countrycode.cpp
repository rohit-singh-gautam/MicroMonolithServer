/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

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
