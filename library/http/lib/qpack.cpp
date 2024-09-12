/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <http/qpack.h>


namespace MMS::http::qpack {

const static_table_t static_table = {
#define QPACK_STATIC_TABLE_ENTRY(x, y, z) {y, z},
    QPACK_STATIC_TABLE_LIST
#undef QPACK_STATIC_TABLE_ENTRY
};

} // namespace MMS::http::qpack