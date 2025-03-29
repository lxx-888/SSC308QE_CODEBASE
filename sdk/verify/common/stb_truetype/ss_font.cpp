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

#include <codecvt>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#define STB_TRUETYPE_IMPLEMENTATION

#include "ss_font.h"

SS_Font::FontFile::MapFileFont SS_Font::FontFile::map_file_font;

SS_Font::FontFile *SS_Font::FontFile::GetIns(const std::string &filepath)
{
    auto itFont = SS_Font::FontFile::map_file_font.find(filepath);
    if (itFont == SS_Font::FontFile::map_file_font.end())
    {
        FontFile *font_file = nullptr;
        try {
            font_file = new FontFile(filepath);
        } catch (std::invalid_argument &e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        } catch (std::bad_alloc &e) {
            std::cerr << e.what() << std::endl;
            return nullptr;
        } catch (...) {
            std::cerr << "catch unknown exception" << std::endl;
            return nullptr;
        }
        auto pair = SS_Font::FontFile::map_file_font.insert(MapFileFont::value_type(filepath, font_file));
        if (!pair.second)
        {
            return nullptr;
        }
        return pair.first->second.get();
    }
    return itFont->second.get();
}

SS_Font::FontFile::FontFile(const std::string &filepath)
{
    std::ifstream fin(filepath, std::ios::binary);
    if (!fin.is_open())
    {
        throw std::invalid_argument("Font file " + filepath + " is not found");
    }
    fin.seekg(0, std::ios_base::end);
    std::streampos sz = fin.tellg();
    if (sz == 0)
    {
        throw std::invalid_argument("Font file size is zero");
    }
    fin.seekg(0, std::ios_base::beg);
    this->ttf_buffer = new unsigned char[sz];
    std::cout << "font file " << filepath << " sz is " << sz << std::endl;
    fin.read(static_cast<char *>(this->ttf_buffer), sz);
    fin.close();
    stbtt_InitFont(&this->stb_font, static_cast<unsigned char *>(this->ttf_buffer),
                   stbtt_GetFontOffsetForIndex(static_cast<unsigned char *>(this->ttf_buffer), 0));
}
SS_Font::FontFile::~FontFile()
{
    if (this->ttf_buffer)
    {
        delete[] static_cast<unsigned char *>(this->ttf_buffer);
    }
}
unsigned char *SS_Font::FontFile::GetBitmap(const wchar_t c, size_t font_size)
{
    unsigned char *bitmap = new unsigned char[font_size * (font_size + 1)];
    if (bitmap == nullptr)
    {
        return nullptr;
    }
    memset(bitmap, 0, font_size * font_size);

    //// reader font
    float scale  = stbtt_ScaleForPixelHeight(&this->stb_font, font_size);
    int   ascent = 0;
    stbtt_GetFontVMetrics(&this->stb_font, &ascent, 0, 0);

    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    stbtt_GetCodepointBitmapBox(&this->stb_font, c, scale, scale, &x0, &y0, &x1, &y1);

    int w = x1 - x0;
    int h = y1 - y0;
    int x = STBTT_ifloor((font_size - w) / 2);
    int y = STBTT_ifloor(ascent * scale) + y0 + 1;

    stbtt_MakeCodepointBitmap(&this->stb_font, bitmap + y * font_size + x, w, h, font_size, scale, scale, c);
    return bitmap;
}

SS_Font::SS_Font(const std::string &filepath, size_t cache) : font_file(FontFile::GetIns(filepath)), cache_size(cache)
{
}

SS_Font::~SS_Font()
{
    for (auto it = this->cache_list.begin(); it != this->cache_list.end(); ++it)
    {
        delete[] it->bitmap;
        it->bitmap = nullptr;
    }
    this->cache_map.clear();
    this->cache_list.clear();
}

unsigned char *SS_Font::GetBitmap(const wchar_t c, size_t font_size)
{
    if (this->font_file == nullptr)
    {
        return nullptr;
    }
    auto mapIt = this->cache_map.find(CacheKey(c, font_size));
    if (mapIt != this->cache_map.end())
    {
        /* if match cache, add weight and return cache bitmap */
        mapIt->second->weight++;
        for (auto lstIt = mapIt->second; lstIt != this->cache_list.end(); ++lstIt)
        {
            if (lstIt->weight > mapIt->second->weight)
            {
                this->cache_list.splice(lstIt, this->cache_list, mapIt->second);
            }
        }
        return mapIt->second->bitmap;
    }

    if (this->cache_list.size() >= this->cache_size)
    {
        /* If cache full, remove first */
        auto it = this->cache_list.begin();
        if (it->bitmap != nullptr)
        {
            delete[] it->bitmap;
            it->bitmap = nullptr;
        }
        this->cache_map.erase(it->key);
        this->cache_list.erase(it);
    }

    CacheVal val;
    val.bitmap = this->font_file->GetBitmap(c, font_size);
    val.key    = CacheKey(c, font_size);
    val.weight = 0;
    this->cache_list.push_front(val);
    this->cache_map[val.key] = this->cache_list.begin();
    return val.bitmap;
}
