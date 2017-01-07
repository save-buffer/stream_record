#include <iostream>
#include <string>
#include <regex>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
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
 - Add PKCS7 Padding to AES128 implementation
 - Implement HTTP
 - Implement SAMPLE-AES
 - Implement parsing of HLS Playlist Files
 - Implement transforming Fragmented MP4 into MP4
 - Implement automatic polling of target Twitch channel 
 - Implement multi-threaded file concatenation
 - (MAYBE?) Implement handling of WebVTT, MP2, and Packed Audio
 - Ask user for max video length
 - Handle gzip content encoding
 - Implement JSON Parsing
*/

int main(void)
{
    http_get("google.com");
    return(0);
}
