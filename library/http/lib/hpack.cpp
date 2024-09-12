/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/hpack.h>

namespace MMS::http::hpack {

const node *huffman_root = created_huffman_tree();

node *created_huffman_tree() {
    node *root = new node();
    constexpr uint16_t symbol_count = sizeof(static_huffman)/sizeof(static_huffman[0]);
    for(uint16_t symbol = 0; symbol < symbol_count; ++symbol) {
        auto symbol_entry = static_huffman[symbol];
        uint32_t code = symbol_entry.code;
        node *curr = root;
        for(uint32_t bit = 1 << (symbol_entry.code_len - 1); bit; bit >>= 1) {
            if ((code & bit) == 0) {
                if (curr->left == nullptr) {
                    curr->left = new node();
                }
                curr = curr->left;
            } else {
                if (curr->right == nullptr) {
                    curr->right = new node();
                }
                curr = curr->right;
            }
        }
        curr->set_leaf(symbol);
    }

    return root;
}

std::string get_huffman_string(const Stream &stream) {
    std::string value;
    const node *curr = huffman_root;
    while(!stream.full()) {
        for(uint8_t bit = 128; bit; bit >>= 1) {
            if ((*stream & bit) == 0) {
                curr = curr-> left;
            } else {
                curr = curr->right;
            }

            if (curr->is_leaf()) {
                if (curr->is_eos()) return value;
                value.push_back(curr->get_symbol());

                curr = huffman_root;
            }
        }

        ++stream;
    }

    return value;
}

void add_huffman_string(Stream &stream, const Stream &valstream) {
    constexpr uint8_t mask[] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
    uint8_t bit_index = 0;
    *stream = 0; // Clean all existing value
    for(;!valstream.full(); ++valstream) {
        const auto &entry = static_huffman[*valstream];
        auto len_left = entry.code_len;
        auto code = entry.code;
        while (true) {
            if (bit_index + len_left <= 8) {
                *stream += (code << (8 - bit_index - len_left)) & mask[bit_index];
                bit_index += len_left;
                if (bit_index == 8) {
                    bit_index = 0;
                    ++stream;
                    *stream = 0;
                }
                break;
            } else {
                // This will copy atleast one bit
                *stream++ += (code >> (len_left + bit_index - 8)) & mask[bit_index];
                len_left -= 8 - bit_index;
                bit_index = 0;
                *stream = 0;
            }
        }
    }
    
    if (bit_index) {
        *stream++ += mask[bit_index];
    }
}

const static_table_t static_table = {
#define HTTP2_STATIC_TABLE_ENTRY(x, y, z) {y, z},
    HTTP2_STATIC_TABLE_LIST
#undef HTTP2_STATIC_TABLE_ENTRY
};

const std::pair<FIELD, std::string> map_table_t::empty { FIELD::IGNORE_THIS, {}};

} // namespace MMS::http::hpack