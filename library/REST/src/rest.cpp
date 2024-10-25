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

namespace MMS::client::rest {
void cache::ProcessRead(const MMS::http::response &response, MMS::client::http::protocol_t *)
{
    auto type = GetResponseTypeFromMIME(response.GetField(MMS::http::FIELD::Content_Type));
    auto responsebody = response.GetBody();
    const Stream stream { responsebody.data(), responsebody.size() };
    impl->Response(response.GetCode(), type, stream);
}
} // namespace MMS::client::rest