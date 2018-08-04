#pragma once

#define BASE64_ENCODE_OUT_SIZE(s)	(((s) + 2) / 3 * 4)
#define BASE64_DECODE_OUT_SIZE(s)	(((s)) / 4 * 3)

unsigned base64_encode(const char *in, unsigned int inlen, char *out);
unsigned base64_decode(const char *in, unsigned int inlen, char *out);
int base64_validate(char ch);
