/////////////////////////////////////////////////////////////////////////////////////////////
// Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
// Copyright (c) 2024 Rohit Jairaj Singh. All rights reserved.                             //
// This code is proprietary and confidential. Unauthorized copying of this file, via any   //
// medium, is strictly prohibited.                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////

#include <server/rest.h>

namespace MMS::server::rest {

void handler::ProcessRead(const MMS::http::request &request, const std::string &relative_path, http::protocol_t *writer) {
    const auto method = request.GetMethod();
#if defined(DEBUG) || defined(NDEBUG)
// This check is already present at caller of this function
// Additional check will be only in debug mode
    if (!IsSupported(method)) {
        writer->WriteError(http::CODE::Method_Not_Allowed, std::format("Method {} not allowed", to_string(method)));
        return;
    }
#endif
    // TODO: Add mappings and translations
    assert(impl);
    auto responsetype = ResponseType::JSON;
    FullStreamAutoAlloc response_body { };
    auto ret = impl->CallAPI(method, relative_path, responsetype, request.GetBody(), response_body);
    auto body_stream = make_const_stream(response_body.begin(), response_body.end());
    writer->Write(ret, body_stream);
}

} // namespace MMS::server::rest