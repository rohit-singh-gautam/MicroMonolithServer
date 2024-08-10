


# HTTP 1.1
Refer to RFC https://www.rfc-editor.org/rfc/rfc2616

## Tokens

### Basic
    OCTET          = <any 8-bit sequence of data>
    CHAR           = <any US-ASCII character (octets 0 - 127)>
    UPALPHA        = <any US-ASCII uppercase letter "A".."Z">
    LOALPHA        = <any US-ASCII lowercase letter "a".."z">
    ALPHA          = UPALPHA | LOALPHA
    DIGIT          = <any US-ASCII digit "0".."9">
    CTL            = <any US-ASCII control character
                    (octets 0 - 31) and DEL (127)>
    CR             = <US-ASCII CR, carriage return (13)>
    LF             = <US-ASCII LF, linefeed (10)>
    SP             = <US-ASCII SP, space (32)>
    HT             = <US-ASCII HT, horizontal-tab (9)>
    <">            = <US-ASCII double-quote mark (34)>

    CRLF           = CR LF
    LWS            = [CRLF] 1*( SP | HT )
    HEX            = "A" | "B" | "C" | "D" | "E" | "F"
                        | "a" | "b" | "c" | "d" | "e" | "f" | DIGIT

    token          = 1*<any CHAR except CTLs or separators>
    separators     = "(" | ")" | "<" | ">" | "@"
                      | "," | ";" | ":" | "\" | <">
                      | "/" | "[" | "]" | "?" | "="
                      | "{" | "}" | SP | HT

    comment        = "(" *( ctext | quoted-pair | comment ) ")"
    ctext          = <any TEXT excluding "(" and ")">

    quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
    qdtext         = <any TEXT except <">>

    quoted-pair    = "\" CHAR

    HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
    http_URL = "http:" "//" host [ ":" port ] [ abs_path [ "?" query ]]

    HTTP-date    = rfc1123-date | rfc850-date | asctime-date
    rfc1123-date = wkday "," SP date1 SP time SP "GMT"
    rfc850-date  = weekday "," SP date2 SP time SP "GMT"
    asctime-date = wkday SP date3 SP time SP 4DIGIT
    date1        = 2DIGIT SP month SP 4DIGIT
                    ; day month year (e.g., 02 Jun 1982)
    date2        = 2DIGIT "-" month "-" 2DIGIT
                    ; day-month-year (e.g., 02-Jun-82)
    date3        = month SP ( 2DIGIT | ( SP 1DIGIT ))
                    ; month day (e.g., Jun  2)
    time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
                    ; 00:00:00 - 23:59:59
    wkday        = "Mon" | "Tue" | "Wed"
                | "Thu" | "Fri" | "Sat" | "Sun"
    weekday      = "Monday" | "Tuesday" | "Wednesday"
                | "Thursday" | "Friday" | "Saturday" | "Sunday"
    month        = "Jan" | "Feb" | "Mar" | "Apr"
                | "May" | "Jun" | "Jul" | "Aug"
                | "Sep" | "Oct" | "Nov" | "Dec"

    delta-seconds  = 1*DIGIT

## HTTP Message

### Message Type

    HTTP-message   = Request | Response

    generic-message =   start-line
                        *(message-header CRLF)
                        CRLF
                        [ message-body ]
    start-line      =   Request-Line | Status-Line

### Message Headers

    message-header = field-name ":" [ field-value ]
    field-name     = token
    field-value    = *( field-content | LWS )
    field-content  = <the OCTETs making up the field-value
                    and consisting of either *TEXT or combinations
                    of token, separators, and quoted-string>


### Message Body

    message-body = entity-body
                | <entity-body encoded as per Transfer-Encoding>

### General Field
    general-header = Cache-Control            ; Section 14.9
                      | Connection               ; Section 14.10
                      | Date                     ; Section 14.18
                      | Pragma                   ; Section 14.32
                      | Trailer                  ; Section 14.40
                      | Transfer-Encoding        ; Section 14.41
                      | Upgrade                  ; Section 14.42
                      | Via                      ; Section 14.45
                      | Warning                  ; Section 14.46


## HTTP Request
    Request       = Request-Line              ; Section 5.1
                        *(( general-header        ; Section 4.5
                         | request-header         ; Section 5.3
                         | entity-header ) CRLF)  ; Section 7.1
                        CRLF
                        [ message-body ]          ; Section 4.3

    Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

    Method         = "OPTIONS"                ; Section 9.2
                      | "GET"                    ; Section 9.3
                      | "HEAD"                   ; Section 9.4
                      | "POST"                   ; Section 9.5
                      | "PUT"                    ; Section 9.6
                      | "DELETE"                 ; Section 9.7
                      | "TRACE"                  ; Section 9.8
                      | "CONNECT"                ; Section 9.9
                      | extension-method
    extension-method = token

    Request-URI    = "*" | absoluteURI | abs_path | authority

    OPTIONS * HTTP/1.1

    Exammple:
        GET /pub/WWW/TheProject.html HTTP/1.1
        Host: www.w3.org

    
    request-header = Accept                   ; Section 14.1
                      | Accept-Charset           ; Section 14.2
                      | Accept-Encoding          ; Section 14.3
                      | Accept-Language          ; Section 14.4
                      | Authorization            ; Section 14.8
                      | Expect                   ; Section 14.20
                      | From                     ; Section 14.22
                      | Host                     ; Section 14.23
                      | If-Match                 ; Section 14.24
                      | If-Modified-Since        ; Section 14.25
                      | If-None-Match            ; Section 14.26
                      | If-Range                 ; Section 14.27
                      | If-Unmodified-Since      ; Section 14.28
                      | Max-Forwards             ; Section 14.31
                      | Proxy-Authorization      ; Section 14.34
                      | Range                    ; Section 14.35
                      | Referer                  ; Section 14.36
                      | TE                       ; Section 14.39
                      | User-Agent               ; Section 14.43

## HTTP Response

    Response      = Status-Line               ; Section 6.1
                       *(( general-header        ; Section 4.5
                        | response-header        ; Section 6.2
                        | entity-header ) CRLF)  ; Section 7.1
                       CRLF
                       [ message-body ]          ; Section 7.2

    Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF


## Status Code
      - 1xx: Informational - Request received, continuing process

      - 2xx: Success - The action was successfully received,
        understood, and accepted

      - 3xx: Redirection - Further action must be taken in order to
        complete the request

      - 4xx: Client Error - The request contains bad syntax or cannot
        be fulfilled

      - 5xx: Server Error - The server failed to fulfill an apparently
        valid request

    Status-Code    =
            "100"  ; Section 10.1.1: Continue
          | "101"  ; Section 10.1.2: Switching Protocols
          | "200"  ; Section 10.2.1: OK
          | "201"  ; Section 10.2.2: Created
          | "202"  ; Section 10.2.3: Accepted
          | "203"  ; Section 10.2.4: Non-Authoritative Information
          | "204"  ; Section 10.2.5: No Content
          | "205"  ; Section 10.2.6: Reset Content
          | "206"  ; Section 10.2.7: Partial Content
          | "300"  ; Section 10.3.1: Multiple Choices
          | "301"  ; Section 10.3.2: Moved Permanently
          | "302"  ; Section 10.3.3: Found
          | "303"  ; Section 10.3.4: See Other
          | "304"  ; Section 10.3.5: Not Modified
          | "305"  ; Section 10.3.6: Use Proxy
          | "307"  ; Section 10.3.8: Temporary Redirect
          | "400"  ; Section 10.4.1: Bad Request
          | "401"  ; Section 10.4.2: Unauthorized
          | "402"  ; Section 10.4.3: Payment Required
          | "403"  ; Section 10.4.4: Forbidden
          | "404"  ; Section 10.4.5: Not Found
          | "405"  ; Section 10.4.6: Method Not Allowed
          | "406"  ; Section 10.4.7: Not Acceptable
          | "407"  ; Section 10.4.8: Proxy Authentication Required
          | "408"  ; Section 10.4.9: Request Time-out
          | "409"  ; Section 10.4.10: Conflict
          | "410"  ; Section 10.4.11: Gone
          | "411"  ; Section 10.4.12: Length Required
          | "412"  ; Section 10.4.13: Precondition Failed
          | "413"  ; Section 10.4.14: Request Entity Too Large
          | "414"  ; Section 10.4.15: Request-URI Too Large
          | "415"  ; Section 10.4.16: Unsupported Media Type
          | "416"  ; Section 10.4.17: Requested range not satisfiable
          | "417"  ; Section 10.4.18: Expectation Failed
          | "500"  ; Section 10.5.1: Internal Server Error
          | "501"  ; Section 10.5.2: Not Implemented
          | "502"  ; Section 10.5.3: Bad Gateway
          | "503"  ; Section 10.5.4: Service Unavailable
          | "504"  ; Section 10.5.5: Gateway Time-out
          | "505"  ; Section 10.5.6: HTTP Version not supported
          | extension-code

      extension-code = 3DIGIT
      Reason-Phrase  = *<TEXT, excluding CR, LF>

### Response Header Fields
    response-header = Accept-Ranges           ; Section 14.5
                       | Age                     ; Section 14.6
                       | ETag                    ; Section 14.19
                       | Location                ; Section 14.30
                       | Proxy-Authenticate      ; Section 14.33
                       | Retry-After             ; Section 14.37
                       | Server                  ; Section 14.38
                       | Vary                    ; Section 14.44
                       | WWW-Authenticate        ; Section 14.47

