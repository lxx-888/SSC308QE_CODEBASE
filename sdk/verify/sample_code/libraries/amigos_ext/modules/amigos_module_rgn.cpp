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

#include <math.h>
#include <pthread.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <list>

#include "amigos_env.h"
#include "amigos_log.h"
#include "amigos_module_init.h"
#include "amigos_module_rgn_metadata_define.h"
#include "amigos_module_base.h"
#include "amigos_surface_rgn.h"
#include "mi_common_datatype.h"
#include "amigos_surface_base.h"
#include "amigos_module_rgn.h"
#include "mi_rgn_datatype.h"
#include "mi_rgn.h"
#include "ss_auto_lock.h"
#include "ss_linker.h"
#include "ss_enum_cast.hpp"
#include "ss_graph.h"
#include "ss_font.h"
#include "ss_packet.h"
#include "ss_thread.h"

#define CHECK_RESULT(expectation, erraction, function, ...) \
    ({                                                      \
        if ((expectation) == (function(__VA_ARGS__)))       \
        {                                                   \
            AMIGOS_INFO("%s Success\n", #function);         \
        }                                                   \
        else                                                \
        {                                                   \
            AMIGOS_ERR("%s Failed\n", #function);           \
            erraction;                                      \
        }                                                   \
    })

#define CHECK_RESULT_NO_LOG(expectation, erraction, function, ...) \
    ({                                                             \
        if ((expectation) != (function(__VA_ARGS__)))              \
        {                                                          \
            AMIGOS_ERR("%s Failed\n", #function);                  \
            erraction;                                             \
        }                                                          \
    })

#define RGN_OSD_TEXT_LAYER (2)
#define RGN_OSD_CANVAS_LAYER (1)
#define RGN_OSD_LINE_LAYER (0)

#define RGN_DEFAULT_COLOR_KEY_VAL (0x23232323)

static MI_RGN_PixelFormat_e _ConvertRawDataTypetoRgnPixelFmt(const raw_video_fmt fmt);
static float                _CalCoordScale(const unsigned int &x);

static unsigned int _CalTinckness(unsigned int width, unsigned int height, AmigosModuleRgn::ThicknessLevel lv);
static unsigned int _CalFontSize(unsigned int width, unsigned int height, AmigosModuleRgn::SizeLevel lv);
static MI_RGN_BlockSize_e _CalMosaicBlockSize(AmigosModuleRgn::SizeLevel lv);
static MI_RGN_BlockSize_e _CalColorInvertBlockSize(unsigned int width, unsigned int height);
static unsigned int _CalDotMatrixSize(unsigned int width, unsigned int height, unsigned int mapWidth,
                                      unsigned int mapHeight, AmigosModuleRgn::SizeLevel lv);

static unsigned int _ConvertColorToArgb1555(unsigned int c, void *);
static unsigned int _ConvertColorToArgb4444(unsigned int c, void *);
static unsigned int _ConvertColorToRgb565(unsigned int c, void *);
static unsigned int _ConvertColorToI2(unsigned int c, void *);
static unsigned int _ConvertColorToI4(unsigned int c, void *);
static unsigned int _ConvertColorToI8(unsigned int c, void *);
static unsigned int _ConvertColorToUyv(unsigned int c, void *);

static unsigned int _CalColorInvert(unsigned int c);

void *ColorInvertProcess(struct ss_thread_buffer *thread_buf);

SS_ENUM_CAST_STR(MI_RGN_AlphaMode_e, {
    {E_MI_RGN_PIXEL_ALPHA, "pixel"},
    {E_MI_RGN_CONSTANT_ALPHA, "const"},
});

SS_ENUM_CAST_STR(MI_RGN_PixelFormat_e, {
    {E_MI_RGN_PIXEL_FORMAT_ARGB1555 , "argb1555" },
    {E_MI_RGN_PIXEL_FORMAT_ARGB4444 , "argb4444" },
    {E_MI_RGN_PIXEL_FORMAT_I2       , "i2"       },
    {E_MI_RGN_PIXEL_FORMAT_I4       , "i4"       },
    {E_MI_RGN_PIXEL_FORMAT_I8       , "i8"       },
    {E_MI_RGN_PIXEL_FORMAT_RGB565   , "rgb565"   },
    {E_MI_RGN_PIXEL_FORMAT_ARGB8888 , "argb8888" },
});

SS_ENUM_CAST_STR(AmigosModuleRgn::ThicknessLevel, {
    {AmigosModuleRgn::E_THICKNESS_LEVEL_NORMAL , "normal" },
    {AmigosModuleRgn::E_THICKNESS_LEVEL_THIN   , "thin"   },
    {AmigosModuleRgn::E_THICKNESS_LEVEL_THICK  , "thick"  },
});

SS_ENUM_CAST_STR(AmigosModuleRgn::SizeLevel, {
    {AmigosModuleRgn::E_SIZE_LEVEL_NORMAL , "normal" },
    {AmigosModuleRgn::E_SIZE_LEVEL_TINY   , "tiny"   },
    {AmigosModuleRgn::E_SIZE_LEVEL_SMALL  , "small"  },
    {AmigosModuleRgn::E_SIZE_LEVEL_LARGE  , "large"  },
    {AmigosModuleRgn::E_SIZE_LEVEL_HUGE   , "huge"   },
});

SS_ENUM_CAST_STR(MI_RGN_CoverMode_e, {
    { E_MI_RGN_COVER_MODE_COLOR  , "color"  },
    { E_MI_RGN_COVER_MODE_MOSAIC , "mosaic" },
});

SS_ENUM_CAST_STR(MI_RGN_ColorInvertWorkMode_e, {
    { E_MI_RGN_COLOR_INVERT_WORK_MODE_AUTO, "auto"  },
    { E_MI_RGN_COLOR_INVERT_WORK_MODE_MANUAL, "manual" },
});

static unsigned int g_alignment[E_MI_RGN_PIXEL_FORMAT_MAX] = {
    [ E_MI_RGN_PIXEL_FORMAT_ARGB1555 ] = 1,
    [ E_MI_RGN_PIXEL_FORMAT_ARGB4444 ] = 1,
    [ E_MI_RGN_PIXEL_FORMAT_I2       ] = 4,
    [ E_MI_RGN_PIXEL_FORMAT_I4       ] = 2,
    [ E_MI_RGN_PIXEL_FORMAT_I8       ] = 1,
    [ E_MI_RGN_PIXEL_FORMAT_RGB565   ] = 1,
    [ E_MI_RGN_PIXEL_FORMAT_ARGB8888 ] = 1,
};

static unsigned int g_bg_color[E_MI_RGN_PIXEL_FORMAT_MAX] = {
    [ E_MI_RGN_PIXEL_FORMAT_ARGB1555 ] = 0,
    [ E_MI_RGN_PIXEL_FORMAT_ARGB4444 ] = 0,
    [ E_MI_RGN_PIXEL_FORMAT_I2       ] = 0,
    [ E_MI_RGN_PIXEL_FORMAT_I4       ] = 0,
    [ E_MI_RGN_PIXEL_FORMAT_I8       ] = 0,
    [ E_MI_RGN_PIXEL_FORMAT_RGB565   ] = 0,
    [ E_MI_RGN_PIXEL_FORMAT_ARGB8888 ] = 0,
};

static SS_Graph::CanvasDesc g_canvas_desc_map[E_MI_RGN_PIXEL_FORMAT_MAX] = {
    [E_MI_RGN_PIXEL_FORMAT_ARGB1555] = {
        .plane = {
            [0] = {
                .bpp = 16,
                .h_sample = 1,
                .v_sample = 1,
                .color_convert = _ConvertColorToArgb1555
            }
        }
    },
    [E_MI_RGN_PIXEL_FORMAT_ARGB4444] = {
        .plane = {
            [0] = {
                .bpp = 16,
                .h_sample = 1,
                .v_sample = 1,
                .color_convert = _ConvertColorToArgb4444
            }
        }
    },
    [E_MI_RGN_PIXEL_FORMAT_I2] = {
        .plane = {
            [0] = {
                .bpp = 2,
                .h_sample = 1,
                .v_sample = 1,
                .color_convert = _ConvertColorToI2
            }
        }
    },
    [E_MI_RGN_PIXEL_FORMAT_I4] = {
        .plane = {
            [0] = {
                .bpp = 4,
                .h_sample = 1,
                .v_sample = 1,
                .color_convert = _ConvertColorToI4
            }
        }
    },
    [E_MI_RGN_PIXEL_FORMAT_I8] = {
        .plane = {
            [0] = {
                .bpp = 8,
                .h_sample = 1,
                .v_sample = 1,
                .color_convert = _ConvertColorToI8
            }
        }
    },
    [E_MI_RGN_PIXEL_FORMAT_RGB565] = {
        .plane = {
            [0] = {
                .bpp = 16,
                .h_sample = 1,
                .v_sample = 1,
                .color_convert = _ConvertColorToRgb565
            }
        }
    },
    [E_MI_RGN_PIXEL_FORMAT_ARGB8888] = {
        .plane = {
            [0] = {
                .bpp = 32,
                .h_sample = 1,
                .v_sample = 1
            }
        }
    },
};

static MI_RGN_PaletteTable_t g_palette_table = {.astElement =
{
    [0]   = {0xff, 0x00, 0x00, 0x00}, // #000000, Black
    [1]   = {0xff, 0x80, 0x00, 0x00}, // #800000, Maroon
    [2]   = {0xff, 0x00, 0x80, 0x00}, // #008000, Green
    [3]   = {0xff, 0x80, 0x80, 0x00}, // #808000, Olive
    [4]   = {0xff, 0x00, 0x00, 0x80}, // #000080, Navy
    [5]   = {0xff, 0x80, 0x00, 0x80}, // #800080, Purple
    [6]   = {0xff, 0x00, 0x80, 0x80}, // #008080, Teal
    [7]   = {0xff, 0xc0, 0xc0, 0xc0}, // #c0c0c0, Silver
    [8]   = {0xff, 0x80, 0x80, 0x80}, // #808080, Grey
    [9]   = {0xff, 0xff, 0x00, 0x00}, // #ff0000, Red
    [10]  = {0xff, 0x00, 0xff, 0x00}, // #00ff00, Lime
    [11]  = {0xff, 0xff, 0xff, 0x00}, // #ffff00, Yellow
    [12]  = {0xff, 0x00, 0x00, 0xff}, // #0000ff, Blue
    [13]  = {0xff, 0xff, 0x00, 0xff}, // #ff00ff, Fuchsia
    [14]  = {0xff, 0x00, 0xff, 0xff}, // #00ffff, Aqua
    [15]  = {0xff, 0xff, 0xff, 0xff}, // #ffffff, White
    [16]  = {0xff, 0x00, 0x00, 0x00}, // #000000, Grey0
    [17]  = {0xff, 0x00, 0x00, 0x5f}, // #00005f, NavyBlue
    [18]  = {0xff, 0x00, 0x00, 0x87}, // #000087, DarkBlue
    [19]  = {0xff, 0x00, 0x00, 0xaf}, // #0000af, Blue3
    [20]  = {0xff, 0x00, 0x00, 0xd7}, // #0000d7, Blue3
    [21]  = {0xff, 0x00, 0x00, 0xff}, // #0000ff, Blue1
    [22]  = {0xff, 0x00, 0x5f, 0x00}, // #005f00, DarkGreen
    [23]  = {0xff, 0x00, 0x5f, 0x5f}, // #005f5f, DeepSkyBlue4
    [24]  = {0xff, 0x00, 0x5f, 0x87}, // #005f87, DeepSkyBlue4
    [25]  = {0xff, 0x00, 0x5f, 0xaf}, // #005faf, DeepSkyBlue4
    [26]  = {0xff, 0x00, 0x5f, 0xd7}, // #005fd7, DodgerBlue3
    [27]  = {0xff, 0x00, 0x5f, 0xff}, // #005fff, DodgerBlue2
    [28]  = {0xff, 0x00, 0x87, 0x00}, // #008700, Green4
    [29]  = {0xff, 0x00, 0x87, 0x5f}, // #00875f, SpringGreen4
    [30]  = {0xff, 0x00, 0x87, 0x87}, // #008787, Turquoise4
    [31]  = {0xff, 0x00, 0x87, 0xaf}, // #0087af, DeepSkyBlue3
    [32]  = {0xff, 0x00, 0x87, 0xd7}, // #0087d7, DeepSkyBlue3
    [33]  = {0xff, 0x00, 0x87, 0xff}, // #0087ff, DodgerBlue1
    [34]  = {0xff, 0x00, 0xaf, 0x00}, // #00af00, Green3
    [35]  = {0xff, 0x00, 0xaf, 0x5f}, // #00af5f, SpringGreen3
    [36]  = {0xff, 0x00, 0xaf, 0x87}, // #00af87, DarkCyan
    [37]  = {0xff, 0x00, 0xaf, 0xaf}, // #00afaf, LightSeaGreen
    [38]  = {0xff, 0x00, 0xaf, 0xd7}, // #00afd7, DeepSkyBlue2
    [39]  = {0xff, 0x00, 0xaf, 0xff}, // #00afff, DeepSkyBlue1
    [40]  = {0xff, 0x00, 0xd7, 0x00}, // #00d700, Green3
    [41]  = {0xff, 0x00, 0xd7, 0x5f}, // #00d75f, SpringGreen3
    [42]  = {0xff, 0x00, 0xd7, 0x87}, // #00d787, SpringGreen2
    [43]  = {0xff, 0x00, 0xd7, 0xaf}, // #00d7af, Cyan3
    [44]  = {0xff, 0x00, 0xd7, 0xd7}, // #00d7d7, DarkTurquoise
    [45]  = {0xff, 0x00, 0xd7, 0xff}, // #00d7ff, Turquoise2
    [46]  = {0xff, 0x00, 0xff, 0x00}, // #00ff00, Green1
    [47]  = {0xff, 0x00, 0xff, 0x5f}, // #00ff5f, SpringGreen2
    [48]  = {0xff, 0x00, 0xff, 0x87}, // #00ff87, SpringGreen1
    [49]  = {0xff, 0x00, 0xff, 0xaf}, // #00ffaf, MediumSpringGreen
    [50]  = {0xff, 0x00, 0xff, 0xd7}, // #00ffd7, Cyan2
    [51]  = {0xff, 0x00, 0xff, 0xff}, // #00ffff, Cyan1
    [52]  = {0xff, 0x5f, 0x00, 0x00}, // #5f0000, DarkRed
    [53]  = {0xff, 0x5f, 0x00, 0x5f}, // #5f005f, DeepPink4
    [54]  = {0xff, 0x5f, 0x00, 0x87}, // #5f0087, Purple4
    [55]  = {0xff, 0x5f, 0x00, 0xaf}, // #5f00af, Purple4
    [56]  = {0xff, 0x5f, 0x00, 0xd7}, // #5f00d7, Purple3
    [57]  = {0xff, 0x5f, 0x00, 0xff}, // #5f00ff, BlueViolet
    [58]  = {0xff, 0x5f, 0x5f, 0x00}, // #5f5f00, Orange4
    [59]  = {0xff, 0x5f, 0x5f, 0x5f}, // #5f5f5f, Grey37
    [60]  = {0xff, 0x5f, 0x5f, 0x87}, // #5f5f87, MediumPurple4
    [61]  = {0xff, 0x5f, 0x5f, 0xaf}, // #5f5faf, SlateBlue3
    [62]  = {0xff, 0x5f, 0x5f, 0xd7}, // #5f5fd7, SlateBlue3
    [63]  = {0xff, 0x5f, 0x5f, 0xff}, // #5f5fff, RoyalBlue1
    [64]  = {0xff, 0x5f, 0x87, 0x00}, // #5f8700, Chartreuse4
    [65]  = {0xff, 0x5f, 0x87, 0x5f}, // #5f875f, DarkSeaGreen4
    [66]  = {0xff, 0x5f, 0x87, 0x87}, // #5f8787, PaleTurquoise4
    [67]  = {0xff, 0x5f, 0x87, 0xaf}, // #5f87af, SteelBlue
    [68]  = {0xff, 0x5f, 0x87, 0xd7}, // #5f87d7, SteelBlue3
    [69]  = {0xff, 0x5f, 0x87, 0xff}, // #5f87ff, CornflowerBlue
    [70]  = {0xff, 0x5f, 0xaf, 0x00}, // #5faf00, Chartreuse3
    [71]  = {0xff, 0x5f, 0xaf, 0x5f}, // #5faf5f, DarkSeaGreen4
    [72]  = {0xff, 0x5f, 0xaf, 0x87}, // #5faf87, CadetBlue
    [73]  = {0xff, 0x5f, 0xaf, 0xaf}, // #5fafaf, CadetBlue
    [74]  = {0xff, 0x5f, 0xaf, 0xd7}, // #5fafd7, SkyBlue3
    [75]  = {0xff, 0x5f, 0xaf, 0xff}, // #5fafff, SteelBlue1
    [76]  = {0xff, 0x5f, 0xd7, 0x00}, // #5fd700, Chartreuse3
    [77]  = {0xff, 0x5f, 0xd7, 0x5f}, // #5fd75f, PaleGreen3
    [78]  = {0xff, 0x5f, 0xd7, 0x87}, // #5fd787, SeaGreen3
    [79]  = {0xff, 0x5f, 0xd7, 0xaf}, // #5fd7af, Aquamarine3
    [80]  = {0xff, 0x5f, 0xd7, 0xd7}, // #5fd7d7, MediumTurquoise
    [81]  = {0xff, 0x5f, 0xd7, 0xff}, // #5fd7ff, SteelBlue1
    [82]  = {0xff, 0x5f, 0xff, 0x00}, // #5fff00, Chartreuse2
    [83]  = {0xff, 0x5f, 0xff, 0x5f}, // #5fff5f, SeaGreen2
    [84]  = {0xff, 0x5f, 0xff, 0x87}, // #5fff87, SeaGreen1
    [85]  = {0xff, 0x5f, 0xff, 0xaf}, // #5fffaf, SeaGreen1
    [86]  = {0xff, 0x5f, 0xff, 0xd7}, // #5fffd7, Aquamarine1
    [87]  = {0xff, 0x5f, 0xff, 0xff}, // #5fffff, DarkSlateGray2
    [88]  = {0xff, 0x87, 0x00, 0x00}, // #870000, DarkRed
    [89]  = {0xff, 0x87, 0x00, 0x5f}, // #87005f, DeepPink4
    [90]  = {0xff, 0x87, 0x00, 0x87}, // #870087, DarkMagenta
    [91]  = {0xff, 0x87, 0x00, 0xaf}, // #8700af, DarkMagenta
    [92]  = {0xff, 0x87, 0x00, 0xd7}, // #8700d7, DarkViolet
    [93]  = {0xff, 0x87, 0x00, 0xff}, // #8700ff, Purple
    [94]  = {0xff, 0x87, 0x5f, 0x00}, // #875f00, Orange4
    [95]  = {0xff, 0x87, 0x5f, 0x5f}, // #875f5f, LightPink4
    [96]  = {0xff, 0x87, 0x5f, 0x87}, // #875f87, Plum4
    [97]  = {0xff, 0x87, 0x5f, 0xaf}, // #875faf, MediumPurple3
    [98]  = {0xff, 0x87, 0x5f, 0xd7}, // #875fd7, MediumPurple3
    [99]  = {0xff, 0x87, 0x5f, 0xff}, // #875fff, SlateBlue1
    [100] = {0xff, 0x87, 0x87, 0x00}, // #878700, Yellow4
    [101] = {0xff, 0x87, 0x87, 0x5f}, // #87875f, Wheat4
    [102] = {0xff, 0x87, 0x87, 0x87}, // #878787, Grey53
    [103] = {0xff, 0x87, 0x87, 0xaf}, // #8787af, LightSlateGrey
    [104] = {0xff, 0x87, 0x87, 0xd7}, // #8787d7, MediumPurple
    [105] = {0xff, 0x87, 0x87, 0xff}, // #8787ff, LightSlateBlue
    [106] = {0xff, 0x87, 0xaf, 0x00}, // #87af00, Yellow4
    [107] = {0xff, 0x87, 0xaf, 0x5f}, // #87af5f, DarkOliveGreen3
    [108] = {0xff, 0x87, 0xaf, 0x87}, // #87af87, DarkSeaGreen
    [109] = {0xff, 0x87, 0xaf, 0xaf}, // #87afaf, LightSkyBlue3
    [110] = {0xff, 0x87, 0xaf, 0xd7}, // #87afd7, LightSkyBlue3
    [111] = {0xff, 0x87, 0xaf, 0xff}, // #87afff, SkyBlue2
    [112] = {0xff, 0x87, 0xd7, 0x00}, // #87d700, Chartreuse2
    [113] = {0xff, 0x87, 0xd7, 0x5f}, // #87d75f, DarkOliveGreen3
    [114] = {0xff, 0x87, 0xd7, 0x87}, // #87d787, PaleGreen3
    [115] = {0xff, 0x87, 0xd7, 0xaf}, // #87d7af, DarkSeaGreen3
    [116] = {0xff, 0x87, 0xd7, 0xd7}, // #87d7d7, DarkSlateGray3
    [117] = {0xff, 0x87, 0xd7, 0xff}, // #87d7ff, SkyBlue1
    [118] = {0xff, 0x87, 0xff, 0x00}, // #87ff00, Chartreuse1
    [119] = {0xff, 0x87, 0xff, 0x5f}, // #87ff5f, LightGreen
    [120] = {0xff, 0x87, 0xff, 0x87}, // #87ff87, LightGreen
    [121] = {0xff, 0x87, 0xff, 0xaf}, // #87ffaf, PaleGreen1
    [122] = {0xff, 0x87, 0xff, 0xd7}, // #87ffd7, Aquamarine1
    [123] = {0xff, 0x87, 0xff, 0xff}, // #87ffff, DarkSlateGray1
    [124] = {0xff, 0xaf, 0x00, 0x00}, // #af0000, Red3
    [125] = {0xff, 0xaf, 0x00, 0x5f}, // #af005f, DeepPink4
    [126] = {0xff, 0xaf, 0x00, 0x87}, // #af0087, MediumVioletRed
    [127] = {0xff, 0xaf, 0x00, 0xaf}, // #af00af, Magenta3
    [128] = {0xff, 0xaf, 0x00, 0xd7}, // #af00d7, DarkViolet
    [129] = {0xff, 0xaf, 0x00, 0xff}, // #af00ff, Purple
    [130] = {0xff, 0xaf, 0x5f, 0x00}, // #af5f00, DarkOrange3
    [131] = {0xff, 0xaf, 0x5f, 0x5f}, // #af5f5f, IndianRed
    [132] = {0xff, 0xaf, 0x5f, 0x87}, // #af5f87, HotPink3
    [133] = {0xff, 0xaf, 0x5f, 0xaf}, // #af5faf, MediumOrchid3
    [134] = {0xff, 0xaf, 0x5f, 0xd7}, // #af5fd7, MediumOrchid
    [135] = {0xff, 0xaf, 0x5f, 0xff}, // #af5fff, MediumPurple2
    [136] = {0xff, 0xaf, 0x87, 0x00}, // #af8700, DarkGoldenrod
    [137] = {0xff, 0xaf, 0x87, 0x5f}, // #af875f, LightSalmon3
    [138] = {0xff, 0xaf, 0x87, 0x87}, // #af8787, RosyBrown
    [139] = {0xff, 0xaf, 0x87, 0xaf}, // #af87af, Grey63
    [140] = {0xff, 0xaf, 0x87, 0xd7}, // #af87d7, MediumPurple2
    [141] = {0xff, 0xaf, 0x87, 0xff}, // #af87ff, MediumPurple1
    [142] = {0xff, 0xaf, 0xaf, 0x00}, // #afaf00, Gold3
    [143] = {0xff, 0xaf, 0xaf, 0x5f}, // #afaf5f, DarkKhaki
    [144] = {0xff, 0xaf, 0xaf, 0x87}, // #afaf87, NavajoWhite3
    [145] = {0xff, 0xaf, 0xaf, 0xaf}, // #afafaf, Grey69
    [146] = {0xff, 0xaf, 0xaf, 0xd7}, // #afafd7, LightSteelBlue3
    [147] = {0xff, 0xaf, 0xaf, 0xff}, // #afafff, LightSteelBlue
    [148] = {0xff, 0xaf, 0xd7, 0x00}, // #afd700, Yellow3
    [149] = {0xff, 0xaf, 0xd7, 0x5f}, // #afd75f, DarkOliveGreen3
    [150] = {0xff, 0xaf, 0xd7, 0x87}, // #afd787, DarkSeaGreen3
    [151] = {0xff, 0xaf, 0xd7, 0xaf}, // #afd7af, DarkSeaGreen2
    [152] = {0xff, 0xaf, 0xd7, 0xd7}, // #afd7d7, LightCyan3
    [153] = {0xff, 0xaf, 0xd7, 0xff}, // #afd7ff, LightSkyBlue1
    [154] = {0xff, 0xaf, 0xff, 0x00}, // #afff00, GreenYellow
    [155] = {0xff, 0xaf, 0xff, 0x5f}, // #afff5f, DarkOliveGreen2
    [156] = {0xff, 0xaf, 0xff, 0x87}, // #afff87, PaleGreen1
    [157] = {0xff, 0xaf, 0xff, 0xaf}, // #afffaf, DarkSeaGreen2
    [158] = {0xff, 0xaf, 0xff, 0xd7}, // #afffd7, DarkSeaGreen1
    [159] = {0xff, 0xaf, 0xff, 0xff}, // #afffff, PaleTurquoise1
    [160] = {0xff, 0xd7, 0x00, 0x00}, // #d70000, Red3
    [161] = {0xff, 0xd7, 0x00, 0x5f}, // #d7005f, DeepPink3
    [162] = {0xff, 0xd7, 0x00, 0x87}, // #d70087, DeepPink3
    [163] = {0xff, 0xd7, 0x00, 0xaf}, // #d700af, Magenta3
    [164] = {0xff, 0xd7, 0x00, 0xd7}, // #d700d7, Magenta3
    [165] = {0xff, 0xd7, 0x00, 0xff}, // #d700ff, Magenta2
    [166] = {0xff, 0xd7, 0x5f, 0x00}, // #d75f00, DarkOrange3
    [167] = {0xff, 0xd7, 0x5f, 0x5f}, // #d75f5f, IndianRed
    [168] = {0xff, 0xd7, 0x5f, 0x87}, // #d75f87, HotPink3
    [169] = {0xff, 0xd7, 0x5f, 0xaf}, // #d75faf, HotPink2
    [170] = {0xff, 0xd7, 0x5f, 0xd7}, // #d75fd7, Orchid
    [171] = {0xff, 0xd7, 0x5f, 0xff}, // #d75fff, MediumOrchid1
    [172] = {0xff, 0xd7, 0x87, 0x00}, // #d78700, Orange3
    [173] = {0xff, 0xd7, 0x87, 0x5f}, // #d7875f, LightSalmon3
    [174] = {0xff, 0xd7, 0x87, 0x87}, // #d78787, LightPink3
    [175] = {0xff, 0xd7, 0x87, 0xaf}, // #d787af, Pink3
    [176] = {0xff, 0xd7, 0x87, 0xd7}, // #d787d7, Plum3
    [177] = {0xff, 0xd7, 0x87, 0xff}, // #d787ff, Violet
    [178] = {0xff, 0xd7, 0xaf, 0x00}, // #d7af00, Gold3
    [179] = {0xff, 0xd7, 0xaf, 0x5f}, // #d7af5f, LightGoldenrod3
    [180] = {0xff, 0xd7, 0xaf, 0x87}, // #d7af87, Tan
    [181] = {0xff, 0xd7, 0xaf, 0xaf}, // #d7afaf, MistyRose3
    [182] = {0xff, 0xd7, 0xaf, 0xd7}, // #d7afd7, Thistle3
    [183] = {0xff, 0xd7, 0xaf, 0xff}, // #d7afff, Plum2
    [184] = {0xff, 0xd7, 0xd7, 0x00}, // #d7d700, Yellow3
    [185] = {0xff, 0xd7, 0xd7, 0x5f}, // #d7d75f, Khaki3
    [186] = {0xff, 0xd7, 0xd7, 0x87}, // #d7d787, LightGoldenrod2
    [187] = {0xff, 0xd7, 0xd7, 0xaf}, // #d7d7af, LightYellow3
    [188] = {0xff, 0xd7, 0xd7, 0xd7}, // #d7d7d7, Grey84
    [189] = {0xff, 0xd7, 0xd7, 0xff}, // #d7d7ff, LightSteelBlue1
    [190] = {0xff, 0xd7, 0xff, 0x00}, // #d7ff00, Yellow2
    [191] = {0xff, 0xd7, 0xff, 0x5f}, // #d7ff5f, DarkOliveGreen1
    [192] = {0xff, 0xd7, 0xff, 0x87}, // #d7ff87, DarkOliveGreen1
    [193] = {0xff, 0xd7, 0xff, 0xaf}, // #d7ffaf, DarkSeaGreen1
    [194] = {0xff, 0xd7, 0xff, 0xd7}, // #d7ffd7, Honeydew2
    [195] = {0xff, 0xd7, 0xff, 0xff}, // #d7ffff, LightCyan1
    [196] = {0xff, 0xff, 0x00, 0x00}, // #ff0000, Red1
    [197] = {0xff, 0xff, 0x00, 0x5f}, // #ff005f, DeepPink2
    [198] = {0xff, 0xff, 0x00, 0x87}, // #ff0087, DeepPink1
    [199] = {0xff, 0xff, 0x00, 0xaf}, // #ff00af, DeepPink1
    [200] = {0xff, 0xff, 0x00, 0xd7}, // #ff00d7, Magenta2
    [201] = {0xff, 0xff, 0x00, 0xff}, // #ff00ff, Magenta1
    [202] = {0xff, 0xff, 0x5f, 0x00}, // #ff5f00, OrangeRed1
    [203] = {0xff, 0xff, 0x5f, 0x5f}, // #ff5f5f, IndianRed1
    [204] = {0xff, 0xff, 0x5f, 0x87}, // #ff5f87, IndianRed1
    [205] = {0xff, 0xff, 0x5f, 0xaf}, // #ff5faf, HotPink
    [206] = {0xff, 0xff, 0x5f, 0xd7}, // #ff5fd7, HotPink
    [207] = {0xff, 0xff, 0x5f, 0xff}, // #ff5fff, MediumOrchid1
    [208] = {0xff, 0xff, 0x87, 0x00}, // #ff8700, DarkOrange
    [209] = {0xff, 0xff, 0x87, 0x5f}, // #ff875f, Salmon1
    [210] = {0xff, 0xff, 0x87, 0x87}, // #ff8787, LightCoral
    [211] = {0xff, 0xff, 0x87, 0xaf}, // #ff87af, PaleVioletRed1
    [212] = {0xff, 0xff, 0x87, 0xd7}, // #ff87d7, Orchid2
    [213] = {0xff, 0xff, 0x87, 0xff}, // #ff87ff, Orchid1
    [214] = {0xff, 0xff, 0xaf, 0x00}, // #ffaf00, Orange1
    [215] = {0xff, 0xff, 0xaf, 0x5f}, // #ffaf5f, SandyBrown
    [216] = {0xff, 0xff, 0xaf, 0x87}, // #ffaf87, LightSalmon1
    [217] = {0xff, 0xff, 0xaf, 0xaf}, // #ffafaf, LightPink1
    [218] = {0xff, 0xff, 0xaf, 0xd7}, // #ffafd7, Pink1
    [219] = {0xff, 0xff, 0xaf, 0xff}, // #ffafff, Plum1
    [220] = {0xff, 0xff, 0xd7, 0x00}, // #ffd700, Gold1
    [221] = {0xff, 0xff, 0xd7, 0x5f}, // #ffd75f, LightGoldenrod2
    [222] = {0xff, 0xff, 0xd7, 0x87}, // #ffd787, LightGoldenrod2
    [223] = {0xff, 0xff, 0xd7, 0xaf}, // #ffd7af, NavajoWhite1
    [224] = {0xff, 0xff, 0xd7, 0xd7}, // #ffd7d7, MistyRose1
    [225] = {0xff, 0xff, 0xd7, 0xff}, // #ffd7ff, Thistle1
    [226] = {0xff, 0xff, 0xff, 0x00}, // #ffff00, Yellow1
    [227] = {0xff, 0xff, 0xff, 0x5f}, // #ffff5f, LightGoldenrod1
    [228] = {0xff, 0xff, 0xff, 0x87}, // #ffff87, Khaki1
    [229] = {0xff, 0xff, 0xff, 0xaf}, // #ffffaf, Wheat1
    [230] = {0xff, 0xff, 0xff, 0xd7}, // #ffffd7, Cornsilk1
    [231] = {0xff, 0xff, 0xff, 0xff}, // #ffffff, Grey100
    [232] = {0xff, 0x80, 0x80, 0x80}, // #080808, Grey3
    [233] = {0xff, 0x12, 0x12, 0x12}, // #121212, Grey7
    [234] = {0xff, 0x1c, 0x1c, 0x1c}, // #1c1c1c, Grey11
    [235] = {0xff, 0x26, 0x26, 0x26}, // #262626, Grey15
    [236] = {0xff, 0x30, 0x30, 0x30}, // #303030, Grey19
    [237] = {0xff, 0x3a, 0x3a, 0x3a}, // #3a3a3a, Grey23
    [238] = {0xff, 0x44, 0x44, 0x44}, // #444444, Grey27
    [239] = {0xff, 0x4e, 0x4e, 0x4e}, // #4e4e4e, Grey30
    [240] = {0xff, 0x58, 0x58, 0x58}, // #585858, Grey35
    [241] = {0xff, 0x62, 0x62, 0x62}, // #626262, Grey39
    [242] = {0xff, 0x6c, 0x6c, 0x6c}, // #6c6c6c, Grey42
    [243] = {0xff, 0x76, 0x76, 0x76}, // #767676, Grey46
    [244] = {0xff, 0x80, 0x80, 0x80}, // #808080, Grey50
    [245] = {0xff, 0x8a, 0x8a, 0x8a}, // #8a8a8a, Grey54
    [246] = {0xff, 0x94, 0x94, 0x94}, // #949494, Grey58
    [247] = {0xff, 0x9e, 0x9e, 0x9e}, // #9e9e9e, Grey62
    [248] = {0xff, 0xa8, 0xa8, 0xa8}, // #a8a8a8, Grey66
    [249] = {0xff, 0xb2, 0xb2, 0xb2}, // #b2b2b2, Grey70
    [250] = {0xff, 0xbc, 0xbc, 0xbc}, // #bcbcbc, Grey74
    [251] = {0xff, 0xc6, 0xc6, 0xc6}, // #c6c6c6, Grey78
    [252] = {0xff, 0xd0, 0xd0, 0xd0}, // #d0d0d0, Grey82
    [253] = {0xff, 0xda, 0xda, 0xda}, // #dadada, Grey85
    [254] = {0xff, 0xe4, 0xe4, 0xe4}, // #e4e4e4, Grey89
    [255] = {0xff, 0xee, 0xee, 0xee}, // #eeeeee, Grey93
}};

/*
 * | group id | handle range |
 * |----------|--------------|
 * |     0    | [0, 128)     |
 * |     1    | [128, 256)   |
 * |     2    | [256, 384)   |
 * |     3    | [384, 512)   |
 * |     4    | [512, 640)   |
 * |     5    | [640, 768)   |
 * |     6    | [768, 896)   |
 * |     7    | [896, 1024)  |
 */
#define RGN_HANDLE_GROUP_NUM            (8)
#define RGN_HANDLE_GROUP_MAX_HANDLE_NUM (MI_RGN_MAX_HANDLE / RGN_HANDLE_GROUP_NUM)
std::vector<std::vector<MI_RGN_HANDLE>> AmigosModuleRgn::RgnAdapter::handle_pool_table(
    RGN_HANDLE_GROUP_NUM, std::vector<MI_RGN_HANDLE>(RGN_HANDLE_GROUP_MAX_HANDLE_NUM, MI_RGN_HANDLE_NULL));
pthread_mutex_t AmigosModuleRgn::RgnAdapter::handle_pool_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t AmigosModuleRgn::RgnAdapter::canvas_mutex      = PTHREAD_MUTEX_INITIALIZER;

static MI_RGN_PixelFormat_e _ConvertRawDataTypetoRgnPixelFmt(const raw_video_fmt fmt)
{
    switch (fmt)
    {
        case RAW_FORMAT_ARGB8888:
            return E_MI_RGN_PIXEL_FORMAT_ARGB8888;
        case RAW_FORMAT_RGB565:
            return E_MI_RGN_PIXEL_FORMAT_RGB565;
        case RAW_FORMAT_ARGB1555:
            return E_MI_RGN_PIXEL_FORMAT_ARGB1555;
        case RAW_FORMAT_ARGB4444:
            return E_MI_RGN_PIXEL_FORMAT_ARGB4444;
        case RAW_FORMAT_I2:
            return E_MI_RGN_PIXEL_FORMAT_I2;
        case RAW_FORMAT_I4:
            return E_MI_RGN_PIXEL_FORMAT_I4;
        case RAW_FORMAT_I8:
            return E_MI_RGN_PIXEL_FORMAT_I8;
        default:
            break;
    }
    return E_MI_RGN_PIXEL_FORMAT_MAX;
}
static float _CalCoordScale(const unsigned int &x)
{
    return (float)x / AMIGOS_RGN_METADATA_COORDINATE_MAX_W;
}
static unsigned int _CalTinckness(unsigned int width, unsigned int height, AmigosModuleRgn::ThicknessLevel lv)
{
    static const unsigned int map_lv_div[] = {[AmigosModuleRgn::ThicknessLevel::E_THICKNESS_LEVEL_THIN]   = 1024,
                                              [AmigosModuleRgn::ThicknessLevel::E_THICKNESS_LEVEL_NORMAL] = 512,
                                              [AmigosModuleRgn::ThicknessLevel::E_THICKNESS_LEVEL_THICK]  = 256};
    unsigned int size = std::max(width, height);
    if (size < 2048)
    {
        size = 2048;
    }
    unsigned int ret = size / map_lv_div[lv];
    return ret == 0 ? 2 : ALIGN_UP(ret, 2);
}
static unsigned int _CalFontSize(unsigned int width, unsigned int height, AmigosModuleRgn::SizeLevel lv)
{
    static const unsigned int map_lv_div[] = {
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_TINY] = 256,  [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_SMALL] = 128,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_NORMAL] = 64, [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_LARGE] = 32,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_HUGE] = 16,
    };
    unsigned int ret = std::max(width, height) / map_lv_div[lv];
    return ret < 8 ? 8 : ret;
}
static unsigned int _CalDotMatrixSize(unsigned int width, unsigned int height, unsigned int mapWidth,
                                      unsigned int mapHeight, enum AmigosModuleRgn::SizeLevel lv)
{
    static unsigned int map_lv_div[] = {
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_TINY] = 16,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_SMALL] = 8,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_NORMAL] = 4,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_LARGE] = 2,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_HUGE] = 1,
    };
    unsigned int size = 0;

    size = width > height ? width / mapWidth : height / mapHeight;
    size = size / map_lv_div[lv];
    return size == 0 ? 2 : size;
}
static MI_RGN_BlockSize_e _CalMosaicBlockSize(AmigosModuleRgn::SizeLevel lv)
{
    static const MI_RGN_BlockSize_e map_lv_block_size[] = {
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_TINY]   = E_MI_RGN_BLOCK_SIZE_4,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_SMALL]  = E_MI_RGN_BLOCK_SIZE_8,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_NORMAL] = E_MI_RGN_BLOCK_SIZE_16,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_LARGE]  = E_MI_RGN_BLOCK_SIZE_32,
        [AmigosModuleRgn::SizeLevel::E_SIZE_LEVEL_HUGE]   = E_MI_RGN_BLOCK_SIZE_64,
    };
    return map_lv_block_size[lv];
}
static MI_RGN_BlockSize_e _CalColorInvertBlockSize(unsigned int width, unsigned int height)
{
    struct
    {
        MI_RGN_BlockSize_e block_size;
        unsigned int       size_pixel;
    } block_size_lst[] = {
        { E_MI_RGN_BLOCK_SIZE_32, 32 },
        { E_MI_RGN_BLOCK_SIZE_64, 64 },
        { E_MI_RGN_BLOCK_SIZE_128, 128 },
        { E_MI_RGN_BLOCK_SIZE_256, 256 },
    };
    unsigned int n = sizeof(block_size_lst) / sizeof(block_size_lst[0]);
    unsigned int val = std::max(width, height) / 64;
    if (val < block_size_lst[0].size_pixel)
    {
        return block_size_lst[0].block_size;
    }
    for (unsigned int i = 1; i < n; ++i)
    {
        if (val < block_size_lst[i].size_pixel)
        {
            if (block_size_lst[i].size_pixel - val > val - block_size_lst[i - 1].size_pixel)
            {
                return block_size_lst[i - 1].block_size;
            }
            else
            {
                return block_size_lst[i].block_size;
            }
        }
    }
    return block_size_lst[n - 1].block_size;
}
static unsigned int _ConvertColorToArgb1555(unsigned int c, void *)
{
    return (((c & 0xff000000) >> 16) & 0x8000) | (((c & 0x00ff0000) >> 9) & 0x7c00)
           | (((c & 0x0000ff00) >> 6) & 0x03e0) | (((c & 0x000000ff) >> 3) & 0x001f);
}
static unsigned int _ConvertColorToArgb4444(unsigned int c, void *)
{
    return (((c & 0xff000000) >> 16) & 0xf000) | (((c & 0x00ff0000) >> 12) & 0x0f00)
           | (((c & 0x0000ff00) >> 8) & 0x00f0) | (((c & 0x000000ff) >> 4) & 0x000f);
}
static unsigned int _ConvertColorToRgb565(unsigned int c, void *)
{
    return (((c & 0x00ff0000) >> 8) & 0xf800) | (((c & 0x0000ff00) >> 5) & 0x07e0) | (((c & 0x000000ff) >> 3) & 0x001f);
}
static unsigned int _ConvertColorToI2(unsigned int c, void *)
{
    unsigned int min_diff = 0xffffffff;
    unsigned int curr_diff = 0;
    unsigned char r = (c & 0xff0000) >> 16;
    unsigned char g = (c & 0x00ff00) >> 8;
    unsigned char b = (c & 0x0000ff);
    unsigned int index = 0;
    for (unsigned int i = 0; i < 4; ++i)
    {
        curr_diff = std::abs(int(g_palette_table.astElement[i].u8Red - r))
                    + std::abs(int(g_palette_table.astElement[i].u8Green - g))
                    + std::abs(int(g_palette_table.astElement[i].u8Blue - b));
        if (curr_diff < min_diff) {
            min_diff = curr_diff;
            index = i;
        }
    }
    return index;
}
static unsigned int _ConvertColorToI4(unsigned int c, void *)
{
    unsigned int min_diff = 0xffffffff;
    unsigned int curr_diff = 0;
    unsigned char r = (c & 0xff0000) >> 16;
    unsigned char g = (c & 0x00ff00) >> 8;
    unsigned char b = (c & 0x0000ff);
    unsigned int index = 0;
    for (unsigned int i = 0; i < 16; ++i)
    {
        curr_diff = std::abs(int(g_palette_table.astElement[i].u8Red - r))
                    + std::abs(int(g_palette_table.astElement[i].u8Green - g))
                    + std::abs(int(g_palette_table.astElement[i].u8Blue - b));
        if (curr_diff < min_diff) {
            min_diff = curr_diff;
            index = i;
        }
    }
    return index;
}
static unsigned int _ConvertColorToI8(unsigned int c, void *)
{
    unsigned int min_diff = 0xffffffff;
    unsigned int curr_diff = 0;
    unsigned char r = (c & 0xff0000) >> 16;
    unsigned char g = (c & 0x00ff00) >> 8;
    unsigned char b = (c & 0x0000ff);
    unsigned int index = 0;
    for (unsigned int i = 0; i < 256; ++i)
    {
        curr_diff = std::abs(int(g_palette_table.astElement[i].u8Red - r))
                    + std::abs(int(g_palette_table.astElement[i].u8Green - g))
                    + std::abs(int(g_palette_table.astElement[i].u8Blue - b));
        if (curr_diff < min_diff) {
            min_diff = curr_diff;
            index = i;
        }
    }
    return index;
}
static unsigned int _ConvertColorToUyv(unsigned int c, void *)
{
    unsigned char r = (c & 0xff0000) >> 16;
    unsigned char g = (c & 0x00ff00) >> 8;
    unsigned char b = (c & 0x0000ff);
    unsigned char y = uint8_t(0.299 * r + 0.587 * g + 0.114 * b);
    unsigned char u = uint8_t(0.5 * r - 0.4187 * g - 0.0813 * b + 128);
    unsigned char v = uint8_t(-0.1687 * r - 0.3313 * g + 0.5 * b + 128);
    return (u << 16) | (y << 8) | v;
}

static unsigned int _CalColorInvert(unsigned int c)
{
    return (c & 0xff000000) | (~(c & 0xffffff));
}

AmigosModuleRgn::RgnAdapter::RgnAdapter(unsigned int handleGroupId) : handleGroupId(handleGroupId)
{
    ss_auto_lock lock(AmigosModuleRgn::RgnAdapter::handle_pool_mutex);
    auto &handle_pool = RgnAdapter::handle_pool_table[handleGroupId % RGN_HANDLE_GROUP_NUM];
    if (static_cast<MI_RGN_HANDLE>(MI_RGN_HANDLE_NULL) == handle_pool.back())
    {
        this->handle = handleGroupId % RGN_HANDLE_GROUP_NUM * RGN_HANDLE_GROUP_MAX_HANDLE_NUM
                       + RGN_HANDLE_GROUP_MAX_HANDLE_NUM - handle_pool.size();
    }
    else
    {
        this->handle = handle_pool.back();
    }
    handle_pool.pop_back();
}
AmigosModuleRgn::RgnAdapter::~RgnAdapter()
{
    ss_auto_lock lock(AmigosModuleRgn::RgnAdapter::handle_pool_mutex);
    auto &handle_pool = RgnAdapter::handle_pool_table[handleGroupId % RGN_HANDLE_GROUP_NUM];
    handle_pool.push_back(this->handle);
}
bool AmigosModuleRgn::RgnAdapter::Create(MI_RGN_Attr_t &attr)
{
    CHECK_RESULT(MI_SUCCESS, return false, MI_RGN_Create, 0, this->handle, &attr);
    return true;
}
bool AmigosModuleRgn::RgnAdapter::Destroy()
{
    CHECK_RESULT(MI_SUCCESS, return false, MI_RGN_Destroy, 0, this->handle);
    return true;
}
bool AmigosModuleRgn::RgnAdapter::SetBitmap(MI_RGN_Bitmap_t &bitmap)
{
    CHECK_RESULT_NO_LOG(MI_SUCCESS, return false, MI_RGN_SetBitMap, 0, this->handle, &bitmap);
    return true;
}
bool AmigosModuleRgn::RgnAdapter::GetCanvas(MI_RGN_CanvasInfo_t &canvas)
{
    pthread_mutex_lock(&RgnAdapter::canvas_mutex);
    CHECK_RESULT_NO_LOG(MI_SUCCESS, pthread_mutex_unlock(&RgnAdapter::canvas_mutex);
                        return false, MI_RGN_GetCanvasInfo, 0, this->handle, &canvas);
    return true;
}
bool AmigosModuleRgn::RgnAdapter::UpdateCanvas()
{
    CHECK_RESULT_NO_LOG(MI_SUCCESS, return false, MI_RGN_UpdateCanvas, 0, this->handle);
    pthread_mutex_unlock(&RgnAdapter::canvas_mutex);
    return true;
}
bool AmigosModuleRgn::RgnAdapter::Attach(const RgnTargetInfo &target, MI_RGN_ChnPortParam_t &param)
{
    MI_RGN_ChnPort_t chnport = target.stChnPort;
    auto it = this->map_attach.find(target);
    if (it != this->map_attach.end() && it->second)
    {
        CHECK_RESULT_NO_LOG(MI_SUCCESS, return false, MI_RGN_SetDisplayAttr, 0, this->handle, &chnport, &param);
    }
    else
    {
        CHECK_RESULT_NO_LOG(MI_SUCCESS, return false, MI_RGN_AttachToChn, 0, this->handle, &chnport, &param);
        this->map_attach[target] = true;
    }
    return true;
}
bool AmigosModuleRgn::RgnAdapter::Detach(const RgnTargetInfo &target)
{
    MI_RGN_ChnPort_t chnport = target.stChnPort;
    auto it = this->map_attach.find(target);
    if (it != this->map_attach.end() && it->second)
    {
        CHECK_RESULT_NO_LOG(MI_SUCCESS, return false, MI_RGN_DetachFromChn, 0, this->handle, &chnport);
        it->second = false;
    }
    return true;
}

AmigosModuleRgn::LinkerRgnIn::LinkerRgnIn(RgnAdapter *rgn_adapter)
    : RgnHandleRes(rgn_adapter)
{
}
AmigosModuleRgn::LinkerRgnIn::~LinkerRgnIn()
{
}
int AmigosModuleRgn::LinkerRgnIn::enqueue(stream_packet_obj &packet)
{
    if (packet->get_type() == "rgn")
    {
        return this->rgn_adapter->UpdateCanvas() ? 0 : -1;
    }
    if (packet->en_type != EN_RAW_FRAME_DATA)
    {
        return -1;
    }
    if (packet->raw_vid_i.plane_num < 1)
    {
        return -1;
    }
    if (!this->CreateResource(packet->raw_vid_i))
    {
        return -1;
    }
    MI_RGN_Bitmap_s bitmap;
    memset(&bitmap, 0, sizeof(MI_RGN_Bitmap_t));
    bitmap.ePixelFormat     = _ConvertRawDataTypetoRgnPixelFmt(packet->raw_vid_i.plane_info[0].fmt);
    bitmap.stSize.u32Width  = packet->raw_vid_i.plane_info[0].width;
    bitmap.stSize.u32Height = packet->raw_vid_i.plane_info[0].height;
    bitmap.pData            = packet->raw_vid.plane_data[0].data[0];
    return this->rgn_adapter->SetBitmap(bitmap) ? 0 : -1;
}
stream_packet_obj AmigosModuleRgn::LinkerRgnIn::dequeue(unsigned int ms)
{
    return nullptr;
}
AmigosModuleRgn::AmigosMiRgnStreamPacket::AmigosMiRgnStreamPacket(const stream_packet_info  &packet_info,
                                                                  const MI_RGN_CanvasInfo_t &canvas)
    : stream_packet_base()
{
    this->en_type = EN_RAW_FRAME_DATA;
    this->raw_vid_i = packet_info.raw_vid_i;
    this->raw_vid.plane_data[0].data[0]   = (char *)canvas.virtAddr;
    this->raw_vid.plane_data[0].stride[0] = canvas.u32Stride;
    stream_packet_info::raw_data_size(this->raw_vid_i.plane_info[0], this->raw_vid.plane_data[0].stride,
                        this->raw_vid.plane_data[0].size);
    this->type_name = "rgn";
}
AmigosModuleRgn::AmigosMiRgnStreamPacket::~AmigosMiRgnStreamPacket()
{
}
AmigosModuleRgn::RgnHandleRes::RgnHandleRes(RgnAdapter *adapter)
    : rgn_adapter(adapter), is_create(false)
{
}
AmigosModuleRgn::RgnHandleRes::~RgnHandleRes()
{
    if (this->is_create)
    {
        this->rgn_adapter->Destroy();
    }
}
bool AmigosModuleRgn::RgnHandleRes::CreateResource(const struct raw_video_info &info)
{
    if (this->is_create)
    {
        return true;
    }
    MI_RGN_Attr_t attr;
    memset(&attr, 0, sizeof(MI_RGN_Attr_t));
    attr.eType                           = E_MI_RGN_TYPE_OSD;
    attr.stOsdInitParam.ePixelFmt        = _ConvertRawDataTypetoRgnPixelFmt(info.plane_info[0].fmt);
    attr.stOsdInitParam.stSize.u32Width  = info.plane_info[0].width;
    attr.stOsdInitParam.stSize.u32Height = info.plane_info[0].height;
    if (!this->rgn_adapter->Create(attr))
    {
        return false;
    }
    this->is_create = true;
    return true;
}
AmigosModuleRgn::StreamPackerRgnIn::StreamPackerRgnIn(RgnAdapter *rgn_adapter)
    : RgnHandleRes(rgn_adapter)
{
}
AmigosModuleRgn::StreamPackerRgnIn::~StreamPackerRgnIn()
{
}
stream_packet_obj AmigosModuleRgn::StreamPackerRgnIn::make(const stream_packet_info &packet_info)
{
    if (packet_info.en_type != EN_RAW_FRAME_DATA)
    {
        return nullptr;
    }
    if (packet_info.raw_vid_i.plane_num < 1)
    {
        return nullptr;
    }
    if (!this->CreateResource(packet_info.raw_vid_i))
    {
        return nullptr;
    }
    MI_RGN_CanvasInfo_t canvas;
    memset(&canvas, 0, sizeof(MI_RGN_CanvasInfo_t));
    if (!this->rgn_adapter->GetCanvas(canvas))
    {
        return nullptr;
    }
    if (_ConvertRawDataTypetoRgnPixelFmt(packet_info.raw_vid_i.plane_info[0].fmt) != canvas.ePixelFmt
        || packet_info.raw_vid_i.plane_info[0].width != canvas.stSize.u32Width
        || packet_info.raw_vid_i.plane_info[0].height != canvas.stSize.u32Height)
    {
        return nullptr;
    }
    return stream_packet_base::make<AmigosMiRgnStreamPacket>(packet_info, canvas);
}
AmigosModuleRgn::RgnControl::RgnControl(unsigned int devId) : devId(devId), is_processed(false)
{
    this->control_mutex = PTHREAD_MUTEX_INITIALIZER;
}
AmigosModuleRgn::RgnControl::~RgnControl() {}
bool AmigosModuleRgn::RgnControl::add_target(const RgnTargetInfo &target)
{
    ss_auto_lock lock(this->control_mutex);
    if (this->is_processed)
    {
        this->_add_target(target);
    }
    return true;
}
bool AmigosModuleRgn::RgnControl::del_target(const RgnTargetInfo &target)
{
    ss_auto_lock lock(this->control_mutex);
    if (this->is_processed)
    {
        this->_del_target(target);
    }
    return true;
}
void AmigosModuleRgn::RgnControl::process(const std::list<RgnTargetInfo>  &target_lst,
                                          stream_packet_obj &packet)
{
    ss_auto_lock lock(this->control_mutex);
    if (this->_check_packet(packet))
    {
        if (!this->is_processed)
        {
            for (auto it = target_lst.begin(); it != target_lst.end(); ++it)
            {
                this->_add_target(*it);
            }
            this->is_processed = true;
        }
        else
        {
            for (auto it = target_lst.begin(); it != target_lst.end(); ++it)
            {
                this->_set_param(*it);
            }
        }
        this->_finish();
    }
}
bool AmigosModuleRgn::RgnControl::set_attr(const std::list<RgnTargetInfo>       &target_lst,
                                           const AmigosSurfaceRgn::RgnInputInfo &info)
{
    int ret = this->_set_attr(info);
    if (-1 == ret)
    {
        return false;
    }
    if (0 == ret)
    {
        return true;
    }
    if (this->is_processed)
    {
        for (auto it = target_lst.begin(); it != target_lst.end(); ++it)
        {
            this->_set_param(*it);
        }
        this->_finish();
    }
    return true;
}

AmigosModuleRgn::CanvasControl::CanvasControl(unsigned int devId, const AmigosSurfaceRgn::CanvasInfo &canvas_info, RgnAdapter *rgn_adapter)
    : RgnControl(devId), rgn_adapter(rgn_adapter)
{
    this->osd_show                            = (MI_BOOL)canvas_info.intShow;
    this->osd_layer                           = RGN_OSD_CANVAS_LAYER;
    this->osd_param.stPoint.u32X              = canvas_info.intPosX;
    this->osd_param.stPoint.u32Y              = canvas_info.intPosY;
    this->osd_param.u8PaletteIdx              = canvas_info.intPaletteIdx;
    this->osd_param.stOsdAlphaAttr.eAlphaMode = ss_enum_cast<MI_RGN_AlphaMode_e>::from_str(canvas_info.strAlphaType);
    if (this->osd_param.stOsdAlphaAttr.eAlphaMode == E_MI_RGN_PIXEL_ALPHA)
    {
        this->osd_param.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = canvas_info.intAlpha0;
        this->osd_param.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = canvas_info.intAlpha1;
    }
    else
    {
        this->osd_param.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = canvas_info.intAlphaVal;
    }
}
AmigosModuleRgn::CanvasControl::~CanvasControl()
{

}
void AmigosModuleRgn::CanvasControl::_add_target(const RgnTargetInfo &target)
{
    MI_RGN_ChnPortParam_t param;
    memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
    param.bShow        = this->osd_show;
    param.u32Layer     = this->osd_layer;
    param.stOsdChnPort = this->osd_param;
    this->rgn_adapter->Attach(target, param);
}
void AmigosModuleRgn::CanvasControl::_del_target(const RgnTargetInfo &target)
{
    this->rgn_adapter->Detach(target);
}
void AmigosModuleRgn::CanvasControl::_set_param(const RgnTargetInfo &target)
{
    // nothing to do
}
bool AmigosModuleRgn::CanvasControl::_check_packet(stream_packet_obj &packet)
{
    return true;
}

AmigosModuleRgn::LineControl::LineControl(unsigned int devId, const AmigosSurfaceRgn::LineInfo &line_info)
    : RgnControl(devId)
{
    this->pixel_fmt       = ss_enum_cast<MI_RGN_PixelFormat_e>::from_str(line_info.strPixelFmt);
    this->color_on        = line_info.intColor;
    this->color_off       = _CalColorInvert(line_info.intColor);
    this->thickness_level_last = this->thickness_level = ss_enum_cast<ThicknessLevel>::from_str(line_info.strThickness);
    this->curr = nullptr;
    this->last = nullptr;
}
AmigosModuleRgn::LineControl::~LineControl()
{
    this->curr = nullptr;
    this->last = nullptr;
}
void AmigosModuleRgn::LineControl::_refresh_lines(RgnAdapter *rgn_adapter, bool is_init)
{
    MI_RGN_CanvasInfo_t canvas;
    memset(&canvas, 0, sizeof(MI_RGN_CanvasInfo_t));
    if (!rgn_adapter->GetCanvas(canvas))
    {
        return;
    }

    struct SS_Graph::Canvas cvs;
    memset(&cvs, 0, sizeof(SS_Graph::Canvas));
    cvs.data[0]   = (char *)canvas.virtAddr;
    cvs.stride[0] = canvas.u32Stride;
    cvs.width     = canvas.stSize.u32Width;
    cvs.height    = canvas.stSize.u32Height;

    SS_Graph::Graph g(g_canvas_desc_map[canvas.ePixelFmt], cvs);
    if (!is_init && this->last)
    {
        unsigned int thickness =
            _CalTinckness(canvas.stSize.u32Width, canvas.stSize.u32Height, this->thickness_level_last);
        this->thickness_level_last = this->thickness_level;
        AmigosRgnMetaDataLines &lines = *(AmigosRgnMetaDataLines *)this->last->meta_data.data;
        for (unsigned int i = 0; i < lines.count; ++i)
        {
            g.DrawLine<unsigned int>(_CalCoordScale, g_bg_color[this->pixel_fmt], lines.lines[i].pt0.x,
                                     lines.lines[i].pt0.y, lines.lines[i].pt1.x, lines.lines[i].pt1.y, thickness);
        }
    }
    if (this->curr)
    {
        unsigned int thickness =
            _CalTinckness(canvas.stSize.u32Width, canvas.stSize.u32Height, this->thickness_level);
        AmigosRgnMetaDataLines &lines = *(AmigosRgnMetaDataLines *)this->curr->meta_data.data;
        for (unsigned int i = 0; i < lines.count; ++i)
        {
            unsigned int color = lines.lines[i].state ? this->color_on : this->color_off;
            g.DrawLine<unsigned int>(_CalCoordScale, color, lines.lines[i].pt0.x, lines.lines[i].pt0.y,
                                     lines.lines[i].pt1.x, lines.lines[i].pt1.y, thickness);
        }
    }

    rgn_adapter->UpdateCanvas();
}
void AmigosModuleRgn::LineControl::_add_target(const RgnTargetInfo &target)
{
    MI_RGN_ChnPortParam_t param = {};
    memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
    param.bShow = TRUE;
    param.u32Layer = RGN_OSD_LINE_LAYER;
    param.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
    param.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
    param.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xff;
    auto it = this->map_timing_desc.find(target.timing);
    if (it != this->map_timing_desc.end())
    {
        if (!it->second.rgn_adapter->Attach(target, param))
        {
            return;
        }
        it->second.use_count++;
        return;
    }
    LineDesc desc;
    memset(&desc, 0, sizeof(LineDesc));
    desc.rgn_adapter = new RgnAdapter(this->devId);

    MI_RGN_Attr_t attr;
    memset(&attr, 0, sizeof(MI_RGN_Attr_t));
    attr.eType                           = E_MI_RGN_TYPE_OSD;
    attr.stOsdInitParam.ePixelFmt        = this->pixel_fmt;
    attr.stOsdInitParam.stSize.u32Width  = ALIGN_UP(target.timing.width, g_alignment[this->pixel_fmt]);
    attr.stOsdInitParam.stSize.u32Height = target.timing.height;
    if (!desc.rgn_adapter->Create(attr))
    {
        delete desc.rgn_adapter;
        return;
    }

    if (!desc.rgn_adapter->Attach(target, param))
    {
        desc.rgn_adapter->Destroy();
        delete desc.rgn_adapter;
        return;
    }
    desc.use_count = 1;
    this->_refresh_lines(desc.rgn_adapter, true);
    this->map_timing_desc[target.timing] = desc;
}
void AmigosModuleRgn::LineControl::_del_target(const RgnTargetInfo &target)
{
    auto it = this->map_timing_desc.find(target.timing);
    if (it == this->map_timing_desc.end())
    {
        return;
    }
    it->second.rgn_adapter->Detach(target);
    it->second.use_count--;
    if (0 == it->second.use_count)
    {
        it->second.rgn_adapter->Destroy();
        delete it->second.rgn_adapter;
        this->map_timing_desc.erase(it);
    }
}
void AmigosModuleRgn::LineControl::_set_param(const RgnTargetInfo &target)
{
    auto it = this->map_timing_desc.find(target.timing);
    if (it == this->map_timing_desc.end())
    {
        return;
    }
    if (!it->second.is_refresh)
    {
        this->_refresh_lines(it->second.rgn_adapter, false);
        it->second.is_refresh = true;
    }
}
bool AmigosModuleRgn::LineControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_LINES)
    {
        return false;
    }
    if (this->curr == nullptr)
    {
        this->curr = stream_packet_base::dup(packet);
        return true;
    }
    if (packet->meta_data_i.size == this->curr->meta_data_i.size)
    {
        if (0 == memcmp(packet->meta_data.data, this->curr->meta_data.data, packet->meta_data_i.size))
        {
            return false;
        }
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
void AmigosModuleRgn::LineControl::_finish()
{
    this->last = stream_packet_base::dup(this->curr);
    for (auto it = this->map_timing_desc.begin(); it != this->map_timing_desc.end(); ++it)
    {
        it->second.is_refresh = false;
    }
}
int AmigosModuleRgn::LineControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "line")
    {
        return -1;
    }
    const AmigosSurfaceRgn::LineInfo &line_info = info.stLineInfo;
    int diff_cnt = 0;
    if (this->color_on != line_info.intColor)
    {
        this->color_on  = line_info.intColor;
        this->color_off = _CalColorInvert(line_info.intColor);
        ++diff_cnt;
    }
    AmigosModuleRgn::ThicknessLevel thickness = ss_enum_cast<ThicknessLevel>::from_str(line_info.strThickness);
    if (this->thickness_level != thickness)
    {
        this->thickness_level = thickness;
        ++diff_cnt;
    }
    return diff_cnt;
}

AmigosModuleRgn::TextControl::TextControl(unsigned int devId, const AmigosSurfaceRgn::TextInfo &text_info)
    : RgnControl(devId)
{
    this->pixel_fmt  = ss_enum_cast<MI_RGN_PixelFormat_e>::from_str(text_info.strPixelFmt);
    this->color      = text_info.intColor;
    this->size_level = ss_enum_cast<SizeLevel>::from_str(text_info.strFontSize);
    this->area_w     = text_info.intAreaW;
    this->area_h     = text_info.intAreaH;
    this->default_x  = text_info.intPosX;
    this->default_y  = text_info.intPosY;
    this->curr = nullptr;
    this->last = nullptr;
    this->font = new SS_Font(text_info.strFontFile, this->area_w);
}
AmigosModuleRgn::TextControl::~TextControl()
{
    this->curr = nullptr;
    this->last = nullptr;
    delete this->font;
    this->font = NULL;
}
void AmigosModuleRgn::TextControl::_refresh_text(RgnAdapter *rgn_adapters[], unsigned int size, bool is_init)
{
    if (!this->curr)
    {
        return;
    }
    for (unsigned int i = 0; i < ((AmigosRgnMetaDataTexts *)(this->curr->meta_data.data))->count && i < TEXT_MAX_NUM;
         ++i)
    {
        if (!rgn_adapters[i])
        {
            rgn_adapters[i] = new RgnAdapter(this->devId);

            MI_RGN_Attr_t attr;
            memset(&attr, 0, sizeof(MI_RGN_Attr_t));
            attr.eType                           = E_MI_RGN_TYPE_OSD;
            attr.stOsdInitParam.ePixelFmt        = this->pixel_fmt;
            attr.stOsdInitParam.stSize.u32Width  = ALIGN_UP(size * this->area_w, g_alignment[this->pixel_fmt]);
            attr.stOsdInitParam.stSize.u32Height = size * this->area_h;

            if (!rgn_adapters[i]->Create(attr))
            {
                delete rgn_adapters[i];
                rgn_adapters[i] = NULL;
                return;
            }
        }
        MI_RGN_CanvasInfo_t canvas;
        memset(&canvas, 0, sizeof(MI_RGN_CanvasInfo_t));
        if (!rgn_adapters[i]->GetCanvas(canvas))
        {
            return;
        }

        struct SS_Graph::Canvas cvs;
        memset(&cvs, 0, sizeof(SS_Graph::Canvas));
        cvs.data[0]   = (char *)canvas.virtAddr;
        cvs.stride[0] = canvas.u32Stride;
        cvs.width     = canvas.stSize.u32Width;
        cvs.height    = canvas.stSize.u32Height;
        SS_Graph::Graph g(g_canvas_desc_map[canvas.ePixelFmt], cvs);

        unsigned int x = 0;
        unsigned int y = 0;
        wchar_t *p  = NULL;
        wchar_t *q  = NULL;
        wchar_t end[] = L"\0";
        if (!is_init && this->last && i < ((AmigosRgnMetaDataTexts *)(this->last->meta_data.data))->count)
        {
            p = ((AmigosRgnMetaDataTexts*)(this->curr->meta_data.data))->texts[i].str;
            q = ((AmigosRgnMetaDataTexts*)(this->last->meta_data.data))->texts[i].str;
        }
        else
        {
            p = ((AmigosRgnMetaDataTexts*)(this->curr->meta_data.data))->texts[i].str;
            q = end;
        }

        // 1. if *p && *q , compare them and refresh different text
        while (*p && *q)
        {
            if (y + size > cvs.height)
            {
                break;
            }

            if (*p == L'\n' && *q == L'\n')
            {
                y += size;
                x = 0;
                ++p;
                ++q;
                continue;
            }

            if (*p == L'\n')
            {
                if (x + size > cvs.width)
                {
                    ++q;
                    continue;
                }
                g.FillRect(g_bg_color[canvas.ePixelFmt], x, y, size, size);
                x += size;
                ++q;
                continue;
            }

            if (*q == L'\n')
            {
                if (x + size > cvs.width)
                {
                    ++p;
                    continue;
                }
                unsigned char *bitmap = this->font->GetBitmap(*p, size);
                if (bitmap)
                {
                    g.DrawBitmap(this->color, x, y, (char *)bitmap, size, size);
                }
                x += size;
                ++p;
                continue;
            }

            if (x + size > cvs.width)
            {
                ++p;
                ++q;
                continue;
            }

            if (*p != *q)
            {
                g.FillRect(g_bg_color[canvas.ePixelFmt], x, y, size, size);
                unsigned char *bitmap = this->font->GetBitmap(*p, size);
                if (bitmap)
                {
                    g.DrawBitmap(this->color, x, y, (char *)bitmap, size, size);
                }
            }
            x += size;
            ++p;
            ++q;
        }
        // 2. if *q not end, clear *q text area
        while (*q)
        {
            if (y + size > cvs.height)
            {
                break;
            }

            if (*q == L'\n')
            {
                y += size;
                x = 0;
                ++q;
                continue;
            }

            if (x + size > cvs.width)
            {
                ++q;
                continue;
            }
            g.FillRect(g_bg_color[canvas.ePixelFmt], x, y, size, size);
            x += size;
            ++q;
        }
        // 3. if *p not end, draw text but not need clear
        while (*p)
        {
            if (y + size > cvs.height)
            {
                break;
            }

            if (*p == L'\n')
            {
                y += size;
                x = 0;
                ++p;
                continue;
            }

            if (x + size > cvs.width)
            {
                ++p;
                continue;
            }
            unsigned char *bitmap = this->font->GetBitmap(*p, size);
            if (bitmap)
            {
                g.DrawBitmap(this->color, x, y, (char *)bitmap, size, size);
            }
            x += size;
            ++p;
        }

        if (!rgn_adapters[i]->UpdateCanvas())
        {
            return;
        }
    }
}
void AmigosModuleRgn::TextControl::_move_text(RgnAdapter *rgn_adapters[], const RgnTargetInfo &target,
                                              unsigned int size)
{
    if (!this->curr)
    {
        return;
    }
    unsigned int i = 0;
    AmigosRgnMetaDataTexts &texts = *(AmigosRgnMetaDataTexts*)this->curr->meta_data.data;
    for (i = 0; i < texts.count && i < TEXT_MAX_NUM; ++i)
    {
        if (!rgn_adapters[i])
        {
            return;
        }
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow    = TRUE;
        param.u32Layer = RGN_OSD_TEXT_LAYER;

        param.stOsdChnPort.stOsdAlphaAttr.eAlphaMode                            = E_MI_RGN_PIXEL_ALPHA;
        param.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
        param.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xff;

        param.stOsdChnPort.stPoint.u32X =
            texts.texts[i].pt.x < AMIGOS_RGN_METADATA_COORDINATE_MAX_W
                ? floor(target.timing.width * (float)texts.texts[i].pt.x / AMIGOS_RGN_METADATA_COORDINATE_MAX_W)
                : this->default_x * size;
        param.stOsdChnPort.stPoint.u32Y =
            texts.texts[i].pt.y < AMIGOS_RGN_METADATA_COORDINATE_MAX_H
                ? floor(target.timing.height * (float)texts.texts[i].pt.y / AMIGOS_RGN_METADATA_COORDINATE_MAX_H)
                : this->default_y * size;
        param.stOsdChnPort.stPoint.u32X = ALIGN_UP(param.stOsdChnPort.stPoint.u32X, g_alignment[this->pixel_fmt]);

        rgn_adapters[i]->Attach(target, param);
    }
    if (this->last)
    {
        for (; i < ((AmigosRgnMetaDataTexts*)(this->last->meta_data.data))->count && i < TEXT_MAX_NUM; ++i)
        {
            if (!rgn_adapters[i])
            {
                return;
            }
            rgn_adapters[i]->Detach(target);
        }
    }
}
void AmigosModuleRgn::TextControl::_add_target(const RgnTargetInfo &target)
{
    unsigned int size = _CalFontSize(target.timing.width, target.timing.height, this->size_level);
    auto         it   = this->map_size_desc.find(size);
    if (it != this->map_size_desc.end())
    {
        this->_move_text(it->second.rgn_adapters, target, size);
        it->second.use_count++;
        return;
    }
    TextDesc desc;
    memset(&desc, 0, sizeof(TextDesc));

    this->_refresh_text(desc.rgn_adapters, size, true);
    this->_move_text(desc.rgn_adapters, target, size);
    desc.use_count            = 1;
    this->map_size_desc[size] = desc;
}
void AmigosModuleRgn::TextControl::_del_target(const RgnTargetInfo &target)
{
    unsigned int size = _CalFontSize(target.timing.width, target.timing.height, this->size_level);
    auto it = this->map_size_desc.find(size);
    if (it == this->map_size_desc.end())
    {
        return;
    }
    it->second.use_count--;
    for (unsigned int i = 0; i < TEXT_MAX_NUM; ++i)
    {
        if (it->second.rgn_adapters[i])
        {
            it->second.rgn_adapters[i]->Detach(target);
        }
    }
    if (0 != it->second.use_count)
    {
        return;
    }
    for (unsigned int i = 0; i < TEXT_MAX_NUM; ++i)
    {
        if (it->second.rgn_adapters[i])
        {
            it->second.rgn_adapters[i]->Destroy();
            delete it->second.rgn_adapters[i];
            it->second.rgn_adapters[i] = NULL;
        }
    }
    this->map_size_desc.erase(it);
}
void AmigosModuleRgn::TextControl::_set_param(const RgnTargetInfo &target)
{
    unsigned int size = _CalFontSize(target.timing.width, target.timing.height, this->size_level);
    auto it = this->map_size_desc.find(size);
    if (it == this->map_size_desc.end())
    {
        return;
    }
    if (!it->second.is_refresh)
    {
        this->_refresh_text(it->second.rgn_adapters, size, false);
        it->second.is_refresh = true;
    }
    this->_move_text(it->second.rgn_adapters, target, size);
}
bool AmigosModuleRgn::TextControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_TEXTS)
    {
        return false;
    }
    if (this->curr == nullptr)
    {
        this->curr = stream_packet_base::dup(packet);
        return true;
    }
    if (packet->meta_data_i.size == this->curr->meta_data_i.size)
    {
        if (0 == memcmp(packet->meta_data.data, this->curr->meta_data.data, packet->meta_data_i.size))
        {
            return false;
        }
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
void AmigosModuleRgn::TextControl::_finish()
{
    this->last = stream_packet_base::dup(this->curr);
    for (auto it = this->map_size_desc.begin(); it != this->map_size_desc.end(); ++it)
    {
        it->second.is_refresh = false;
    }
}
int AmigosModuleRgn::TextControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "text")
    {
        return -1;
    }
    const AmigosSurfaceRgn::TextInfo &text_info = info.stTextInfo;

    int diff_cnt = 0;
    if (this->default_x != text_info.intPosX)
    {
        this->default_x = text_info.intPosX;
        ++diff_cnt;
    }
    if (this->default_y != text_info.intPosY)
    {
        this->default_y = text_info.intPosY;
        ++diff_cnt;
    }
    if (this->color != text_info.intColor)
    {
        this->last  = nullptr;
        this->color = text_info.intColor;
        ++diff_cnt;
    }
    return diff_cnt;
}

AmigosModuleRgn::OsdFrameControl::OsdFrameControl(unsigned int devId, const AmigosSurfaceRgn::LineInfo &line_info)
    : RgnControl(devId)
{
    this->pixel_fmt       = ss_enum_cast<MI_RGN_PixelFormat_e>::from_str(line_info.strPixelFmt);
    this->color_on        = line_info.intColor;
    this->color_off       = _CalColorInvert(line_info.intColor);
    this->thickness_level_last = this->thickness_level = ss_enum_cast<ThicknessLevel>::from_str(line_info.strThickness);
    this->curr = nullptr;
    this->last = nullptr;
}
AmigosModuleRgn::OsdFrameControl::~OsdFrameControl()
{
    this->curr = nullptr;
    this->last = nullptr;
}
void AmigosModuleRgn::OsdFrameControl::_refresh_frames(RgnAdapter *rgn_adapter, bool is_init)
{
    MI_RGN_CanvasInfo_t canvas;
    memset(&canvas, 0, sizeof(MI_RGN_CanvasInfo_t));
    if (!rgn_adapter->GetCanvas(canvas))
    {
        return;
    }

    struct SS_Graph::Canvas cvs;
    memset(&cvs, 0, sizeof(SS_Graph::Canvas));
    cvs.data[0]   = (char *)canvas.virtAddr;
    cvs.stride[0] = canvas.u32Stride;
    cvs.width     = canvas.stSize.u32Width;
    cvs.height    = canvas.stSize.u32Height;

    SS_Graph::Graph g(g_canvas_desc_map[canvas.ePixelFmt], cvs);
    if (!is_init && this->last)
    {
        unsigned int thickness =
            _CalTinckness(canvas.stSize.u32Width, canvas.stSize.u32Height, this->thickness_level_last);
        this->thickness_level_last = this->thickness_level;
        AmigosRgnMetaDataFrames &frames = *(AmigosRgnMetaDataFrames *)this->last->meta_data.data;
        for (unsigned int i = 0; i < frames.count; ++i)
        {
            g.DrawFrame<unsigned int>(_CalCoordScale, g_bg_color[this->pixel_fmt], frames.areas[i].rect.x,
                                      frames.areas[i].rect.y, frames.areas[i].rect.w, frames.areas[i].rect.h,
                                      thickness);
        }
    }
    if (this->curr)
    {
        unsigned int thickness = _CalTinckness(canvas.stSize.u32Width, canvas.stSize.u32Height, this->thickness_level);
        AmigosRgnMetaDataFrames &frames = *(AmigosRgnMetaDataFrames *)this->curr->meta_data.data;
        for (unsigned int i = 0; i < frames.count; ++i)
        {
            unsigned int color = frames.areas[i].state ? this->color_on : this->color_off;
            g.DrawFrame<unsigned int>(_CalCoordScale, color, frames.areas[i].rect.x, frames.areas[i].rect.y,
                                      frames.areas[i].rect.w, frames.areas[i].rect.h, thickness);
        }
    }

    rgn_adapter->UpdateCanvas();
}
void AmigosModuleRgn::OsdFrameControl::_add_target(const RgnTargetInfo &target)
{
    MI_RGN_ChnPortParam_t param = {};
    memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
    param.u32Layer                                                = RGN_OSD_LINE_LAYER;
    param.bShow                                                   = TRUE;
    param.stOsdChnPort.stOsdAlphaAttr.eAlphaMode                  = E_MI_RGN_CONSTANT_ALPHA;
    param.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0x40;
    auto it = this->map_timing_desc.find(target.timing);
    if (it != this->map_timing_desc.end())
    {
        if (!it->second.rgn_adapter->Attach(target, param))
        {
            return;
        }
        it->second.use_count++;
        return;
    }
    FrameDesc desc;
    memset(&desc, 0, sizeof(FrameDesc));
    desc.rgn_adapter = new RgnAdapter(this->devId);

    MI_RGN_Attr_t attr;
    memset(&attr, 0, sizeof(MI_RGN_Attr_t));
    attr.eType                           = E_MI_RGN_TYPE_OSD;
    attr.stOsdInitParam.ePixelFmt        = this->pixel_fmt;
    attr.stOsdInitParam.stSize.u32Width  = ALIGN_UP(target.timing.width, g_alignment[this->pixel_fmt]);
    attr.stOsdInitParam.stSize.u32Height = target.timing.height;
    if (!desc.rgn_adapter->Create(attr))
    {
        delete desc.rgn_adapter;
        return;
    }

    if (!desc.rgn_adapter->Attach(target, param))
    {
        desc.rgn_adapter->Destroy();
        delete desc.rgn_adapter;
        return;
    }
    desc.use_count = 1;
    this->_refresh_frames(desc.rgn_adapter, true);
    this->map_timing_desc[target.timing] = desc;
}
void AmigosModuleRgn::OsdFrameControl::_del_target(const RgnTargetInfo &target)
{
    auto it = this->map_timing_desc.find(target.timing);
    if (it == this->map_timing_desc.end())
    {
        return;
    }
    it->second.rgn_adapter->Detach(target);
    it->second.use_count--;
    if (0 == it->second.use_count)
    {
        it->second.rgn_adapter->Destroy();
        delete it->second.rgn_adapter;
        this->map_timing_desc.erase(it);
    }
}
void AmigosModuleRgn::OsdFrameControl::_set_param(const RgnTargetInfo &target)
{
    auto it = this->map_timing_desc.find(target.timing);
    if (it == this->map_timing_desc.end())
    {
        return;
    }
    if (!it->second.is_refresh)
    {
        this->_refresh_frames(it->second.rgn_adapter, false);
        it->second.is_refresh = true;
    }
}
bool AmigosModuleRgn::OsdFrameControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_FRAMES)
    {
        return false;
    }
    if (this->curr == nullptr)
    {
        this->curr = stream_packet_base::dup(packet);
        return true;
    }
    if (packet->meta_data_i.size == this->curr->meta_data_i.size)
    {
        if (0 == memcmp(packet->meta_data.data, this->curr->meta_data.data, packet->meta_data_i.size))
        {
            return false;
        }
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
void AmigosModuleRgn::OsdFrameControl::_finish()
{
    this->last = stream_packet_base::dup(this->curr);
    for (auto it = this->map_timing_desc.begin(); it != this->map_timing_desc.end(); ++it)
    {
        it->second.is_refresh = false;
    }
}
int AmigosModuleRgn::OsdFrameControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "osd_frame")
    {
        return -1;
    }
    const AmigosSurfaceRgn::LineInfo &line_info = info.stLineInfo;
    int diff_cnt = 0;
    if (this->color_on != line_info.intColor)
    {
        this->color_on  = line_info.intColor;
        this->color_off = _CalColorInvert(line_info.intColor);
        ++diff_cnt;
    }
    AmigosModuleRgn::ThicknessLevel thickness = ss_enum_cast<ThicknessLevel>::from_str(line_info.strThickness);
    if (this->thickness_level != thickness)
    {
        this->thickness_level = thickness;
        ++diff_cnt;
    }
    return diff_cnt;
}

AmigosModuleRgn::OsdDotMatrixControl::OsdDotMatrixControl(unsigned int devId, const AmigosSurfaceRgn::DotMatrixInfo &dot_matrix_info)
    : RgnControl(devId)
{
    this->pixel_fmt       = ss_enum_cast<MI_RGN_PixelFormat_e>::from_str(dot_matrix_info.strPixelFmt);
    this->color           = dot_matrix_info.intColor;
    this->size_level_last = this->size_level = ss_enum_cast<SizeLevel>::from_str(dot_matrix_info.strSize);
    this->curr = nullptr;
    this->last = nullptr;
}
AmigosModuleRgn::OsdDotMatrixControl::~OsdDotMatrixControl()
{
    this->curr = nullptr;
    this->last = nullptr;
}
void AmigosModuleRgn::OsdDotMatrixControl::_refresh_dotmatrix(RgnAdapter *rgn_adapter, bool is_init)
{
    MI_RGN_CanvasInfo_t canvas;
    memset(&canvas, 0, sizeof(MI_RGN_CanvasInfo_t));
    if (!rgn_adapter->GetCanvas(canvas))
    {
        return;
    }

    struct SS_Graph::Canvas cvs;
    memset(&cvs, 0, sizeof(SS_Graph::Canvas));
    cvs.data[0]   = (char *)canvas.virtAddr;
    cvs.stride[0] = canvas.u32Stride;
    cvs.width     = canvas.stSize.u32Width;
    cvs.height    = canvas.stSize.u32Height;

    SS_Graph::Graph g(g_canvas_desc_map[canvas.ePixelFmt], cvs);
    if (!is_init && this->last)
    {
        AmigosRgnMetaDataMap &map = *(AmigosRgnMetaDataMap *)this->last->meta_data.data;
        unsigned int size =
            _CalDotMatrixSize(canvas.stSize.u32Width, canvas.stSize.u32Height, map.w, map.h, this->size_level_last);
        this->size_level_last = this->size_level;
        g.DrawDotMatrix(g_bg_color[this->pixel_fmt], map.data, map.w, map.h, size);
    }
    if (this->curr)
    {
        AmigosRgnMetaDataMap &map = *(AmigosRgnMetaDataMap *)this->curr->meta_data.data;
        unsigned int size =
            _CalDotMatrixSize(canvas.stSize.u32Width, canvas.stSize.u32Height, map.w, map.h, this->size_level);
        g.DrawDotMatrix(this->color, map.data, map.w, map.h, size);
    }

    rgn_adapter->UpdateCanvas();
}
void AmigosModuleRgn::OsdDotMatrixControl::_add_target(const RgnTargetInfo &target)
{
    MI_RGN_ChnPortParam_t param = {};
    memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
    param.u32Layer                                                = RGN_OSD_LINE_LAYER;
    param.bShow                                                   = TRUE;
    param.stOsdChnPort.stOsdAlphaAttr.eAlphaMode                  = E_MI_RGN_CONSTANT_ALPHA;
    param.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.u8ConstantAlpha = 0x40;
    auto it = this->map_timing_desc.find(target.timing);
    if (it != this->map_timing_desc.end())
    {
        if (!it->second.rgn_adapter->Attach(target, param))
        {
            return;
        }
        it->second.use_count++;
        return;
    }
    DotMatrixDesc desc;
    memset(&desc, 0, sizeof(DotMatrixDesc));
    desc.rgn_adapter = new RgnAdapter(this->devId);

    MI_RGN_Attr_t attr;
    memset(&attr, 0, sizeof(MI_RGN_Attr_t));
    attr.eType                           = E_MI_RGN_TYPE_OSD;
    attr.stOsdInitParam.ePixelFmt        = this->pixel_fmt;
    attr.stOsdInitParam.stSize.u32Width  = ALIGN_UP(target.timing.width, g_alignment[this->pixel_fmt]);
    attr.stOsdInitParam.stSize.u32Height = target.timing.height;
    if (!desc.rgn_adapter->Create(attr))
    {
        delete desc.rgn_adapter;
        return;
    }

    if (!desc.rgn_adapter->Attach(target, param))
    {
        desc.rgn_adapter->Destroy();
        delete desc.rgn_adapter;
        return;
    }
    desc.use_count = 1;
    this->_refresh_dotmatrix(desc.rgn_adapter, true);
    this->map_timing_desc[target.timing] = desc;
}
void AmigosModuleRgn::OsdDotMatrixControl::_del_target(const RgnTargetInfo &target)
{
    auto it = this->map_timing_desc.find(target.timing);
    if (it == this->map_timing_desc.end())
    {
        return;
    }
    it->second.rgn_adapter->Detach(target);
    it->second.use_count--;
    if (0 == it->second.use_count)
    {
        it->second.rgn_adapter->Destroy();
        delete it->second.rgn_adapter;
        this->map_timing_desc.erase(it);
    }
}
void AmigosModuleRgn::OsdDotMatrixControl::_set_param(const RgnTargetInfo &target)
{
    auto it = this->map_timing_desc.find(target.timing);
    if (it == this->map_timing_desc.end())
    {
        return;
    }
    if (!it->second.is_refresh)
    {
        this->_refresh_dotmatrix(it->second.rgn_adapter, false);
        it->second.is_refresh = true;
    }
}
bool AmigosModuleRgn::OsdDotMatrixControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_MAP)
    {
        return false;
    }
    if (this->curr == nullptr)
    {
        this->curr = stream_packet_base::dup(packet);
        return true;
    }
    if (packet->meta_data_i.size == this->curr->meta_data_i.size)
    {
        if (0 == memcmp(packet->meta_data.data, this->curr->meta_data.data, packet->meta_data_i.size))
        {
            return false;
        }
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
void AmigosModuleRgn::OsdDotMatrixControl::_finish()
{
    this->last = stream_packet_base::dup(this->curr);
    for (auto it = this->map_timing_desc.begin(); it != this->map_timing_desc.end(); ++it)
    {
        it->second.is_refresh = false;
    }
}
int AmigosModuleRgn::OsdDotMatrixControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "osd_frame")
    {
        return -1;
    }
    const AmigosSurfaceRgn::DotMatrixInfo &dot_matrix_info = info.stDotMatrixInfo;
    int diff_cnt = 0;
    if (this->color != dot_matrix_info.intColor)
    {
        this->color = dot_matrix_info.intColor;
        ++diff_cnt;
    }
    AmigosModuleRgn::SizeLevel size = ss_enum_cast<SizeLevel>::from_str(dot_matrix_info.strSize);
    if (this->size_level != size)
    {
        this->size_level = size;
        ++diff_cnt;
    }
    return diff_cnt;
}

AmigosModuleRgn::CoverControl::CoverControl(unsigned int devId, const AmigosSurfaceRgn::CoverInfo &cover_info)
    : RgnControl(devId)
{
    this->color         = _ConvertColorToUyv(cover_info.intColor, NULL);
    this->mode          = ss_enum_cast<MI_RGN_CoverMode_e>::from_str(cover_info.strType);
    this->block_size_lv = ss_enum_cast<SizeLevel>::from_str(cover_info.strBlockSize);
    this->use_count     = 0;
    this->curr          = nullptr;
    memset(this->rgn_adapters, 0, sizeof(RgnAdapter *) * COVER_MAX_NUM);
}
AmigosModuleRgn::CoverControl::~CoverControl()
{
    this->curr = nullptr;
}
void AmigosModuleRgn::CoverControl::_move_cover(const RgnTargetInfo &target)
{
    MI_RGN_BlockSize_e block_size = _CalMosaicBlockSize(this->block_size_lv);
    unsigned int i = 0;
    AmigosRgnMetaDataCovers &covers = *(AmigosRgnMetaDataCovers*)this->curr->meta_data.data;
    for (i = 0; i < covers.count && i < COVER_MAX_NUM; ++i)
    {
        if (!this->rgn_adapters[i])
        {
            MI_RGN_Attr_t attr;
            memset(&attr, 0, sizeof(MI_RGN_Attr_t));
            attr.eType = E_MI_RGN_TYPE_COVER;
            this->rgn_adapters[i] = new RgnAdapter(this->devId);
            if (!this->rgn_adapters[i]->Create(attr))
            {
                delete this->rgn_adapters[i];
                this->rgn_adapters[i] = NULL;
                break;
            }
        }
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow                           = TRUE;
        param.stCoverChnPort.eMode            = this->mode;
        param.stCoverChnPort.eAreaType        = E_MI_RGN_AREA_TYPE_RECT;
        param.stCoverChnPort.stRect.s32X      = covers.areas[i].rect.x;
        param.stCoverChnPort.stRect.s32Y      = covers.areas[i].rect.y;
        param.stCoverChnPort.stRect.u32Width  = covers.areas[i].rect.w;
        param.stCoverChnPort.stRect.u32Height = covers.areas[i].rect.h;
        if (this->mode == E_MI_RGN_COVER_MODE_COLOR)
        {
            param.stCoverChnPort.stColorAttr.u32Color = this->color;
        }
        else
        {
            param.stCoverChnPort.stMosaicAttr.eBlkSize = block_size;
        }
        this->rgn_adapters[i]->Attach(target, param);
    }
    for (; i < COVER_MAX_NUM; ++i)
    {
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow = FALSE;
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Detach(target);
        }
    }
}
void AmigosModuleRgn::CoverControl::_add_target(const RgnTargetInfo &target)
{
    this->_move_cover(target);
    this->use_count++;
}
void AmigosModuleRgn::CoverControl::_del_target(const RgnTargetInfo &target)
{
    if (this->use_count == 0)
    {
        return;
    }
    --this->use_count;
    for (unsigned int i = 0; i < COVER_MAX_NUM; ++i)
    {
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Detach(target);
        }
    }
    if (0 != this->use_count)
    {
        return;
    }
    for (unsigned int i = 0; i < COVER_MAX_NUM; ++i)
    {
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Destroy();
            delete this->rgn_adapters[i];
            this->rgn_adapters[i] = NULL;
        }
    }
}
void AmigosModuleRgn::CoverControl::_set_param(const RgnTargetInfo &target)
{
    this->_move_cover(target);
}
bool AmigosModuleRgn::CoverControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_COVERS)
    {
        return false;
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
int AmigosModuleRgn::CoverControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "cover")
    {
        return -1;
    }
    const AmigosSurfaceRgn::CoverInfo &cover_info = info.stCoverInfo;

    int diff_cnt = 0;
    unsigned int color = _ConvertColorToUyv(cover_info.intColor, NULL);
    if (this->color != color)
    {
        this->color = color;
        ++diff_cnt;
    }
    MI_RGN_CoverMode_e type = ss_enum_cast<MI_RGN_CoverMode_e>::from_str(cover_info.strType);
    if (this->mode != type)
    {
        this->mode = type;
        ++diff_cnt;
    }
    SizeLevel level = ss_enum_cast<SizeLevel>::from_str(cover_info.strBlockSize);
    if (this->block_size_lv != level)
    {
        this->block_size_lv = level;
        ++diff_cnt;
    }
    return diff_cnt;
}
AmigosModuleRgn::FrameControl::FrameControl(unsigned int devId, const AmigosSurfaceRgn::FrameInfo &frame_info)
    : RgnControl(devId)
{
    this->color        = _ConvertColorToUyv(frame_info.intColor, NULL);
    this->thickness_lv = ss_enum_cast<ThicknessLevel>::from_str(frame_info.strThickness);
    this->use_count    = 0;
    this->curr         = nullptr;
    memset(this->rgn_adapters, 0, sizeof(RgnAdapter *) * FRAME_MAX_NUM);
}
AmigosModuleRgn::FrameControl::~FrameControl()
{
    this->curr = nullptr;
}
void AmigosModuleRgn::FrameControl::_move_frame(const RgnTargetInfo &target)
{
    unsigned int thickness = _CalTinckness(target.timing.width, target.timing.height, this->thickness_lv);

    unsigned int i = 0;
    AmigosRgnMetaDataFrames &frames = *(AmigosRgnMetaDataFrames*)this->curr->meta_data.data;
    for (i = 0; i < frames.count && i < FRAME_MAX_NUM; ++i)
    {
        if (!this->rgn_adapters[i])
        {
            MI_RGN_Attr_t attr;
            memset(&attr, 0, sizeof(MI_RGN_Attr_t));
            attr.eType = E_MI_RGN_TYPE_FRAME;
            this->rgn_adapters[i] = new RgnAdapter(this->devId);
            if (!this->rgn_adapters[i]->Create(attr))
            {
                delete this->rgn_adapters[i];
                this->rgn_adapters[i] = NULL;
                break;
            }
        }
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow                           = TRUE;
        param.stFrameChnPort.u32Color = this->color;
        param.stFrameChnPort.u8Thickness = (MI_U8)thickness;
        param.stFrameChnPort.stRect.s32X      = frames.areas[i].rect.x;
        param.stFrameChnPort.stRect.s32Y      = frames.areas[i].rect.y;
        param.stFrameChnPort.stRect.u32Width  = frames.areas[i].rect.w;
        param.stFrameChnPort.stRect.u32Height = frames.areas[i].rect.h;
        this->rgn_adapters[i]->Attach(target, param);
    }
    for (; i < FRAME_MAX_NUM; ++i)
    {
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow = FALSE;
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Detach(target);
        }
    }
}
void AmigosModuleRgn::FrameControl::_add_target(const RgnTargetInfo &target)
{
    this->_move_frame(target);
    this->use_count++;
}
void AmigosModuleRgn::FrameControl::_del_target(const RgnTargetInfo &target)
{
    if (this->use_count == 0)
    {
        return;
    }
    --this->use_count;
    for (unsigned int i = 0; i < FRAME_MAX_NUM; ++i)
    {
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Detach(target);
        }
    }
    if (0 != this->use_count)
    {
        return;
    }
    for (unsigned int i = 0; i < FRAME_MAX_NUM; ++i)
    {
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Destroy();
            delete this->rgn_adapters[i];
            this->rgn_adapters[i] = NULL;
        }
    }
}
void AmigosModuleRgn::FrameControl::_set_param(const RgnTargetInfo &target)
{
    this->_move_frame(target);
}
bool AmigosModuleRgn::FrameControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_FRAMES)
    {
        return false;
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
int AmigosModuleRgn::FrameControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "frame")
    {
        return -1;
    }
    const AmigosSurfaceRgn::FrameInfo &frame_info = info.stFrameInfo;

    int diff_cnt = 0;
    unsigned int color = _ConvertColorToUyv(frame_info.intColor, NULL);
    if (this->color != color)
    {
        this->color = color;
        ++diff_cnt;
    }
    ThicknessLevel thickness_lv = ss_enum_cast<ThicknessLevel>::from_str(frame_info.strThickness);
    if (this->thickness_lv != thickness_lv)
    {
        this->thickness_lv = thickness_lv;
        ++diff_cnt;
    }
    return diff_cnt;
}

AmigosModuleRgn::PolyControl::PolyControl(unsigned int devId, const AmigosSurfaceRgn::CoverInfo &poly_info)
    : RgnControl(devId)
{
    this->color         = _ConvertColorToUyv(poly_info.intColor, NULL);
    this->mode          = ss_enum_cast<MI_RGN_CoverMode_e>::from_str(poly_info.strType);
    this->block_size_lv = ss_enum_cast<SizeLevel>::from_str(poly_info.strBlockSize);
    this->use_count     = 0;
    this->curr          = nullptr;
    memset(this->rgn_adapters, 0, sizeof(RgnAdapter *) * POLY_MAX_NUM);
}
AmigosModuleRgn::PolyControl::~PolyControl()
{
    this->curr = nullptr;
}
void AmigosModuleRgn::PolyControl::_move_poly(const RgnTargetInfo &target)
{
    MI_RGN_BlockSize_e block_size = _CalMosaicBlockSize(this->block_size_lv);
    unsigned int i = 0;
    AmigosRgnMetaDataPolys &polys = *(AmigosRgnMetaDataPolys *)this->curr->meta_data.data;
    for (i = 0; i < polys.count && i < POLY_MAX_NUM; ++i)
    {
        if (!this->rgn_adapters[i])
        {
            MI_RGN_Attr_t attr;
            memset(&attr, 0, sizeof(MI_RGN_Attr_t));
            attr.eType = E_MI_RGN_TYPE_COVER;
            this->rgn_adapters[i] = new RgnAdapter(this->devId);
            if (!this->rgn_adapters[i]->Create(attr))
            {
                delete this->rgn_adapters[i];
                this->rgn_adapters[i] = NULL;
                break;
            }
        }
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow                             = TRUE;
        param.stCoverChnPort.eMode              = this->mode;
        param.stCoverChnPort.eAreaType          = E_MI_RGN_AREA_TYPE_POLY;
        param.stCoverChnPort.stPoly.u8VertexNum = polys.polys[i].vectexNum;
        for (unsigned int j = 0; j < polys.polys[i].vectexNum; ++j)
        {
            param.stCoverChnPort.stPoly.astCoord[j].u32X = polys.polys[i].vectexArr[j].x;
            param.stCoverChnPort.stPoly.astCoord[j].u32Y = polys.polys[i].vectexArr[j].y;
        }
        if (this->mode == E_MI_RGN_COVER_MODE_COLOR)
        {
            param.stCoverChnPort.stColorAttr.u32Color = this->color;
        }
        else
        {
            param.stCoverChnPort.stMosaicAttr.eBlkSize = block_size;
        }
        this->rgn_adapters[i]->Attach(target, param);
    }
    for (; i < POLY_MAX_NUM; ++i)
    {
        MI_RGN_ChnPortParam_t param;
        memset(&param, 0, sizeof(MI_RGN_ChnPortParam_t));
        param.bShow = FALSE;
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Detach(target);
        }
    }
}
void AmigosModuleRgn::PolyControl::_add_target(const RgnTargetInfo &target)
{
    this->_move_poly(target);
    this->use_count++;
}
void AmigosModuleRgn::PolyControl::_del_target(const RgnTargetInfo &target)
{
    if (this->use_count == 0)
    {
        return;
    }
    --this->use_count;
    for (unsigned int i = 0; i < POLY_MAX_NUM; ++i)
    {
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Detach(target);
        }
    }
    if (0 != this->use_count)
    {
        return;
    }
    for (unsigned int i = 0; i < POLY_MAX_NUM; ++i)
    {
        if (this->rgn_adapters[i])
        {
            this->rgn_adapters[i]->Destroy();
            delete this->rgn_adapters[i];
            this->rgn_adapters[i] = NULL;
        }
    }
}
void AmigosModuleRgn::PolyControl::_set_param(const RgnTargetInfo &target)
{
    this->_move_poly(target);
}
bool AmigosModuleRgn::PolyControl::_check_packet(stream_packet_obj &packet)
{
    if (packet == nullptr)
    {
        return false;
    }
    if (packet->en_type != EN_USER_META_DATA || packet->meta_data_i.reserved != E_META_DATA_POLYS)
    {
        return false;
    }
    this->curr = stream_packet_base::dup(packet);
    return true;
}
int AmigosModuleRgn::PolyControl::_set_attr(const AmigosSurfaceRgn::RgnInputInfo &info)
{
    if (info.strMode != "poly")
    {
        return -1;
    }
    const AmigosSurfaceRgn::CoverInfo &poly_info = info.stCoverInfo;

    int diff_cnt = 0;
    unsigned int color = _ConvertColorToUyv(poly_info.intColor, NULL);
    if (this->color != color)
    {
        this->color = color;
        ++diff_cnt;
    }
    MI_RGN_CoverMode_e type = ss_enum_cast<MI_RGN_CoverMode_e>::from_str(poly_info.strType);
    if (this->mode != type)
    {
        this->mode = type;
        ++diff_cnt;
    }
    SizeLevel level = ss_enum_cast<SizeLevel>::from_str(poly_info.strBlockSize);
    if (this->block_size_lv != level)
    {
        this->block_size_lv = level;
        ++diff_cnt;
    }
    return diff_cnt;
}
AmigosModuleRgn::AmigosModuleRgn(const std::string &strInSection)
    : AmigosSurfaceRgn(strInSection), AmigosModuleMiBase(this)
{
    this->rgn_lock = PTHREAD_RWLOCK_INITIALIZER;
    this->color_invert_thread_alive = false;
    this->threadHandle = nullptr;
    memset(&this->color_invert_attr, 0, sizeof(MI_RGN_ColorInvertAttr_t));
}
AmigosModuleRgn::~AmigosModuleRgn()
{

}
unsigned int AmigosModuleRgn::GetModId() const
{
    return E_MI_MODULE_ID_RGN;
}
unsigned int AmigosModuleRgn::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModuleRgn::GetOutputType(unsigned int port) const
{
    return 0;
}
void AmigosModuleRgn::_Init()
{
    CHECK_RESULT(MI_SUCCESS, return, MI_RGN_Init, 0, &g_palette_table);
    for (auto it = this->mapRgnInputInfo.begin(); it != this->mapRgnInputInfo.end(); ++it)
    {
        if (it->second.strMode == "canvas")
        {
            this->map_canvas_rgn_adapter[it->first] = new RgnAdapter(this->stModInfo.devId);
        }
    }
}
void AmigosModuleRgn::_Deinit()
{
    for (auto it = this->map_canvas_rgn_adapter.begin(); it != this->map_canvas_rgn_adapter.end(); ++it)
    {
        delete it->second;
        it->second = nullptr;
    }
    this->map_canvas_rgn_adapter.clear();
    CHECK_RESULT(MI_SUCCESS, return, MI_RGN_DeInit, 0);
    this->target_lst.clear();
}
void AmigosModuleRgn::_Start()
{
    for (auto targetIt = this->stRgnInfo.lstAttach.begin(); targetIt != this->stRgnInfo.lstAttach.end(); ++targetIt)
    {
        RgnTargetInfo target;
        memset(&target, 0, sizeof(RgnTargetInfo));
        if (!this->_ConvertAttachInfoToTargetInfo(*targetIt, target))
        {
            AMIGOS_ERR("_ConvertAttachInfoToTargetInfo Failed, Ignore %s-%s_%d\n", targetIt->strMod.c_str(),
                       targetIt->intIsInPort ? "IN" : "OUT", targetIt->intPort);
            continue;
        }
        this->target_lst.push_back(target);
    }
    this->SetColorInvert(this->stRgnInfo, true);
}
void AmigosModuleRgn::_Stop()
{
    if (this->color_invert_thread_alive)
    {
        ss_thread_stop(this->threadHandle);
        ss_thread_close(this->threadHandle);
        this->threadHandle = nullptr;
        this->color_invert_thread_alive = false;
    }
}
int AmigosModuleRgn::_DataReceiver(unsigned int inPortId, stream_packet_obj &packet)
{
    ss_auto_rdlock lock(this->rgn_lock);
    auto it = this->map_rgn_control.find(inPortId);
    if (it == this->map_rgn_control.end())
    {
        return -1;
    }
    it->second->process(this->target_lst, packet);
    return 0;
}
void AmigosModuleRgn::_StartIn(unsigned int inPortId)
{
    if (this->map_rgn_control.end() != this->map_rgn_control.find(inPortId))
    {
        return;
    }
    auto infoIt = this->mapRgnInputInfo.find(inPortId);
    if (infoIt == this->mapRgnInputInfo.end())
    {
        return;
    }

    RgnControl *pControl = NULL;
    auto it = this->map_canvas_rgn_adapter.find(inPortId);
    if (it != this->map_canvas_rgn_adapter.end())
    {
        pControl = new CanvasControl(this->stModInfo.devId, infoIt->second.stCanvasInfo, it->second);
    }
    else if (infoIt->second.strMode == "line")
    {
        pControl = new LineControl(this->stModInfo.devId, infoIt->second.stLineInfo);
    }
    else if (infoIt->second.strMode == "text")
    {
        pControl = new TextControl(this->stModInfo.devId, infoIt->second.stTextInfo);
    }
    else if (infoIt->second.strMode == "osd_frame")
    {
        pControl = new OsdFrameControl(this->stModInfo.devId, infoIt->second.stLineInfo);
    }
    else if (infoIt->second.strMode == "osd_dot_matrix")
    {
        pControl = new OsdDotMatrixControl(this->stModInfo.devId, infoIt->second.stDotMatrixInfo);
    }
    else if (infoIt->second.strMode == "cover")
    {
        pControl = new CoverControl(this->stModInfo.devId, infoIt->second.stCoverInfo);
    }
    else if (infoIt->second.strMode == "frame")
    {
        pControl = new FrameControl(this->stModInfo.devId, infoIt->second.stFrameInfo);
    }
    else if (infoIt->second.strMode == "poly")
    {
        pControl = new PolyControl(this->stModInfo.devId, infoIt->second.stCoverInfo);
    }

    if (!pControl)
    {
        return;
    }
    this->map_rgn_control[inPortId] = pControl;
}
void AmigosModuleRgn::_StopIn(unsigned int inPortId)
{
    auto it = this->map_rgn_control.find(inPortId);
    if (it == this->map_rgn_control.end())
    {
        return;
    }
    for (auto targetIt = this->target_lst.begin(); targetIt != this->target_lst.end(); ++targetIt)
    {
        it->second->del_target(*targetIt);
    }
    delete it->second;
    this->map_rgn_control.erase(it);
}
ss_linker_base *AmigosModuleRgn::_CreateInputNegativeLinker(unsigned int inPortId)
{
    auto it = this->map_canvas_rgn_adapter.find(inPortId);
    if (it != this->map_canvas_rgn_adapter.end())
    {
        return new LinkerRgnIn(it->second);
    }
    return new LinkerSyncNegative(inPortId, this);
}
stream_packer *AmigosModuleRgn::_CreateInputStreamPacker(unsigned int inPortId, bool &bFast)
{
    auto it = this->map_canvas_rgn_adapter.find(inPortId);
    if (it != this->map_canvas_rgn_adapter.end())
    {
        return new StreamPackerRgnIn(it->second);
    }
    return new StreamPackerNormal();
}
bool AmigosModuleRgn::Attach(const AmigosSurfaceRgn::RgnModAttachInfo &attach_info, bool bUserTiming)
{
    RgnTargetInfo target;
    memset(&target, 0, sizeof(RgnTargetInfo));
    if (!this->_ConvertAttachInfoToTargetInfo(attach_info, target, !bUserTiming))
    {
        AMIGOS_ERR("_ConvertAttachInfoToTargetInfo Failed, Ignore %s-%s_%d\n", attach_info.strMod.c_str(),
                   attach_info.intIsInPort ? "IN" : "OUT", attach_info.intPort);
        return false;
    }
    ss_auto_rwlock lock(this->rgn_lock);
    for (auto targetIt = this->target_lst.begin(); targetIt != this->target_lst.end(); ++targetIt)
    {
        if (target == *targetIt)
        {
            return false;
        }
    }
    for (auto it = this->map_rgn_control.begin(); it != this->map_rgn_control.end(); ++it)
    {
        it->second->add_target(target);
    }
    if (this->color_invert_attr.bEnable)
    {
        MI_RGN_ColorInvertAttr_s attr = this->color_invert_attr;
        attr.eBlkSizeVert = attr.eBlkSizeHori = _CalColorInvertBlockSize(target.timing.width, target.timing.height);
        CHECK_RESULT(MI_SUCCESS, return false, MI_RGN_SetColorInvertAttr, 0, &target.stChnPort, &attr);
    }
    this->target_lst.push_back(target);
    return true;
}
bool AmigosModuleRgn::Detach(const AmigosSurfaceRgn::RgnModAttachInfo &attach_info)
{
    RgnTargetInfo target;
    memset(&target, 0, sizeof(RgnTargetInfo));
    if (!this->_ConvertAttachInfoToTargetInfo(attach_info, target, false))
    {
        AMIGOS_ERR("_ConvertAttachInfoToTargetInfo Failed, Ignore %s-%s_%d\n", attach_info.strMod.c_str(),
                   attach_info.intIsInPort ? "IN" : "OUT", attach_info.intPort);
        return false;
    }
    ss_auto_rwlock lock(this->rgn_lock);
    for (auto targetIt = this->target_lst.begin(); targetIt != this->target_lst.end(); ++targetIt)
    {
        if (target == *targetIt)
        {
            for (auto it = this->map_rgn_control.begin(); it != this->map_rgn_control.end(); ++it)
            {
                it->second->del_target(*targetIt);
            }
            this->target_lst.erase(targetIt);
            return true;
        }
    }
    return false;
}
bool AmigosModuleRgn::SetAttr(unsigned int inPortId, const AmigosSurfaceRgn::RgnInputInfo &info)
{
    ss_auto_rdlock lock(this->rgn_lock);
    auto it = this->map_rgn_control.find(inPortId);
    if (this->map_rgn_control.end() == it)
    {
        return false;
    }
    return it->second->set_attr(this->target_lst, info);
}
bool AmigosModuleRgn::SetColorInvert(const AmigosSurfaceRgn::RgnInfo &info, bool is_init)
{
    MI_RGN_ColorInvertWorkMode_e work_mode =
        ss_enum_cast<MI_RGN_ColorInvertWorkMode_e>::from_str(info.strColorInvertMode);

    pthread_rwlock_wrlock(&this->rgn_lock);
    if (!is_init)
    {
        unsigned int diff_cnt = 0;
        if (this->color_invert_attr.bEnable != info.intColorInvertEn)
        {
            this->color_invert_attr.bEnable = info.intColorInvertEn;
            ++diff_cnt;
        }
        if (this->color_invert_attr.eWorkMode != work_mode)
        {
            this->color_invert_attr.eWorkMode = work_mode;
            ++diff_cnt;
        }
        if (this->color_invert_attr.u8ThresholdLow != info.intColorInvertThresholdL)
        {
            this->color_invert_attr.u8ThresholdLow = info.intColorInvertThresholdL;
            ++diff_cnt;
        }
        if (this->color_invert_attr.u8ThresholdHigh != info.intColorInvertThresholdH)
        {
            this->color_invert_attr.u8ThresholdHigh = info.intColorInvertThresholdH;
            ++diff_cnt;
        }
        if (diff_cnt == 0)
        {
            pthread_rwlock_unlock(&this->rgn_lock);
            return true;
        }
    }
    else
    {
        this->color_invert_attr.bEnable         = info.intColorInvertEn;
        this->color_invert_attr.eWorkMode       = work_mode;
        this->color_invert_attr.u8ThresholdLow  = info.intColorInvertThresholdL;
        this->color_invert_attr.u8ThresholdHigh = info.intColorInvertThresholdH;
    }
    bool need_start_thread =
        this->color_invert_attr.eWorkMode == E_MI_RGN_COLOR_INVERT_WORK_MODE_MANUAL && this->color_invert_attr.bEnable;
    pthread_rwlock_unlock(&this->rgn_lock);

    if (this->color_invert_thread_alive && !need_start_thread)
    {
        ss_thread_stop(this->threadHandle);
        ss_thread_close(this->threadHandle);
        this->threadHandle = nullptr;
        this->color_invert_thread_alive = false;
    }

    pthread_rwlock_rdlock(&this->rgn_lock);
    for (auto it = this->target_lst.begin(); it != this->target_lst.end(); ++it)
    {
        MI_RGN_ColorInvertAttr_s attr = this->color_invert_attr;
        attr.eBlkSizeVert = attr.eBlkSizeHori = _CalColorInvertBlockSize(it->timing.width, it->timing.height);
        CHECK_RESULT_NO_LOG(MI_SUCCESS, continue, MI_RGN_SetColorInvertAttr, 0, &it->stChnPort, &attr);
    }
    pthread_rwlock_unlock(&this->rgn_lock);

    if (!this->color_invert_thread_alive && need_start_thread)
    {
        std::string    thread_key = this->GetModIdStr() + "_CI";
        ss_thread_attr ss_attr;
        memset(&ss_attr, 0, sizeof(ss_thread_attr));
        ss_attr.do_signal          = NULL;
        ss_attr.do_monitor         = ColorInvertProcess;
        ss_attr.monitor_cycle_sec  = 1;
        ss_attr.monitor_cycle_nsec = 0;
        ss_attr.is_reset_timer     = 0;
        ss_attr.in_buf.buf         = this;
        ss_attr.in_buf.size        = 0;
        snprintf(ss_attr.thread_name, 128, "%s", thread_key.c_str());
        this->threadHandle = ss_thread_open(&ss_attr);
        ss_thread_start_monitor(this->threadHandle);
        this->color_invert_thread_alive = true;
    }
    return true;
}
bool AmigosModuleRgn::Process(unsigned int inPortId, stream_packet_obj &packet)
{
    ss_auto_rdlock lock(this->rgn_lock);
    auto it = this->map_rgn_control.find(inPortId);
    if (this->map_rgn_control.end() == it)
    {
        return false;
    }
    it->second->process(this->target_lst, packet);
    return true;
}
bool AmigosModuleRgn::_ConvertAttachInfoToTargetInfo(const RgnModAttachInfo &attach_info, RgnTargetInfo &target,
                                                     bool bUseEnvTiming)
{
    AmigosEnv &modEnv = this->env.Ext(attach_info.strMod.c_str());

    std::string strModId = modEnv["MOD"];
    std::string strDevId = modEnv["DEV"];
    std::string strChnId = modEnv["CHN"];
    if (strModId == "" || strChnId == "" || strDevId == "")
    {
        return false;
    }

    try
    {
        target.stChnPort.eModId     = (MI_ModuleId_e)std::stoi(strModId);
        target.stChnPort.s32DevId   = std::stoi(strDevId);
        target.stChnPort.s32ChnId   = std::stoi(strChnId);
        target.stChnPort.s32PortId  = attach_info.intPort;
        target.stChnPort.bInputPort = attach_info.intIsInPort;
    }
    catch (...)
    {
        return false;
    }

    if (!bUseEnvTiming)
    {
        target.timing.width  = attach_info.intTimingW;
        target.timing.height = attach_info.intTimingH;
        return true;
    }
    std::string strW, strH;
    if (attach_info.intIsInPort)
    {
        strW = modEnv.In(attach_info.intPort)["WIDTH"];
        strH = modEnv.In(attach_info.intPort)["HEIGHT"];
    }
    else
    {
        strW = modEnv.Out(attach_info.intPort)["WIDTH"];
        strH = modEnv.Out(attach_info.intPort)["HEIGHT"];
    }
    if (strW == "" || strH == "")
    {
        target.timing.width  = attach_info.intTimingW;
        target.timing.height = attach_info.intTimingH;
        AMIGOS_WRN("Env not found, user user timing\n");
        return true;
    }
    try
    {
        target.timing.width  = std::stoi(strW);
        target.timing.height = std::stoi(strH);
    }
    catch (...)
    {
        target.timing.width  = attach_info.intTimingW;
        target.timing.height = attach_info.intTimingH;
        AMIGOS_WRN("Env not found, user user timing\n");
    }
    return true;
}

void *ColorInvertProcess(struct ss_thread_buffer *thread_buf)
{
    AmigosModuleRgn *pMyClass = (AmigosModuleRgn *)thread_buf->buf;

    ss_auto_rdlock lock(pMyClass->rgn_lock);
    if (pMyClass->color_invert_attr.eWorkMode != E_MI_RGN_COLOR_INVERT_WORK_MODE_MANUAL)
    {
        return NULL;
    }
    for (auto it = pMyClass->target_lst.begin(); it != pMyClass->target_lst.end(); ++it)
    {
        MI_RGN_LumaInfo_t luma_info = {};
        memset(&luma_info, 0, sizeof(MI_RGN_LumaInfo_t));
        MI_S32 s32Ret = MI_RGN_GetLuma(0, &it->stChnPort, &luma_info);
        if (s32Ret != MI_SUCCESS && s32Ret != E_MI_ERR_NOBUF)
        {
            if (s32Ret == MI_ERR_RGN_NOBUF)
            {
                continue;
            }
            AMIGOS_ERR("MI_RGN_GetLuma Failed ret = %d\n", s32Ret);
            continue;
        }
        memset((void*)luma_info.virtAddr, 0xff, luma_info.u32Stride * luma_info.stSize.u32Height);
        MI_RGN_ColorInvertAttr_s attr = pMyClass->color_invert_attr;
        attr.bApplyMap                = TRUE;
        attr.eBlkSizeVert = attr.eBlkSizeHori = _CalColorInvertBlockSize(it->timing.width, it->timing.height);
        CHECK_RESULT_NO_LOG(MI_SUCCESS, continue, MI_RGN_SetColorInvertAttr, 0, &it->stChnPort, &attr);
    }
    return NULL;
}

AMIGOS_MODULE_INIT("RGN", AmigosModuleRgn);

