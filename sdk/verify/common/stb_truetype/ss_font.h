/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef __SS_FONT_H__
#define __SS_FONT_H__

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>

#include "stb_truetype.h"

class SS_Font
{
private:
    class FontFile
    {
    public:
        static FontFile *GetIns(const std::string &filepath);
        unsigned char   *GetBitmap(const wchar_t c, size_t font_size);
        ~FontFile();

    private:
        explicit FontFile(const std::string &filepath);
        FontFile(const FontFile &)            = delete;
        FontFile &operator=(const FontFile &) = delete;

    private:
        /* ttf file buffer */
        void *ttf_buffer;
        /* ttf font info */
        stbtt_fontinfo stb_font;

    private:
        using MapFileFont = std::map<std::string, std::shared_ptr<FontFile>>;
        static MapFileFont map_file_font;
    };

    /* bitmap cache */
    struct CacheKey
    {
        wchar_t c;
        size_t  size;
        CacheKey(wchar_t c, size_t size) : c(c), size(size) {}
        bool operator<(const CacheKey &other) const
        {
            return this->size != other.size ? this->size < other.size : this->c < other.c;
        }
        bool operator==(const CacheKey &other) const
        {
            return this->size == other.size && this->c == other.c;
        }
    };

    struct CacheVal
    {
        CacheKey       key;
        unsigned char *bitmap;
        size_t         weight;
        CacheVal() : key('0', 0), bitmap(nullptr), weight(0) {}
    };

public:
    explicit SS_Font(const std::string &filepath, size_t cache = 20);
    ~SS_Font();
    unsigned char *GetBitmap(const wchar_t c, size_t font_size);

    SS_Font(const SS_Font &)            = delete;
    SS_Font &operator=(const SS_Font &) = delete;

private:
    FontFile                                         *font_file;
    std::list<CacheVal>                               cache_list;
    std::map<CacheKey, std::list<CacheVal>::iterator> cache_map;
    size_t                                            cache_size;
};

#endif /* __SS_FONT_H__ */
