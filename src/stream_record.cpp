#include <iostream>
#include <string>
#include <regex>
#include <unordered_map>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <wininet.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>

typedef float real32;
typedef float real64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#include "aes.cpp"
#include "http.cpp"

/*
TODO(sasha): (In no particular order)
 - Finish Encryption
     - Add PKCS7 Padding to AES128 implementation
     - Implement SAMPLE-AES
 - Implement HTTP
     - Implement JSON Parsing
 - Implement parsing of HLS Playlist Files
 - Implement transforming Fragmented MP4 into MP4
 - Implement automatic polling of target Twitch channel 
 - Implement multi-threaded file concatenation
 - (MAYBE?) Implement handling of WebVTT, MP2, and Packed Audio
 - Ask user for max video length
 - Handle gzip content encoding

*/

int main(void)
{
    FILE *ex_html;
    http_header header;
    srand(time(NULL));
//    header.emplace("Client-ID", "t5rswxchxpav3qqrhlnjdso9321r2g");
    http_params params;
    params.emplace("player", "twitchweb");
    params.emplace("p", std::to_string((rand() % 999999)));
    params.emplace("type", "any");
    params.emplace("allow_source", "true");
    params.emplace("allow_audio_only", "true");
    params.emplace("allow_spectre", "true");
//    params.emplace("cookie", "0");
    params.emplace("oauth_token", "t5rswxchxpav3qqrhlnjdso9321r2g");
//    params.emplace("allow_host", "0");
    
//    std::string payload = "{\"player\": \"twitchweb\"\r\n\"p\": 12345\r\n\"type\": \"any\"\r\n\"allow_source\": \"true\",\"allow_audio_only\": \"true\",\"allow_spectre\": \"false\"}";
    std::string payload = "";
    http_response example = https_get("usher.ttvnw.net/api/channel/hls/mang0.m3u8", params, header, payload);
    ex_html = fopen("twitch.html", "w+");
    for(int i = 0; i < example.response_length; i++)
    {
	fwrite((char *)example.response + i, 1, 1, ex_html);
    }
    getchar();
    return(0);
}
