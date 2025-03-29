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
//#include <stdio.h>
//#include <stdlib.h>	//for rand, srand
///#include <string.h> //for memset
#include <math.h>
#include "rs_enc.h"
#include <nand.h>
//#include "Apollo.h"
#define START_PCKT 0
#define END_PCKT 1
// external buffers
unsigned short gf_table[Q];                                        // GF table
//unsigned short gx[V+1]={400, 665, 677, 29, 1006, 427, 778, 255, 1};//g(0)~g(2t)
unsigned short gx[V+1]={836, 587, 58, 928, 663, 323, 51, 510, 1};
/*-----------------------------------------------------------------------------
 FUNCTION:  gf_gen()
 DESCRIPTION:
    function to generate the GF(2^m) table

 INPUT:
    none

 OUTPUT:
    none
 RETURN:
     N/A

 USAGE:
    unsigned short gf_table[Q];     // declare external buffer
    gf_gen();                       // generate table

 GLOBAL AFFECTED:
     gf_table[]
 REFERENCE:

 HISTORY:
-------------------------------------------------------------------------------
 Date        Rev.    Author                detail
 2006/5/26   0.9     CY Su              first version created
-------------------------------------------------------------------------------*/
void
byte_err (unsigned short int err, int loc, unsigned short int *dst)
{
  //printf("Adding Error at loc %d, data %#x\n", loc, dst[loc]);
  dst[loc] ^= err;
}

void gf_gen()
{
    short k,j;
    unsigned short c[M],c_tmp;
    unsigned int tmp;
    //memset(c, 0, M*sizeof(unsigned short));
    for(tmp=0;tmp<M;tmp++)
        c[tmp]=0x00;
    c[0]=1;
	gf_table[0]=0;
	gf_table[1]=1;
    for (k=2;k<Q;k++) {
		c_tmp = c[M-1];
        for (j=M-1;j>3;j--)
            c[j] = c[j-1];
		c[3] = (c_tmp+c[2])%2;
        c[2] = c[1];
        c[1] = c[0];
        c[0] = c_tmp;

        bit2sym(c,&gf_table[k]);
    }
}
/*-----------------------------------------------------------------------------
 FUNCTION:  gf_mult()
 DESCRIPTION:
    function to multiply two variables in GF(2^m)

 INPUT:
    unsigned short sym1: first variable in GF(2^m)(decimal format)
    unsigned short sym2: 2nd variable in GF(2^m)  (decimal format)
 OUTPUT:
    none
 RETURN:
     unsigned short result: multiplication result (decimal format)

 USAGE:
    unsigned short gf_table[Q];     // declare external buffer
    unsigned short sym1,sym2,sym3;
    gf_gen();                       // generate table
    sym1 = 10; sym2=520;
    sym3=gf_mult(sym1,sym2);        // call function to multiply

 GLOBAL AFFECTED:
     none
 REFERENCE:

 HISTORY:
-------------------------------------------------------------------------------
 Date        Rev.    Author                detail
 2006/5/26   0.9     CY Su              first version created
-------------------------------------------------------------------------------*/
unsigned short gf_mult(unsigned short sym1, unsigned short sym2)
{
    unsigned short e, sym_out;
    unsigned short a[M],b[M],c[M],d[M];
    short i,j,k;
    // Step 1: convert symbol to binary format
    sym2bit(sym1, a);
    sym2bit(sym2, b);

    // Step 2:calculate coeff of a^M-1...a^0
    for (k=M-1;k>=0;k--) {
        c[k]=0;
        for (j=0;j<=k;j++)
            c[k] = (c[k]+ a[j] * b[k-j])%2;
    }
    // Step 3:
    // calculate coeff of a^(2M-2)...a^M
    // convert high order into low order using gf_table[]
    for (k=1;k<M;k++){
        i = 2*M-k-1;
        e=0;
        for (j=1;j<=k;j++)
            e+=a[i-(M-j)]*b[M-j];
        e%=2;
        if (e) {
            sym2bit(gf_table[i+1],d);
            for (j=0;j<M;j++)
                c[j]=(c[j]+d[j])%2;
        }
    }
    // Step 4:convert binary format to symbol
    bit2sym(c,&sym_out);
    return (sym_out);
}
/*-----------------------------------------------------------------------------
 FUNCTION:  gf_add()
 DESCRIPTION:
    function to add two variables in GF(2^m)

 INPUT:
    unsigned short sym1: first variable in GF(2^m)(decimal format)
    unsigned short sym2: 2nd variable in GF(2^m)  (decimal format)
 OUTPUT:
    none
 RETURN:
     unsigned short result: multiplication result (decimal format)

 USAGE:
    unsigned short gf_table[Q];     // declare external buffer
    unsigned short sym1,sym2,sym3;
    gf_gen();                       // generate table
    sym1 = 10; sym2=520;
    sym3=gf_add(sym1,sym2);        // call function to multiply

 GLOBAL AFFECTED:
     none
 REFERENCE:

 HISTORY:
-------------------------------------------------------------------------------
 Date        Rev.    Author                detail
 2006/5/26   0.9     CY Su              first version created
-------------------------------------------------------------------------------*/
unsigned short gf_add(unsigned short sym1, unsigned short sym2)
{
    unsigned short a[M],b[M],c[M];
    unsigned short sym_out;
    short k;
    sym2bit(sym1, a);
    sym2bit(sym2, b);
    for (k=0;k<M;k++)
        c[k] = (a[k] + b[k])%2;
    bit2sym(c, &sym_out);
    return (sym_out);
}
//-----------------------------------------------------------------------------
// convert binary format to symbol
// *bvec: pointer to binary vector corresponding to gf_table[]
// *sym : output of converted symbol decimal format)
//-----------------------------------------------------------------------------

void bit2sym(unsigned short *bvec, unsigned short *sym)
{
    short k;
    unsigned short c;
    c=0;
    for (k=0;k<M;k++)
        c+=(bvec[k]<<k);
    *sym = c;
}
//-----------------------------------------------------------------------------
// convert symbol to binary format
// *bvec: pointer to output binary vector corresponding to gf_table[]
// *sym : input symbol to be converted (decimal format)
//-----------------------------------------------------------------------------
void sym2bit(unsigned short sym, unsigned short *bvec)
{
    short k=M-1;
    unsigned short c;
    unsigned int tmp;
    //memset(bvec,0, M*sizeof(unsigned short));
    for(tmp=0;tmp<M;tmp++)
        bvec[tmp]=0x00;
    while (sym>0 && k>=0) {
        c = 1<<(k);
        if (sym>=c) {
            sym-=c;
            bvec[k]=1;
        }
        k--;
    }
}
/*-----------------------------------------------------------------------------
 FUNCTION:  rs_enc()
 DESCRIPTION:
    function to perform RS (423,415,t=4) encoding

 INPUT:
    unsigned short sym[]: pointer to input symbol
    Note: - The size of sym[] should be 423 unsigned short buffer where
            the first 415 symbol,sym[0:414] contains the input symbols
            to be encoded.
          - After calling this function, 8 parity symbols will be appended
            at the last 8 positions, sym[415:422].
          - sym[] is represented by decimal format (LSB of 10-bit vector
            corresponds to a^0)
          - call gf_table() before calling this funciton


 OUTPUT:
    unsigned short sym[]: encoded symbol sym[0:422] where sym[0] corresponds
                          to the first symbol
 RETURN:
     none

 USAGE:
    unsigned short gf_table[Q];     // declare external buffer
    unsigned short sym[423];
    gf_gen();                       // generate table
    sym[]=....                      // generate input symbols
    rs_enc(sym);                    // call encoding function

 GLOBAL AFFECTED:
     none
 REFERENCE:

 HISTORY:
-------------------------------------------------------------------------------
 Date        Rev.    Author                detail
 2006/5/26   0.9     CY Su              first version created
-------------------------------------------------------------------------------*/

void rs_enc(unsigned short *sym)
{
    unsigned short buf[V];
    unsigned short val1,val2;
    short i,k;
    unsigned int tmp;
    //memset(buf, 0, V * sizeof(unsigned short));
    for(tmp=0;tmp<V;tmp++)
        buf[tmp] = 0x00;
    // Step 1:
    // calculate the remainder of X^2t*a(x) divided by g(x)
    for (i=0;i<N_INFO_SYM;i++) {
 		val1 = gf_add(sym[i],buf[V-1]);
        for (k=V-1;k>0;k--) {
			val2 = gf_mult(gx[k],val1);
            buf[k] = gf_add(val2,buf[k-1]);
        }
        buf[0] = gf_mult(gx[0],val1);
		// printf for RTL
		/*
		for (k=0;k<V;k++)
			printf("%x\n",buf[k]);
		*/
    }
    // Step 2:
    // append the parity symbols
	// printf for RTL
	//for (i=0;i<V;i++)
	//	printf("%x\n",buf[i]);
	for (i=0;i<V;i++)
        sym[N_INFO_SYM+i] = buf[V-i-1];
}
//-----------------------------------------------------------------------------
// convert word-wise data to symbol-wise format
// *word_in : pointer to input word-wise data (decimal format of 16-bit data)
//            word_in[0] corresponds to the first input word
// *sym_out : pointer to output symbol-wise data(decimal format of 10-bit data)
//            sym_out[0] corresponds to the first output symbol
// The converion method is to separate each word into 2 bytes and insert 2 '0's
// in front of each byte to obtain 10-bit symbol
//-----------------------------------------------------------------------------
// Date        Rev.    Author                detail
// 2006/5/26   0.9     CY Su              first version created
// 2006/5/30   1.0     CY Su              change the byte input to 16-bit word
// 2006/06/02  1.1     CY Su              change to converion method as
//                                        mentioned above
//-----------------------------------------------------------------------------


void word2sym(unsigned short *word_in, unsigned short *sym_out)
{
	short k;
	for (k=0;k<N_INFO_WORD;k++) {
		sym_out[2*k]  = word_in[k]>>8;
		sym_out[2*k+1]= word_in[k]&0x00FF;
	}
}
//-----------------------------------------------------------------------------
// convert symbol-wise data to word-wise format for message part only,
// call sym2word_parity() for parity sym conversion!
// *sym_in   : pointer to input symbol-wise data (decimal format of 10-bit data)
//             sym_in[0] corresponds to the first input symbol
// Note: We assume each symbol comprise one message byte with 2 redundant bits
//       in front of it. Thus, it's easy to convert sym back to word via shift
// *word_out : pointer to output word-wise data(decimal format of 16-bit data)v
//             word_out[0] corresponds to the first output word
//-----------------------------------------------------------------------------
// Date        Rev.    Author                detail
// 2006/5/26   0.9     CY Su              first version created
//-----------------------------------------------------------------------------
void sym2word(unsigned short *sym_in, unsigned short *word_out)
{
	short k;
	for (k=0;k<N_INFO_WORD;k++) {
		word_out[k]=sym_in[2*k]<<8;
		word_out[k]|=sym_in[2*k+1];
	}
}
//-----------------------------------------------------------------------------
// convert symbol-wise parity to word-wise format
// *sym_in   : pointer to input symbol-wise parity (decimal format of 10-bit
//             data) sym_in[0] corresponds to the first input symbol
//
// *word_out : pointer to output word-wise parity(decimal format of 16-bit data)
//             word_out[0] corresponds to the first output byte
//-----------------------------------------------------------------------------
// Date        Rev.    Author                detail
// 2006/5/26   0.9     CY Su              first version created
// 2006/5/30   1.0     CY Su              change the byte output to 16-bit word
//										  output
// 1006/06/02  1.1     CY Su              change the original function to
//                                        process only the parity part
//-----------------------------------------------------------------------------

void sym2word_parity(unsigned short *sym_in, unsigned short *word_out, short len)
{
	unsigned long buf,b;
	unsigned short a;
	short k,j,n_res_bit;
	if (len<3) {
		//printf("length of input symbol is too short, len must>=3\n");
		return;
	}
	buf=(sym_in[0]<<M)|sym_in[1];
	n_res_bit = 2*M;   // residual bits available
	k = 0;
	j = 2;
	while (k<len) {
		if (n_res_bit>=16){
			n_res_bit -=16;
			a= buf>>n_res_bit;
			word_out[k++]=a;
			b = (1<<(n_res_bit))-1;
			buf = (buf & b)<<M;
		} else {
			if (n_res_bit>=6) {
				buf |=sym_in[j++];
				n_res_bit +=M;
			} else {
				buf = (buf<<M)|(sym_in[j]<<M)|sym_in[j+1];
				n_res_bit +=2*M;
				j+=2;
			}
		}
	}
}

void Calculate_ECC(unsigned char *pBuf,unsigned char *RDD)
{
    unsigned short sym[N_INFO_SYM+V], parity[5];
	unsigned short inbuf[N_INFO_WORD+1],outbuf[N_INFO_WORD];
	unsigned short target_out[N_INFO_WORD+5];
	unsigned short sym_inter[N_INFO_SYM+10];
	unsigned short add_error_content;

    int k,i,pckt_idx;
	unsigned long seed=12345;
	//FILE *fid,*fout,*ferror, *fgood;
	int add_error_no, add_error_pos;
	int p_error_only;
	// first test: all symbols equal to 5, and compare result with matlab
	/*
	for (k=0;k<N_INFO_SYM;k++)
        sym[k] = 5;
	gf_gen();               // generate GF(2^10) table
	rs_enc(sym);            // encoding
	for (k=0;k<V;k++)       // dump parity symbols
        printf("%u\n",sym[N_INFO_SYM+k]);
	printf("\n");
	*/
	// 2nd test: verify the test pattern
	/*
    fid = fopen("pattern.txt","r");
    for (k=0;k<32;k++) {
		fscanf(fid,"%4x %4x %4x %4x %4x %4x %4x %4x\n",
				   &inbuf[8*k],&inbuf[8*k+1],&inbuf[8*k+2],&inbuf[8*k+3],
				   &inbuf[8*k+4],&inbuf[8*k+5],&inbuf[8*k+6],&inbuf[8*k+7]);

	}
	fscanf(fid, "%4x %4x %4x\n",&inbuf[8*32],&inbuf[8*32+1],&inbuf[8*32+2]);
	for (k=0;k<N_INFO_WORD;k++) {
		sym[2*k]  = inbuf[k]>>8;
		sym[2*k+1]= inbuf[k]&0x00FF;
	}

	gf_gen();               // generate GF(2^10) table
	rs_enc(sym);            // encoding
	sym2word_parity(&sym[N_INFO_SYM],parity,5);
	for (k=0;k<5;k++)       // dump parity symbols
        printf("%04X\n",parity[k]);
	printf("\n");
    */
    // 3rd test: random data

	//fout = fopen("test_in.dat","w"); // test_in.dat is the input for RTL test bench
	//ferror = fopen("error.dat","w"); // test_in.dat is the input for RTL test bench
	//fgood = fopen("good.dat","w");
	gf_gen();
    for (pckt_idx=0;pckt_idx<END_PCKT;pckt_idx++)
    {
    	//srand(seed);
    	//fid_in = fopen("cis.txt","r"); //read the undecoded codewords
    	//for (k=0;k<32;k++) { //8*32= 256 ,get 256 16-bit words
    	//	fscanf(fid_in,"%4x %4x %4x %4x %4x %4x %4x %4x\n",
    	//	&inbuf[8*k],&inbuf[8*k+1],&inbuf[8*k+2],&inbuf[8*k+3],
    	//	&inbuf[8*k+4],&inbuf[8*k+5],&inbuf[8*k+6],&inbuf[8*k+7]);
    	//}
        //fscanf(fid_in,"%4x %4x %4x",
    	//	&inbuf[8*k],&inbuf[8*k+1],&inbuf[8*k+2]);
        //fclose(fid_in);
		for(k=0;k<256;k++)
		{
			inbuf[k] = (((unsigned short)pBuf[(k<<1)+1])<<8) + pBuf[k<<1];
		}
		inbuf[k++] = (((unsigned short)RDD[1])<<8) + RDD[0];
		inbuf[k++] = (((unsigned short)RDD[3])<<8) + RDD[2];
		inbuf[k++] = (((unsigned short)RDD[5])<<8) + RDD[4];
        /*
    	for (k=0;k<N_INFO_WORD;k++)
    		inbuf[k] = floor(65535 * rand()/32767);
    		//inbuf[k]=65535;
    		//inbuf[k] = rand()%65536;
        */
    	word2sym(inbuf,sym);    // convert word-wise data to symbol-wise format

    							// call this function after decoding
        rs_enc(sym);            // encoding
    	sym2word(sym,outbuf);   // convert sym-wise data to word-wise format

    	if (pckt_idx>=START_PCKT) {
    		for (k=0;k<N_INFO_WORD;k++)
    		{
    			//if(outbuf[k]!=inbuf[k])
    			//	printf("ERROR!");
    			//fprintf(fgood,"%04X ",inbuf[k]);
    			//if((k+1)%8==0)
    				//fprintf(fgood,"\n");
    		}
    	}

    	sym2word_parity(&sym[N_INFO_SYM],parity,5); // convert sym to word format

    	if (pckt_idx>=START_PCKT) {
    		//for (k=0;k<5;k++)       // dump parity symbols
    		//	fprintf(fgood,"%04X ",parity[k]);
    		//fprintf(fgood,"\n");
    	}


    	for (k=0;k<N_INFO_WORD;k++) {
    		sym_inter[2*k]  = sym[2*k];
    		sym_inter[2*k+1]= sym[2*k+1];
    	}
    	for (k=0;k<5;k++) {
    		sym_inter[2*(k+N_INFO_WORD)]  = parity[k]>>8;
    		sym_inter[2*(k+N_INFO_WORD)+1]= parity[k]&0x00FF;
    	}


    	//p_error_only = ( rand()%15 == 0 );
    	p_error_only = 0;
        //add_error_no = rand()%6;
        add_error_no = 0;
    	//p_error_only = ( rand()%1 == 0 );
    	//add_error_no = rand()%3;
        //printf("add_error_no %d\n",add_error_no);
    	//fprintf(ferror,"%d ",pckt_idx+1);
    	/*
    	while (add_error_no>0) {
        //for (i=0;i<add_error_no;i++) {
    		if ( (rand()%3)==0 ) { // add two errors at the same word
    			if (p_error_only) {
    				add_error_pos = rand()%10 + 517;	// actually it should be 518, but may result memory bug due to "two errors at the same word"
    			}
    			else {
    				add_error_pos = (rand()%264)*2;
    			}
    			add_error_content = rand()%256;
    			byte_err(add_error_content,add_error_pos,sym_inter);
    			//fprintf(ferror,"pos:%d val:%x ",add_error_pos,add_error_content);
    			add_error_pos = add_error_pos+1;
    			add_error_content = rand()%256;
    			byte_err(add_error_content,add_error_pos,sym_inter);
    			//fprintf(ferror,"pos:%d val:%x ",add_error_pos,add_error_content);
    			add_error_no=add_error_no-2;
    		}
    		else {
    			if (p_error_only) {
    				add_error_pos = rand()%10 + 518;
    			}
    			else {
    				add_error_pos = rand()%528;
    			}
    			add_error_content = rand()%256;
    			byte_err(add_error_content,add_error_pos,sym_inter);
    			//fprintf(ferror,"pos:%d val:%x ",add_error_pos,add_error_content);
    			add_error_no--;
    		}
    	  //printf("%d\n",add_error_no);
    	  //printf("pos:%d err:%d\n",add_error_pos,add_error_content);
        }
        */
    	//fprintf(ferror,"\n");
		//RDD[0]=RDD[1]=RDD[2]=RDD[3]=RDD[4]=RDD[5]=0xFF;
    	if (pckt_idx>=START_PCKT) {
    		for (k=0;k<(N_INFO_WORD+5);k++) {
    			target_out[k]=sym_inter[2*k]<<8;
    			target_out[k]|=sym_inter[2*k+1];
    			//fprintf(fout,"%04X ",target_out[k]);
    			//if((k+1)%8==0)
    			//	fprintf(fout,"\n");
    		}
    	}
		for (k=N_INFO_WORD;k<(N_INFO_WORD+5);k++) {
			RDD[7+((k-N_INFO_WORD)<<1)] = (unsigned char)sym_inter[2*k];
			RDD[6+((k-N_INFO_WORD)<<1)] = (unsigned char)sym_inter[2*k+1];
    	}
    }
	//fclose(fid);
	//fclose(fout);
	//fclose(ferror);
}

int ecc_cal(int ECC_MODE,unsigned char *pSourceData,unsigned char *pSpareArea,int REDU_LENGTH,unsigned char *pOutputData,char debug)
{
    int err;
    U32 u32_i;
    #if 0
    if (RS_4_512 == ECC_MODE)
    {
        Calculate_ECC(pSourceData, pSpareArea);
        return 1;
    }
	else
    #endif
	{     
        printf("%d\n",ECC_MODE,REDU_LENGTH);
        #if ECC_LIB_FCIE3
        err = (*pFn_ecc_encode)(3, ECC_MODE, pSourceData, pSpareArea, REDU_LENGTH, pOutputData, debug);  
        #elif ECC_LIB_FCIE4
        err = (*pFn_ecc_encode)(4, ECC_MODE, pSourceData, pSpareArea, REDU_LENGTH, pOutputData, debug); 
        #elif ECC_LIB_FCIE5        
        err = (*pFn_ecc_encode)(5, ECC_MODE, pSourceData, pSpareArea, REDU_LENGTH, pOutputData, debug);
        #endif
	    return err;
    }
	    
    return 0;
}

int gen_ecc_with_sector_size(char *SourceBuffer, char *SpareBuf, int ECC_Mode)
{
	int res;
	int ecc_offset, ecc_len;
	U16 u16_tmp, u16_i, *pu16;
	U8  u8_tmp[512];
    U32 u32_i;

	ecc_len = pNAND_ECC_CODELEN[ECC_Mode];
    u16_tmp = 4-((gu16_SectorSpareByteCnt - ecc_len)%4);

    #if ECC_LIB_FCIE5
    ecc_offset = (gu16_SectorSpareByteCnt - ecc_len)+u16_tmp;
    switch(u16_tmp)
    {
        case 1:
        SpareBuf[gu16_SectorSpareByteCnt - ecc_len]=0;   
        break;
        case 2:
        SpareBuf[gu16_SectorSpareByteCnt - ecc_len]=0;
        SpareBuf[gu16_SectorSpareByteCnt - ecc_len+1]=0;
        break; 
        case 3:
        SpareBuf[gu16_SectorSpareByteCnt - ecc_len]=0;
        SpareBuf[gu16_SectorSpareByteCnt - ecc_len+1]=0;
        SpareBuf[gu16_SectorSpareByteCnt - ecc_len+2]=0;
        break;  
        default:
        break;          
    }

    #else
	ecc_offset = gu16_SectorSpareByteCnt - ecc_len;
    #endif

	
	#if ECC_LIB_FCIE5

	res = ecc_cal(
		NAND_ECC_TYPE_CIS2LIB[ECC_Mode], 
		SourceBuffer, SpareBuf, ecc_offset, 
		u8_tmp, 0);

	#else

	res = ecc_cal(
		NAND_ECC_TYPE_CIS2LIB[ECC_Mode], 
		SourceBuffer, SpareBuf, ecc_offset, 
		SpareBuf + ecc_offset, 0);
	#endif

    #if ECC_LIB_FCIE5
	memcpy(SpareBuf + ecc_offset - u16_tmp, u8_tmp, ecc_len);
	/*
    if((gu16_SectorSpareByteCnt - ecc_len)%4)
    {
       for(u16_i=0; u16_i< gu16_SectorSpareByteCnt;u16_i++)
       {
            if(u16_i< (gu16_SectorSpareByteCnt - ecc_len))
               u8_tmp[u16_i] = SpareBuf[u16_i];
            else
               u8_tmp[u16_i] = SpareBuf[u16_i+u16_tmp];
       }
       memcpy(SpareBuf,u8_tmp,gu16_SectorSpareByteCnt);
    }
	*/
    #endif

	return res;
}

int gen_ecc_with_page_size(char *SourceBuffer, int ECC_Mode)
{
	int res, i;
    
	for( i = 0 ; i < spi_nand.u16_PageByteCnt/ gu16_SectorByteCnt; i ++)
	{
		res = gen_ecc_with_sector_size(
			SourceBuffer + gu16_SectorByteCnt * i, 
			SourceBuffer + spi_nand.u16_PageByteCnt + gu16_SectorSpareByteCnt * i, 
			ECC_Mode);

		// patch for bug(s) in ECC lib
	}
	return res;
}

