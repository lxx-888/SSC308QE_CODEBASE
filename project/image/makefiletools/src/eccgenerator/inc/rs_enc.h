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
#define Q               1024    // 2^10
#define N               (Q-1)   // N=Q-1=2^10-1
#define M               10
#define K               1015
#define V               8       // 2*t
#define N_INFO_WORD     259     // (512+6) bytes = 259 WORDS
#define N_PARITY_WORD   5
#define N_INFO_SYM      518 //415
#define N_ZERO_PADDING  6

void Calculate_ECC(unsigned char *pBuf,unsigned char *RDD);
int ecc_cal(int ECC_MODE,unsigned char *pSourceData,unsigned char *pSpareArea,int REDU_LENGTH,unsigned char *pOutputData,char debug);
extern void gf_gen();
extern unsigned short gf_mult(unsigned short sym1, unsigned short sym2);
extern unsigned short gf_add(unsigned short sym1, unsigned short sym2);
extern void bit2sym(unsigned short *bvec, unsigned short *sym);
extern void sym2bit(unsigned short sym, unsigned short *bvec);
extern void word2sym(unsigned short *word_in, unsigned short *sym_out);
extern void sym2word(unsigned short *sym_in, unsigned short *word_out);
extern void sym2word_parity(unsigned short *sym_in, unsigned short *word_out, short len);
