/*
 * Copyright (c) 2025 - Aayush Anil Mishra.
 * See LICENSE for more information
 * https://github.com/aayushmisramechatronics/esp32cam-to-gdrive/Base64
 */
// Use correct header for AVR microcontrollers
// Include proper PROGMEM headers for AVR or standard platforms
#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#include <pgmspace.h>
#endif

// Base64 alphabet stored in flash memory (program space)
const char PROGMEM b64_alphabet[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Convert 3 bytes into 4 Base64 values
inline void a3_to_a4(unsigned char* a4, unsigned char* a3) {
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) | ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) | ((a3[2] & 0xc0) >> 6);
    a4[3] = a3[2] & 0x3f;
}

// Convert 4 Base64 values back into 3 bytes
inline void a4_to_a3(unsigned char* a3, unsigned char* a4) {
    a3[0] = (a4[0] << 2) | ((a4[1] & 0x30) >> 4);
    a3[1] = ((a4[1] & 0x0f) << 4) | ((a4[2] & 0x3c) >> 2);
    a3[2] = ((a4[2] & 0x03) << 6) | a4[3];
}

// Convert Base64 character to its 6-bit value
inline unsigned char b64_lookup(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';       // 0–25
    if (c >= 'a' && c <= 'z') return c - 71;        // 26–51
    if (c >= '0' && c <= '9') return c + 4;         // 52–61
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;  // Invalid character
}

// Encode input to Base64
int base64_encode(char* output, const char* input, int inputLen) {
    int i = 0, j = 0, encLen = 0;
    unsigned char a3[3], a4[4];

    while (inputLen--) {
        a3[i++] = *(input++);
        if (i == 3) {
            a3_to_a4(a4, a3);
            for (i = 0; i < 4; ++i)
                output[encLen++] = pgm_read_byte(&b64_alphabet[a4[i]]);
            i = 0;
        }
    }

    // Handle remaining bytes (padding)
    if (i) {
        for (j = i; j < 3; ++j) a3[j] = '\0';
        a3_to_a4(a4, a3);
        for (j = 0; j < i + 1; ++j)
            output[encLen++] = pgm_read_byte(&b64_alphabet[a4[j]]);
        while (i++ < 3)
            output[encLen++] = '=';
    }

    output[encLen] = '\0';  // Null-terminate output
    return encLen;          // Return length of encoded string
}

// Decode Base64 input
int base64_decode(char* output, const char* input, int inputLen) {
    int i = 0, j = 0, decLen = 0;
    unsigned char a3[3], a4[4];

    while (inputLen--) {
        if (*input == '=') break;  // End of encoded input
        a4[i++] = *(input++);
        if (i == 4) {
            for (i = 0; i < 4; ++i)
                a4[i] = b64_lookup(a4[i]);
            a4_to_a3(a3, a4);
            for (i = 0; i < 3; ++i)
                output[decLen++] = a3[i];
            i = 0;
        }
    }

    // Handle leftover Base64 chars
    if (i) {
        for (j = i; j < 4; ++j) a4[j] = 0;
        for (j = 0; j < 4; ++j) a4[j] = b64_lookup(a4[j]);
        a4_to_a3(a3, a4);
        for (j = 0; j < i - 1; ++j)
            output[decLen++] = a3[j];
    }

    output[decLen] = '\0';  // Null-terminate
    return decLen;          // Return length of decoded string
}

// Get length of Base64-encoded output for input of length plainLen
int base64_enc_len(int plainLen) {
    return ((plainLen + 2) / 3) * 4;
}

// Get length of decoded output from Base64 string
int base64_dec_len(const char* input, int inputLen) {
    int numEq = 0;
    for (int i = inputLen - 1; i >= 0 && input[i] == '='; --i)
        numEq++;  // Count '=' padding
    return ((6 * inputLen) / 8) - numEq;
}
