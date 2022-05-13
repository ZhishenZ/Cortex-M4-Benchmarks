/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
typedef struct {
    int32_t real, imag;
} COMPLEX;

/* function prototypes for fft and filter functions */
void fft(COMPLEX *,int32_t);
int32_t fir_filter(int32_t input,int32_t *coef,int32_t n,int32_t *history);
int32_t iir_filter(int32_t input,int32_t *coef,int32_t n,int32_t *history);
int32_t gaussian(void);
int32_t my_abs(int32_t n);

void setup_codec(int32_t),key_down(),int_enable(),int_disable();
int32_t flags(int32_t);

int32_t getinput(void);
void sendout(int32_t),flush();

int32_t encode(int32_t,int32_t);
void decode(int32_t);
int32_t filtez(int32_t *bpl,int32_t *dlt);
void upzero(int32_t dlt,int32_t *dlti,int32_t *bli);
int32_t filtep(int32_t rlt1,int32_t al1,int32_t rlt2,int32_t al2);
int32_t quantl(int32_t el,int32_t detl);
/* int32_t invqxl(int32_t il,int32_t detl,int32_t *code_table,int32_t mode); */
int32_t logscl(int32_t il,int32_t nbl);
int32_t scalel(int32_t nbl,int32_t shift_constant);
int32_t uppol2(int32_t al1,int32_t al2,int32_t plt,int32_t plt1,int32_t plt2);
int32_t uppol1(int32_t al1,int32_t apl2,int32_t plt,int32_t plt1);
/* int32_t invqah(int32_t ih,int32_t deth); */
int32_t logsch(int32_t ih,int32_t nbh);
void reset();
int32_t my_fabs(int32_t n);
int32_t my_cos(int32_t n);
int32_t my_sin(int32_t n);
/* variables for transimit quadrature mirror filter here */
int32_t tqmf[24];

/* QMF filter coefficients:
scaled by a factor of 4 compared to G722 CCITT recommendation */
#if(DATAINFLASH)
static const int32_t h[24] = {
	    12,   -44,   -44,   212,    48,  -624,   128,  1448,
	  -840, -3220,  3804, 15504, 15504,  3804, -3220,  -840,
	  1448,   128,  -624,    48,   212,   -44,   -44,    12
	};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t h[24] = {
	    12,   -44,   -44,   212,    48,  -624,   128,  1448,
	  -840, -3220,  3804, 15504, 15504,  3804, -3220,  -840,
	  1448,   128,  -624,    48,   212,   -44,   -44,    12
	};
#else
int32_t h[24] = {
	    12,   -44,   -44,   212,    48,  -624,   128,  1448,
	  -840, -3220,  3804, 15504, 15504,  3804, -3220,  -840,
	  1448,   128,  -624,    48,   212,   -44,   -44,    12
	};
#endif
#endif


int32_t xl,xh;

/* variables for receive quadrature mirror filter here */
int32_t accumc[11],accumd[11];

/* outputs of decode() */
int32_t xout1,xout2;

int32_t xs,xd;

/* variables for encoder (hi and lo) here */

int32_t il,szl,spl,sl,el;

#if(DATAINFLASH)
static const int32_t qq4_code4_table[16] = {
	     0,  -20456,  -12896,   -8968,   -6288,   -4240,   -2584,   -1200,
	 20456,   12896,    8968,    6288,    4240,    2584,    1200,       0
	};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t qq4_code4_table[16] = {
	     0,  -20456,  -12896,   -8968,   -6288,   -4240,   -2584,   -1200,
	 20456,   12896,    8968,    6288,    4240,    2584,    1200,       0
	};
#else
int32_t qq4_code4_table[16] = {
     0,  -20456,  -12896,   -8968,   -6288,   -4240,   -2584,   -1200,
 20456,   12896,    8968,    6288,    4240,    2584,    1200,       0
};
#endif
#endif


#if(DATAINFLASH)
static const int32_t qq5_code5_table[32] = {
		  -280,    -280,  -23352,  -17560,  -14120,  -11664,   -9752,   -8184,
		 -6864,   -5712,   -4696,   -3784,   -2960,   -2208,   -1520,    -880,
		 23352,   17560,   14120,   11664,    9752,    8184,    6864,    5712,
		  4696,    3784,    2960,    2208,    1520,     880,     280,    -280
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t qq5_code5_table[32] = {
		  -280,    -280,  -23352,  -17560,  -14120,  -11664,   -9752,   -8184,
		 -6864,   -5712,   -4696,   -3784,   -2960,   -2208,   -1520,    -880,
		 23352,   17560,   14120,   11664,    9752,    8184,    6864,    5712,
		  4696,    3784,    2960,    2208,    1520,     880,     280,    -280
		};
#else
int32_t qq5_code5_table[32] = {
  -280,    -280,  -23352,  -17560,  -14120,  -11664,   -9752,   -8184,
 -6864,   -5712,   -4696,   -3784,   -2960,   -2208,   -1520,    -880,
 23352,   17560,   14120,   11664,    9752,    8184,    6864,    5712,
  4696,    3784,    2960,    2208,    1520,     880,     280,    -280
};
#endif
#endif


#if(DATAINFLASH)
static const int32_t qq6_code6_table[64] = {
		  -136,    -136,    -136,    -136,  -24808,  -21904,  -19008,  -16704,
		-14984,  -13512,  -12280,  -11192,  -10232,   -9360,   -8576,   -7856,
		 -7192,   -6576,   -6000,   -5456,   -4944,   -4464,   -4008,   -3576,
		 -3168,   -2776,   -2400,   -2032,   -1688,   -1360,   -1040,    -728,
		 24808,   21904,   19008,   16704,   14984,   13512,   12280,   11192,
		 10232,    9360,    8576,    7856,    7192,    6576,    6000,    5456,
		  4944,    4464,    4008,    3576,    3168,    2776,    2400,    2032,
		  1688,    1360,    1040,     728,     432,     136,    -432,    -136
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t qq6_code6_table[64] = {
		  -136,    -136,    -136,    -136,  -24808,  -21904,  -19008,  -16704,
		-14984,  -13512,  -12280,  -11192,  -10232,   -9360,   -8576,   -7856,
		 -7192,   -6576,   -6000,   -5456,   -4944,   -4464,   -4008,   -3576,
		 -3168,   -2776,   -2400,   -2032,   -1688,   -1360,   -1040,    -728,
		 24808,   21904,   19008,   16704,   14984,   13512,   12280,   11192,
		 10232,    9360,    8576,    7856,    7192,    6576,    6000,    5456,
		  4944,    4464,    4008,    3576,    3168,    2776,    2400,    2032,
		  1688,    1360,    1040,     728,     432,     136,    -432,    -136
		};
#else
int32_t qq6_code6_table[64] = {
  -136,    -136,    -136,    -136,  -24808,  -21904,  -19008,  -16704,
-14984,  -13512,  -12280,  -11192,  -10232,   -9360,   -8576,   -7856,
 -7192,   -6576,   -6000,   -5456,   -4944,   -4464,   -4008,   -3576,
 -3168,   -2776,   -2400,   -2032,   -1688,   -1360,   -1040,    -728,
 24808,   21904,   19008,   16704,   14984,   13512,   12280,   11192,
 10232,    9360,    8576,    7856,    7192,    6576,    6000,    5456,
  4944,    4464,    4008,    3576,    3168,    2776,    2400,    2032,
  1688,    1360,    1040,     728,     432,     136,    -432,    -136
};
#endif
#endif


int32_t delay_bpl[6];

int32_t delay_dltx[6];

#if(DATAINFLASH)
static const int32_t wl_code_table[16] = {
		   -60,  3042,  1198,   538,   334,   172,    58,   -30,
		  3042,  1198,   538,   334,   172,    58,   -30,   -60
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t wl_code_table[16] = {
   -60,  3042,  1198,   538,   334,   172,    58,   -30,
  3042,  1198,   538,   334,   172,    58,   -30,   -60
};
#else
int32_t wl_code_table[16] = {
   -60,  3042,  1198,   538,   334,   172,    58,   -30,
  3042,  1198,   538,   334,   172,    58,   -30,   -60
};
#endif
#endif


#if(DATAINFLASH)
static const int32_t wl_table[8] = {
		   -60,   -30,    58,   172,   334,   538,  1198,  3042
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t wl_table[8] = {
		   -60,   -30,    58,   172,   334,   538,  1198,  3042
		};
#else
int32_t wl_table[8] = {
   -60,   -30,    58,   172,   334,   538,  1198,  3042
};
#endif
#endif


#if(DATAINFLASH)
static const int32_t ilb_table[32] = {
		  2048,  2093,  2139,  2186,  2233,  2282,  2332,  2383,
		  2435,  2489,  2543,  2599,  2656,  2714,  2774,  2834,
		  2896,  2960,  3025,  3091,  3158,  3228,  3298,  3371,
		  3444,  3520,  3597,  3676,  3756,  3838,  3922,  4008
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t ilb_table[32] = {
  2048,  2093,  2139,  2186,  2233,  2282,  2332,  2383,
  2435,  2489,  2543,  2599,  2656,  2714,  2774,  2834,
  2896,  2960,  3025,  3091,  3158,  3228,  3298,  3371,
  3444,  3520,  3597,  3676,  3756,  3838,  3922,  4008
};
#else
int32_t ilb_table[32] = {
  2048,  2093,  2139,  2186,  2233,  2282,  2332,  2383,
  2435,  2489,  2543,  2599,  2656,  2714,  2774,  2834,
  2896,  2960,  3025,  3091,  3158,  3228,  3298,  3371,
  3444,  3520,  3597,  3676,  3756,  3838,  3922,  4008
};
#endif
#endif


int32_t         nbl;                  /* delay line */
int32_t         al1,al2;
int32_t         plt,plt1,plt2;
int32_t         rs;
int32_t         dlt;
int32_t         rlt,rlt1,rlt2;

/* decision levels - pre-multiplied by 8, 0 to indicate end */
#if(DATAINFLASH)
static const int32_t decis_levl[30] = {
		   280,   576,   880,  1200,  1520,  1864,  2208,  2584,
		  2960,  3376,  3784,  4240,  4696,  5200,  5712,  6288,
		  6864,  7520,  8184,  8968,  9752, 10712, 11664, 12896,
		 14120, 15840, 17560, 20456, 23352, 32767
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t decis_levl[30] = {
		   280,   576,   880,  1200,  1520,  1864,  2208,  2584,
		  2960,  3376,  3784,  4240,  4696,  5200,  5712,  6288,
		  6864,  7520,  8184,  8968,  9752, 10712, 11664, 12896,
		 14120, 15840, 17560, 20456, 23352, 32767
		};
#else
int32_t decis_levl[30] = {
   280,   576,   880,  1200,  1520,  1864,  2208,  2584,
  2960,  3376,  3784,  4240,  4696,  5200,  5712,  6288,
  6864,  7520,  8184,  8968,  9752, 10712, 11664, 12896,
 14120, 15840, 17560, 20456, 23352, 32767
};
#endif
#endif


int32_t         detl;

/* quantization table 31 int64_t to make quantl look-up easier,
last entry is for mil=30 case when wd is max */
#if(DATAINFLASH)
static const int32_t quant26bt_pos[31] = {
	    61,    60,    59,    58,    57,    56,    55,    54,
	    53,    52,    51,    50,    49,    48,    47,    46,
	    45,    44,    43,    42,    41,    40,    39,    38,
	    37,    36,    35,    34,    33,    32,    32
	};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t quant26bt_pos[31] = {
	    61,    60,    59,    58,    57,    56,    55,    54,
	    53,    52,    51,    50,    49,    48,    47,    46,
	    45,    44,    43,    42,    41,    40,    39,    38,
	    37,    36,    35,    34,    33,    32,    32
	};
#else
int32_t quant26bt_pos[31] = {
    61,    60,    59,    58,    57,    56,    55,    54,
    53,    52,    51,    50,    49,    48,    47,    46,
    45,    44,    43,    42,    41,    40,    39,    38,
    37,    36,    35,    34,    33,    32,    32
};
#endif
#endif


/* quantization table 31 int64_t to make quantl look-up easier,
last entry is for mil=30 case when wd is max */
#if(DATAINFLASH)
static const int32_t quant26bt_neg[31] = {
	    63,    62,    31,    30,    29,    28,    27,    26,
	    25,    24,    23,    22,    21,    20,    19,    18,
	    17,    16,    15,    14,    13,    12,    11,    10,
	     9,     8,     7,     6,     5,     4,     4
	};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t quant26bt_neg[31] = {
	    63,    62,    31,    30,    29,    28,    27,    26,
	    25,    24,    23,    22,    21,    20,    19,    18,
	    17,    16,    15,    14,    13,    12,    11,    10,
	     9,     8,     7,     6,     5,     4,     4
	};
#else
int32_t quant26bt_neg[31] = {
    63,    62,    31,    30,    29,    28,    27,    26,
    25,    24,    23,    22,    21,    20,    19,    18,
    17,    16,    15,    14,    13,    12,    11,    10,
     9,     8,     7,     6,     5,     4,     4
};
#endif
#endif


int32_t         deth;
int32_t         sh;         /* this comes from adaptive predictor */
int32_t         eh;

#if(DATAINFLASH)
static const int32_t qq2_code2_table[4] = {
		  -7408,   -1616,   7408,  1616
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t qq2_code2_table[4] = {
		  -7408,   -1616,   7408,  1616
		};
#else
int32_t qq2_code2_table[4] = {
  -7408,   -1616,   7408,  1616
};
#endif
#endif


#if(DATAINFLASH)
static const int32_t wh_code_table[4] = {
		   798,   -214,    798,   -214
		};
#else
#ifdef DATAINCCMRAM
__attribute__((section(".ccmramdata"))) int32_t wh_code_table[4] = {
		   798,   -214,    798,   -214
		};
#else
int32_t wh_code_table[4] = {
   798,   -214,    798,   -214
};
#endif
#endif



int32_t         dh,ih;
int32_t         nbh,szh;
int32_t         sph,ph,yh,rh;

int32_t         delay_dhx[6];

int32_t         delay_bph[6];

int32_t         ah1,ah2;
int32_t         ph1,ph2;
int32_t         rh1,rh2;

/* variables for decoder here */
int32_t         ilr,yl,rl;
int32_t         dec_deth,dec_detl,dec_dlt;

int32_t         dec_del_bpl[6];

int32_t         dec_del_dltx[6];

int32_t     dec_plt,dec_plt1,dec_plt2;
int32_t     dec_szl,dec_spl,dec_sl;
int32_t     dec_rlt1,dec_rlt2,dec_rlt;
int32_t     dec_al1,dec_al2;
int32_t     dl;
int32_t     dec_nbl,dec_yh,dec_dh,dec_nbh;

/* variables used in filtez */
int32_t         dec_del_bph[6];

int32_t         dec_del_dhx[6];

int32_t         dec_szh;
/* variables used in filtep */
int32_t         dec_rh1,dec_rh2;
int32_t         dec_ah1,dec_ah2;
int32_t         dec_ph,dec_sph;

int32_t     dec_sh,dec_rh;

int32_t     dec_ph1,dec_ph2;

/* G722 encode function two ints in, one 8 bit output */

/* put input samples in xin1 = first value, xin2 = second value */
/* returns il and ih stored together */

/* MAX: 1 */
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t my_abs(int32_t n)
{
  int32_t m;

  if (n >= 0) m = n;
  else m = -n;
  return m;
}

/* MAX: 1 */
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t my_fabs(int32_t n)
{
  int32_t f;

  if (n >= 0) f = n;
  else f = -n;
  return f;
}

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t my_sin(int32_t rad)
{
  int32_t diff;
  int32_t app=0;

  int32_t inc = 1;

  /* MAX dependent on rad's value, say 50 */
  while (rad > 2*PI)
      rad -= 2*PI;
  /* MAX dependent on rad's value, say 50 */
  while (rad < -2*PI)
      rad += 2*PI;
   diff = rad;
   app = diff;
  diff = (diff * (-(rad*rad))) /
     ((2 * inc) * (2 * inc + 1));
  app = app + diff;
  inc++;
  /* REALLY: while(my_fabs(diff) >= 0.00001) { */
  /* MAX: 1000 */
  while(my_fabs(diff) >= 1) {
    diff = (diff * (-(rad*rad))) /
	((2 * inc) * (2 * inc + 1));
    app = app + diff;
    inc++;
  }

  return app;
}

/*
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t my_cos(int32_t rad)
{
  return (my_sin (PI / 2 - rad));
}*/


/* MAX: 1 */
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t encode(int32_t xin1,int32_t xin2)
{
    int32_t i;
    int32_t *h_ptr,*tqmf_ptr,*tqmf_ptr1;
    int64_t  xa,xb;
    int32_t decis;

/* transmit quadrature mirror filters implemented here */
    h_ptr = h;
    tqmf_ptr = tqmf;
    xa = (int64_t)(*tqmf_ptr++) * (*h_ptr++);
    xb = (int64_t)(*tqmf_ptr++) * (*h_ptr++);
/* main multiply accumulate loop for samples and coefficients */
    /* MAX: 10 */
    for(i = 0 ; i < 10 ; i++) {
        xa += (int64_t)(*tqmf_ptr++) * (*h_ptr++);
        xb += (int64_t)(*tqmf_ptr++) * (*h_ptr++);
    }
/* final mult/accumulate */
    xa += (int64_t)(*tqmf_ptr++) * (*h_ptr++);
    xb += (int64_t)(*tqmf_ptr) * (*h_ptr++);

/* update delay line tqmf */
    tqmf_ptr1 = tqmf_ptr - 2;
    /* MAX: 22 */
    for(i = 0 ; i < 22 ; i++)
        *tqmf_ptr-- = *tqmf_ptr1--;
    *tqmf_ptr-- = xin1;
    *tqmf_ptr = xin2;

/* scale outputs */
    xl = (xa + xb) >> 15;
    xh = (xa - xb) >> 15;

/* end of quadrature mirror filter code */

/* starting with lower sub band encoder */

/* filtez - compute predictor output section - zero section */
    szl = filtez(delay_bpl,delay_dltx);

/* filtep - compute predictor output signal (pole section) */
    spl = filtep(rlt1,al1,rlt2,al2);

/* compute the predictor output value in the lower sub_band encoder */
    sl = szl + spl;
    el = xl - sl;

/* quantl: quantize the difference signal */
    il = quantl(el,detl);

/* invqxl: computes quantized difference signal */
/* for invqbl, truncate by 2 lsbs, so mode = 3 */
    dlt = ((int64_t)detl*qq4_code4_table[il >> 2]) >> 15;

/* logscl: updates logarithmic quant. scale factor in low sub band */
    nbl = logscl(il,nbl);

/* scalel: compute the quantizer scale factor in the lower sub band */
/* calling parameters nbl and 8 (constant such that scalel can be scaleh) */
    detl = scalel(nbl,8);

/* parrec - simple addition to compute recontructed signal for adaptive pred */
    plt = dlt + szl;

/* upzero: update zero section predictor coefficients (sixth order)*/
/* calling parameters: dlt, dlt1, dlt2, ..., dlt6 from dlt */
/*  bpli (linear_buffer in which all six values are delayed */
/* return params:      updated bpli, delayed dltx */
    upzero(dlt,delay_dltx,delay_bpl);

/* uppol2- update second predictor coefficient apl2 and delay it as al2 */
/* calling parameters: al1, al2, plt, plt1, plt2 */
    al2 = uppol2(al1,al2,plt,plt1,plt2);

/* uppol1 :update first predictor coefficient apl1 and delay it as al1 */
/* calling parameters: al1, apl2, plt, plt1 */
    al1 = uppol1(al1,al2,plt,plt1);

/* recons : compute recontructed signal for adaptive predictor */
    rlt = sl + dlt;

/* done with lower sub_band encoder; now implement delays for next time*/
    rlt2 = rlt1;
    rlt1 = rlt;
    plt2 = plt1;
    plt1 = plt;

/* high band encode */

    szh = filtez(delay_bph,delay_dhx);

    sph = filtep(rh1,ah1,rh2,ah2);

/* predic: sh = sph + szh */
    sh = sph + szh;
/* subtra: eh = xh - sh */
    eh = xh - sh;

/* quanth - quantization of difference signal for higher sub-band */
/* quanth: in-place for speed params: eh, deth (has init. value) */
    if(eh >= 0) {
        ih = 3;     /* 2,3 are pos codes */
    }
    else {
        ih = 1;     /* 0,1 are neg codes */
    }
    decis = (564L*(int64_t)deth) >> 12L;
    if(my_abs(eh) > decis) ih--;     /* mih = 2 case */

/* invqah: compute the quantized difference signal, higher sub-band*/
    dh = ((int64_t)deth*qq2_code2_table[ih]) >> 15L ;

/* logsch: update logarithmic quantizer scale factor in hi sub-band*/
    nbh = logsch(ih,nbh);

/* note : scalel and scaleh use same code, different parameters */
    deth = scalel(nbh,10);

/* parrec - add pole predictor output to quantized diff. signal */
    ph = dh + szh;

/* upzero: update zero section predictor coefficients (sixth order) */
/* calling parameters: dh, dhi, bphi */
/* return params: updated bphi, delayed dhx */
    upzero(dh,delay_dhx,delay_bph);

/* uppol2: update second predictor coef aph2 and delay as ah2 */
/* calling params: ah1, ah2, ph, ph1, ph2 */
    ah2 = uppol2(ah1,ah2,ph,ph1,ph2);

/* uppol1:  update first predictor coef. aph2 and delay it as ah1 */
    ah1 = uppol1(ah1,ah2,ph,ph1);

/* recons for higher sub-band */
    yh = sh + dh;

/* done with higher sub-band encoder, now Delay for next time */
    rh2 = rh1;
    rh1 = yh;
    ph2 = ph1;
    ph1 = ph;

/* multiplex ih and il to get signals together */
    return(il | (ih << 6));
}

/* decode function, result in xout1 and xout2 */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
void decode(int32_t input)
{
    int32_t i;
    int64_t  xa1,xa2;    /* qmf accumulators */
    int32_t *h_ptr,*ac_ptr,*ac_ptr1,*ad_ptr,*ad_ptr1;

/* split transmitted word from input into ilr and ih */
    ilr = input & 0x3f;
    ih = input >> 6;

/* LOWER SUB_BAND DECODER */

/* filtez: compute predictor output for zero section */
    dec_szl = filtez(dec_del_bpl,dec_del_dltx);

/* filtep: compute predictor output signal for pole section */
    dec_spl = filtep(dec_rlt1,dec_al1,dec_rlt2,dec_al2);

    dec_sl = dec_spl + dec_szl;

/* invqxl: compute quantized difference signal for adaptive predic */
    dec_dlt = ((int64_t)dec_detl*qq4_code4_table[ilr >> 2]) >> 15;

/* invqxl: compute quantized difference signal for decoder output */
    dl = ((int64_t)dec_detl*qq6_code6_table[il]) >> 15;

    rl = dl + dec_sl;

/* logscl: quantizer scale factor adaptation in the lower sub-band */
    dec_nbl = logscl(ilr,dec_nbl);

/* scalel: computes quantizer scale factor in the lower sub band */
    dec_detl = scalel(dec_nbl,8);

/* parrec - add pole predictor output to quantized diff. signal */
/* for partially reconstructed signal */
    dec_plt = dec_dlt + dec_szl;

/* upzero: update zero section predictor coefficients */
    upzero(dec_dlt,dec_del_dltx,dec_del_bpl);

/* uppol2: update second predictor coefficient apl2 and delay it as al2 */
    dec_al2 = uppol2(dec_al1,dec_al2,dec_plt,dec_plt1,dec_plt2);

/* uppol1: update first predictor coef. (pole setion) */
    dec_al1 = uppol1(dec_al1,dec_al2,dec_plt,dec_plt1);

/* recons : compute recontructed signal for adaptive predictor */
    dec_rlt = dec_sl + dec_dlt;

/* done with lower sub band decoder, implement delays for next time */
    dec_rlt2 = dec_rlt1;
    dec_rlt1 = dec_rlt;
    dec_plt2 = dec_plt1;
    dec_plt1 = dec_plt;

/* HIGH SUB-BAND DECODER */

/* filtez: compute predictor output for zero section */
    dec_szh = filtez(dec_del_bph,dec_del_dhx);

/* filtep: compute predictor output signal for pole section */
    dec_sph = filtep(dec_rh1,dec_ah1,dec_rh2,dec_ah2);

/* predic:compute the predictor output value in the higher sub_band decoder */
    dec_sh = dec_sph + dec_szh;

/* invqah: in-place compute the quantized difference signal */
    dec_dh = ((int64_t)dec_deth*qq2_code2_table[ih]) >> 15L ;

/* logsch: update logarithmic quantizer scale factor in hi sub band */
    dec_nbh = logsch(ih,dec_nbh);

/* scalel: compute the quantizer scale factor in the higher sub band */
    dec_deth = scalel(dec_nbh,10);

/* parrec: compute partially recontructed signal */
    dec_ph = dec_dh + dec_szh;

/* upzero: update zero section predictor coefficients */
    upzero(dec_dh,dec_del_dhx,dec_del_bph);

/* uppol2: update second predictor coefficient aph2 and delay it as ah2 */
    dec_ah2 = uppol2(dec_ah1,dec_ah2,dec_ph,dec_ph1,dec_ph2);

/* uppol1: update first predictor coef. (pole setion) */
    dec_ah1 = uppol1(dec_ah1,dec_ah2,dec_ph,dec_ph1);

/* recons : compute recontructed signal for adaptive predictor */
    rh = dec_sh + dec_dh;

/* done with high band decode, implementing delays for next time here */
    dec_rh2 = dec_rh1;
    dec_rh1 = rh;
    dec_ph2 = dec_ph1;
    dec_ph1 = dec_ph;

/* end of higher sub_band decoder */

/* end with receive quadrature mirror filters */
    xd = rl - rh;
    xs = rl + rh;

/* receive quadrature mirror filters implemented here */
    h_ptr = h;
    ac_ptr = accumc;
    ad_ptr = accumd;
    xa1 = (int64_t)xd * (*h_ptr++);
    xa2 = (int64_t)xs * (*h_ptr++);
/* main multiply accumulate loop for samples and coefficients */
    for(i = 0 ; i < 10 ; i++) {
        xa1 += (int64_t)(*ac_ptr++) * (*h_ptr++);
        xa2 += (int64_t)(*ad_ptr++) * (*h_ptr++);
    }
/* final mult/accumulate */
    xa1 += (int64_t)(*ac_ptr) * (*h_ptr++);
    xa2 += (int64_t)(*ad_ptr) * (*h_ptr++);

/* scale by 2^14 */
    xout1 = xa1 >> 14;
    xout2 = xa2 >> 14;

/* update delay lines */
    ac_ptr1 = ac_ptr - 1;
    ad_ptr1 = ad_ptr - 1;
    for(i = 0 ; i < 10 ; i++) {
        *ac_ptr-- = *ac_ptr1--;
        *ad_ptr-- = *ad_ptr1--;
    }
    *ac_ptr = xd;
    *ad_ptr = xs;

    return;
}

/* clear all storage locations */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
void reset()
{
    int32_t i;

    detl = dec_detl = 32;   /* reset to min scale factor */
    deth = dec_deth = 8;
    nbl = al1 = al2 = plt1 = plt2 = rlt1 = rlt2 = 0;
    nbh = ah1 = ah2 = ph1 = ph2 = rh1 = rh2 = 0;
    dec_nbl = dec_al1 = dec_al2 = dec_plt1 = dec_plt2 = dec_rlt1 = dec_rlt2 = 0;
    dec_nbh = dec_ah1 = dec_ah2 = dec_ph1 = dec_ph2 = dec_rh1 = dec_rh2 = 0;

    for(i = 0 ; i < 6 ; i++) {
        delay_dltx[i] = 0;
        delay_dhx[i] = 0;
        dec_del_dltx[i] = 0;
        dec_del_dhx[i] = 0;
    }

    for(i = 0 ; i < 6 ; i++) {
        delay_bpl[i] = 0;
        delay_bph[i] = 0;
        dec_del_bpl[i] = 0;
        dec_del_bph[i] = 0;
    }

    for(i = 0 ; i < 23 ; i++) tqmf[i] = 0;

    for(i = 0 ; i < 11 ; i++) {
        accumc[i] = 0;
        accumd[i] = 0;
    }
    return;
}

/* filtez - compute predictor output signal (zero section) */
/* input: bpl1-6 and dlt1-6, output: szl */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t filtez(int32_t *bpl,int32_t *dlt)
{
    int32_t i;
    int64_t  zl;
    zl = (int64_t)(*bpl++) * (*dlt++);
    /* MAX: 6 */
    for(i = 1 ; i < 6 ; i++)
        zl += (int64_t)(*bpl++) * (*dlt++);

    return((int32_t)(zl >> 14));   /* x2 here */
}

/* filtep - compute predictor output signal (pole section) */
/* input rlt1-2 and al1-2, output spl */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t filtep(int32_t rlt1,int32_t al1,int32_t rlt2,int32_t al2)
{
    int64_t  pl,pl2;
    pl = 2*rlt1;
    pl = (int64_t)al1*pl;
    pl2 = 2*rlt2;
    pl += (int64_t)al2*pl2;
    return((int32_t)(pl >> 15));
}

/* quantl - quantize the difference signal in the lower sub-band */
#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t quantl(int32_t el,int32_t detl)
{
    int32_t ril,mil;
    int64_t  wd,decis;
/* abs of difference signal */
    wd = my_abs(el);
/* determine mil based on decision levels and detl gain */
    /* MAX: 30 */
    for(mil = 0 ; mil < 30 ; mil++) {
        decis = (decis_levl[mil]*(int64_t)detl) >> 15L;
        if(wd <= decis) break;
    }
/* if mil=30 then wd is less than all decision levels */
    if(el >= 0) ril = quant26bt_pos[mil];
    else ril = quant26bt_neg[mil];
    return(ril);
}

/* invqxl is either invqbl or invqal depending on parameters passed */
/* returns dlt, code table is pre-multiplied by 8 */

/*    int32_t invqxl(int32_t il,int32_t detl,int32_t *code_table,int32_t mode) */
/*    { */
/*        int64_t  dlt; */
/*       dlt = (int64_t)detl*code_table[il >> (mode-1)]; */
/*        return((int32_t)(dlt >> 15)); */
/*    } */

/* logscl - update log quantizer scale factor in lower sub-band */
/* note that nbl is passed and returned */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t logscl(int32_t il,int32_t nbl)
{
    int64_t  wd;
    wd = ((int64_t)nbl * 127L) >> 7L;   /* line 706*/
    nbl = (int32_t)wd + wl_code_table[il >> 2];
    if(nbl < 0) nbl = 0;
    if(nbl > 18432) nbl = 18432;
    return(nbl);
}

/* scalel: compute quantizer scale factor in lower or upper sub-band*/

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t scalel(int32_t nbl,int32_t shift_constant)
{
    int32_t wd1,wd2,wd3;
    wd1 = (nbl >> 6) & 31;
    wd2 = nbl >> 11;
    wd3 = ilb_table[wd1] >> (shift_constant + 1 - wd2);
    return(wd3 << 3);
}

/* upzero - inputs: dlt, dlti[0-5], bli[0-5], outputs: updated bli[0-5] */
/* also implements delay of bli and update of dlti from dlt */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
void upzero(int32_t dlt,int32_t *dlti,int32_t *bli)
{
    int32_t i,wd2,wd3;
/*if dlt is zero, then no sum into bli */
    if(dlt == 0) {
      for(i = 0 ; i < 6 ; i++) {
        bli[i] = (int32_t)((255L*bli[i]) >> 8L); /* leak factor of 255/256 */
      }
    }
    else {
      for(i = 0 ; i < 6 ; i++) {
        if((int64_t)dlt*dlti[i] >= 0) wd2 = 128; else wd2 = -128;
        wd3 = (int32_t)((255L*bli[i]) >> 8L);    /* leak factor of 255/256 */
        bli[i] = wd2 + wd3;
      }
    }
/* implement delay line for dlt */
    dlti[5] = dlti[4];
    dlti[4] = dlti[3];
    dlti[3] = dlti[2];
    dlti[1] = dlti[0];
    dlti[0] = dlt;
    return;
}

/* uppol2 - update second predictor coefficient (pole section) */
/* inputs: al1, al2, plt, plt1, plt2. outputs: apl2 */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t uppol2(int32_t al1,int32_t al2,int32_t plt,int32_t plt1,int32_t plt2)
{
    int64_t  wd2,wd4;
    int32_t apl2;
    wd2 = 4L*(int64_t)al1;
    if((int64_t)plt*plt1 >= 0L) wd2 = -wd2;    /* check same sign */
    wd2 = wd2 >> 7;                  /* gain of 1/128 */
    if((int64_t)plt*plt2 >= 0L) {
        wd4 = wd2 + 128;             /* same sign case */
    }
    else {
        wd4 = wd2 - 128;
    }
    apl2 = wd4 + (127L*(int64_t)al2 >> 7L);  /* leak factor of 127/128 */

/* apl2 is limited to +-.75 */
    if(apl2 > 12288) apl2 = 12288;
    if(apl2 < -12288) apl2 = -12288;
    return(apl2);
}

/* uppol1 - update first predictor coefficient (pole section) */
/* inputs: al1, apl2, plt, plt1. outputs: apl1 */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t uppol1(int32_t al1,int32_t apl2,int32_t plt,int32_t plt1)
{
    int64_t  wd2;
    int32_t wd3,apl1;
    wd2 = ((int64_t)al1*255L) >> 8L;   /* leak factor of 255/256 */
    if((int64_t)plt*plt1 >= 0L) {
        apl1 = (int32_t)wd2 + 192;      /* same sign case */
    }
    else {
        apl1 = (int32_t)wd2 - 192;
    }
/* note: wd3= .9375-.75 is always positive */
    wd3 = 15360 - apl2;             /* limit value */
    if(apl1 > wd3) apl1 = wd3;
    if(apl1 < -wd3) apl1 = -wd3;
    return(apl1);
}
/* INVQAH: inverse adaptive quantizer for the higher sub-band */
/* returns dh, code table is pre-multiplied by 8 */

/*  int32_t invqah(int32_t ih,int32_t deth) */
/*  { */
/*        int64_t  rdh; */
/*        rdh = ((int64_t)deth*qq2_code2_table[ih]) >> 15L ; */
/*        return((int32_t)(rdh )); */
/*  } */

#if(CCMRAM)
__attribute__((section(".ccmram")))
#endif
int32_t logsch(int32_t ih,int32_t nbh)
{
    int32_t wd;
    wd = ((int64_t)nbh * 127L) >> 7L;       /* leak factor 127/128 */
    nbh = wd + wh_code_table[ih];
    if(nbh < 0) nbh = 0;
    if(nbh > 22528) nbh = 22528;
    return(nbh);
}

#if(INPUTDATAINFLASH)
static const
#else
#ifdef INPUTDATAINCCMRAM
__attribute__((section(".ccmram")))
#endif
#endif
int32_t test_data[SIZE*2] //STORE_IN_TEXT_SEGMENT -> no. avr simulator cannot handle this
#if(SIZE==100)
  = {1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122}
#else
#if(SIZE==80)
  = {1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122}
#else
#if(SIZE==60)
  = {1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122}
#else
#if(SIZE==40)
  = {1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122,
		  1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122}
#else
#if(SIZE==20)
  = {1743516170,-1957350540,-957146004,-1463803137,964060706,-2064589322,2135299305,-160525180,-132422533,-989218076,-1337054415,-775545438,-97024386,-145310492,1321759373,391162527,2097763249,-1293631903,1269394543,-617937096,698093065,-1703278086,2103693930,-715852426,1808732205,1761663427,-1996449686,584495908,1480197414,1750166071,-2023724882,1483659619,291569981,-557330350,-2012182269,290066398,382697006,1062984940,-689526068,-2097238122}
#endif
#endif
#endif
#endif
#endif
;


#if(INPUTDATAINFLASH)
static const
#endif
int32_t compressedInFlash[SIZE]
#if(DECODE)
#if(SIZE==100)
={253,132,32,132,32,132,32,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132}
#else
#if(SIZE==80)
={253,132,32,132,32,132,32,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132}
#else
#if(SIZE==60)
={253,132,32,132,32,132,32,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132}
#else
#if(SIZE==40)
={253,132,32,132,32,132,32,4,4,32,32,132,4,32,32,32,32,4,32,132,
		32,160,160,132,160,132,4,4,4,32,32,132,4,32,32,32,32,4,32,132}
#else
#if(SIZE==20)
={253,132,32,132,32,132,32,4,4,32,32,132,4,32,32,32,32,4,32,132}
#endif
#endif
#endif
#endif
#endif
#endif
;

static int32_t compressed[SIZE],result[SIZE*2];
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  int cpu_frequency=72;				/* in MHz */
  uint8_t gu8_MSG[60] = {'\0'};
  volatile uint32_t time1, time2, time3, time4;
  int32_t j,f;

  /* reset, initialize required memory */
  reset();

  /* read in amplitude and frequency for test data */
  /*  scanf("%d",&j);
  scanf("%d",&f); */
  j = 10; f = 2000;  /* k�rs men, anv�nds inte */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#if(DECODE)
	/* first encode */
	  time1 = TIM2->CNT;
	for(int i = 0 ; i < IN_END ; i += 2)
		compressed[i/2] = encode(test_data[i],test_data[i+1]);
		/*if(compressedInFlash[i/2]!=compressed[i/2]){
			sprintf(gu8_MSG, "bad!",compressed[i/2]);
			HAL_UART_Transmit(&huart1, gu8_MSG, sizeof(gu8_MSG), 0xFFFF);
		}*/
	time2 = TIM2->CNT;
	HAL_Delay(1000);

	/* decode */
	time3 = TIM2->CNT;
	for(int i = 0 ; i < IN_END ; i += 2) {

		//decode(compressedInFlash[i/2]);
		decode(compressed[i/2]);
		result[i] = xout1;
		result[i+1] = xout2;
	}
#else
	/* record the time first time */
	time3 = TIM2->CNT;
	/* encode */
	for(int i = 0 ; i < IN_END ; i += 2)
	compressed[i/2] = encode(test_data[i],test_data[i+1]);
#endif
	/* record the time second time */
	time4 = TIM2->CNT;

	/*execution_time =(int)((double)(time2-time1)/cpu_frequency*1000);
	sprintf(gu8_MSG, "\n\rexecution time: %ld\n\r",execution_time);
	HAL_UART_Transmit(&huart1, gu8_MSG, sizeof(gu8_MSG), 0xFFFF);*/
	execution_time =(int)((double)(time4-time3)/cpu_frequency*1000);
	sprintf(gu8_MSG, "\n\r decode execution time: %ld\n\r",execution_time);
	HAL_UART_Transmit(&huart1, gu8_MSG, sizeof(gu8_MSG), 0xFFFF);
	HAL_Delay(1000);
	Reset_Handler();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_EVEN;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
