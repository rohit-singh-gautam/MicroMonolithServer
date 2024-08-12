/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <rohit/cmd/configparser.h>
#include <rohit/cmd/typecast.h>
#include <libgen.h>

namespace rohit {

enum class parser_state {
    OPTION,
    VALUE
};

bool commandline::parser(int argc, char *argv[]) {
    name = basename(argv[0]);
    parser_state state = parser_state::OPTION;
    commandline_option *optionentry;
    for(int arg_index = 1; arg_index < argc; ++arg_index) {
        char *arg_curr = argv[arg_index];
        switch (state) {
        case parser_state::OPTION:
            if (arg_curr[0] != '-') return false;
            if (arg_curr[1] == '-') {
                // Multicharacter options
                auto optionentry_itr = options_name.find(arg_curr + 2);
                if (optionentry_itr == options_name.end()) return false;
                optionentry = optionentry_itr->second;
            } else {
                // Single character option
                auto optionentry_itr = options_ch.find(arg_curr[1]);
                if (optionentry_itr == options_ch.end()) return false;
                optionentry = optionentry_itr->second;
            }

            if (optionentry->type_id == type_identifier::bool_t) {
                bool_t *store_bl = (bool_t *)optionentry->variable;
                *store_bl = true;
            } else state = parser_state::VALUE;

            break;

        case parser_state::VALUE:
            if (! to_type(optionentry->type_id, optionentry->variable, arg_curr)) return false;

            state = parser_state::OPTION;
            
            break;
        }
    }

    if (help) {
        return false;
    }

    return state == parser_state::OPTION;
}

// Create help in form
// Usage: <appname> <usage>
//      Description ...
//
// Summary:
//      Summary ...
//
// Parameters:
//      <Parameters>
//
const std::string commandline::usage() const {
    std::string usage_line = "Usage: " + name + " ";
    for (auto &option: options) {
        usage_line += option.get_short_description();
    }
    usage_line += "\n\t" + short_description + "\n\nSummary:\n\t" + summary + "\n\nParameters:\n";
    for (auto &option: options) {
        usage_line += option.get_parameter_description();
    }
    return usage_line;
}

} // namespace rohit