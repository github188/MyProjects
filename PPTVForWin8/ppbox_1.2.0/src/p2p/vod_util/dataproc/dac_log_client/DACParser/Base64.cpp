// Base64.cpp
#include "Common.h"
#include "Base64.h"

namespace Util {

    using namespace std;

    string & Base64::GetDelimiter()
    {
        static string delimiter = "\r\n";
        return delimiter;
    }

    char const * Base64::GetEnBase64Table()
    {
        static char const EnBase64Table[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        return EnBase64Table;
    }

    char const * Base64::GetDeBase64Table()
    {
        static char const DeBase64Table[] =
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            62,        // '+'
            0, 0, 0,
            63,        // '/'
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61,        // '0'-'9'
            0, 0, 0, 0, 0, 0, 0,
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
            13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,        // 'A'-'Z'
            0, 0, 0, 0, 0, 0,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
            39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        // 'a'-'z'
        };
        return DeBase64Table;
    }

    size_t Base64::DoEncode(char const * pSrc, size_t nSrcLen, char * pDst)
    {
        // 将来也可以考虑扩充到unicode版
        size_t const nMaxLineLen = _MAX_LINE_LENGTH;
        byte c1, c2, c3;    // 输入缓冲区读出3个字节
        size_t nDstLen = 0;             // 输出的字符计数
        size_t nLineLen = 0;            // 输出的行长度计数
        size_t nDiv = nSrcLen / 3;      // 输入数据长度除以3得到的倍数
        size_t nMod = nSrcLen % 3;      // 输入数据长度除以3得到的余数
        char const * const EnBase64Tab = GetEnBase64Table();
        // 每次取3个字节，编码成4个字符
        for (size_t i = 0; i < nDiv; i ++)
        {
            // 取3个字节
            c1 = *pSrc++;
            c2 = *pSrc++;
            c3 = *pSrc++;
            // 编码成4个字符
            *pDst++ = EnBase64Tab[c1 >> 2];
            *pDst++ = EnBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f];
            *pDst++ = EnBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f];
            *pDst++ = EnBase64Tab[c3 & 0x3f];
            nLineLen += 4;
            nDstLen += 4;
            // 输出换行？
            if (nLineLen > nMaxLineLen - 4)
            {
                string const& delimiter = GetDelimiter();
                std::copy(delimiter.begin(), delimiter.end(), pDst);
                nLineLen = 0;
                nDstLen += delimiter.size();
            }
        }
        // 编码余下的字节
        if (nMod == 1)
        {
            c1 = *pSrc++;
            *pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
            *pDst++ = EnBase64Tab[((c1 & 0x03) << 4)];
            *pDst++ = '=';
            *pDst++ = '=';
            nLineLen += 4;
            nDstLen += 4;
        }
        else if (nMod == 2)
        {
            c1 = *pSrc++;
            c2 = *pSrc++;
            *pDst++ = EnBase64Tab[(c1 & 0xfc) >> 2];
            *pDst++ = EnBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
            *pDst++ = EnBase64Tab[((c2 & 0x0f) << 2)];
            *pDst++ = '=';
            nDstLen += 4;
        }
        return nDstLen;
    }

    size_t Base64::DoDecode(char const * pSrc, size_t nSrcLen, char * pDst)
    {
        size_t nValue;             // 解码用到的长整数
        size_t nDstLen = 0;
        size_t i = 0;
        // 取4个字符，解码到一个长整数，再经过移位得到3个字节
        char const * DeBase64Tab = GetDeBase64Table();
        while (i < nSrcLen)
        {
            if (*pSrc != '\r' && *pSrc!='\n')
            {
                nValue = DeBase64Tab[*pSrc++] << 18;
                nValue += DeBase64Tab[*pSrc++] << 12;
                *pDst++ = (nValue & 0x00ff0000) >> 16;
                nDstLen++;
                if (*pSrc != '=')
                {
                    nValue += DeBase64Tab[*pSrc++] << 6;
                    *pDst++ = (nValue & 0x0000ff00) >> 8;
                    nDstLen++;
                    if (*pSrc != '=')
                    {
                        nValue += DeBase64Tab[*pSrc++];
                        *pDst++ =nValue & 0x000000ff;
                        nDstLen++;
                    }
                }
                i += 4;
            }
            else        // 回车换行，跳过
            {
                pSrc++;
                i++;
            }
        }
        return nDstLen;
    }

    size_t Base64::calcEnoughEncodedLength(size_t length)
    {
        size_t len = length / 3;
        if (length % 3 > 0)
            ++len;
        len *= 4;
        size_t line = len / _MAX_LINE_LENGTH;
        if (len % _MAX_LINE_LENGTH > 0)
            ++line;
        len += line * 2;
        return len;
    }

    size_t Base64::calcEnoughDecodedLength(size_t length)
    {
        return length;
    }

    Base64::byte Base64::getFirstSixBits(Base64::byte val)
    {
        return (val & 0xFC) >> 2;
    }

    Base64::byte Base64::getLastSixBits(Base64::byte val)
    {
        return val & 0x3F;
    }

    string Base64::Encode(string const & str)
    {
        string result;
        size_t size = calcEnoughEncodedLength(str.size());
        if (size > 0)
        {
            result.resize(size);
            size_t len = DoEncode(str.data(), str.size(), &result[0]);
            result.resize(len);
        }
        return result;
    }

    string Base64::Decode(string const & str)
    {
        string result;
        size_t size = calcEnoughDecodedLength(str.size());
        if (size > 0)
        {
            result.resize(size);
            size_t len = DoDecode(str.data(), str.size(), &result[0]);
            result.resize(len);
        }
        return result;
    }

}
