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

#pragma once
#include <string>
#include <unordered_map>
#include <mms/base/error.h>

#ifndef LIST_DEFINITION_END
#define LIST_DEFINITION_END
#endif

namespace MMS::http {
#define HTTP_VERSION_LIST \
    HTTP_VERSION_ENTRY(VER_UNKNOWN, "Unknown Version") \
    HTTP_VERSION_ENTRY(VER_1_1, "HTTP/1.1") \
    HTTP_VERSION_ENTRY(VER_2, "HTTP/2.0") \
    LIST_DEFINITION_END

#define HTTP_FIELD_LIST \
    /* HTTP2 Pseudo Header */ \
    HTTP_FIELD_ENTRY(Authority, "Authority") \
    HTTP_FIELD_ENTRY(Method, "Method") \
    HTTP_FIELD_ENTRY(Path, "Path") \
    HTTP_FIELD_ENTRY(Scheme, "Scheme") \
    HTTP_FIELD_ENTRY(Status, "Status") \
    /* General Header */ \
    HTTP_FIELD_ENTRY(Cache_Control, "Cache-Control") \
    HTTP_FIELD_ENTRY(Connection, "Connection") \
    HTTP_FIELD_ENTRY(Date, "Date") \
    HTTP_FIELD_ENTRY(Pragma, "Pragma") \
    HTTP_FIELD_ENTRY(Trailer, "Trailer") \
    HTTP_FIELD_ENTRY(Transfer_Encoding, "Transfer-Encoding") \
    HTTP_FIELD_ENTRY(Upgrade, "Upgrade") \
    HTTP_FIELD_ENTRY(Via, "Via") \
    HTTP_FIELD_ENTRY(Warning, "Warning") \
    \
    /* Request Header */ \
    HTTP_FIELD_ENTRY(Accept, "Accept") \
    HTTP_FIELD_ENTRY(Accept_Charset, "Accept-Charset") \
    HTTP_FIELD_ENTRY(Accept_Encoding, "Accept-Encoding") \
    HTTP_FIELD_ENTRY(Accept_Language, "Accept-Language") \
    HTTP_FIELD_ENTRY(Authorization, "Authorization") \
    HTTP_FIELD_ENTRY(Expect, "Expect") \
    HTTP_FIELD_ENTRY(From, "From") \
    HTTP_FIELD_ENTRY(Host, "Host") \
    HTTP_FIELD_ENTRY(If_Match, "If-Match") \
    HTTP_FIELD_ENTRY(If_Modified_Since, "If-Modified-Since") \
    HTTP_FIELD_ENTRY(If_None_Match, "If-None-Match") \
    HTTP_FIELD_ENTRY(If_Range, "If-Range") \
    HTTP_FIELD_ENTRY(If_Unmodified_Since, "If-Unmodified-Since") \
    HTTP_FIELD_ENTRY(Max_Forwards, "Max-Forwards") \
    HTTP_FIELD_ENTRY(Proxy_Authorization, "Proxy-Authorization") \
    HTTP_FIELD_ENTRY(Range, "Range") \
    HTTP_FIELD_ENTRY(Referer, "Referer") \
    HTTP_FIELD_ENTRY(TE, "TE") \
    HTTP_FIELD_ENTRY(User_Agent, "User-Agent") \
    HTTP_FIELD_ENTRY(HTTP2_Settings, "HTTP2-Settings") \
    \
    /* Response Header */ \
    HTTP_FIELD_ENTRY(Accept_Ranges, "Accept-Ranges") \
    HTTP_FIELD_ENTRY(Age, "Age") \
    HTTP_FIELD_ENTRY(ETag, "ETag") \
    HTTP_FIELD_ENTRY(Location, "Location") \
    HTTP_FIELD_ENTRY(Proxy_Authenticate, "Proxy-Authenticate") \
    HTTP_FIELD_ENTRY(Retry_After, "Retry-After") \
    HTTP_FIELD_ENTRY(Server, "Server") \
    HTTP_FIELD_ENTRY(Vary, "Vary") \
    HTTP_FIELD_ENTRY(WWW_Authenticate, "WWW-Authenticate") \
    \
    HTTP_FIELD_ENTRY(Allow, "Allow") \
    HTTP_FIELD_ENTRY(Content_Encoding, "Content-Encoding") \
    HTTP_FIELD_ENTRY(Content_Language, "Content-Language") \
    HTTP_FIELD_ENTRY(Content_Length, "Content-Length") \
    HTTP_FIELD_ENTRY(Content_Location, "Content-Location") \
    HTTP_FIELD_ENTRY(Content_MD5, "Content-MD5") \
    HTTP_FIELD_ENTRY(Content_Range, "Content-Range") \
    HTTP_FIELD_ENTRY(Content_Type, "Content-Type") \
    HTTP_FIELD_ENTRY(Expires, "Expires") \
    HTTP_FIELD_ENTRY(Last_Modified, "Last-Modified") \
    \
    /* Others */ \
    HTTP_FIELD_ENTRY(A_IM, "A-IM") \
    HTTP_FIELD_ENTRY(Accept_Additions, "Accept-Additions") \
    HTTP_FIELD_ENTRY(Accept_CH, "Accept-CH") \
    HTTP_FIELD_ENTRY(Accept_Datetime, "Accept-Datetime") \
    HTTP_FIELD_ENTRY(Accept_Features, "Accept-Features") \
    HTTP_FIELD_ENTRY(Accept_Patch, "Accept-Patch") \
    HTTP_FIELD_ENTRY(Accept_Post, "Accept-Post") \
    HTTP_FIELD_ENTRY(ALPN, "ALPN") \
    HTTP_FIELD_ENTRY(Also_Control, "Also-Control") \
    HTTP_FIELD_ENTRY(Alt_Svc, "Alt-Svc") \
    HTTP_FIELD_ENTRY(Alt_Used, "Alt-Used") \
    HTTP_FIELD_ENTRY(Alternates, "Alternates") \
    HTTP_FIELD_ENTRY(Apply_To_Redirect_Ref, "Apply-To-Redirect-Ref") \
    HTTP_FIELD_ENTRY(Authentication_Control, "Authentication-Control") \
    HTTP_FIELD_ENTRY(Authentication_Info, "Authentication-Info") \
    HTTP_FIELD_ENTRY(C_Ext, "C-Ext") \
    HTTP_FIELD_ENTRY(C_Man, "C-Man") \
    HTTP_FIELD_ENTRY(C_Opt, "C-Opt") \
    HTTP_FIELD_ENTRY(C_PEP, "C-PEP") \
    HTTP_FIELD_ENTRY(C_PEP_Info, "C-PEP-Info") \
    HTTP_FIELD_ENTRY(Cal_Managed_ID, "Cal-Managed-ID") \
    HTTP_FIELD_ENTRY(CalDAV_Timezones, "CalDAV-Timezones") \
    HTTP_FIELD_ENTRY(CDN_Loop, "CDN-Loop") \
    HTTP_FIELD_ENTRY(Cert_Not_After, "Cert-Not-After") \
    HTTP_FIELD_ENTRY(Cert_Not_Before, "Cert-Not-Before") \
    HTTP_FIELD_ENTRY(Close, "Close") \
    HTTP_FIELD_ENTRY(Content_Base, "Content-Base") \
    HTTP_FIELD_ENTRY(Content_Disposition, "Content-Disposition") \
    HTTP_FIELD_ENTRY(Content_ID, "Content-ID") \
    HTTP_FIELD_ENTRY(Content_Script_Type, "Content-Script-Type") \
    HTTP_FIELD_ENTRY(Content_Style_Type, "Content-Style-Type") \
    HTTP_FIELD_ENTRY(Content_Version, "Content-Version") \
    HTTP_FIELD_ENTRY(Cookie, "Cookie") \
    HTTP_FIELD_ENTRY(Cookie2, "Cookie2") \
    HTTP_FIELD_ENTRY(DASL, "DASL") \
    HTTP_FIELD_ENTRY(DAV, "DAV") \
    HTTP_FIELD_ENTRY(Default_Style, "Default-Style") \
    HTTP_FIELD_ENTRY(Delta_Base, "Delta-Base") \
    HTTP_FIELD_ENTRY(Depth, "Depth") \
    HTTP_FIELD_ENTRY(Derived_From, "Derived-From") \
    HTTP_FIELD_ENTRY(Destination, "Destination") \
    HTTP_FIELD_ENTRY(Differential_ID, "Differential-ID") \
    HTTP_FIELD_ENTRY(Digest, "Digest") \
    HTTP_FIELD_ENTRY(Early_Data, "Early-Data") \
    HTTP_FIELD_ENTRY(Expect_CT, "Expect-CT") \
    HTTP_FIELD_ENTRY(Ext, "Ext") \
    HTTP_FIELD_ENTRY(Forwarded, "Forwarded") \
    HTTP_FIELD_ENTRY(GetProfile, "GetProfile") \
    HTTP_FIELD_ENTRY(Hobareg, "Hobareg") \
    HTTP_FIELD_ENTRY(IM, "IM") \
    HTTP_FIELD_ENTRY(If, "If") \
    HTTP_FIELD_ENTRY(If_Schedule_Tag_Match, "If-Schedule-Tag-Match") \
    HTTP_FIELD_ENTRY(Include_Referred_Token_Binding_ID, "Include-Referred-Token-Binding-ID") \
    HTTP_FIELD_ENTRY(Keep_Alive, "Keep-Alive") \
    HTTP_FIELD_ENTRY(Label, "Label") \
    HTTP_FIELD_ENTRY(Link, "Link") \
    HTTP_FIELD_ENTRY(Lock_Token, "Lock-Token") \
    HTTP_FIELD_ENTRY(Man, "Man") \
    HTTP_FIELD_ENTRY(Memento_Datetime, "Memento-Datetime") \
    HTTP_FIELD_ENTRY(Meter, "Meter") \
    HTTP_FIELD_ENTRY(MIME_Version, "MIME-Version") \
    HTTP_FIELD_ENTRY(Negotiate, "Negotiate") \
    HTTP_FIELD_ENTRY(OData_EntityId, "OData-EntityId") \
    HTTP_FIELD_ENTRY(OData_Isolation, "OData-Isolation") \
    HTTP_FIELD_ENTRY(OData_MaxVersion, "OData-MaxVersion") \
    HTTP_FIELD_ENTRY(OData_Version, "OData-Version") \
    HTTP_FIELD_ENTRY(Opt, "Opt") \
    HTTP_FIELD_ENTRY(Optional_WWW_Authenticate, "Optional-WWW-Authenticate") \
    HTTP_FIELD_ENTRY(Ordering_Type, "Ordering-Type") \
    HTTP_FIELD_ENTRY(Origin, "Origin") \
    HTTP_FIELD_ENTRY(OSCORE, "OSCORE") \
    HTTP_FIELD_ENTRY(Overwrite, "Overwrite") \
    HTTP_FIELD_ENTRY(P3P, "P3P") \
    HTTP_FIELD_ENTRY(PEP, "PEP") \
    HTTP_FIELD_ENTRY(PICS_Label, "PICS-Label") \
    HTTP_FIELD_ENTRY(Pep_Info, "Pep-Info") \
    HTTP_FIELD_ENTRY(Position, "Position") \
    HTTP_FIELD_ENTRY(Prefer, "Prefer") \
    HTTP_FIELD_ENTRY(Preference_Applied, "Preference-Applied") \
    HTTP_FIELD_ENTRY(ProfileObject, "ProfileObject") \
    HTTP_FIELD_ENTRY(Protocol, "Protocol") \
    HTTP_FIELD_ENTRY(Protocol_Info, "Protocol-Info") \
    HTTP_FIELD_ENTRY(Protocol_Query, "Protocol-Query") \
    HTTP_FIELD_ENTRY(Protocol_Request, "Protocol-Request") \
    HTTP_FIELD_ENTRY(Proxy_Authentication_Info, "Proxy-Authentication-Info") \
    HTTP_FIELD_ENTRY(Proxy_Features, "Proxy-Features") \
    HTTP_FIELD_ENTRY(Proxy_Instruction, "Proxy-Instruction") \
    HTTP_FIELD_ENTRY(Public, "Public") \
    HTTP_FIELD_ENTRY(Public_Key_Pins, "Public-Key-Pins") \
    HTTP_FIELD_ENTRY(Public_Key_Pins_Report_Only, "Public-Key-Pins-Report-Only") \
    HTTP_FIELD_ENTRY(Redirect_Ref, "Redirect-Ref") \
    HTTP_FIELD_ENTRY(Replay_Nonce, "Replay-Nonce") \
    HTTP_FIELD_ENTRY(Safe, "Safe") \
    HTTP_FIELD_ENTRY(Schedule_Reply, "Schedule-Reply") \
    HTTP_FIELD_ENTRY(Schedule_Tag, "Schedule-Tag") \
    HTTP_FIELD_ENTRY(Sec_Token_Binding, "Sec-Token-Binding") \
    HTTP_FIELD_ENTRY(Sec_WebSocket_Accept, "Sec-WebSocket-Accept") \
    HTTP_FIELD_ENTRY(Sec_WebSocket_Extensions, "Sec-WebSocket-Extensions") \
    HTTP_FIELD_ENTRY(Sec_WebSocket_Key, "Sec-WebSocket-Key") \
    HTTP_FIELD_ENTRY(Sec_WebSocket_Protocol, "Sec-WebSocket-Protocol") \
    HTTP_FIELD_ENTRY(Sec_WebSocket_Version, "Sec-WebSocket-Version") \
    HTTP_FIELD_ENTRY(Security_Scheme, "Security-Scheme") \
    HTTP_FIELD_ENTRY(Set_Cookie, "Set-Cookie") \
    HTTP_FIELD_ENTRY(Set_Cookie2, "Set-Cookie2") \
    HTTP_FIELD_ENTRY(SetProfile, "SetProfile") \
    HTTP_FIELD_ENTRY(SLUG, "SLUG") \
    HTTP_FIELD_ENTRY(SoapAction, "SoapAction") \
    HTTP_FIELD_ENTRY(Status_URI, "Status-URI") \
    HTTP_FIELD_ENTRY(Strict_Transport_Security, "Strict-Transport-Security") \
    HTTP_FIELD_ENTRY(Sunset, "Sunset") \
    HTTP_FIELD_ENTRY(Surrogate_Capability, "Surrogate-Capability") \
    HTTP_FIELD_ENTRY(Surrogate_Control, "Surrogate-Control") \
    HTTP_FIELD_ENTRY(TCN, "TCN") \
    HTTP_FIELD_ENTRY(Timeout, "Timeout") \
    HTTP_FIELD_ENTRY(Topic, "Topic") \
    HTTP_FIELD_ENTRY(TTL, "TTL") \
    HTTP_FIELD_ENTRY(Urgency, "Urgency") \
    HTTP_FIELD_ENTRY(URI, "URI") \
    HTTP_FIELD_ENTRY(Variant_Vary, "Variant-Vary") \
    HTTP_FIELD_ENTRY(Want_Digest, "Want-Digest") \
    HTTP_FIELD_ENTRY(X_Content_Type_Options, "X-Content-Type-Options") \
    HTTP_FIELD_ENTRY(X_Frame_Options, "X-Frame-Options") \
    /* Provisional Header */ \
    HTTP_FIELD_ENTRY(Access_Control, "Access-Control") \
    HTTP_FIELD_ENTRY(Access_Control_Allow_Credentials, "Access-Control-Allow-Credentials") \
    HTTP_FIELD_ENTRY(Access_Control_Allow_Headers, "Access-Control-Allow-Headers") \
    HTTP_FIELD_ENTRY(Access_Control_Allow_Methods, "Access-Control-Allow-Methods") \
    HTTP_FIELD_ENTRY(Access_Control_Allow_Origin, "Access-Control-Allow-Origin") \
    HTTP_FIELD_ENTRY(Access_Control_Max_Age, "Access-Control-Max-Age") \
    HTTP_FIELD_ENTRY(Access_Control_Request_Method, "Access-Control-Request-Method") \
    HTTP_FIELD_ENTRY(Access_Control_Request_Headers, "Access-Control-Request-Headers") \
    HTTP_FIELD_ENTRY(AMP_Cache_Transform, "AMP-Cache-Transform") \
    HTTP_FIELD_ENTRY(Compliance, "Compliance") \
    HTTP_FIELD_ENTRY(Content_Transfer_Encoding, "Content-Transfer-Encoding") \
    HTTP_FIELD_ENTRY(Cost, "Cost") \
    HTTP_FIELD_ENTRY(EDIINT_Features, "EDIINT-Features") \
    HTTP_FIELD_ENTRY(Isolation, "Isolation") \
    HTTP_FIELD_ENTRY(Message_ID, "Message-ID") \
    HTTP_FIELD_ENTRY(Method_Check, "Method-Check") \
    HTTP_FIELD_ENTRY(Method_Check_Expires, "Method-Check-Expires") \
    HTTP_FIELD_ENTRY(Non_Compliance, "Non-Compliance") \
    HTTP_FIELD_ENTRY(Optional, "Optional") \
    HTTP_FIELD_ENTRY(OSLC_Core_Version, "OSLC-Core-Version") \
    HTTP_FIELD_ENTRY(Referer_Root, "Referer-Root") \
    HTTP_FIELD_ENTRY(Repeatability_Client_ID, "Repeatability-Client-ID") \
    HTTP_FIELD_ENTRY(Repeatability_First_Sent, "Repeatability-First-Sent") \
    HTTP_FIELD_ENTRY(Repeatability_Request_ID, "Repeatability-Request-ID") \
    HTTP_FIELD_ENTRY(Repeatability_Result, "Repeatability-Result") \
    HTTP_FIELD_ENTRY(Resolution_Hint, "Resolution-Hint") \
    HTTP_FIELD_ENTRY(Resolver_Location, "Resolver-Location") \
    HTTP_FIELD_ENTRY(SubOK, "SubOK") \
    HTTP_FIELD_ENTRY(Subst, "Subst") \
    HTTP_FIELD_ENTRY(Timing_Allow_Origin, "Timing-Allow-Origin") \
    HTTP_FIELD_ENTRY(Title, "Title") \
    HTTP_FIELD_ENTRY(Traceparent, "Traceparent") \
    HTTP_FIELD_ENTRY(Tracestate, "Tracestate") \
    HTTP_FIELD_ENTRY(UA_Color, "UA-Color") \
    HTTP_FIELD_ENTRY(UA_Media, "UA-Media") \
    HTTP_FIELD_ENTRY(UA_Pixels, "UA-Pixels") \
    HTTP_FIELD_ENTRY(UA_Resolution, "UA-Resolution") \
    HTTP_FIELD_ENTRY(UA_Windowpixels, "UA-Windowpixels") \
    HTTP_FIELD_ENTRY(Version, "Version") \
    HTTP_FIELD_ENTRY(X_Device_Accept, "X-Device-Accept") \
    HTTP_FIELD_ENTRY(X_Device_Accept_Charset, "X-Device-Accept-Charset") \
    HTTP_FIELD_ENTRY(X_Device_Accept_Encoding, "X-Device-Accept-Encoding") \
    HTTP_FIELD_ENTRY(X_Device_Accept_Language, "X-Device-Accept-Language") \
    HTTP_FIELD_ENTRY(X_Device_User_Agent, "X-Device-User-Agent") \
    HTTP_FIELD_ENTRY(X_XSS_Protection, "X-XSS-Protection") \
    HTTP_FIELD_ENTRY(X_Forwarded_For, "X-Forwarded-For") \
    /* Others */ \
    HTTP_FIELD_ENTRY(Refresh, "Refresh") \
    HTTP_FIELD_ENTRY(Purpose, "Purpose") \
    HTTP_FIELD_ENTRY(Content_Security_Policy, "Content-Security-Policy") \
    HTTP_FIELD_ENTRY(Upgrade_Insecure_Requests, "Upgrade-Insecure-Requests") \
    HTTP_FIELD_ENTRY(Access_Control_Expose_Headers, "Access-Control-Expose-Headers") \
    /* Do not use */ \
    HTTP_FIELD_ENTRY(IGNORE_THIS, "IGNORE THIS") \
    LIST_DEFINITION_END // HTTP_FIELD_LIST

#define HTTP_METHOD_LIST \
    HTTP_METHOD_ENTRY(IGNORE_THIS) \
    HTTP_METHOD_ENTRY(OPTIONS) \
    HTTP_METHOD_ENTRY(GET) \
    HTTP_METHOD_ENTRY(HEAD) \
    HTTP_METHOD_ENTRY(POST) \
    HTTP_METHOD_ENTRY(PUT) \
    HTTP_METHOD_ENTRY(DELETE) \
    HTTP_METHOD_ENTRY(TRACE) \
    HTTP_METHOD_ENTRY(CONNECT) \
    HTTP_METHOD_ENTRY(PRI) \
    LIST_DEFINITION_END // HTTP_METHOD_LIST

#define HTTP_CODE_LIST \
    HTTP_CODE_ENTRY(0, Unknown, "Unknown") \
    /* Informational 1xx */ \
    HTTP_CODE_ENTRY(100, Continue, "Continue") \
    HTTP_CODE_ENTRY(101, Switching_Protocols, "Switching Protocols") \
    \
    /* Successful 2xx */ \
    HTTP_CODE_ENTRY(200, OK, "OK") \
    HTTP_CODE_ENTRY(201, Created, "Created") \
    HTTP_CODE_ENTRY(202, Accepted, "Accepted") \
    HTTP_CODE_ENTRY(203, Non_Authoritative_Information, "Non-Authoritative Information") \
    HTTP_CODE_ENTRY(204, No_Content,  "No Content") \
    HTTP_CODE_ENTRY(205, Reset_Content, "Reset Content") \
    HTTP_CODE_ENTRY(206, Partial_Content, "Partial Content") \
    \
    /* Redirection 3xx */ \
    HTTP_CODE_ENTRY(300, Multiple_Choices, "Multiple Choices") \
    HTTP_CODE_ENTRY(301, Moved_Permanently, "Moved Permanently") \
    HTTP_CODE_ENTRY(302, Found, "Found") \
    HTTP_CODE_ENTRY(303, See_Other, "See Other") \
    HTTP_CODE_ENTRY(304, Not_Modified, "Not Modified") \
    HTTP_CODE_ENTRY(305, Use_Proxy, "Use Proxy") \
    \
    /* 306 is unused */ \
    HTTP_CODE_ENTRY(307, Temporary_Redirect, "Temporary Redirect") \
    \
    /* Client Error 4xx */ \
    HTTP_CODE_ENTRY(400, Bad_Request, "Bad Request") \
    HTTP_CODE_ENTRY(401, Unauthorized, "Unauthorized") \
    HTTP_CODE_ENTRY(402, Payment_Required, "Payment Required") \
    HTTP_CODE_ENTRY(403, Forbidden, "Forbidden") \
    HTTP_CODE_ENTRY(404, Not_Found, "Not Found") \
    HTTP_CODE_ENTRY(405, Method_Not_Allowed, "Method Not Allowed") \
    HTTP_CODE_ENTRY(406, Not_Acceptable, "Not Acceptable") \
    HTTP_CODE_ENTRY(407, Proxy_Authentication_Required, "Proxy Authentication Required") \
    HTTP_CODE_ENTRY(408, Request_Timeout, "Request Timeout") \
    HTTP_CODE_ENTRY(409, Conflict, "Conflict") \
    HTTP_CODE_ENTRY(410, Gone, "Gone") \
    HTTP_CODE_ENTRY(411, Length_Required, "Length Required") \
    HTTP_CODE_ENTRY(412, Precondition_Failed, "Precondition Failed") \
    HTTP_CODE_ENTRY(413, Request_Entity_Too_Large, "Request Entity Too Large") \
    HTTP_CODE_ENTRY(414, Request_URI_Too_Long, "Request-URI Too Long") \
    HTTP_CODE_ENTRY(415, Unsupported_Media_Type, "Unsupported Media Type") \
    HTTP_CODE_ENTRY(416, Requested_Range_Not_Satisfiable, "Requested Range Not Satisfiable") \
    HTTP_CODE_ENTRY(417, Expectation_Failed, "Expectation Failed") \
    \
    /* Server Error 5xx */ \
    HTTP_CODE_ENTRY(500, Internal_Server_Error, "Internal Server Error") \
    HTTP_CODE_ENTRY(501, Not_Implemented, "Not Implemented") \
    HTTP_CODE_ENTRY(502, Bad_Gateway, "Bad Gateway") \
    HTTP_CODE_ENTRY(503, Service_Unavailable, "Service Unavailable") \
    HTTP_CODE_ENTRY(504, Gateway_Timeout, "Gateway Timeout") \
    HTTP_CODE_ENTRY(505, HTTP_Version_Not_Supported, "HTTP Version Not Supported") \
    LIST_DEFINITION_END // HTTP_CODE_LIST

enum class VERSION {
#define HTTP_VERSION_ENTRY(x, y) x,
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
};

enum class FIELD {
#define HTTP_FIELD_ENTRY(x, y) x,
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
};

enum class CODE {
#define HTTP_CODE_ENTRY(x, y, z) _##x = x, y = x,
    HTTP_CODE_LIST
#undef HTTP_CODE_ENTRY
};

enum class METHOD {
#define HTTP_METHOD_ENTRY(x) x,
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
};

static constexpr std::string to_string(const VERSION version) {
    switch(version) {
    default:
#define HTTP_VERSION_ENTRY(x, y) case VERSION::x: return { y, sizeof(y) - 1};
    HTTP_VERSION_LIST
#undef HTTP_VERSION_ENTRY
    }
}

static constexpr std::string to_string(const FIELD field) {
    switch(field) {
    default:
#define HTTP_FIELD_ENTRY(x, y) case FIELD::x: return { y, sizeof(y) - 1};
    HTTP_FIELD_LIST
#undef HTTP_FIELD_ENTRY
    }
}

static constexpr std::string to_string(const CODE code) {
    switch(code) {
    default:
#define HTTP_CODE_ENTRY(x, y, z) case CODE::_##x: return { z, sizeof(z) -1 };
    HTTP_CODE_LIST
#undef HTTP_CODE_ENTRY
    }
}

static constexpr std::string to_string(const METHOD method) {
    switch(method) {
    default:
#define HTTP_METHOD_ENTRY(x) case METHOD::x: return { #x, sizeof(#x) -1 };
    HTTP_METHOD_LIST
#undef HTTP_METHOD_ENTRY
    }
}

typedef std::unordered_map<FIELD, std::string> fields_t;

extern const std::unordered_map<std::string, FIELD> field_map;
extern const std::unordered_map<std::string, METHOD> method_map;

// This is raw code may does not map to actual string bug atoi
// "100" will map to CODE::_100
extern const std::unordered_map<std::string, CODE> code_map_raw;

constexpr auto to_field(const std::string &fieldname) {
    auto fielditr = field_map.find(fieldname);;
    if (fielditr == std::end(field_map)) {
        return FIELD::IGNORE_THIS;
    }
    return fielditr->second;
}

constexpr auto to_method(const std::string &methodname) {
    auto methoditr = method_map.find(methodname);;
    if (methoditr == std::end(method_map)) {
        return METHOD::IGNORE_THIS;
    }
    return methoditr->second;
}

constexpr auto to_version(const std::string &versiontext) {
    if (versiontext.compare("HTTP/1.1") == 0) return VERSION::VER_1_1;
    else if (versiontext.compare("HTTP/2.0") == 0) return VERSION::VER_2;
    else return VERSION::VER_UNKNOWN;
}

constexpr auto to_code_map(const std::string &codetext) {
    auto codeitr = code_map_raw.find(codetext);;
    if (codeitr == std::end(code_map_raw)) {
        return CODE::_0;
    }
    return codeitr->second;
}

} // namespace MMS::http