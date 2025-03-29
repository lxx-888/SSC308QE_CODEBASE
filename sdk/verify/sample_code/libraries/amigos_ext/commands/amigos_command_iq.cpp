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
#include "ss_cmd_base.h"
#include "amigos_module_iq.h"
#include "mi_isp_awb.h"
#include "mi_isp_ae.h"
#include "mi_isp_cus3a_api.h"
#include "mi_iqserver.h"

static MI_U16 RGBGamma_L[3][256] =
{
    {
        0,  20,  40,  61,  82, 102, 122, 140, 158, 175, 191, 206, 220, 234, 247, 261,
        273, 286, 298, 309, 321, 331, 342, 352, 362, 371, 380, 389, 397, 405, 413, 421,
        429, 436, 444, 451, 459, 466, 473, 479, 486, 493, 499, 505, 511, 517, 523, 529,
        535, 540, 545, 551, 556, 561, 566, 571, 576, 580, 585, 590, 595, 599, 604, 608,
        613, 617, 621, 625, 629, 633, 636, 640, 644, 647, 651, 654, 657, 661, 664, 667,
        670, 674, 677, 680, 683, 686, 689, 692, 695, 698, 701, 704, 708, 711, 714, 717,
        720, 723, 726, 729, 732, 735, 738, 741, 744, 746, 749, 752, 755, 758, 761, 763,
        766, 769, 772, 775, 777, 780, 783, 786, 789, 791, 794, 797, 799, 802, 804, 807,
        809, 812, 814, 816, 818, 821, 823, 825, 827, 829, 831, 833, 835, 837, 839, 841,
        843, 845, 847, 849, 851, 853, 855, 857, 859, 861, 863, 865, 867, 869, 870, 872,
        874, 876, 878, 879, 881, 883, 885, 886, 888, 890, 891, 893, 894, 896, 898, 899,
        901, 903, 905, 906, 908, 910, 911, 913, 915, 916, 918, 920, 922, 923, 925, 927,
        929, 930, 932, 934, 936, 938, 939, 941, 943, 945, 947, 949, 950, 952, 954, 955,
        957, 959, 960, 962, 963, 965, 966, 968, 969, 971, 972, 974, 975, 977, 978, 979,
        981, 982, 984, 985, 986, 988, 989, 991, 992, 993, 995, 996, 997, 999,1000,1001,
        1003,1004,1005,1007,1008,1009,1010,1012,1013,1014,1016,1017,1018,1019,1021,1023
    },
    {
        0,  20,  40,  61,  82, 102, 122, 140, 158, 175, 191, 206, 220, 234, 247, 261,
        273, 286, 298, 309, 321, 331, 342, 352, 362, 371, 380, 389, 397, 405, 413, 421,
        429, 436, 444, 451, 459, 466, 473, 479, 486, 493, 499, 505, 511, 517, 523, 529,
        535, 540, 545, 551, 556, 561, 566, 571, 576, 580, 585, 590, 595, 599, 604, 608,
        613, 617, 621, 625, 629, 633, 636, 640, 644, 647, 651, 654, 657, 661, 664, 667,
        670, 674, 677, 680, 683, 686, 689, 692, 695, 698, 701, 704, 708, 711, 714, 717,
        720, 723, 726, 729, 732, 735, 738, 741, 744, 746, 749, 752, 755, 758, 761, 763,
        766, 769, 772, 775, 777, 780, 783, 786, 789, 791, 794, 797, 799, 802, 804, 807,
        809, 812, 814, 816, 818, 821, 823, 825, 827, 829, 831, 833, 835, 837, 839, 841,
        843, 845, 847, 849, 851, 853, 855, 857, 859, 861, 863, 865, 867, 869, 870, 872,
        874, 876, 878, 879, 881, 883, 885, 886, 888, 890, 891, 893, 894, 896, 898, 899,
        901, 903, 905, 906, 908, 910, 911, 913, 915, 916, 918, 920, 922, 923, 925, 927,
        929, 930, 932, 934, 936, 938, 939, 941, 943, 945, 947, 949, 950, 952, 954, 955,
        957, 959, 960, 962, 963, 965, 966, 968, 969, 971, 972, 974, 975, 977, 978, 979,
        981, 982, 984, 985, 986, 988, 989, 991, 992, 993, 995, 996, 997, 999,1000,1001,
        1003,1004,1005,1007,1008,1009,1010,1012,1013,1014,1016,1017,1018,1019,1021,1023
    },
    {
        0,  20,  40,  61,  82, 102, 122, 140, 158, 175, 191, 206, 220, 234, 247, 261,
        273, 286, 298, 309, 321, 331, 342, 352, 362, 371, 380, 389, 397, 405, 413, 421,
        429, 436, 444, 451, 459, 466, 473, 479, 486, 493, 499, 505, 511, 517, 523, 529,
        535, 540, 545, 551, 556, 561, 566, 571, 576, 580, 585, 590, 595, 599, 604, 608,
        613, 617, 621, 625, 629, 633, 636, 640, 644, 647, 651, 654, 657, 661, 664, 667,
        670, 674, 677, 680, 683, 686, 689, 692, 695, 698, 701, 704, 708, 711, 714, 717,
        720, 723, 726, 729, 732, 735, 738, 741, 744, 746, 749, 752, 755, 758, 761, 763,
        766, 769, 772, 775, 777, 780, 783, 786, 789, 791, 794, 797, 799, 802, 804, 807,
        809, 812, 814, 816, 818, 821, 823, 825, 827, 829, 831, 833, 835, 837, 839, 841,
        843, 845, 847, 849, 851, 853, 855, 857, 859, 861, 863, 865, 867, 869, 870, 872,
        874, 876, 878, 879, 881, 883, 885, 886, 888, 890, 891, 893, 894, 896, 898, 899,
        901, 903, 905, 906, 908, 910, 911, 913, 915, 916, 918, 920, 922, 923, 925, 927,
        929, 930, 932, 934, 936, 938, 939, 941, 943, 945, 947, 949, 950, 952, 954, 955,
        957, 959, 960, 962, 963, 965, 966, 968, 969, 971, 972, 974, 975, 977, 978, 979,
        981, 982, 984, 985, 986, 988, 989, 991, 992, 993, 995, 996, 997, 999,1000,1001,
        1003,1004,1005,1007,1008,1009,1010,1012,1013,1014,1016,1017,1018,1019,1021,1023
    }
};

static MI_U16 RGBGamma_N[3][256] =
{
    {
        0,   8,  14,  24,  31,  40,  51,  59,  71,  83,  94, 104, 117, 126, 139, 155,
        169, 182, 195, 212, 225, 238, 254, 266, 281, 290, 301, 314, 325, 336, 346, 356,
        365, 374, 384, 393, 400, 410, 415, 424, 434, 441, 449, 457, 465, 471, 479, 487,
        494, 500, 508, 514, 520, 528, 533, 540, 546, 553, 559, 566, 571, 578, 583, 588,
        593, 598, 605, 609, 615, 619, 624, 629, 633, 638, 642, 647, 650, 654, 658, 661,
        665, 669, 672, 675, 679, 682, 686, 690, 693, 696, 700, 703, 706, 709, 712, 715,
        718, 721, 724, 728, 730, 733, 736, 740, 743, 745, 748, 751, 753, 756, 759, 762,
        765, 768, 770, 772, 775, 777, 781, 784, 786, 789, 792, 794, 797, 799, 802, 804,
        807, 809, 811, 814, 816, 818, 821, 824, 826, 828, 831, 833, 835, 837, 839, 842,
        844, 846, 848, 850, 852, 854, 856, 858, 859, 861, 863, 865, 866, 869, 871, 873,
        875, 876, 878, 880, 882, 883, 885, 886, 888, 890, 892, 893, 895, 897, 898, 900,
        901, 904, 905, 906, 908, 910, 911, 914, 916, 917, 919, 920, 922, 924, 926, 927,
        928, 930, 932, 933, 935, 936, 938, 939, 941, 943, 944, 946, 947, 949, 950, 952,
        954, 955, 958, 959, 961, 962, 964, 965, 967, 968, 969, 971, 972, 974, 976, 977,
        979, 980, 982, 983, 985, 986, 987, 989, 990, 992, 993, 995, 996, 997, 999,1002,
        1003,1004,1006,1007,1008,1010,1011,1012,1014,1015,1017,1018,1019,1021,1022,1023
    },
    {
        0,   8,  14,  24,  31,  40,  51,  59,  71,  83,  94, 104, 117, 126, 139, 155,
        169, 182, 195, 212, 225, 238, 254, 266, 281, 290, 301, 314, 325, 336, 346, 356,
        365, 374, 384, 393, 400, 410, 415, 424, 434, 441, 449, 457, 465, 471, 479, 487,
        494, 500, 508, 514, 520, 528, 533, 540, 546, 553, 559, 566, 571, 578, 583, 588,
        593, 598, 605, 609, 615, 619, 624, 629, 633, 638, 642, 647, 650, 654, 658, 661,
        665, 669, 672, 675, 679, 682, 686, 690, 693, 696, 700, 703, 706, 709, 712, 715,
        718, 721, 724, 728, 730, 733, 736, 740, 743, 745, 748, 751, 753, 756, 759, 762,
        765, 768, 770, 772, 775, 777, 781, 784, 786, 789, 792, 794, 797, 799, 802, 804,
        807, 809, 811, 814, 816, 818, 821, 824, 826, 828, 831, 833, 835, 837, 839, 842,
        844, 846, 848, 850, 852, 854, 856, 858, 859, 861, 863, 865, 866, 869, 871, 873,
        875, 876, 878, 880, 882, 883, 885, 886, 888, 890, 892, 893, 895, 897, 898, 900,
        901, 904, 905, 906, 908, 910, 911, 914, 916, 917, 919, 920, 922, 924, 926, 927,
        928, 930, 932, 933, 935, 936, 938, 939, 941, 943, 944, 946, 947, 949, 950, 952,
        954, 955, 958, 959, 961, 962, 964, 965, 967, 968, 969, 971, 972, 974, 976, 977,
        979, 980, 982, 983, 985, 986, 987, 989, 990, 992, 993, 995, 996, 997, 999,1002,
        1003,1004,1006,1007,1008,1010,1011,1012,1014,1015,1017,1018,1019,1021,1022,1023
    },
    {
        0,   8,  14,  24,  31,  40,  51,  59,  71,  83,  94, 104, 117, 126, 139, 155,
        169, 182, 195, 212, 225, 238, 254, 266, 281, 290, 301, 314, 325, 336, 346, 356,
        365, 374, 384, 393, 400, 410, 415, 424, 434, 441, 449, 457, 465, 471, 479, 487,
        494, 500, 508, 514, 520, 528, 533, 540, 546, 553, 559, 566, 571, 578, 583, 588,
        593, 598, 605, 609, 615, 619, 624, 629, 633, 638, 642, 647, 650, 654, 658, 661,
        665, 669, 672, 675, 679, 682, 686, 690, 693, 696, 700, 703, 706, 709, 712, 715,
        718, 721, 724, 728, 730, 733, 736, 740, 743, 745, 748, 751, 753, 756, 759, 762,
        765, 768, 770, 772, 775, 777, 781, 784, 786, 789, 792, 794, 797, 799, 802, 804,
        807, 809, 811, 814, 816, 818, 821, 824, 826, 828, 831, 833, 835, 837, 839, 842,
        844, 846, 848, 850, 852, 854, 856, 858, 859, 861, 863, 865, 866, 869, 871, 873,
        875, 876, 878, 880, 882, 883, 885, 886, 888, 890, 892, 893, 895, 897, 898, 900,
        901, 904, 905, 906, 908, 910, 911, 914, 916, 917, 919, 920, 922, 924, 926, 927,
        928, 930, 932, 933, 935, 936, 938, 939, 941, 943, 944, 946, 947, 949, 950, 952,
        954, 955, 958, 959, 961, 962, 964, 965, 967, 968, 969, 971, 972, 974, 976, 977,
        979, 980, 982, 983, 985, 986, 987, 989, 990, 992, 993, 995, 996, 997, 999,1002,
        1003,1004,1006,1007,1008,1010,1011,1012,1014,1015,1017,1018,1019,1021,1022,1023
    }
};

static MI_U16 RGBGamma_H[3][256] =
{
    {
        0,   7,  14,  22,  31,  39,  49,  58,  68,  78,  88,  98, 109, 119, 130, 141,
        152, 163, 174, 185, 196, 207, 218, 228, 238, 248, 258, 268, 278, 287, 297, 306,
        315, 324, 333, 342, 350, 359, 367, 375, 383, 391, 399, 407, 414, 422, 429, 437,
        444, 451, 458, 465, 472, 478, 485, 491, 498, 504, 510, 516, 522, 527, 533, 539,
        544, 550, 556, 561, 566, 572, 577, 582, 587, 592, 597, 602, 607, 612, 617, 621,
        626, 631, 635, 640, 643, 647, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688,
        692, 696, 700, 704, 708, 712, 716, 720, 724, 728, 732, 735, 739, 743, 747, 751,
        754, 759, 764, 768, 772, 775, 779, 783, 787, 790, 794, 797, 800, 804, 807, 810,
        813, 816, 819, 822, 825, 828, 831, 834, 836, 839, 842, 844, 847, 848, 851, 853,
        855, 857, 860, 862, 864, 867, 869, 872, 874, 876, 879, 881, 884, 886, 889, 891,
        894, 897, 899, 902, 904, 907, 909, 912, 914, 917, 919, 921, 923, 926, 928, 930,
        932, 934, 936, 938, 940, 942, 944, 945, 947, 949, 951, 953, 954, 956, 958, 959,
        961, 963, 964, 966, 968, 969, 971, 972, 974, 975, 977, 978, 979, 981, 982, 984,
        985, 986, 987, 989, 990, 991, 992, 993, 995, 996, 997, 998, 999,1000,1001,1002,
        1003,1004,1005,1005,1006,1007,1008,1009,1009,1010,1011,1012,1012,1013,1013,1014,
        1015,1015,1016,1016,1016,1017,1017,1018,1018,1018,1019,1019,1019,1019,1019,1019
    },
    {
        0,   7,  14,  22,  31,  39,  49,  58,  68,  78,  88,  98, 109, 119, 130, 141,
        152, 163, 174, 185, 196, 207, 218, 228, 238, 248, 258, 268, 278, 287, 297, 306,
        315, 324, 333, 342, 350, 359, 367, 375, 383, 391, 399, 407, 414, 422, 429, 437,
        444, 451, 458, 465, 472, 478, 485, 491, 498, 504, 510, 516, 522, 527, 533, 539,
        544, 550, 556, 561, 566, 572, 577, 582, 587, 592, 597, 602, 607, 612, 617, 621,
        626, 631, 635, 640, 643, 647, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688,
        692, 696, 700, 704, 708, 712, 716, 720, 724, 728, 732, 735, 739, 743, 747, 751,
        754, 759, 764, 768, 772, 775, 779, 783, 787, 790, 794, 797, 800, 804, 807, 810,
        813, 816, 819, 822, 825, 828, 831, 834, 836, 839, 842, 844, 847, 848, 851, 853,
        855, 857, 860, 862, 864, 867, 869, 872, 874, 876, 879, 881, 884, 886, 889, 891,
        894, 897, 899, 902, 904, 907, 909, 912, 914, 917, 919, 921, 923, 926, 928, 930,
        932, 934, 936, 938, 940, 942, 944, 945, 947, 949, 951, 953, 954, 956, 958, 959,
        961, 963, 964, 966, 968, 969, 971, 972, 974, 975, 977, 978, 979, 981, 982, 984,
        985, 986, 987, 989, 990, 991, 992, 993, 995, 996, 997, 998, 999,1000,1001,1002,
        1003,1004,1005,1005,1006,1007,1008,1009,1009,1010,1011,1012,1012,1013,1013,1014,
        1015,1015,1016,1016,1016,1017,1017,1018,1018,1018,1019,1019,1019,1019,1019,1019
    },
    {
        0,   7,  14,  22,  31,  39,  49,  58,  68,  78,  88,  98, 109, 119, 130, 141,
        152, 163, 174, 185, 196, 207, 218, 228, 238, 248, 258, 268, 278, 287, 297, 306,
        315, 324, 333, 342, 350, 359, 367, 375, 383, 391, 399, 407, 414, 422, 429, 437,
        444, 451, 458, 465, 472, 478, 485, 491, 498, 504, 510, 516, 522, 527, 533, 539,
        544, 550, 556, 561, 566, 572, 577, 582, 587, 592, 597, 602, 607, 612, 617, 621,
        626, 631, 635, 640, 643, 647, 652, 656, 660, 664, 668, 672, 676, 680, 684, 688,
        692, 696, 700, 704, 708, 712, 716, 720, 724, 728, 732, 735, 739, 743, 747, 751,
        754, 759, 764, 768, 772, 775, 779, 783, 787, 790, 794, 797, 800, 804, 807, 810,
        813, 816, 819, 822, 825, 828, 831, 834, 836, 839, 842, 844, 847, 848, 851, 853,
        855, 857, 860, 862, 864, 867, 869, 872, 874, 876, 879, 881, 884, 886, 889, 891,
        894, 897, 899, 902, 904, 907, 909, 912, 914, 917, 919, 921, 923, 926, 928, 930,
        932, 934, 936, 938, 940, 942, 944, 945, 947, 949, 951, 953, 954, 956, 958, 959,
        961, 963, 964, 966, 968, 969, 971, 972, 974, 975, 977, 978, 979, 981, 982, 984,
        985, 986, 987, 989, 990, 991, 992, 993, 995, 996, 997, 998, 999,1000,1001,1002,
        1003,1004,1005,1005,1006,1007,1008,1009,1009,1010,1011,1012,1012,1013,1013,1014,
        1015,1015,1016,1016,1016,1017,1017,1018,1018,1018,1019,1019,1019,1019,1019,1019
    }
};


static MI_U16 YUVGamma_L[3][256] =
{
    {
        0,  4,   8,   12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56, 60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 173, 177, 181, 185, 189,
        193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 237, 241, 245, 249, 253,
        257, 261, 265, 269, 273, 277, 281, 285, 289, 293, 297, 301, 305, 309, 313, 317,
        321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 365, 369, 373, 377, 381,
        385, 389, 393, 397, 401, 405, 409, 413, 417, 421, 425, 429, 433, 437, 441, 445,
        449, 453, 457, 461, 465, 469, 473, 477, 481, 485, 489, 493, 497, 501, 505, 509,
        514, 518, 522, 526, 530, 534, 538, 542, 546, 550, 554, 558, 562, 566, 570, 574,
        578, 582, 586, 590, 594, 598, 602, 606, 610, 614, 618, 622, 626, 630, 634, 638,
        642, 646, 650, 654, 658, 662, 666, 670, 674, 678, 682, 686, 690, 694, 698, 702,
        706, 710, 714, 718, 722, 726, 730, 734, 738, 742, 746, 750, 754, 758, 762, 766,
        770, 774, 778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 818, 822, 826, 830,
        834, 838, 842, 846, 850, 855, 859, 863, 867, 871, 875, 879, 883, 887, 891, 895,
        899, 903, 907, 911, 915, 919, 923, 927, 931, 935, 939, 943, 947, 951, 955, 959,
        963, 967, 971, 975, 979, 983, 987, 991, 995, 999,1003,1007,1011,1015,1019,1023
    },
    {
        0,  4,  8,    12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
        192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
        256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
        384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444,
        448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496, 500, 504, 511
    },
    {
        0,  4,  8,    12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
        192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
        256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
        384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444,
        448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496, 500, 504, 511
    },
};

static MI_U16 YUVGamma_N[3][256] =
{
    {
        0,  11,  22,  33,  42,  51,  60,  68,  76,  83,  90,  96, 102, 108, 113, 118,
        123, 126, 130, 133, 136, 140, 143, 146, 150, 153, 156, 159, 162, 165, 168, 171,
        174, 176, 179, 182, 184, 187, 190, 192, 195, 197, 200, 202, 205, 207, 210, 212,
        215, 217, 219, 222, 224, 227, 229, 232, 234, 237, 240, 242, 245, 248, 251, 253,
        257, 260, 263, 266, 269, 273, 276, 280, 284, 288, 292, 296, 301, 306, 310, 315,
        321, 325, 329, 333, 337, 341, 345, 349, 353, 357, 361, 365, 369, 373, 377, 381,
        385, 389, 393, 397, 401, 405, 409, 413, 417, 421, 425, 429, 433, 437, 441, 445,
        449, 453, 457, 461, 465, 469, 473, 477, 481, 485, 489, 493, 497, 501, 505, 509,
        514, 518, 522, 526, 530, 534, 538, 542, 546, 550, 554, 558, 562, 566, 570, 574,
        578, 582, 586, 590, 594, 598, 602, 606, 610, 614, 618, 622, 626, 630, 634, 638,
        642, 646, 650, 654, 658, 662, 666, 670, 674, 678, 682, 686, 690, 694, 698, 702,
        706, 710, 714, 718, 722, 726, 730, 734, 738, 742, 746, 750, 754, 758, 762, 766,
        770, 774, 778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 818, 822, 826, 830,
        834, 838, 842, 846, 850, 855, 859, 863, 867, 871, 875, 879, 883, 887, 891, 895,
        899, 903, 907, 911, 915, 919, 923, 927, 931, 935, 939, 943, 947, 951, 955, 959,
        963, 967, 971, 975, 979, 983, 987, 991, 995, 999,1003,1007,1011,1015,1019,1023
    },
    {
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
        192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
        256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
        384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444,
        448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496, 500, 504, 511
    },
    {
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
        192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
        256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
        384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444,
        448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496, 500, 504, 511
    }
};

static MI_U16 YUVGamma_H[3][256] =
{
    {
        0,  14,  28,  42,  55,  67,  79,  90, 100, 111, 120, 130, 139, 147, 155, 163,
        171, 178, 184, 191, 197, 203, 209, 214, 219, 224, 229, 234, 238, 243, 247, 251,
        254, 258, 262, 265, 269, 272, 275, 278, 281, 284, 287, 290, 293, 296, 298, 301,
        304, 306, 308, 311, 313, 315, 318, 320, 322, 324, 326, 328, 329, 331, 333, 334,
        336, 337, 339, 341, 343, 345, 347, 349, 351, 353, 355, 356, 358, 360, 362, 364,
        365, 367, 369, 371, 373, 375, 377, 379, 380, 382, 384, 387, 389, 391, 393, 395,
        397, 400, 402, 405, 408, 410, 413, 416, 419, 422, 426, 429, 433, 436, 440, 444,
        449, 452, 456, 459, 463, 467, 471, 475, 479, 483, 487, 491, 495, 500, 504, 509,
        513, 518, 522, 526, 530, 534, 538, 542, 546, 550, 554, 558, 562, 566, 570, 574,
        578, 582, 586, 590, 594, 598, 602, 606, 610, 614, 618, 622, 626, 630, 634, 638,
        642, 646, 650, 654, 658, 662, 666, 670, 674, 678, 682, 686, 690, 694, 698, 702,
        706, 710, 714, 718, 722, 726, 730, 734, 738, 742, 746, 750, 754, 758, 762, 766,
        770, 774, 778, 782, 786, 790, 794, 798, 802, 806, 810, 814, 818, 822, 826, 830,
        834, 838, 842, 846, 850, 855, 859, 863, 867, 871, 875, 879, 883, 887, 891, 895,
        899, 903, 907, 911, 915, 919, 923, 927, 931, 935, 939, 943, 947, 951, 955, 959,
        963, 967, 971, 975, 979, 983, 987, 991, 995, 999,1003,1007,1011,1015,1019, 1023
    },
    {
        0,  4,    8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56,  60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
        192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
        256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
        384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444,
        448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496, 500, 504, 511
    },
    {
        0,   4,   8,  12,  16,  20,  24,  28,  32,  36,  40,  44,  48,  52,  56, 60,
        64,  68,  72,  76,  80,  84,  88,  92,  96, 100, 104, 108, 112, 116, 120, 124,
        128, 132, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188,
        192, 196, 200, 204, 208, 212, 216, 220, 224, 228, 232, 236, 240, 244, 248, 252,
        256, 260, 264, 268, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316,
        320, 324, 328, 332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380,
        384, 388, 392, 396, 400, 404, 408, 412, 416, 420, 424, 428, 432, 436, 440, 444,
        448, 452, 456, 460, 464, 468, 472, 476, 480, 484, 488, 492, 496, 500, 504, 511
    }
};

static int IspColorGray(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    bool enable = ss_cmd_atoi(in_strs[1].c_str());

    MI_ISP_IQ_ColorToGrayType_t tColor2Gray;

    tColor2Gray.bEnable = (MI_ISP_IQ_Bool_e)enable;
    MI_ISP_IQ_SetColorToGray(dev, chn, &tColor2Gray);

    ss_print(PRINT_LV_TRACE,"dev(%d) ch(%d) gray enable = %d \n", dev,chn, enable);
    return 0;
}

static int IspAwb(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    auto mode = in_strs[1];
    int Bgain = ss_cmd_atoi(in_strs[2].c_str());
    int Gbgian = ss_cmd_atoi(in_strs[3].c_str());
    int Grgain = ss_cmd_atoi(in_strs[4].c_str());
    int Rgain = ss_cmd_atoi(in_strs[5].c_str());

    MI_ISP_AWB_AttrType_t _awbAttr;

    MI_ISP_AWB_GetAttr(dev, chn, &_awbAttr);

    if(mode == "auto")
        _awbAttr.eOpType =  E_SS_AWB_MODE_AUTO;
    else
        _awbAttr.eOpType =  E_SS_AWB_MODE_MANUAL;
    _awbAttr.stManualParaAPI.u16Bgain = (MI_U16)Bgain;
    _awbAttr.stManualParaAPI.u16Gbgain = (MI_U16)Gbgian;
    _awbAttr.stManualParaAPI.u16Grgain = (MI_U16)Grgain;
    _awbAttr.stManualParaAPI.u16Rgain = (MI_U16)Rgain;

    MI_ISP_AWB_SetAttr(dev, chn, &_awbAttr);

    ss_print(PRINT_LV_TRACE,"dev(%d) ch(%d) mode = %s \n", dev,chn, mode.c_str());
    ss_print(PRINT_LV_TRACE,"Bgain:%d.\n", _awbAttr.stManualParaAPI.u16Bgain);
    ss_print(PRINT_LV_TRACE,"Gbgain:%d.\n", _awbAttr.stManualParaAPI.u16Gbgain);
    ss_print(PRINT_LV_TRACE,"Grgain:%d.\n", _awbAttr.stManualParaAPI.u16Grgain);
    ss_print(PRINT_LV_TRACE,"Rgain:%d.\n", _awbAttr.stManualParaAPI.u16Rgain);
    return 0;
}

static int IspAeStrate(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int chn = stInfo.chnId;
    auto mode = in_strs[1];
    //brighttone, darktone, auto

    MI_ISP_AE_StrategyType_t sSetAeStrategy;
    MI_ISP_AE_GetStrategy(dev, chn, &sSetAeStrategy);

    if("brighttone" == mode)
    {
        ss_print(PRINT_LV_TRACE,"AE_SetStrategy eAEStrategyMode:BrightPriority\n");
        sSetAeStrategy.eAEStrategyMode = E_SS_AE_STRATEGY_BRIGHTTONE;
        sSetAeStrategy.stLowerOffset.u16NumOfPoints = 3;
        sSetAeStrategy.stLowerOffset.u32X[0] = -65536;
        sSetAeStrategy.stLowerOffset.u32X[1] = 0;
        sSetAeStrategy.stLowerOffset.u32X[2] = 81920;
        sSetAeStrategy.stLowerOffset.u32Y[0] = 200;
        sSetAeStrategy.stLowerOffset.u32Y[1] = 200;
        sSetAeStrategy.stLowerOffset.u32Y[2] = 200;
        sSetAeStrategy.stUpperOffset.u16NumOfPoints = 3;
        sSetAeStrategy.stUpperOffset.u32X[0] = -65536;
        sSetAeStrategy.stUpperOffset.u32X[1] = 0;
        sSetAeStrategy.stUpperOffset.u32X[2] = 81920;
        sSetAeStrategy.stUpperOffset.u32Y[0] = 0;
        sSetAeStrategy.stUpperOffset.u32Y[1] = 0;
        sSetAeStrategy.stUpperOffset.u32Y[2] = 0;
        sSetAeStrategy.u32AutoSensitivity=0;
        sSetAeStrategy.u32Weighting=1024;
        sSetAeStrategy.u32AutoStrength=0;
        sSetAeStrategy.u32BrightToneSensitivity=1024;
        sSetAeStrategy.u32BrightToneStrength=1024;
        sSetAeStrategy.u32DarkToneSensitivity=0;
        sSetAeStrategy.u32DarkToneStrength=0;
    }
    else if("darktone" == mode)
    {
        ss_print(PRINT_LV_TRACE,"AE_SetStrategy eAEStrategyMode:DarkPriority\n");
        sSetAeStrategy.eAEStrategyMode = E_SS_AE_STRATEGY_DARKTONE;
        sSetAeStrategy.stLowerOffset.u16NumOfPoints = 3;
        sSetAeStrategy.stLowerOffset.u32Y[0] = -65536;
        sSetAeStrategy.stLowerOffset.u32Y[1] = 0;
        sSetAeStrategy.stLowerOffset.u32Y[2] = 81920;
        sSetAeStrategy.stLowerOffset.u32Y[0] = 0;
        sSetAeStrategy.stLowerOffset.u32Y[1] = 0;
        sSetAeStrategy.stLowerOffset.u32Y[2] = 0;
        sSetAeStrategy.stUpperOffset.u16NumOfPoints = 3;
        sSetAeStrategy.stUpperOffset.u32Y[0] = -65536;
        sSetAeStrategy.stUpperOffset.u32Y[1] = 0;
        sSetAeStrategy.stUpperOffset.u32Y[2] = 81920;
        sSetAeStrategy.stUpperOffset.u32Y[0] = 200;
        sSetAeStrategy.stUpperOffset.u32Y[1] = 200;
        sSetAeStrategy.stUpperOffset.u32Y[2] = 200;
        sSetAeStrategy.u32AutoSensitivity=0;
        sSetAeStrategy.u32Weighting=1024;
        sSetAeStrategy.u32AutoStrength=0;
        sSetAeStrategy.u32BrightToneSensitivity=0;
        sSetAeStrategy.u32BrightToneStrength=0;
        sSetAeStrategy.u32DarkToneSensitivity=1024;
        sSetAeStrategy.u32DarkToneStrength=1024;
    }
    else
    {
        ss_print(PRINT_LV_TRACE,"AE_SetStrategy eAEStrategyMode:Auto mode\n");
        sSetAeStrategy.eAEStrategyMode = E_SS_AE_STRATEGY_AUTO;
        sSetAeStrategy.stLowerOffset.u16NumOfPoints = 3;
        sSetAeStrategy.stLowerOffset.u32Y[0] = -65536;
        sSetAeStrategy.stLowerOffset.u32Y[1] = 0;
        sSetAeStrategy.stLowerOffset.u32Y[2] = 81920;
        sSetAeStrategy.stLowerOffset.u32Y[0] = 0;
        sSetAeStrategy.stLowerOffset.u32Y[1] = 0;
        sSetAeStrategy.stLowerOffset.u32Y[2] = 0;
        sSetAeStrategy.stUpperOffset.u16NumOfPoints = 3;
        sSetAeStrategy.stUpperOffset.u32Y[0] = -65536;
        sSetAeStrategy.stUpperOffset.u32Y[1] = 0;
        sSetAeStrategy.stUpperOffset.u32Y[2] = 81920;
        sSetAeStrategy.stUpperOffset.u32Y[0] = 0;
        sSetAeStrategy.stUpperOffset.u32Y[1] = 0;
        sSetAeStrategy.stUpperOffset.u32Y[2] = 0;
        sSetAeStrategy.u32AutoSensitivity=1024;
        sSetAeStrategy.u32Weighting=1024;
        sSetAeStrategy.u32AutoStrength=1024;
        sSetAeStrategy.u32BrightToneSensitivity=0;
        sSetAeStrategy.u32BrightToneStrength=0;
        sSetAeStrategy.u32DarkToneSensitivity=0;
        sSetAeStrategy.u32DarkToneStrength=0;
    }

    MI_ISP_AE_SetStrategy(dev, chn, &sSetAeStrategy);
    ss_print(PRINT_LV_TRACE,"MI_ISP_SetExposure %s\n", mode.c_str());
    return 0;
}

static int IspContrast(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nValue = ss_cmd_atoi(in_strs[1].c_str());

    MI_ISP_IQ_ContrastType_t Contrast;
    MI_ISP_IQ_GetContrast(dev, ch, &Contrast);

    Contrast.bEnable = (MI_ISP_IQ_Bool_e)TRUE;

    Contrast.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    Contrast.stManual.stParaAPI.u32Lev = (MI_U32)nValue;
    MI_ISP_IQ_SetContrast(dev, ch, &Contrast);
    ss_print(PRINT_LV_TRACE,"Contrast:%d\n", nValue);
    return 0;
}

static int IspBrightness(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nValue = ss_cmd_atoi(in_strs[1].c_str());

    MI_ISP_IQ_BrightnessType_t Brightness;
    MI_ISP_IQ_GetBrightness(dev, ch, &Brightness);

    Brightness.bEnable = (MI_ISP_IQ_Bool_e)TRUE;

    Brightness.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    Brightness.stManual.stParaAPI.u32Lev = (MI_U32)nValue;
    MI_ISP_IQ_SetBrightness(dev, ch, &Brightness);
    ss_print(PRINT_LV_TRACE,"Brightness:%d\n", nValue);
    return 0;
}

static int IspRgbGamma(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    auto GammaMode = in_strs[1];
    auto mode = in_strs[2];
    int nValue = 1;
    MI_U32 j = 0x0;

    if("normal" == mode)
        nValue = 2;
    else if("high" == mode)
        nValue = 3;

    if("rgb" == GammaMode)    //RGBGamma
    {
        MI_U16 *p_RGBGAMA_R = NULL;
        MI_U16 *p_RGBGAMA_G = NULL;
        MI_U16 *p_RGBGAMA_B = NULL;
        MI_ISP_IQ_RgbGammaType_t *Gamma = new MI_ISP_IQ_RgbGammaType_t;
        if(NULL == Gamma)
            return 0;

        MI_ISP_IQ_GetRgbGamma(dev, ch, Gamma);

        Gamma->bEnable = (MI_ISP_IQ_Bool_e)TRUE;
        Gamma->enOpType = E_SS_IQ_OP_TYP_MANUAL;

        //nValue;  1 Low contrast, 2 Normal, 3 High contrast

        switch(nValue)
        {
            case 1:
                p_RGBGAMA_R = RGBGamma_L[0];
                p_RGBGAMA_G = RGBGamma_L[1];
                p_RGBGAMA_B = RGBGamma_L[2];
                break;

            case 2:
                p_RGBGAMA_R = RGBGamma_N[0];
                p_RGBGAMA_G = RGBGamma_N[1];
                p_RGBGAMA_B = RGBGamma_N[2];
                break;

            case 3:
                p_RGBGAMA_R = RGBGamma_H[0];
                p_RGBGAMA_G = RGBGamma_H[1];
                p_RGBGAMA_B = RGBGamma_H[2];
                break;

            default:
                ss_print(PRINT_LV_TRACE,"%s:%d Input wrong RGBGamma index:%d, only support 1, 2, or 3.\n", __func__, __LINE__, nValue);
                delete Gamma;
                Gamma = NULL;
                return -1;
        }

        //printf("%s:%d p_RGBGAMA_R=%p, p_RGBGAMA_G=%p, p_RGBGAMA_B=%p\n", __func__, __LINE__, p_RGBGAMA_R, p_RGBGAMA_G, p_RGBGAMA_B);

        for(j = 0; j < 256; j++)
        {
            Gamma->stManual.stParaAPI.u16LutB[j] = *(p_RGBGAMA_B + j);
            Gamma->stManual.stParaAPI.u16LutG[j] = *(p_RGBGAMA_G + j);
            Gamma->stManual.stParaAPI.u16LutR[j] = *(p_RGBGAMA_R + j);
            //printf("%s:%d p_RGBGAMA_R+%d=%d, p_RGBGAMA_G+%d=%d, p_RGBGAMA_B+%d=%d\n", __func__, __LINE__,j,*(p_RGBGAMA_R + j),j,*(p_RGBGAMA_G + j),j,*(p_RGBGAMA_B + j));
        }

        MI_ISP_IQ_SetRgbGamma(dev, ch, Gamma);

        for(j = 0; j < 256; j++)
        {
            ss_print(PRINT_LV_TRACE,"Gamma R:[%hu] Gamma G:[%hu] Gamma b:[%hu]\n", Gamma->stManual.stParaAPI.u16LutR[j],
                   Gamma->stManual.stParaAPI.u16LutG[j],
                   Gamma->stManual.stParaAPI.u16LutB[j]);
        }
        delete Gamma;
        Gamma = NULL;
        return 0;
    }

    if("yuv" == GammaMode)
    {
        MI_U16 *p_YUVGAMA_Y = NULL;
        MI_U16 *p_YUVGAMA_U = NULL;
        MI_U16 *p_YUVGAMA_V = NULL;
        MI_ISP_IQ_YuvGammaType_t *yuvgamma = new MI_ISP_IQ_YuvGammaType_t;
        if(NULL == yuvgamma)
            return 0;

        MI_ISP_IQ_GetYuvGamma(dev, ch, yuvgamma);

        yuvgamma->bEnable = (MI_ISP_IQ_Bool_e)TRUE;
        yuvgamma->enOpType = E_SS_IQ_OP_TYP_MANUAL;

        //nValue;  // 1 Low contrast, 2 Normal, 3 High contrast
        switch(nValue)
        {
            case 1:
                p_YUVGAMA_Y = YUVGamma_L[0];
                p_YUVGAMA_U = YUVGamma_L[1];
                p_YUVGAMA_V = YUVGamma_L[2];
                break;

            case 2:
                p_YUVGAMA_Y = YUVGamma_N[0];
                p_YUVGAMA_U = YUVGamma_N[1];
                p_YUVGAMA_V = YUVGamma_N[2];
                break;

            case 3:
                p_YUVGAMA_Y = YUVGamma_H[0];
                p_YUVGAMA_U = YUVGamma_H[1];
                p_YUVGAMA_V = YUVGamma_H[2];
                break;

            default:
                ss_print(PRINT_LV_TRACE,"%s:%d Input wrong YUVGamma index:%d, only support 1, 2, or 3.\n", __func__, __LINE__, nValue);
                delete yuvgamma;
                yuvgamma = NULL;
                return -1;
        }

        //printf("%s:%d p_YUVGAMA_Y=%p, p_YUVGAMA_U=%p, p_YUVGAMA_V=%p\n", __func__, __LINE__, p_YUVGAMA_Y, p_YUVGAMA_U, p_YUVGAMA_V);

        for(j = 0; j < 128; j++)
        {
            yuvgamma->stManual.stParaAPI.u16LutY[j] = *(p_YUVGAMA_Y + j);
            yuvgamma->stManual.stParaAPI.s16LutV[j] = *(p_YUVGAMA_V + j);
            yuvgamma->stManual.stParaAPI.s16LutU[j] = *(p_YUVGAMA_U + j);
            ss_print(PRINT_LV_TRACE,"Gamma Y:[%hu] Gamma V:[%hu] Gamma U:[%hu]\n", yuvgamma->stManual.stParaAPI.u16LutY[j],
                   yuvgamma->stManual.stParaAPI.s16LutV[j],
                   yuvgamma->stManual.stParaAPI.s16LutU[j]);
        }

        for(j = 128; j < 256; j++)
        {
            yuvgamma->stManual.stParaAPI.u16LutY[j] = *(p_YUVGAMA_Y + j);
            ss_print(PRINT_LV_TRACE,"Gamma Y:[%hu]\n", yuvgamma->stManual.stParaAPI.u16LutY[j]);
        }

        MI_ISP_IQ_SetYuvGamma(dev, ch, yuvgamma);
        delete yuvgamma;
        yuvgamma = NULL;
        return 0;
    }
    return -1;
}

static int IspSatration(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nValue = ss_cmd_atoi(in_strs[1].c_str());

    MI_ISP_IQ_SaturationType_t Saturation;

    MI_ISP_IQ_GetSaturation(dev, ch, &Saturation);

    if(nValue > 127)
    {
        ss_print(PRINT_LV_TRACE,"IQ_SetSaturation u8SatAllStr [0,127], %d\n", nValue);
        nValue = 127;
    }

    Saturation.stManual.stParaAPI.u8SatAllStr = (MI_U8)nValue;//0~127

    Saturation.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    Saturation.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    MI_ISP_IQ_SetSaturation(dev, ch, &Saturation);

    ss_print(PRINT_LV_TRACE,"Saturation:%u\n", Saturation.stManual.stParaAPI.u8SatAllStr);
    return 0;
}

static int IspLightness(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nValue = ss_cmd_atoi(in_strs[1].c_str());

    MI_ISP_IQ_LightnessType_t Lightness;

    MI_ISP_IQ_GetLightness(dev, ch, &Lightness);

    if(nValue > 100)
    {
        ss_print(PRINT_LV_TRACE,"IQ_LightNess value:[0,100], %d\n", nValue);
        nValue = 100;
    }

    Lightness.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    Lightness.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    Lightness.stManual.stParaAPI.u32Lev = (MI_U32)nValue;
    MI_ISP_IQ_SetLightness(dev, ch, &Lightness);

    ss_print(PRINT_LV_TRACE,"set dev(%d),chn(%d),Lev(%d)\n", dev,ch,nValue);
    return 0;
}

static int IspSharpness(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nUDValue = ss_cmd_atoi(in_strs[1].c_str());
    int nDValue = ss_cmd_atoi(in_strs[2].c_str());

    MI_ISP_IQ_SharpnessType_t Sharpnessv1;
    MI_ISP_IQ_GetSharpness(dev, ch, &Sharpnessv1);

    Sharpnessv1.bEnable = (MI_ISP_IQ_Bool_e)TRUE;

    Sharpnessv1.enOpType = E_SS_IQ_OP_TYP_MANUAL;

    if(nUDValue > 127)
    {
        ss_print(PRINT_LV_TRACE,"UD out of rang, [0, 127], %d\n", nUDValue);
        nUDValue = 127;
    }
    if(nDValue > 127)
    {
        ss_print(PRINT_LV_TRACE,"U out of rang, [0, 127], %d\n", nDValue);
        nDValue = 127;
    }

    Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[0] = nUDValue;
    Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[1] = nUDValue;
    Sharpnessv1.stManual.stParaAPI.u8SharpnessUD[2] = nUDValue;
    Sharpnessv1.stManual.stParaAPI.u8SharpnessD[0] = nDValue;
    Sharpnessv1.stManual.stParaAPI.u8SharpnessD[1] = nDValue;
    Sharpnessv1.stManual.stParaAPI.u8SharpnessD[2] = nDValue;

    MI_ISP_IQ_SetSharpness(dev, ch, &Sharpnessv1);

    ss_print(PRINT_LV_TRACE,"set dev(%d),chn(%d),UDValue(%d),DValue(%d)\n", dev,ch,nUDValue,nDValue);
    return 0;
}

static int IspAeFlicker(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    MI_ISP_AE_FlickerType_e type = ss_enum_cast<MI_ISP_AE_FlickerType_e>::from_str(in_strs[1].c_str());

    MI_ISP_AE_FlickerType_e Flicker;

    MI_ISP_AE_GetFlicker(dev, ch, &Flicker);

    Flicker = (MI_ISP_AE_FlickerType_e)type;
    if(Flicker < E_SS_AE_FLICKER_TYPE_DISABLE || Flicker > E_SS_AE_FLICKER_TYPE_MAX)
    {
        Flicker = E_SS_AE_FLICKER_TYPE_DISABLE;
    }
    MI_ISP_AE_SetFlicker(dev, ch, &Flicker);
    return 0;
}

static int IspAeFlickerEx(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    MI_ISP_AE_FlickerDetectType_e type = ss_enum_cast<MI_ISP_AE_FlickerDetectType_e>::from_str(in_strs[1].c_str());

    MI_ISP_AE_FlickerExType_t Flicker;

    MI_ISP_AE_GetFlickerEx(dev, ch, &Flicker);

    if(E_SS_AE_FLICKER_TYPE_DETECT_60HZ == type || E_SS_AE_FLICKER_TYPE_DETECT_50HZ == type)
    {
        Flicker.bEnable = E_SS_AE_TRUE;
        Flicker.eFlickerType = type;
    }
    else
    {
        Flicker.bEnable = E_SS_AE_FALSE;
    }
    MI_ISP_AE_SetFlickerEx(dev, ch, &Flicker);
    ss_print(PRINT_LV_TRACE,"AeFlickerEx set %s\n", in_strs[1].c_str());

    return 0;
}

static int IspCcm(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nCCM[9];
    nCCM[0] = ss_cmd_atoi(in_strs[1].c_str());
    nCCM[1] = ss_cmd_atoi(in_strs[2].c_str());
    nCCM[2] = ss_cmd_atoi(in_strs[3].c_str());
    nCCM[3] = ss_cmd_atoi(in_strs[4].c_str());
    nCCM[4] = ss_cmd_atoi(in_strs[5].c_str());
    nCCM[5] = ss_cmd_atoi(in_strs[6].c_str());
    nCCM[6] = ss_cmd_atoi(in_strs[7].c_str());
    nCCM[7] = ss_cmd_atoi(in_strs[8].c_str());
    nCCM[8] = ss_cmd_atoi(in_strs[9].c_str());

    MI_U16 value = 0x0;
    int count = 0;
    MI_ISP_IQ_RgbMatrixType_t RGBMatrix;

    MI_ISP_IQ_GetRgbMatrix(dev, ch, &RGBMatrix);

    RGBMatrix.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    RGBMatrix.enOpType = E_SS_IQ_OP_TYP_MANUAL;

    for(count = 0; count < 9; count++)
    {
        value = nCCM[count];
        if( value > 8191)
        {
            ss_print(PRINT_LV_TRACE,"ccm value out of rang, [0, 8191], %d\n", value );
            value = 8191;
        }

        //input: 0 1 2     3 4 5    6 7 8
        //mapping: 0 1 2 3  4 5 6 7  8 9 10 11
        if (count >= 3 && count <=5)
            RGBMatrix.stManual.u16CCM[count+1] = value;
        else if (count >= 6 && count <=8)
            RGBMatrix.stManual.u16CCM[count+2] = value;
        else
            RGBMatrix.stManual.u16CCM[count] = value;
    }

    MI_ISP_IQ_SetRgbMatrix(dev, ch, &RGBMatrix);
    return 0;
}

static int IspFalseColor(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int nSMid = ss_cmd_atoi(in_strs[1].c_str());
    int nUMid = ss_cmd_atoi(in_strs[2].c_str());

    MI_ISP_IQ_FalseColorType_t FalseColorV1;

    MI_ISP_IQ_GetFalseColor(dev, ch, &FalseColorV1);
    FalseColorV1.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    FalseColorV1.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    FalseColorV1.stManual.stParaAPI.u8ColorSpaceSel = 1;
    if(nSMid > 7)
    {
        ss_print(PRINT_LV_TRACE,"u8Preserve value out of range, [0, 7], %d\n", nSMid);
        nSMid = 7;
    }

    if(nUMid > 31)
    {
        ss_print(PRINT_LV_TRACE,"u8Strength value out of range, [0, 31], %d\n", nUMid);
        nUMid = 31;
    }
    FalseColorV1.stManual.stParaAPI.u8Preserve = (MI_U8)nSMid;
    FalseColorV1.stManual.stParaAPI.u8Strength = (MI_U8)nUMid;
    MI_ISP_IQ_SetFalseColor(dev, ch, &FalseColorV1);
    ss_print(PRINT_LV_TRACE,"u8Preserve = %d\n", FalseColorV1.stManual.stParaAPI.u8Preserve);
    ss_print(PRINT_LV_TRACE,"u8Strength = %d\n", FalseColorV1.stManual.stParaAPI.u8Strength);
    return 0;
}

static int IspCrosstalk(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int u16Offset = ss_cmd_atoi(in_strs[1].c_str());
    int u8Threshold = ss_cmd_atoi(in_strs[2].c_str());
    int u8Strength = ss_cmd_atoi(in_strs[3].c_str());

    MI_ISP_IQ_CrosstalkType_t Crosstalk;

    MI_ISP_IQ_GetCrossTalk(dev, ch, &Crosstalk);

    Crosstalk.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    Crosstalk.enOpType = E_SS_IQ_OP_TYP_MANUAL;

    Crosstalk.stManual.stParaAPI.u16Offset = (MI_U16)u16Offset;
    if(Crosstalk.stManual.stParaAPI.u16Offset > 4095)
        Crosstalk.stManual.stParaAPI.u16Offset = 4095;

    Crosstalk.stManual.stParaAPI.u8Threshold = u8Threshold;
    if(Crosstalk.stManual.stParaAPI.u8Threshold > 255)
    {
        ss_print(PRINT_LV_TRACE,"u16ThresholdV2 value out of range, [0, 255], %d\n", u8Threshold);
        Crosstalk.stManual.stParaAPI.u8Threshold = 255;
    }
    Crosstalk.stManual.stParaAPI.u8Strength = u8Strength;
    if(Crosstalk.stManual.stParaAPI.u8Strength > 31)
    {
        ss_print(PRINT_LV_TRACE,"u16ThresholdV2 value out of range, [0, 31], %d\n", u8Strength);
        Crosstalk.stManual.stParaAPI.u8Strength = 31;
    }

    ss_print(PRINT_LV_TRACE,"u16Offset:%d, u8Threshold:%d, u8Strength:%d\n", \
              u16Offset,u8Threshold,u8Strength);
    MI_ISP_IQ_SetCrossTalk(dev, ch, &Crosstalk);
    return 0;
}

static int IspDp(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    bool bHotPixEn = ss_cmd_atoi(in_strs[1].c_str());
    int u16HotPixCompSlpoe = ss_cmd_atoi(in_strs[2].c_str());
    bool bDarkPixEn = ss_cmd_atoi(in_strs[3].c_str());
    int u16DarkPixCompSlpoe = ss_cmd_atoi(in_strs[4].c_str());

    MI_ISP_IQ_DynamicDpType_t DefectPixel;

    MI_ISP_IQ_GetDynamicDp(dev, ch, &DefectPixel);

    DefectPixel.stManual.stParaAPI.bHotPixEn = (MI_ISP_IQ_Bool_e)bHotPixEn;

    DefectPixel.stManual.stParaAPI.u16HotPixCompSlpoe = u16HotPixCompSlpoe;

    DefectPixel.stManual.stParaAPI.bDarkPixEn = (MI_ISP_IQ_Bool_e)bDarkPixEn;

    DefectPixel.stManual.stParaAPI.u16DarkPixCompSlpoe = (MI_U16)u16DarkPixCompSlpoe;

    DefectPixel.bEnable = (MI_ISP_IQ_Bool_e)TRUE;

    DefectPixel.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    MI_ISP_IQ_SetDynamicDp(dev, ch, &DefectPixel);
    return 0;
}

static int IspBlackLevel(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int b = ss_cmd_atoi(in_strs[1].c_str());
    int gb = ss_cmd_atoi(in_strs[2].c_str());
    int gr = ss_cmd_atoi(in_strs[3].c_str());
    int r = ss_cmd_atoi(in_strs[4].c_str());

    MI_ISP_IQ_ObcType_t BlackLevel;

    MI_ISP_IQ_GetObc(dev, ch, &BlackLevel);

    BlackLevel.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    BlackLevel.enOpType = E_SS_IQ_OP_TYP_MANUAL;

    BlackLevel.stManual.stParaAPI.u16ValB = b;
    if(BlackLevel.stManual.stParaAPI.u16ValB > 65535)
    {
        ss_print(PRINT_LV_TRACE,"ValB out of range. [0, 255]. %d\n",BlackLevel.stManual.stParaAPI.u16ValB );
        BlackLevel.stManual.stParaAPI.u16ValB = 65535;
    }

    BlackLevel.stManual.stParaAPI.u16ValGb = gb;
    if(BlackLevel.stManual.stParaAPI.u16ValGb > 65535)
    {
        ss_print(PRINT_LV_TRACE,"ValGB out of range. [0, 255]. %d\n", BlackLevel.stManual.stParaAPI.u16ValGb);
        BlackLevel.stManual.stParaAPI.u16ValGb = 65535;
    }

    BlackLevel.stManual.stParaAPI.u16ValGr = gr;
    if(BlackLevel.stManual.stParaAPI.u16ValGr > 65535)
    {
        ss_print(PRINT_LV_TRACE,"ValGB out of range. [0, 255]. %d\n", BlackLevel.stManual.stParaAPI.u16ValGr);
        BlackLevel.stManual.stParaAPI.u16ValGr = 65535;
    }

    BlackLevel.stManual.stParaAPI.u16ValR = r;
    if(BlackLevel.stManual.stParaAPI.u16ValR > 65535)
    {
        ss_print(PRINT_LV_TRACE,"ValGB out of range. [0, 255]. %d\n", BlackLevel.stManual.stParaAPI.u16ValR);
        BlackLevel.stManual.stParaAPI.u16ValR = 65535;
    }

    MI_ISP_IQ_SetObc(dev, ch, &BlackLevel);
    return 0;
}

static int IspDefog(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    bool enable = ss_cmd_atoi(in_strs[1].c_str());
    int strength = ss_cmd_atoi(in_strs[2].c_str());

    MI_ISP_IQ_DefogType_t Defog;
    MI_ISP_IQ_GetDefog(dev, ch,&Defog);

    Defog.bEnable = (MI_ISP_IQ_Bool_e)enable;
    Defog.enOpType=E_SS_IQ_OP_TYP_MANUAL;
    Defog.stManual.stParaAPI.u16StrengthByY[0]=strength;
    if(Defog.stManual.stParaAPI.u16StrengthByY[0]>4095)
    {
        ss_print(PRINT_LV_TRACE,"strength out of range. [0, 4095]. %d\n", Defog.stManual.stParaAPI.u16StrengthByY[0]);
        Defog.stManual.stParaAPI.u16StrengthByY[0]=4095;
    }
    ss_print(PRINT_LV_TRACE,"set strength %d,  bEnable =%d\n", Defog.stManual.stParaAPI.u16StrengthByY[0],Defog.bEnable);
    MI_ISP_IQ_SetDefog(dev, ch,&Defog);
    return 0;
}

static int IspAe(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    auto mode = in_strs[1];
    auto table_mode = in_strs[2];
    auto luma = ss_cmd_atoi(in_strs[2].c_str());

    MI_ISP_AE_WinWeightType_t sWinWeight;
    MI_ISP_AE_ExpoLimitType_t sExpoLimit;
    MI_ISP_AE_IntpLutType_t AEtarget;
    int i = 0;
    MI_U32 Weight_average[1024] =
    {
        253,253,253,253,254,254,254,254,254,254,254,254,253,253,253,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,253,253,254,254,254,254,254,254,254,254,254,254,253,253,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,253,254,254,254,254,254,254,254,254,254,254,254,254,253,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,254,254,254,254,254,254,254,254,254,254,254,254,254,254,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,254,254,255,255,255,255,254,254,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,254,255,255,255,255,255,255,254,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,255,255,255,255,255,255,255,255,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,255,255,255,255,255,255,255,255,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,255,255,255,255,255,255,255,255,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,255,255,255,255,255,255,255,255,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,254,255,255,255,255,255,255,254,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        254,254,254,254,254,254,255,255,255,255,254,254,254,254,254,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,254,254,254,254,254,254,254,254,254,254,254,254,254,254,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,253,254,254,254,254,254,254,254,254,254,254,254,254,253,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,253,253,254,254,254,254,254,254,254,254,254,254,253,253,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        253,253,253,253,254,254,254,254,254,254,254,254,253,253,253,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    MI_U32 Weight_center[1024] =
    {
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,254,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,255,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,255,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,255,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,255,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,255,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,255,255,255,255,255,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    MI_U32 Weight_spot[1024] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,2,3,3,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,2,4,7,9,9,7,4,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,4,9,16,23,23,16,9,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,2,7,16,34,57,57,34,16,7,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,3,9,23,57,131,131,57,23,9,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,3,9,23,57,131,131,57,23,9,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,2,7,16,34,57,57,34,16,7,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,4,9,16,23,23,16,9,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,1,2,4,7,9,9,7,4,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,2,3,3,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

    // 1 weight table, 2 exposure limit, 3 target luma,5 default ;
    if("table" == mode)
    {
        // 1 average table, 2 center table, 3 spot tale;
        if("average" == table_mode)
        {
            MI_ISP_AE_GetWinWgt(dev, ch, &sWinWeight);
            sWinWeight.eTypeID = E_SS_AE_WEIGHT_AVERAGE;
            for(i = 0; i < 1024; i++)
            {
                sWinWeight.stParaAPI.u8AverageTbl[i] = Weight_average[i];
            }
            MI_ISP_AE_SetWinWgt(dev, ch, &sWinWeight);
        }
        else if("center" == table_mode)
        {
            MI_ISP_AE_GetWinWgt(dev, ch, &sWinWeight);
            sWinWeight.eTypeID = E_SS_AE_WEIGHT_CENTER;
            for(i = 0; i < 1024; i++)
            {
                sWinWeight.stParaAPI.u8AverageTbl[i] = Weight_center[i];
            }
            MI_ISP_AE_SetWinWgt(dev, ch, &sWinWeight);
        }
        else if("spot" == table_mode)
        {
            MI_ISP_AE_GetWinWgt(dev, ch, &sWinWeight);
            sWinWeight.eTypeID = E_SS_AE_WEIGHT_SPOT;
            for(i = 0; i < 1024; i++)
            {
                sWinWeight.stParaAPI.u8AverageTbl[i] = Weight_spot[i];
            }
            MI_ISP_AE_SetWinWgt(dev, ch, &sWinWeight);
        }
        else
        {
            ss_print(PRINT_LV_TRACE,"mode out of range. [1, 3]. %d\n", table_mode);
        }
        return 0;
    }
    if("limit" == mode)
    {
        MI_ISP_AE_GetExposureLimit(dev, ch, &sExpoLimit);
        sExpoLimit.u32MinISPGain = 1024;
        sExpoLimit.u32MinSensorGain = 1024;
        sExpoLimit.u32MinShutterUS = 150;
        sExpoLimit.u32MaxShutterUS = 4000;
        sExpoLimit.u32MaxSensorGain = 1024;
        sExpoLimit.u32MaxISPGain = 1024;
        sExpoLimit.u32MinFNx10 =21;
        sExpoLimit.u32MaxFNx10 =21;
        MI_ISP_AE_SetExposureLimit(dev, ch, &sExpoLimit);
        return 0;
    }
    if("luma" == mode)
    {
        //if((MI_U32)tAeAttr->y.luma==0)
        //{
        // printf("Please set target_luma:");
        // scanf("%d", &((MI_U32)tAeAttr->y.luma));
        //}
        MI_ISP_AE_GetExposureLimit(dev, ch, &sExpoLimit);
        sExpoLimit.u32MinISPGain = 1024;
        sExpoLimit.u32MinSensorGain = 1024;
        sExpoLimit.u32MinShutterUS = 150;
        sExpoLimit.u32MaxShutterUS = 40000;
        sExpoLimit.u32MaxSensorGain = 1024*1024;
        sExpoLimit.u32MaxISPGain = 1024*8;
        sExpoLimit.u32MinFNx10 =21;
        sExpoLimit.u32MaxFNx10 =21;
        MI_ISP_AE_SetExposureLimit(dev, ch, &sExpoLimit);

        MI_ISP_AE_GetTarget(dev, ch, &AEtarget);
        AEtarget.u16NumOfPoints=16;
        for(i=0; i<16; i++)
        {
            AEtarget.u32X[i]=(MI_S32)0;
            AEtarget.u32Y[i]=(MI_S32)luma;
        }

        MI_ISP_AE_SetTarget(dev, ch,&AEtarget);
        return 0;
    }

    MI_ISP_AE_GetWinWgt(dev, ch, &sWinWeight);
    sWinWeight.eTypeID = E_SS_AE_WEIGHT_AVERAGE;
    for(i = 0; i < 1024; i++)
    {
        sWinWeight.stParaAPI.u8AverageTbl[i] = Weight_average[i];
    }
    MI_ISP_AE_SetWinWgt(dev, ch, &sWinWeight);
    MI_ISP_AE_GetExposureLimit(dev, ch, &sExpoLimit);
    sExpoLimit.u32MinISPGain = 1024;
    sExpoLimit.u32MinSensorGain = 1024;
    sExpoLimit.u32MinShutterUS = 150;
    sExpoLimit.u32MaxShutterUS = 40000;
    sExpoLimit.u32MaxSensorGain = 1024*1024;
    sExpoLimit.u32MaxISPGain = 1024*8;
    sExpoLimit.u32MinFNx10 =21;
    sExpoLimit.u32MaxFNx10 =21;
    MI_ISP_AE_SetExposureLimit(dev, ch, &sExpoLimit);
    MI_ISP_AE_GetTarget(dev, ch, &AEtarget);
    AEtarget.u16NumOfPoints=16;
    for(i=0; i<16; i++)
    {
        AEtarget.u32X[i]=(MI_S32)0;
        AEtarget.u32Y[i]=(MI_S32)470;
    }

    MI_ISP_AE_SetTarget(dev, ch,&AEtarget);

    return 0;
}

static int IspAeExpoInfo(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;

    MI_ISP_AE_ExpoInfoType_t ExpInfo;

    MI_ISP_AE_QueryExposureInfo(dev, ch, &ExpInfo);
    //MIXER_DBG("u32LVx10 = %u\n", ExpInfo.u32LVx10);
    //MIXER_DBG("s32BV = %u\n", ExpInfo.s32BV);
    ss_print(PRINT_LV_TRACE,"bIsStable:%d, Again:%d, Dgain:%d, extime:%d \n", ExpInfo.bIsStable,
           ExpInfo.stExpoValueLong.u32SensorGain, ExpInfo.stExpoValueLong.u32ISPGain, ExpInfo.stExpoValueLong.u32US);

    return 0;
}

static int IspIq3dnr(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int u8MdThd = ss_cmd_atoi(in_strs[1].c_str());
    int u16MdGain = ss_cmd_atoi(in_strs[2].c_str());
    int u8TfStr = ss_cmd_atoi(in_strs[3].c_str());
    //int u8TfStrEx = ss_cmd_atoi(in_strs[4].c_str());
    int u8TfLut = ss_cmd_atoi(in_strs[5].c_str());

    int i;
    MI_ISP_IQ_Nr3dType_t nNR3D;

    MI_ISP_IQ_GetNr3d(dev, ch, &nNR3D);

    nNR3D.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    nNR3D.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    nNR3D.stManual.stParaAPI.u8MdThd=u8MdThd;
    if(nNR3D.stManual.stParaAPI.u8MdThd > 255)
    {
        ss_print(PRINT_LV_TRACE,"u16MdThd out of range. [0, 255]. %d\n", nNR3D.stManual.stParaAPI.u8MdThd);
        nNR3D.stManual.stParaAPI.u8MdThd = 255;
    }
    nNR3D.stManual.stParaAPI.u16MdGain=u16MdGain;
    if(nNR3D.stManual.stParaAPI.u16MdGain > 10230)
    {
        ss_print(PRINT_LV_TRACE,"u16MdGain out of range. [0, 10230]. %d\n", nNR3D.stManual.stParaAPI.u16MdGain);
        nNR3D.stManual.stParaAPI.u16MdGain = 10230;
    }
    nNR3D.stManual.stParaAPI.u8TfStrY = (MI_U8)u8TfStr;
    if(nNR3D.stManual.stParaAPI.u8TfStrY > 64)
    {
        ss_print(PRINT_LV_TRACE,"u8TfStr out of range. [0, 64]. %d\n", nNR3D.stManual.stParaAPI.u8TfStrY);
        nNR3D.stManual.stParaAPI.u8TfStrY = 64;
    }

#if 0   //no define u8TfStrEx
    nNR3D.stManual.stParaAPI.u8TfStrEx = (MI_U8)u8TfStrEx;
    if(nNR3D.stManual.stParaAPI.u8TfStrEx > 64)
    {
        MIXER_WARN("u8TfStrEx out of range. [0, 64]. %d\n", nNR3D.stManual.stParaAPI.u8TfStrEx );
        nNR3D.stManual.stParaAPI.u8TfStrEx  = 64;
    }
#endif
    for(i=0; i<15; i++)
    {
        nNR3D.stManual.stParaAPI.u8TfLut[i] = (MI_U16)u8TfLut;
        if(nNR3D.stManual.stParaAPI.u8TfLut[i] > 255)
        {
            ss_print(PRINT_LV_TRACE,"u8TfStrEx out of range. [0, 255]. %d\n", nNR3D.stManual.stParaAPI.u8TfLut[i]);
            nNR3D.stManual.stParaAPI.u8TfLut[i] = 255;
        }
    }

    MI_ISP_IQ_SetNr3d(dev, ch, &nNR3D);

    return 0;
}

static int IspAwbAttr(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int u16WhiteBgain = ss_cmd_atoi(in_strs[1].c_str());
    int u16WhiteRgain = ss_cmd_atoi(in_strs[2].c_str());
    int u8AreaSize = ss_cmd_atoi(in_strs[3].c_str());

    MI_ISP_AWB_AttrType_t WBAttr;
    MI_ISP_AWB_AttrExType_t WBAttrEX;
    MI_ISP_AWB_GetAttr(dev, ch, &WBAttr);
    WBAttr.eOpType = E_SS_AWB_MODE_AUTO;
    WBAttr.stAutoParaAPI.eAlgType = (MI_ISP_AWB_AlgoType_e)1;    //AWB_ALG_ADVANCE;
    MI_ISP_AWB_SetAttr(dev, ch, &WBAttr);

    MI_ISP_AWB_GetAttrEx(dev, ch, &WBAttrEX);
    WBAttrEX.bExtraLightEn = (MI_ISP_AWB_Bool_e)TRUE;
    WBAttrEX.stLightInfo[0].bExclude = MI_ISP_AWB_Bool_e(TRUE);

    WBAttrEX.stLightInfo[0].u16WhiteBgain = u16WhiteBgain;
    if(WBAttrEX.stLightInfo[0].u16WhiteBgain > 4095)
    {
        ss_print(PRINT_LV_TRACE,"AWB_SetAttrEx u16WhiteBgain [256, 4095]. %d\n", WBAttrEX.stLightInfo[0].u16WhiteBgain);
        WBAttrEX.stLightInfo[0].u16WhiteBgain = 4095;
    }
    else if(WBAttrEX.stLightInfo[0].u16WhiteBgain < 256)
    {
        ss_print(PRINT_LV_TRACE,"AWB_SetAttrEx u16WhiteBgain [256,4095]. %d\n", WBAttrEX.stLightInfo[0].u16WhiteBgain);
        WBAttrEX.stLightInfo[0].u16WhiteBgain = 4095;
    }

    WBAttrEX.stLightInfo[0].u16WhiteRgain = u16WhiteRgain;
    if(WBAttrEX.stLightInfo[0].u16WhiteRgain > 4095)
    {
        ss_print(PRINT_LV_TRACE,"AWB_SetAttrEx u16WhiteRgain [256,4095]. %d\n", WBAttrEX.stLightInfo[0].u16WhiteRgain);
        WBAttrEX.stLightInfo[0].u16WhiteRgain = 4095;
    }
    else if(WBAttrEX.stLightInfo[0].u16WhiteRgain < 256)
    {
        ss_print(PRINT_LV_TRACE,"AWB_SetAttrEx u16WhiteRgain [256,4095]. %d\n", WBAttrEX.stLightInfo[0].u16WhiteRgain);
        WBAttrEX.stLightInfo[0].u16WhiteRgain = 4095;
    }

    WBAttrEX.stLightInfo[0].u8AreaSize = u8AreaSize;
    if(WBAttrEX.stLightInfo[0].u8AreaSize > 32)
    {
        ss_print(PRINT_LV_TRACE,"AWB_SetAttrEx u8AreaSize [1,32]. %d\n", WBAttrEX.stLightInfo[0].u8AreaSize);
        WBAttrEX.stLightInfo[0].u8AreaSize = 32;
    }
    else if(WBAttrEX.stLightInfo[0].u8AreaSize < 1)
    {
        ss_print(PRINT_LV_TRACE,"AWB_SetAttrEx u8AreaSize [1,32]. %d\n", WBAttrEX.stLightInfo[0].u8AreaSize);
        WBAttrEX.stLightInfo[0].u8AreaSize = 1;
    }

    MI_ISP_AWB_SetAttrEx(dev, ch, &WBAttrEX);

    ss_print(PRINT_LV_TRACE,"LightInfo[0].u2WhiteBgain = %hu\n", WBAttrEX.stLightInfo[0].u16WhiteBgain);
    ss_print(PRINT_LV_TRACE,"LightInfo[0].u2WhiteRgain = %hu\n", WBAttrEX.stLightInfo[0].u16WhiteRgain);
    ss_print(PRINT_LV_TRACE,"LightInfo[0].uAreaSize = %d\n", WBAttrEX.stLightInfo[0].u8AreaSize);

    return 0;
}

static int IspAwbQueryRYInfo(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;

    MI_ISP_AWB_QueryInfoType_t WBInfo;

    MI_ISP_AWB_QueryInfo(dev, ch, &WBInfo);

    ss_print(PRINT_LV_TRACE,"\n===== AWB ExpoInfo =====\n");
    ss_print(PRINT_LV_TRACE,"bIsStable:%d, Rgain:%d, Grgain:%d, Gbgain:%d, Bgain:%d, ColorTemp:%d\n", WBInfo.bIsStable, WBInfo.u16Rgain,
           WBInfo.u16Grgain, WBInfo.u16Gbgain, WBInfo.u16Bgain, WBInfo.u16ColorTemp);

    ss_print(PRINT_LV_TRACE,"WPInd:%d\n", WBInfo.u8WPInd);
    ss_print(PRINT_LV_TRACE,"MultiLSDetected:%d\n", WBInfo.bMultiLSDetected);
    if(TRUE == WBInfo.bMultiLSDetected)
    {
        ss_print(PRINT_LV_TRACE,"FirstLSInd:%d\n", WBInfo.u8FirstLSInd);
        ss_print(PRINT_LV_TRACE,"SecondLSInd:%d\n", WBInfo.u8SecondLSInd);
    }

    return 0;
}

static int IspAfInfo(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    //const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    //int dev = stInfo.devId;
    //int ch = stInfo.chnId;

    return 0;
}

static int IspIqNrdespike(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;

    int BlendRatio = ss_cmd_atoi(in_strs[1].c_str());
    int StrengthCenterNeighbor = ss_cmd_atoi(in_strs[2].c_str());
    int StrengthMeanStd = ss_cmd_atoi(in_strs[3].c_str());
    int StrengthCornerCross = ss_cmd_atoi(in_strs[4].c_str());

    MI_ISP_IQ_NrDespikeType_t spike;
    MI_ISP_IQ_GetNrDeSpike(dev, ch, &spike);

    spike.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    spike.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    spike.stManual.stParaAPI.u8BlendRatio = (MI_U8)BlendRatio;
    if(spike.stManual.stParaAPI.u8BlendRatio > 15)
    {
        ss_print(PRINT_LV_TRACE,"BlendRatio out of range. [0, 15]. %d\n", spike.stManual.stParaAPI.u8BlendRatio);
        spike.stManual.stParaAPI.u8BlendRatio = 15;
    }
    spike.stManual.stParaAPI.u8StrengthCenterNeighbor = (MI_U8)StrengthCenterNeighbor;
    if(spike.stManual.stParaAPI.u8StrengthCenterNeighbor > 5)
    {
        ss_print(PRINT_LV_TRACE,"StrengthCenterNeighbor out of range. [0, 5]. %d\n", spike.stManual.stParaAPI.u8StrengthCenterNeighbor);
        spike.stManual.stParaAPI.u8StrengthCenterNeighbor = 5;
    }
    spike.stManual.stParaAPI.u8StrengthMeanStd = (MI_U8)StrengthMeanStd;
    if(spike.stManual.stParaAPI.u8StrengthMeanStd > 5)
    {
        ss_print(PRINT_LV_TRACE,"StrengthMeanStd out of range. [0, 5]. %d\n", spike.stManual.stParaAPI.u8StrengthMeanStd);
        spike.stManual.stParaAPI.u8StrengthMeanStd = 5;
    }
    spike.stManual.stParaAPI.u8StrengthCornerCross = (MI_U8)StrengthCornerCross;
    if(spike.stManual.stParaAPI.u8StrengthCornerCross > 5)
    {
        ss_print(PRINT_LV_TRACE,"StrengthCornerCross out of range. [0, 5]. %d\n", spike.stManual.stParaAPI.u8StrengthCornerCross);
        spike.stManual.stParaAPI.u8StrengthCornerCross = 5;
    }

    ss_print(PRINT_LV_TRACE,"set BlendRatio %d,StrengthCenterNeighbor %d,StrengthMeanStd %d,StrengthCornerCross %d\n", \
              spike.stManual.stParaAPI.u8BlendRatio,spike.stManual.stParaAPI.u8StrengthCenterNeighbor,\
              spike.stManual.stParaAPI.u8StrengthMeanStd,spike.stManual.stParaAPI.u8StrengthCornerCross);
    MI_ISP_IQ_SetNrDeSpike(dev, ch, &spike);

    return 0;
}

static int IspNrluma(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;

    //int Nrluma_str = ss_cmd_atoi(in_strs[1].c_str());

    MI_ISP_IQ_NrLumaType_t nrluma;

    MI_ISP_IQ_GetNrLuma(dev, ch, &nrluma);
    //nrluma.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    //nrluma.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    //nrluma.stManual.stParaAPI.u8Strength=Nrluma_str;
    //if(nrluma.stManual.stParaAPI.u8Strength>63)
    {
        //ss_print(PRINT_LV_TRACE,"Nrluma_str out of range. [0, 63]. %d\n", nrluma.stManual.stParaAPI.u8Strength);
        //nrluma.stManual.stParaAPI.u8Strength=63;
    }
    MI_ISP_IQ_SetNrLuma(dev, ch, &nrluma);
    //ss_print(PRINT_LV_TRACE,"Nrluma_str set Strength: %d\n", nrluma.stManual.stParaAPI.u8Strength);

    return 0;
}

static int IspHsv(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int s16HueLut = ss_cmd_atoi(in_strs[1].c_str());
    int u16SatLut = ss_cmd_atoi(in_strs[2].c_str());

    int j = 0x0;
    MI_ISP_IQ_HsvType_t HSV;

    MI_ISP_IQ_GetHsv(dev, ch, &HSV);

    HSV.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    HSV.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    HSV.stManual.stParaAPI.s16YByHueLut[0] = (MI_S16)s16HueLut;
    if(HSV.stManual.stParaAPI.s16YByHueLut[0]  < -64)
    {
        ss_print(PRINT_LV_TRACE,"IQ_SetHSV s16HueLut [-64,64]. %d\n", HSV.stManual.stParaAPI.s16YByHueLut[0] );
        HSV.stManual.stParaAPI.s16YByHueLut[0]  = -64;
    }
    else if(HSV.stManual.stParaAPI.s16YByHueLut[0]  > 64)
    {
        ss_print(PRINT_LV_TRACE,"IQ_SetHSV s16HueLut [-64,64]. %d\n", HSV.stManual.stParaAPI.s16YByHueLut[0] );
        HSV.stManual.stParaAPI.s16YByHueLut[0]  = 64;
    }
    HSV.stManual.stParaAPI.u16YBySatLut[0] = (MI_U16)u16SatLut;
    if(HSV.stManual.stParaAPI.u16YBySatLut[0] > 255)
    {
        ss_print(PRINT_LV_TRACE,"IQ_SetHSV s16HueLut [0, 255]. %d\n", HSV.stManual.stParaAPI.u16YBySatLut[0]);
        HSV.stManual.stParaAPI.u16YBySatLut[0] = 255;
    }

    for(j = 0; j < HSV_HUE_NUM; j++)
    {
        HSV.stManual.stParaAPI.s16YByHueLut[j] = HSV.stManual.stParaAPI.s16YByHueLut[0];
    }
    for(j = 0; j < HSV_SAT_NUM; j++)
    {
        HSV.stManual.stParaAPI.u16YBySatLut[j] = HSV.stManual.stParaAPI.u16YBySatLut[0];
    }
    MI_ISP_IQ_SetHsv(dev, ch, &HSV);
    ss_print(PRINT_LV_TRACE,"s16HueLut:%d", (MI_S16)HSV.stManual.stParaAPI.s16YByHueLut[0]);
    ss_print(PRINT_LV_TRACE,"u16SatLut:%u\n", HSV.stManual.stParaAPI.u16YBySatLut[0]);

    return 0;
}

static int IspRgbIr(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    bool bRemovelEn = ss_cmd_atoi(in_strs[1].c_str());
    int Ratio_r = ss_cmd_atoi(in_strs[2].c_str());
    int Ratio_g = ss_cmd_atoi(in_strs[3].c_str());
    int Ratio_b = ss_cmd_atoi(in_strs[4].c_str());

    MI_ISP_IQ_RgbirType_t RGBIR;
    MI_U16 j = 0x0;

    MI_ISP_IQ_GetRgbir(dev, ch, &RGBIR);

    RGBIR.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    RGBIR.enOpType = E_SS_IQ_OP_TYP_MANUAL;

    RGBIR.stManual.stParaAPI.bRemovelEn = (MI_ISP_IQ_Bool_e)bRemovelEn;

    RGBIR.stManual.stParaAPI.u16Ratio_R[0] = (MI_U16)Ratio_r;
    for(j=0; j<RGBIR_BY_Y_NUM; j++)
        RGBIR.stManual.stParaAPI.u16Ratio_R[j] = RGBIR.stManual.stParaAPI.u16Ratio_R[0];

    RGBIR.stManual.stParaAPI.u16Ratio_G[0] = (MI_U16)Ratio_g;
    for(j=0; j<RGBIR_BY_Y_NUM; j++)
        RGBIR.stManual.stParaAPI.u16Ratio_G[j] = RGBIR.stManual.stParaAPI.u16Ratio_G[0];

    RGBIR.stManual.stParaAPI.u16Ratio_B[0] = (MI_U16)Ratio_b;
    for(j=0; j<RGBIR_BY_Y_NUM; j++)
        RGBIR.stManual.stParaAPI.u16Ratio_B[j] = RGBIR.stManual.stParaAPI.u16Ratio_B[0];

    MI_ISP_IQ_SetRgbir(dev, ch, &RGBIR);

    ss_print(PRINT_LV_TRACE,"Ratio_R:%d ", RGBIR.stManual.stParaAPI.u16Ratio_R[0]);
    ss_print(PRINT_LV_TRACE,"Ratio_G:%d ", RGBIR.stManual.stParaAPI.u16Ratio_G[0]);
    ss_print(PRINT_LV_TRACE,"Ratio_B:%d\n", RGBIR.stManual.stParaAPI.u16Ratio_B[0]);

    return 0;
}

static int IspWdr(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    int u8BrightLimit = ss_cmd_atoi(in_strs[1].c_str());
    int u8DarkLimit = ss_cmd_atoi(in_strs[2].c_str());
    int u8Strength = ss_cmd_atoi(in_strs[3].c_str());

    MI_ISP_IQ_WdrType_t Wdr;

    MI_ISP_IQ_GetWdr(dev, ch, &Wdr);

    Wdr.bEnable = (MI_ISP_IQ_Bool_e)TRUE;
    Wdr.enOpType = E_SS_IQ_OP_TYP_MANUAL;
    Wdr.stManual.stParaAPI.u8BrightLimit  = (MI_U8)u8BrightLimit;
    Wdr.stManual.stParaAPI.u8DarkLimit = (MI_U8)u8DarkLimit;
    Wdr.stManual.stParaAPI.u8Strength = (MI_U16)u8Strength;
    MI_ISP_IQ_SetWdr(dev, ch, &Wdr);

    ss_print(PRINT_LV_TRACE,"u8BrightLimit:%d ", Wdr.stManual.stParaAPI.u8BrightLimit);
    ss_print(PRINT_LV_TRACE,"u8DarkLimit:%d ", Wdr.stManual.stParaAPI.u8DarkLimit);
    ss_print(PRINT_LV_TRACE,"u8Strength:%d ", Wdr.stManual.stParaAPI.u8Strength);

    return 0;
}

static int IspCaliBin(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    const char * pCaliBinPath = in_strs[1].c_str();
    MI_ISP_IQ_CaliItem_e eItem = ss_enum_cast<MI_ISP_IQ_CaliItem_e>::from_str(in_strs[2].c_str());

    ss_print(PRINT_LV_TRACE,"[dev%d Ch%d] load CaliBin item %d\n",dev, ch,eItem);
    MI_ISP_ApiCmdLoadCaliData(dev, ch, eItem, (char *)pCaliBinPath);

    return 0;
}

static int IspAeState(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch = stInfo.chnId;
    auto AeState = in_strs[1];

    MI_ISP_AE_SmStateType_e AE_state = E_SS_AE_STATE_NORMAL;

    if(AeState == "pause")
        AE_state =  E_SS_AE_STATE_PAUSE;
    else
        AE_state = E_SS_AE_STATE_NORMAL;
    MI_ISP_AE_SetState(dev, ch, &AE_state);

    return 0;
}

static int IspIqServer(vector<string> &in_strs)
{
    AmigosModuleIq *pMyClass = GET_PRIVATE_CLASS_OBJ(AmigosModuleIq, AmigosModuleBase);
    sslog << "MY MOD ID IS: " << pMyClass->GetModId() << std::endl;
    const AmigosSurfaceBase::ModInfo &stInfo = pMyClass->GetModInfo();
    int dev = stInfo.devId;
    int ch  = stInfo.chnId;
    auto state = in_strs[1];

    if(state == "close")
        MI_IQSERVER_Close();
    else
        MI_IQSERVER_Open();

    ss_print(PRINT_LV_TRACE,"IQ server %s\n",dev, ch,state.c_str());

    return 0;
}

MOD_CMDS(AmiCmdIq)
{
    ADD_CMD("isp_color_gray", IspColorGray, 1);
    ADD_CMD_HELP("isp_color_gray", "[enable]", "enable[0/1]: 0 disbale, 1 enable");
    ADD_CMD("isp_awb", IspAwb, 5);
    ADD_CMD_HELP("isp_awb", "[mode] [Bgain] [Gbgain] [Grgain] [Rgain]", "[mode]: auto, manual, [gain]:0~0x2000");
    ADD_CMD("isp_ae_strate", IspAeStrate, 1);
    ADD_CMD_HELP("isp_ae_strate", "[mode]", "[mode]:brighttone, darktone, auto");
    ADD_CMD("isp_contrast", IspContrast, 1);
    ADD_CMD_HELP("isp_contrast", "[level]", "[level]:0~100");
    ADD_CMD("isp_brightness", IspBrightness, 1);
    ADD_CMD_HELP("isp_brightness", "[level]", "[level]:0~100");
    ADD_CMD("isp_rgb_gamma", IspRgbGamma, 2);
    ADD_CMD_HELP("isp_rgb_gamma", "[gamma_mode] [mode]", "[gamma_mode]:rgb, yuv, [mode]:low, normal, higt");
    ADD_CMD("isp_satration", IspSatration, 1);
    ADD_CMD_HELP("isp_satration", "[level]", "[level]:0~127");
    ADD_CMD("isp_lightness", IspLightness, 1);
    ADD_CMD_HELP("isp_lightness", "[level]", "[level]:0~100");
    ADD_CMD("isp_sharpness", IspSharpness, 2);
    ADD_CMD_HELP("isp_sharpness", "[SharpnessUD] [SharpnessD]", "[SharpnessUD/SharpnessD]:0~127");
    ADD_CMD("isp_ae_flicker", IspAeFlicker, 1);
    ADD_CMD_HELP("isp_ae_flicker", "[mode]", "[mode]:disable, 60hz, 50hz, auto");
    ADD_CMD("isp_ae_flicker_ex", IspAeFlickerEx, 1);
    ADD_CMD_HELP("isp_ae_flicker_ex", "[mode]", "[mode]:disable, 60hz, 50hz");
    ADD_CMD("isp_ccm", IspCcm, 9);
    ADD_CMD_HELP("isp_ccm", "[v1]...[v9]", "[v]:0~8191,(ccm level)");
    ADD_CMD("isp_false_color", IspFalseColor, 2);
    ADD_CMD_HELP("isp_false_color", "[StrengthMid] [ChromaThrdOfStrengthMid]", "[StrengthMid]: 0~7, [ChromaThrdOfStrengthMid]:0~31");
    ADD_CMD("isp_crosstalk", IspCrosstalk, 3);
    ADD_CMD_HELP("isp_crosstalk", "[ThresholdOffsetV2] [ThresholdV2] [StrengthV2]", "[ThresholdOffsetV2]:0~4095, [ThresholdV2]:0~255, [StrengthV2]:0~31");
    ADD_CMD("isp_dp", IspDp, 4);
    ADD_CMD_HELP("isp_dp", "[HotPixEn] [HotPixCompSlpoe] [DarkPixEn] [DarkPixCompSlpoe]","[HotPixEn/DarkPixEn]:0~1, [HotPixCompSlpoe/DarkPixCompSlpoe]:0~255");
    ADD_CMD("isp_blackLevel", IspBlackLevel, 4);
    ADD_CMD_HELP("isp_blackLevel", "[ValB] [ValGb] [ValGr] [ValR]", "[Val]:0~255");
    ADD_CMD("isp_defog", IspDefog, 2);
    ADD_CMD_HELP("isp_defog", "[Enable] [Strength]", "[Enable]:0~1, [Strength]:0~4095");
    ADD_CMD("isp_ae", IspAe, 2);
    ADD_CMD_HELP("isp_ae", "[mode] [table_luma_mode]", "[mode]:table, limit, luma",
        "when (mode=table) [table_luma_mode]:average, center, spot","when(mode=limit) [table_luma_mode]:mean nothing",
        "when (mode=luma) [table_luma_mode]:1~2000");
    ADD_CMD("isp_ae_expo_info", IspAeExpoInfo, 0);
    ADD_CMD_HELP("isp_ae_expo_info", "No argument", "isp_ae_expo_info");
    ADD_CMD("isp_iq_3dnr", IspIq3dnr, 5);
    ADD_CMD_HELP("isp_iq_3dnr", "[MdThd] [MdGain] [TfStr] [TfStrEx] [TfLut]", "[MdThd/MdGain/TfStr/TfStrEx/TfLut]:0~64");
    ADD_CMD("isp_awb_attr", IspAwbAttr, 3);
    ADD_CMD_HELP("isp_awb_attr", "[WhiteBgain] [WhiteRgain] [AreaSize]", "[WhiteBgain/WhiteRgain]:256~4095, [AreaSize]:1~32");
    ADD_CMD("isp_awb_query_info", IspAwbQueryRYInfo, 0);
    ADD_CMD_HELP("isp_awb_query_info", "No argument", "isp_awb_query_info");
    ADD_CMD("isp_af_info", IspAfInfo, 0);
    ADD_CMD_HELP("isp_af_info", "No argument", "isp_af_info");
    ADD_CMD("isp_iq_nrdespike", IspIqNrdespike, 4);
    ADD_CMD_HELP("isp_iq_nrdespike", "[BlendRatio] [StrengthCenterNeighbor] [StrengthMeanStd] [StrengthCornerCross]",
        "[BlendRatio]:0~15,[StrengthCenterNeighbor/StrengthMeanStd/StrengthCornerCross]:0~5");
    ADD_CMD("isp_nrluma", IspNrluma, 1);
    ADD_CMD_HELP("isp_nrluma", "[Strength]", "[Strength]:0~63");
    ADD_CMD("isp_hsv", IspHsv, 2);
    ADD_CMD_HELP("isp_hsv", "[HueLut] [SatLut]", "[HueLut]:-64~64, [SatLut]:0~255");
    ADD_CMD("isp_rgb_ir", IspRgbIr, 4);
    ADD_CMD_HELP("isp_rgb_ir", "[RemovelEn] [Ratio_R] [Ratio_G] [Ratio_B]", "[RemovelEn]:0,1 [Ratio_R/Ratio_G/Ratio_B]:0~65535");
    ADD_CMD("isp_wdr", IspWdr, 3);
    ADD_CMD_HELP("isp_wdr", "[BrightLimit] [DarkLimit] [Strength]", "[BrightLimit/DarkLimit/Strength]:0~255");
    ADD_CMD("isp_cali_bin", IspCaliBin, 2);
    ADD_CMD_HELP("isp_cali_bin", "[file] [mode]", "[file]: CaliData file path, [mode]:awb, obc, sdc, alsc, lsc");
    ADD_CMD("isp_ae_state", IspAeState, 1);
    ADD_CMD_HELP("isp_ae_state", "[mode]", "[mode]: normal, pause");
    ADD_CMD("isp_iq_server", IspIqServer, 1);
    ADD_CMD_HELP("isp_iq_server", "[state]", "[mode]: close, open");
}
