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

#ifndef __SS_GRAPH_H__
#define __SS_GRAPH_H__

#include <cmath>
namespace SS_Graph
{

#define SS_GRAPH_CANVAS_PLANE_MAX_NUM (3)

typedef unsigned int Color;
typedef unsigned int AbsCoord;

struct CanvasDesc
{
    struct {
        unsigned char bpp;
        unsigned char h_sample;
        unsigned char v_sample;
        Color (*color_convert)(Color c, void *user);
        void *user_data;
    } plane[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
};

struct Canvas
{
    AbsCoord     width;
    AbsCoord     height;
    char        *data[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
    AbsCoord     stride[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
};

typedef void (*FillBpp)(void *, size_t, size_t, unsigned int);

class Graph
{
public:
    explicit Graph(const CanvasDesc &desc, const Canvas &canvas);
    virtual ~Graph();
    void FillColor(const Color &c);

    void SetPixel(const Color &c, const AbsCoord &x, const AbsCoord &y);
    void FillRect(const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w, const AbsCoord &h);
    void FillEllipse(const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w, const AbsCoord &h);
    void DrawFrame(const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w, const AbsCoord &h,
                   const AbsCoord &thickness = 2);
    void DrawLine(const Color &c, const AbsCoord &x0, const AbsCoord &y0, const AbsCoord &x1, const AbsCoord &y1,
                         const AbsCoord &thickness = 2);
    void DrawBitmap(const Color &c, const AbsCoord &x, const AbsCoord &y, const char *bitmap, const AbsCoord &w,
                           const AbsCoord &h);
    void DrawDotMatrix(const Color &c, const char *dotMatrix, const AbsCoord &w, const AbsCoord &h,
                            const AbsCoord &size = 2);

    template <typename RelaCoord>
    void SetPixel(float (*Scale)(const RelaCoord &val), const Color &c, const RelaCoord &x, const RelaCoord &y)
    {
        AbsCoord abs_x = std::floor(this->canvas.width * Scale(x));
        AbsCoord abs_y = std::floor(this->canvas.height * Scale(y));
        this->SetPixel(c, abs_x, abs_y);
    }

    template <typename RelaCoord>
    void FillRect(float (*Scale)(const RelaCoord &val), const Color &c, const RelaCoord &x, const RelaCoord &y,
                  const RelaCoord &w, const RelaCoord &h)
    {
        AbsCoord abs_x = std::floor(this->canvas.width * Scale(x));
        AbsCoord abs_y = std::floor(this->canvas.height * Scale(y));
        AbsCoord abs_w = std::ceil(this->canvas.width * Scale(w));
        AbsCoord abs_h = std::ceil(this->canvas.height * Scale(h));
        this->FillRect(c, abs_x, abs_y, abs_w, abs_h);
    }
    template <typename RelaCoord>
    void FillEllipse(float (*Scale)(const RelaCoord &val), const Color &c, const RelaCoord &x, const RelaCoord &y,
                     const RelaCoord &w, const RelaCoord &h)
    {
        AbsCoord abs_x = std::floor(this->canvas.width * Scale(x));
        AbsCoord abs_y = std::floor(this->canvas.height * Scale(y));
        AbsCoord abs_w = std::ceil(this->canvas.width * Scale(w));
        AbsCoord abs_h = std::ceil(this->canvas.height * Scale(h));
        this->FillEllipse(c, abs_x, abs_y, abs_w, abs_h);
    }
    template <typename RelaCoord>
    void DrawFrame(float (*Scale)(const RelaCoord &val), const Color &c, const RelaCoord &x, const RelaCoord &y,
                   const RelaCoord &w, const RelaCoord &h, const RelaCoord &thickness = 2)
    {
        AbsCoord abs_x = std::floor(this->canvas.width * Scale(x));
        AbsCoord abs_y = std::floor(this->canvas.height * Scale(y));
        AbsCoord abs_w = std::ceil(this->canvas.width * Scale(w));
        AbsCoord abs_h = std::ceil(this->canvas.height * Scale(h));
        this->DrawFrame(c, abs_x, abs_y, abs_w, abs_h, thickness);
    }
    template <typename RelaCoord>
    void DrawLine(float (*Scale)(const RelaCoord &val), const Color &c, const RelaCoord &x0, const RelaCoord &y0,
                  const RelaCoord &x1, const RelaCoord &y1, const RelaCoord &thickness = 2)
    {
        AbsCoord abs_x0 = std::floor(this->canvas.width * Scale(x0));
        AbsCoord abs_y0 = std::floor(this->canvas.height * Scale(y0));
        AbsCoord abs_x1 = std::ceil(this->canvas.width * Scale(x1));
        AbsCoord abs_y1 = std::ceil(this->canvas.height * Scale(y1));
        this->DrawLine(c, abs_x0, abs_y0, abs_x1, abs_y1, thickness);
    }
    template <typename RelaCoord>
    void DrawBitmap(float (*Scale)(const RelaCoord &val), const Color &c, const RelaCoord &x, const RelaCoord &y,
                    const char *bitmap, const AbsCoord &w, const AbsCoord &h)
    {
        AbsCoord abs_x = std::floor(this->canvas.width * Scale(x));
        AbsCoord abs_y = std::floor(this->canvas.height * Scale(y));
        this->DrawBitmap(c, abs_x, abs_y, bitmap, w, h);
    }

private:
    void FillColor(const unsigned int plane_idx, const Color &c);
    void SetPixel(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y);
    void FillRect(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y, const AbsCoord &w,
                  const AbsCoord &h);
    void FillEllipse(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y,
                     const AbsCoord &w, const AbsCoord &h);
    void DrawFrame(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y,
                   const AbsCoord &w, const AbsCoord &h, const AbsCoord &thickness);
    void DrawLine(const unsigned int plane_idx, const Color &c, const AbsCoord &x0, const AbsCoord &y0,
                  const AbsCoord &x1, const AbsCoord &y1, const AbsCoord &thickness);
    void DrawBitmap(const unsigned int plane_idx, const Color &c, const AbsCoord &x, const AbsCoord &y,
                    const char *bitmap, const AbsCoord &w, const AbsCoord &h);
    void DrawDotMatrix(const unsigned int plane_idx, const Color &c, const char *dotMatrix, const AbsCoord &w,
                       const AbsCoord &h, const AbsCoord &size);

private:
    CanvasDesc   desc;
    Canvas       canvas;
    FillBpp      fillbpp[SS_GRAPH_CANVAS_PLANE_MAX_NUM];
    unsigned int planeNum;
};

};     // namespace SS_Graph
#endif /* __SS_GRAPH_H__ */
