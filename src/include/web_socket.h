#ifndef TRACKER_WEB_SOCKET_H
#define TRACKER_WEB_SOCKET_H

#include <cstring>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include "base64.h"
#include <openssl/sha.h>

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

using namespace std;

enum WebSocketFrameType {
    ERROR_FRAME = 0xFF00,
    INCOMPLETE_FRAME = 0xFE00,

    OPENING_FRAME = 0x3300,
    CLOSING_FRAME = 0x3400,

    INCOMPLETE_TEXT_FRAME = 0x01,
    INCOMPLETE_BINARY_FRAME = 0x02,

    TEXT_FRAME = 0x81,
    BINARY_FRAME = 0x82,

    PING_FRAME = 0x19,
    PONG_FRAME = 0x1A
};

string ws_calc_accept_key(const string &key);

string ws_response_handshake(const string &key);

int
ws_make_frame(WebSocketFrameType frame_type,
              unsigned char *msg, int msg_length,
              unsigned char *buffer, int buffer_size);

WebSocketFrameType
ws_receive_frame(unsigned char *in_buffer, int in_length, unsigned char *out_buffer, int out_size, int *out_length);

#endif //TRACKER_WEB_SOCKET_H
