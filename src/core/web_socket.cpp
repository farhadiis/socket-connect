#include "web_socket.h"

typedef unsigned char byte;

string ws_calc_accept_key(const string &key) {
    string webSocketKey = key + WS_GUID;
    SHA_CTX ctx;
    byte digest[SHA_DIGEST_LENGTH];
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, webSocketKey.c_str(), webSocketKey.length());
    SHA1_Final(digest, &ctx);
    return base64_encode(digest, SHA_DIGEST_LENGTH);
}

string ws_response_handshake(const string &key) {
    return "HTTP/1.1 101 Switching Protocols\r\n"
           "Upgrade: websocket\r\n"
           "Connection: Upgrade\r\n"
           "Sec-WebSocket-Accept: " + key + "\r\n\r\n";
}

int ws_make_frame(WebSocketFrameType frame_type,
                  unsigned char *msg, int msg_length,
                  unsigned char *buffer, int buffer_size) {
    int pos = 0;
    int size = msg_length;
    buffer[pos++] = (unsigned char) frame_type; // text frame

    if (size <= 125) {
        buffer[pos++] = size;
    } else if (size <= 65535) {
        buffer[pos++] = 126; //16 bit length follows

        buffer[pos++] = (size >> 8) & 0xFF; // leftmost first
        buffer[pos++] = size & 0xFF;
    } else { // >2^16-1 (65535)
        buffer[pos++] = 127; //64 bit length follows

        // write 8 bytes length (significant first)

        // since msg_length is int it can be no longer than 4 bytes = 2^32-1
        // padd zeroes for the first 4 bytes
        for (int i = 3; i >= 0; i--) {
            buffer[pos++] = 0;
        }
        // write the actual 32bit msg_length in the next 4 bytes
        for (int i = 3; i >= 0; i--) {
            buffer[pos++] = ((size >> 8 * i) & 0xFF);
        }
    }
    memcpy((void *) (buffer + pos), msg, size);
    return (size + pos);
}

WebSocketFrameType
ws_receive_frame(unsigned char *in_buffer, int in_length, unsigned char *out_buffer, int out_size, int *out_length) {
    //printf("getTextFrame()\n");
    if (in_length < 3) return INCOMPLETE_FRAME;

    unsigned char msg_opcode = in_buffer[0] & 0x0F;
    unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
    unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

    // *** message decoding

    int payload_length = 0;
    int pos = 2;
    int length_field = in_buffer[1] & (~0x80);
    unsigned int mask = 0;

    //printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

    if (length_field <= 125) {
        payload_length = length_field;
    } else if (length_field == 126) { //msglen is 16bit!
        //payload_length = in_buffer[2] + (in_buffer[3]<<8);
        payload_length = (
                (in_buffer[2] << 8) |
                (in_buffer[3])
        );
        pos += 2;
    } else if (length_field == 127) { //msglen is 64bit!
        payload_length = (int) (
                ((unsigned long) in_buffer[2] << 56u) |
                ((unsigned long) in_buffer[3] << 48u) |
                ((unsigned long) in_buffer[4] << 40u) |
                ((unsigned long) in_buffer[5] << 32u) |
                ((unsigned long) in_buffer[6] << 24u) |
                ((unsigned long) in_buffer[7] << 16u) |
                ((unsigned long) in_buffer[8] << 8u) |
                ((unsigned long) in_buffer[9])
        );
        pos += 8;
    }

    //printf("PAYLOAD_LEN: %08x\n", payload_length);
    if (in_length < payload_length + pos) {
        return INCOMPLETE_FRAME;
    }

    if (msg_masked) {
        mask = *((unsigned int *) (in_buffer + pos));
        //printf("MASK: %08x\n", mask);
        pos += 4;

        // unmask data:
        unsigned char *c = in_buffer + pos;
        for (int i = 0; i < payload_length; i++) {
            c[i] = c[i] ^ ((unsigned char *) (&mask))[i % 4];
        }
    }

    if (payload_length > out_size) {
        //TODO: if output buffer is too small -- ERROR or resize(free and allocate bigger one) the buffer ?
    }

    memcpy((void *) out_buffer, (void *) (in_buffer + pos), payload_length);
    out_buffer[payload_length] = 0;
    *out_length = payload_length + 1;

    //printf("TEXT: %s\n", out_buffer);

    if (msg_opcode == 0x0) return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME; // continuation frame ?
    if (msg_opcode == 0x1) return (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;
    if (msg_opcode == 0x2) return (msg_fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME;
    if (msg_opcode == 0x8) return CLOSING_FRAME;
    if (msg_opcode == 0x9) return PING_FRAME;
    if (msg_opcode == 0xA) return PONG_FRAME;

    return ERROR_FRAME;
}