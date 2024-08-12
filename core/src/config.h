/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <cstdint>
namespace MMS {
namespace config {
    constexpr bool debug = true;

    constexpr uint64_t attempt_to_write = 20;
    constexpr int64_t attempt_to_write_wait_in_ms = 50;
    constexpr int socket_backlog = 5;
    constexpr int64_t log_thread_wait_in_millis = 50;

} // namespace config
} // namespace MMS
