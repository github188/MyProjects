//------------------------------------------------------------------------------------------
//     Copyright (c)2005-2010 PPLive Corporation.  All rights reserved.
//------------------------------------------------------------------------------------------

#include "Common.h"
#include <iostream>
#include "struct/Base.h"
#include "MD5.h"

using namespace std;

/************************************************************
*  md5 function implementation
************************************************************/
//MD5CTX CMD5::m_ctx;
//unsigned char CMD5::m_digest[16];

char* CMD5::Guid2Str(const GUID* guid, char* buf)
{
    int len;
    memset(buf,0, 33);
    len = sprintf(buf,
        "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
        (unsigned int)(guid->Data1), guid->Data2, guid->Data3,
        guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
        guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
    return buf;
}

Guid CMD5::GenerateRID(vector<MD5> block_md5s, unsigned int block_count)
{
    CMD5 md5_instance_;
    unsigned int i;

    md5_instance_.MD5Init();
    for (i = 0; i != block_count; ++i) {
        md5_instance_.MD5Update((unsigned char*)&block_md5s[i], 16);
    }
    md5_instance_.MD5Final();
    Guid guid_;
    memcpy(&guid_, md5_instance_.m_digest, 16);
    return guid_;
}

Guid CMD5::GenerateMD5(const unsigned char *data, unsigned int data_len)
{
    CMD5 md5_instance_;
    md5_instance_.MD5Init();
    md5_instance_.MD5Update(data, data_len);
    md5_instance_.MD5Final();
    Guid guid_;
    memcpy(&guid_, md5_instance_.m_digest, 16);
    return guid_;
}

/************************************************************
*  md5 base implementation
************************************************************/

void CMD5::Encode (unsigned char* output, unsigned long* input, unsigned int len)
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (unsigned char)(input[i] & 0xff);
        output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
        output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
        output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
    }
}

/*
* Decodes input (unsigned char) into output (unsigned long). Assumes len is
* a multiple of 4.
*/

void CMD5::Decode (unsigned long* output, const unsigned char* input, unsigned int len)
{
    unsigned int i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((unsigned long)input[j]) | (((unsigned long)input[j+1]) << 8) |
        (((unsigned long)input[j+2]) << 16) | (((unsigned long)input[j+3]) << 24);
}
//#endif /* i386 */

static unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions. */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits. */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/*
* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
* Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
    }
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
    }
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
    }
#define II(a, b, c, d, x, s, ac) { \
    (a) += I ((b), (c), (d)) + (x) + (unsigned long)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
    }

/* MD5 initialization. Begins an MD5 operation, writing a new m_ctx. */

void CMD5::MD5Init()
{
    m_ctx.count[0] = m_ctx.count[1] = 0;

    /* Load magic initialization constants.  */
    m_ctx.state[0] = 0x67452301;
    m_ctx.state[1] = 0xefcdab89;
    m_ctx.state[2] = 0x98badcfe;
    m_ctx.state[3] = 0x10325476;
}

/*
* MD5 block update operation. Continues an MD5 message-m_digest
* operation, processing another message block, and updating the
* m_ctx.
*/

void CMD5::MD5Update (const unsigned char* input, unsigned int inputLen)
{
    unsigned int i, index, partLen;

    /* Compute number of bytes mod 64 */
    index = (unsigned int)((m_ctx.count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((m_ctx.count[0] += ((unsigned long)inputLen << 3))
        < ((unsigned long)inputLen << 3))
        m_ctx.count[1]++;
    m_ctx.count[1] += ((unsigned long)inputLen >> 29);

    partLen = 64 - index;

    /* Transform as many times as possible. */
    if (inputLen >= partLen) {
        memcpy((void *)&m_ctx.buffer[index], (void *)input,
            partLen);
        MD5Transform (m_ctx.state, m_ctx.buffer);

        for (i = partLen; i + 63 < inputLen; i += 64)
            MD5Transform (m_ctx.state, &input[i]);

        index = 0;
    }
    else
        i = 0;

    /* Buffer remaining input */
    memcpy ((void *)&m_ctx.buffer[index], (void *)&input[i],
        inputLen-i);
}

/*
* MD5 finalization. Ends an MD5 message-m_digest operation, writing the
* the message m_digest and zeroizing the m_ctx.
*/

void CMD5::MD5Final ()
{
    unsigned char bits[8];
    unsigned int index, padLen;

    /* Save number of bits */
    Encode (bits, m_ctx.count, 8);

    /* Pad out to 56 mod 64. */
    index = (unsigned int)((m_ctx.count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update (PADDING, padLen);

    /* Append length (before padding) */
    MD5Update (bits, 8);

    /* Store state in m_digest */
    Encode (m_digest, m_ctx.state, 16);

    /* Zeroize sensitive information. */
    memset (&m_ctx, 0, sizeof (m_ctx));
}

/* MD5 basic transformation. Transforms state based on block. */

void CMD5::MD5Transform (unsigned long state[4], const unsigned char block[64])
{
    unsigned long a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode (x, block, 64);

    /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
    FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
    GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
    HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
    II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information. */
    memset ((void *)x, 0, sizeof (x));
}