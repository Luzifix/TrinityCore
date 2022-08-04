/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Base64.h"
#include "BaseEncoding.h"
#include "Errors.h"

struct B64Impl
{
    static constexpr std::size_t BITS_PER_CHAR = 6;

    static constexpr char PADDING = '=';
    static constexpr char Encode(uint8 v)
    {
        ASSERT(v < 0x40);
        if (v < 26)  return 'A' + v;
        if (v < 52)  return 'a' + (v - 26);
        if (v < 62)  return '0' + (v - 52);
        if (v == 62) return '+';
        else         return '/';
    }

    static constexpr uint8 DECODE_ERROR = 0xff;
    static constexpr uint8 Decode(uint8 v)
    {
        if (('A' <= v) && (v <= 'Z')) return (v - 'A');
        if (('a' <= v) && (v <= 'z')) return (v - 'a') + 26;
        if (('0' <= v) && (v <= '9')) return (v - '0') + 52;
        if (v == '+') return 62;
        if (v == '/') return 63;
        return DECODE_ERROR;
    }
};

/*static*/ std::string Trinity::Encoding::Base64::Encode(std::vector<uint8> const& data)
{
    return Trinity::Impl::GenericBaseEncoding<B64Impl>::Encode(data);
}

/*static*/ Optional<std::vector<uint8>> Trinity::Encoding::Base64::Decode(std::string const& data)
{
    return Trinity::Impl::GenericBaseEncoding<B64Impl>::Decode(data);
}


/*static*/ std::string Trinity::Encoding::Base64::EncodeString(std::string stringToEncode)
{
    unsigned char const* stringToEncodeChars = reinterpret_cast<const unsigned char*>(stringToEncode.c_str());
    uint32 stringToEncodeLength = stringToEncode.length();

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (stringToEncodeLength--) {
        char_array_3[i++] = *(stringToEncodeChars++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64Chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64Chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';

    }

    return ret;
}

/*static*/ std::string Trinity::Encoding::Base64::DecodeString(std::string stringToDecode)
{
    size_t in_len = stringToDecode.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (stringToDecode[in_] != '=') && IsBase64Char(stringToDecode[in_])) {
        char_array_4[i++] = stringToDecode[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64Chars.find(char_array_4[i]) & 0xff;

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = base64Chars.find(char_array_4[j]) & 0xff;

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}
