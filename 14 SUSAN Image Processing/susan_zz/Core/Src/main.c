/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#define PROGRAM_ON_CCM1
//#define PROGRAM_ON_CCM2

#include <stdio.h>
#include "wcclibm.h"
#include "wccfile.h"
#include "wccmalloc.h"
#define EXP_A 184
#define EXP_C 16249



float susan_expf( float y )
{
  union {
    float d;
    struct {
      #ifdef LITTLE_ENDIAN
      short j, i;
      #else
      short i, j;
      #endif
    } n;
  } eco;
  eco.n.i = EXP_A * ( y ) + ( EXP_C );
  eco.n.j = 0;
  return eco.d;
}


float susan_sqrtf( float val )
{
  float x = val / 10;

  float dx;

  float diff;
  float min_tol = 0.00001f;

  int i, flag;


  flag = 0;
  if ( val == 0 ) x = 0;
  else {
    for ( i = 1; i < 20; i++ ) {
      if ( !flag ) {
        dx = ( val - ( x * x ) ) / ( 2.0f * x );
        x = x + dx;
        diff = val - ( x * x );
        if ( fabs( diff ) <= min_tol ) flag = 1;
      }
    }
  }
  return ( x );
}

/* ********** Optional settings */

typedef int        TOTAL_TYPE;

#define SEVEN_SUPP           /* size for non-max corner suppression; SEVEN_SUPP or FIVE_SUPP */
#define MAX_CORNERS   1000  /* max corners per frame */

#define  FTOI(a) ( (a) < 0 ? ((int)(a-0.5)) : ((int)(a+0.5)) )
#define  abs(a)  ( (a) < 0 ? -a : a )
typedef  unsigned char uchar;
typedef  struct {
  int x, y, info, dx, dy, I;
} corner_rep;
typedef  corner_rep CORNER_LIST[ MAX_CORNERS ];

extern char susan_input[ 7292 ];
struct wccFILE susan_file;
float susan_dt;
int susan_bt;
int susan_principle_conf;
int susan_thin_post_proc;
int susan_three_by_three;
int susan_drawing_mode;
int susan_susan_quick;
int susan_max_no_corners;
int susan_max_no_edges;


#ifdef PROGRAM_ON_CCM1
__attribute__((section(".ccmram")))
#endif
int susan_getint( struct wccFILE *fd )
{
  int c, i;
  char dummy[ 10000 ];

  c = susan_wccfgetc( fd );
  while ( 1 ) { /* find next integer */
    if ( c == '#' ) /* if we're at a comment, read to end of line */
      susan_wccfgets( dummy, 9000, fd );
    if ( c == EOF ) {
      /* "Image is not binary PGM." */
    }
    if ( c >= '0' && c <= '9' )
      break;   /* found what we were looking for */
    c = susan_wccfgetc( fd );
  }

  /* we're at the start of a number, continue until we hit a non-number */
  i = 0;
  while ( 1 ) {
    i = ( i * 10 ) + ( c - '0' );
    c = susan_wccfgetc( fd );
    if ( c == EOF ) return ( i );
    if ( c < '0' || c > '9' ) break;
  }

  return ( i );
}


#ifdef PROGRAM_ON_CCM1
__attribute__((section(".ccmram")))
#endif
void susan_get_image( struct wccFILE *fd,
                      unsigned char **in, int *x_size, int *y_size )
{
  char header [ 100 ];

  susan_wccfseek( fd, 0, WCCSEEK_SET );

  /* {{{ read header */

  header[ 0 ] = susan_wccfgetc( fd );
  header[ 1 ] = susan_wccfgetc( fd );
  if ( !( header[ 0 ] == 'P' && header[ 1 ] == '5' ) ) {
    /* "Image does not have binary PGM header." */
  }

  *x_size = susan_getint( fd );
  *y_size = susan_getint( fd );
  // dummy read
  susan_getint( fd );

  /* }}} */

  *in = ( uchar * ) susan_wccmalloc( *x_size * *y_size );

  if ( susan_wccfread( *in, 1, *x_size * *y_size, fd ) == 0 ) {
    /* "Image is wrong size.\n" */
  }
}


void susan_put_image( int x_size, int y_size )
{
  int i;
  for ( i = 0; i < x_size * y_size; i++ )
    ;
}


void susan_int_to_uchar( char *r, uchar *in, int size )
{
  int i, max_r = r[ 0 ], min_r = r[ 0 ];

  for ( i = 0; i < size; i++ ) {
    if ( r[ i ] > max_r )
      max_r = r[ i ];
    if ( r[ i ] < min_r )
      min_r = r[ i ];
  }

  if ( max_r == min_r ) {
    /* Should not occur in TACLeBench. */
    for ( i = 0; i < size; i++ )
      in[ i ] = ( uchar )( 0 );

    return;
  }
  max_r -= min_r;

  for ( i = 0; i < size; i++ )
    in[ i ] = ( uchar )( ( int )( ( int )( r[ i ] - min_r ) * 255 ) / max_r );
}


void susan_setup_brightness_lut( uchar **bp, int thresh, int form )
{
  int   k;
  float temp;

  *bp = ( unsigned char * )susan_wccmalloc( 516 );
  *bp = *bp + 258;

  for ( k = -256; k < 257; k++ ) {
    temp = ( ( float )k ) / ( ( float )thresh );
    temp = temp * temp;
    if ( form == 6 )
      temp = temp * temp * temp;
    temp = 100.0 * susan_expf( -temp );
    *( *bp + k ) = ( uchar )temp;
  }
}


void susan_principle( uchar *in, char *r, uchar *bp,
                      int max_no, int x_size, int y_size )
{
  int   i, j, n;
  uchar *p, *cp;

  susan_wccmemset( r, 0, x_size * y_size * sizeof( int ) );

  _Pragma( "loopbound min 0 max 0" )
  for ( i = 3; i < y_size - 3; i++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( j = 3; j < x_size - 3; j++ ) {
      n = 100;
      p = in + ( i - 3 ) * x_size + j - 1;
      cp = bp + in[ i * x_size + j ];

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 3;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 5;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 6;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += 2;
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 6;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 5;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 3;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );

      if ( n <= max_no )
        r[ i * x_size + j ] = max_no - n;
    }
  }
}


void susan_principle_small( uchar *in, char *r, uchar *bp,
                            int max_no, int x_size, int y_size )
{
  int   i, j, n;
  uchar *p, *cp;

  susan_wccmemset( r, 0, x_size * y_size * sizeof( int ) );

  _Pragma( "loopbound min 0 max 0" )
  for ( i = 1; i < y_size - 1; i++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( j = 1; j < x_size - 1; j++ ) {
      n = 100;
      p = in + ( i - 1 ) * x_size + j - 1;
      cp = bp + in[ i * x_size + j ];

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 2;

      n += *( cp - *p );
      p += 2;
      n += *( cp - *p );
      p += x_size - 2;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );

      if ( n <= max_no )
        r[ i * x_size + j ] = max_no - n;
    }
  }
}


uchar susan_median( uchar *in, int i, int j, int x_size )
{
  int p[ 8 ], k, l, tmp;

  p[ 0 ] = in[ ( i - 1 ) * x_size + j - 1 ];
  p[ 1 ] = in[ ( i - 1 ) * x_size + j   ];
  p[ 2 ] = in[ ( i - 1 ) * x_size + j + 1 ];
  p[ 3 ] = in[ ( i  ) * x_size + j - 1 ];
  p[ 4 ] = in[ ( i  ) * x_size + j + 1 ];
  p[ 5 ] = in[ ( i + 1 ) * x_size + j - 1 ];
  p[ 6 ] = in[ ( i + 1 ) * x_size + j   ];
  p[ 7 ] = in[ ( i + 1 ) * x_size + j + 1 ];

  _Pragma( "loopbound min 0 max 0" )
  for ( k = 0; k < 7; k++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( l = 0; l < ( 7 - k ); l++ ) {
      if ( p[ l ] > p[ l + 1 ] ) {
        tmp = p[ l ];
        p[ l ] = p[ l + 1 ];
        p[ l + 1 ] = tmp;
      }
    }
  }

  return ( ( p[ 3 ] + p[ 4 ] ) / 2 );
}


/* this enlarges "in" so that borders can be dealt with easily */

void susan_enlarge( uchar **in, uchar *tmp_image,
                    int *x_size, int *y_size, int border )
{
  int   i, j;

  _Pragma( "loopbound min 95 max 95" )
  for ( i = 0; i < *y_size; i++ ) { /* copy *in into tmp_image */
    susan_wccmemcpy( tmp_image + ( i + border ) * ( *x_size + 2 * border ) + border,
                     *in + i * *x_size, *x_size );
  }

  _Pragma( "loopbound min 7 max 7" )
  for ( i = 0; i < border;
        i++ ) { /* copy top and bottom rows; invert as many as necessary */
    susan_wccmemcpy( tmp_image + ( border - 1 - i ) * ( *x_size + 2 * border ) +
                     border,
                     *in + i * *x_size, *x_size );
    susan_wccmemcpy( tmp_image + ( *y_size + border + i ) *
                     ( *x_size + 2 * border ) + border,
                     *in + ( *y_size - i - 1 ) * *x_size, *x_size );
  }

  _Pragma( "loopbound min 7 max 7" )
  for ( i = 0; i < border; i++ ) { /* copy left and right columns */
    _Pragma( "loopbound min 109 max 109" )
    for ( j = 0; j < *y_size + 2 * border; j++ ) {
      tmp_image[ j * ( *x_size + 2 * border ) + border - 1 - i ] = tmp_image[ j *
          ( *x_size + 2 * border ) + border + i ];
      tmp_image[ j * ( *x_size + 2 * border ) + *x_size + border + i ] = tmp_image[ j *
          ( *x_size + 2 * border ) + *x_size + border - 1 - i ];
    }
  }

  *x_size += 2 * border; /* alter image size */
  *y_size += 2 * border;
  *in = tmp_image;    /* repoint in */
}


void susan_smoothing( int three_by_three, uchar *in, float dt,
                      int x_size, int y_size, uchar *bp )
{
  float temp;
  int   n_max, increment, mask_size,
        i, j, x, y, area, brightness, tmp, centre;
  uchar *ip, *dp, *dpt, *cp, *out = in,
                              *tmp_image;
  TOTAL_TYPE total;

  /* {{{ setup larger image and border sizes */

  if ( three_by_three == 0 )
    mask_size = ( ( int )( 1.5 * dt ) ) + 1;
  else
    mask_size = 1;

  if ( dt > 15 ) {
    /*  "Distance_thresh too big for integer arithmetic." */
    // Either reduce it to <=15 or recompile with variable "total"
    // as a float: see top "defines" section.
  }

  if ( ( 2 * mask_size + 1 > x_size ) || ( 2 * mask_size + 1 > y_size ) ) {
    /*  "Mask size too big for image." */
  }

  tmp_image = ( uchar * )susan_wccmalloc( ( x_size + mask_size * 2 ) *
                                          ( y_size + mask_size * 2 ) );
  susan_enlarge( &in, tmp_image, &x_size, &y_size, mask_size );
  if ( three_by_three == 0 ) {
    /* large Gaussian masks */
    /* {{{ setup distance lut */

    n_max = ( mask_size * 2 ) + 1;

    increment = x_size - n_max;

    dp     = ( unsigned char * )susan_wccmalloc( n_max * n_max );
    dpt    = dp;
    temp   = -( dt * dt );

    _Pragma( "loopbound min 15 max 15" )
    for ( i = -mask_size; i <= mask_size; i++ ) {
      _Pragma( "loopbound min 15 max 15" )
      for ( j = -mask_size; j <= mask_size; j++ ) {
        x = ( int ) ( 100.0 * susan_expf( ( ( float )( ( i * i ) +
                                            ( j * j ) ) ) / temp ) );
        *dpt++ = ( unsigned char )x;
      }
    }

    /* {{{ main section */
    _Pragma( "loopbound min 95 max 95" )
    for ( i = mask_size; i < y_size - mask_size; i++ ) {
      _Pragma( "loopbound min 76 max 76" )
      for ( j = mask_size; j < x_size - mask_size; j++ ) {
        area = 0;
        total = 0;
        dpt = dp;
        ip = in + ( ( i - mask_size ) * x_size ) + j - mask_size;
        centre = in[ i * x_size + j ];
        cp = bp + centre;
        _Pragma( "loopbound min 15 max 15" )
        for ( y = -mask_size; y <= mask_size; y++ ) {
          _Pragma( "loopbound min 15 max 15" )
          for ( x = -mask_size; x <= mask_size; x++ ) {
            brightness = *ip++;
            tmp = *dpt++ * *( cp - brightness );
            area += tmp;
            total += tmp * brightness;
          }
          ip += increment;
        }
        tmp = area - 10000;
        if ( tmp == 0 )
          *out++ = susan_median( in, i, j, x_size );
        else
          *out++ = ( ( total - ( centre * 10000 ) ) / tmp );
      }
    }
  } else {
    /* 3x3 constant mask */

    /* {{{ main section */
    _Pragma( "loopbound min 15 max 15" )            //neue Änderung min max 107
    for ( i = 1; i < y_size - 1; i++ ) {
      _Pragma( "loopbound min 15 max 15" )          //neue Änderung min max 88
      for ( j = 1; j < x_size - 1; j++ ) {
        area = 0;
        total = 0;
        ip = in + ( ( i - 1 ) * x_size ) + j - 1;
        centre = in[ i * x_size + j ];
        cp = bp + centre;

        brightness = *ip++;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        brightness = *ip++;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        brightness = *ip;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        ip += x_size - 2;
        brightness = *ip++;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        brightness = *ip++;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        brightness = *ip;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        ip += x_size - 2;
        brightness = *ip++;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        brightness = *ip++;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;
        brightness = *ip;
        tmp = *( cp - brightness );
        area += tmp;
        total += tmp * brightness;

        tmp = area - 100;
        if ( tmp == 0 )
          *out++ = susan_median( in, i, j, x_size );
        else
          *out++ = ( total - ( centre * 100 ) ) / tmp;
      }
    }
  }
}


void susan_edge_draw( uchar *in, uchar *mid,
                      int x_size, int y_size, int drawing_mode )
{
  int   i;
  uchar *inp, *midp;

  if ( drawing_mode == 0 ) {
    /* mark 3x3 white block around each edge point */
    midp = mid;
    _Pragma( "loopbound min 7220 max 7220" )
    for ( i = 0; i < x_size * y_size; i++ ) {
      if ( *midp < 8 ) {
        inp = in + ( midp - mid ) - x_size - 1;
        *inp++ = 255;
        *inp++ = 255;
        *inp = 255;
        inp += x_size - 2;
        *inp++ = 255;
        inp++;
        *inp = 255;
        inp += x_size - 2;
        *inp++ = 255;
        *inp++ = 255;
        *inp = 255;
      }
      midp++;
    }
  }

  /* now mark 1 black pixel at each edge point */
  midp = mid;
  _Pragma( "loopbound min 7220 max 7220" )
  for ( i = 0; i < x_size * y_size; i++ ) {
    if ( *midp < 8 )
      *( in + ( midp - mid ) ) = 0;
    midp++;
  }
}


void susan_thin( char *r, uchar *mid, int x_size, int y_size )
{
  int   l[ 9 ], centre,
        b01, b12, b21, b10,
        p1, p2, p3, p4,
        b00, b02, b20, b22,
        m, n, a, b, x, y, i, j;
  uchar *mp;

  _Pragma( "loopbound min 87 max 87" )
  for ( i = 4; i < y_size - 4; i++ ) {
    _Pragma( "loopbound min 68 max 68" )
    for ( j = 4; j < x_size - 4; j++ ) {
      if ( mid[ i * x_size + j ] < 8 ) {
        centre = r[ i * x_size + j ];
        /* {{{ count number of neighbours */

        mp = mid + ( i - 1 ) * x_size + j - 1;

        n = ( *mp < 8 ) +
            ( *( mp + 1 ) < 8 ) +
            ( *( mp + 2 ) < 8 ) +
            ( *( mp + x_size ) < 8 ) +
            ( *( mp + x_size + 2 ) < 8 ) +
            ( *( mp + x_size + x_size ) < 8 ) +
            ( *( mp + x_size + x_size + 1 ) < 8 ) +
            ( *( mp + x_size + x_size + 2 ) < 8 );

        /* {{{ n==0 no neighbours - remove point */

        if ( n == 0 )
          mid[ i * x_size + j ] = 100;

        /* {{{ n==1 - extend line if I can */

        /* extension is only allowed a few times - the value of mid is used to control this */

        if ( ( n == 1 ) && ( mid[ i * x_size + j ] < 6 ) ) {
          /* find maximum neighbour weighted in direction opposite the
             neighbour already present. e.g.
             have: O O O  weight r by 0 2 3
                   X X O              0 0 4
                   O O O              0 2 3     */

          l[ 0 ] = r[ ( i - 1 ) * x_size + j - 1 ];
          l[ 1 ] = r[ ( i - 1 ) * x_size + j ];
          l[ 2 ] = r[ ( i - 1 ) * x_size + j + 1 ];
          l[ 3 ] = r[ ( i  ) * x_size + j - 1 ];
          l[ 4 ] = 0;
          l[ 5 ] = r[ ( i  ) * x_size + j + 1 ];
          l[ 6 ] = r[ ( i + 1 ) * x_size + j - 1 ];
          l[ 7 ] = r[ ( i + 1 ) * x_size + j ];
          l[ 8 ] = r[ ( i + 1 ) * x_size + j + 1 ];

          if ( mid[ ( i - 1 )*x_size + j - 1 ] < 8 )        {
            l[ 0 ] = 0;
            l[ 1 ] = 0;
            l[ 3 ] = 0;
            l[ 2 ] *= 2;
            l[ 6 ] *= 2;
            l[ 5 ] *= 3;
            l[ 7 ] *= 3;
            l[ 8 ] *= 4;
          } else {
            if ( mid[ ( i - 1 )*x_size + j ] < 8 )   {
              l[ 1 ] = 0;
              l[ 0 ] = 0;
              l[ 2 ] = 0;
              l[ 3 ] *= 2;
              l[ 5 ] *= 2;
              l[ 6 ] *= 3;
              l[ 8 ] *= 3;
              l[ 7 ] *= 4;
            } else {
              if ( mid[ ( i - 1 )*x_size + j + 1 ] < 8 ) {
                l[ 2 ] = 0;
                l[ 1 ] = 0;
                l[ 5 ] = 0;
                l[ 0 ] *= 2;
                l[ 8 ] *= 2;
                l[ 3 ] *= 3;
                l[ 7 ] *= 3;
                l[ 6 ] *= 4;
              } else {
                if ( mid[ ( i )*x_size + j - 1 ] < 8 )   {
                  l[ 3 ] = 0;
                  l[ 0 ] = 0;
                  l[ 6 ] = 0;
                  l[ 1 ] *= 2;
                  l[ 7 ] *= 2;
                  l[ 2 ] *= 3;
                  l[ 8 ] *= 3;
                  l[ 5 ] *= 4;
                } else {
                  if ( mid[ ( i )*x_size + j + 1 ] < 8 )   {
                    l[ 5 ] = 0;
                    l[ 2 ] = 0;
                    l[ 8 ] = 0;
                    l[ 1 ] *= 2;
                    l[ 7 ] *= 2;
                    l[ 0 ] *= 3;
                    l[ 6 ] *= 3;
                    l[ 3 ] *= 4;
                  } else {
                    if ( mid[ ( i + 1 )*x_size + j - 1 ] < 8 ) {
                      l[ 6 ] = 0;
                      l[ 3 ] = 0;
                      l[ 7 ] = 0;
                      l[ 0 ] *= 2;
                      l[ 8 ] *= 2;
                      l[ 1 ] *= 3;
                      l[ 5 ] *= 3;
                      l[ 2 ] *= 4;
                    } else {
                      if ( mid[ ( i + 1 )*x_size + j ] < 8 )   {
                        l[ 7 ] = 0;
                        l[ 6 ] = 0;
                        l[ 8 ] = 0;
                        l[ 3 ] *= 2;
                        l[ 5 ] *= 2;
                        l[ 0 ] *= 3;
                        l[ 2 ] *= 3;
                        l[ 1 ] *= 4;
                      } else {
                        if ( mid[ ( i + 1 )*x_size + j + 1 ] < 8 ) {
                          l[ 8 ] = 0;
                          l[ 5 ] = 0;
                          l[ 7 ] = 0;
                          l[ 6 ] *= 2;
                          l[ 2 ] *= 2;
                          l[ 1 ] *= 3;
                          l[ 3 ] *= 3;
                          l[ 0 ] *= 4;
                        }
                      }
                    }
                  }
                }
              }
            }
          }

          m = 0;   /* find the highest point */
          _Pragma( "loopbound min 3 max 3" )
          for ( y = 0; y < 3; y++ )
            _Pragma( "loopbound min 3 max 3" )
            for ( x = 0; x < 3; x++ )
              if ( l[ y + y + y + x ] > m ) {
                m = l[ y + y + y + x ];
                a = y;
                b = x;
              }

          if ( m > 0 ) {
            if ( mid[ i * x_size + j ] < 4 )
              mid[ ( i + a - 1 )*x_size + j + b - 1 ] = 4;
            else
              mid[ ( i + a - 1 )*x_size + j + b - 1 ] = mid[ i * x_size + j ] + 1;
            if ( ( a + a + b ) < 3 ) { /* need to jump back in image */
              i += a - 1;
              j += b - 2;
              if ( i < 4 ) i = 4;
              if ( j < 4 ) j = 4;
            }
          }
        }

        /* {{{ n==2 */

        if ( n == 2 ) {
          /* put in a bit here to straighten edges */
          b00 = mid[ ( i - 1 ) * x_size + j - 1 ] < 8; /* corners of 3x3 */
          b02 = mid[ ( i - 1 ) * x_size + j + 1 ] < 8;
          b20 = mid[ ( i + 1 ) * x_size + j - 1 ] < 8;
          b22 = mid[ ( i + 1 ) * x_size + j + 1 ] < 8;
          if ( ( ( b00 + b02 + b20 + b22 ) == 2 ) && ( ( b00 | b22 ) & ( b02 | b20 ) ) ) {
            /* case: move a point back into line.
                e.g. X O X  CAN  become X X X
                     O X O              O O O
                     O O O              O O O    */
            if ( b00 ) {
              if ( b02 ) {
                x = 0;
                y = -1;
              } else     {
                x = -1;
                y = 0;
              }
            } else {
              if ( b02 ) {
                x = 1;
                y = 0;
              } else     {
                x = 0;
                y = 1;
              }
            }
            if ( ( ( float )r[ ( i + y )*x_size + j + x ] / ( float )centre ) > 0.7 ) {
              if ( ( ( x == 0 ) && ( mid[ ( i + ( 2 * y ) )*x_size + j ] > 7 ) &&
                     ( mid[ ( i + ( 2 * y ) )*x_size + j - 1 ] > 7 ) &&
                     ( mid[ ( i + ( 2 * y ) )*x_size + j + 1 ] > 7 ) ) ||
                   ( ( y == 0 ) && ( mid[ ( i )*x_size + j + ( 2 * x ) ] > 7 ) &&
                     ( mid[ ( i + 1 )*x_size + j + ( 2 * x ) ] > 7 ) &&
                     ( mid[ ( i - 1 )*x_size + j + ( 2 * x ) ] > 7 ) ) ) {
                mid[ ( i )*x_size + j ] = 100;
                mid[ ( i + y )*x_size + j + x ] = 3; /* no jumping needed */
              }
            }
          } else {
            b01 = mid[ ( i - 1 ) * x_size + j   ] < 8;
            b12 = mid[ ( i  ) * x_size + j + 1 ] < 8;
            b21 = mid[ ( i + 1 ) * x_size + j   ] < 8;
            b10 = mid[ ( i  ) * x_size + j - 1 ] < 8;
            /* {{{ right angle ends - not currently used */

            #ifdef IGNORETHIS
            if ( ( b00 & b01 ) | ( b00 & b10 ) | ( b02 & b01 ) | ( b02 & b12 ) |
                 ( b20 & b10 ) | ( b20 & b21 ) | ( b22 & b21 ) | ( b22 & b12 ) ) {
              /* case; right angle ends. clean up.
                 e.g.; X X O  CAN  become X X O
                       O X O              O O O
                       O O O              O O O        */
              if ( ( ( b01 ) & ( mid[ ( i - 2 )*x_size + j - 1 ] > 7 ) &
                     ( mid[ ( i - 2 )*x_size + j ] > 7 ) & ( mid[ ( i - 2 )*x_size + j + 1 ] > 7 )&
                     ( ( b00 & ( ( 2 * r[ ( i - 1 )*x_size + j + 1 ] ) > centre ) ) | ( b02 & ( (
                           2 * r[ ( i - 1 )*x_size + j - 1 ] ) > centre ) ) ) ) |
                   ( ( b10 ) & ( mid[ ( i - 1 )*x_size + j - 2 ] > 7 ) & ( mid[ ( i )*x_size + j - 2 ]
                       > 7 ) & ( mid[ ( i + 1 )*x_size + j - 2 ] > 7 )&
                     ( ( b00 & ( ( 2 * r[ ( i + 1 )*x_size + j - 1 ] ) > centre ) ) | ( b20 & ( (
                           2 * r[ ( i - 1 )*x_size + j - 1 ] ) > centre ) ) ) ) |
                   ( ( b12 ) & ( mid[ ( i - 1 )*x_size + j + 2 ] > 7 ) & ( mid[ ( i )*x_size + j + 2 ]
                       > 7 ) & ( mid[ ( i + 1 )*x_size + j + 2 ] > 7 )&
                     ( ( b02 & ( ( 2 * r[ ( i + 1 )*x_size + j + 1 ] ) > centre ) ) | ( b22 & ( (
                           2 * r[ ( i - 1 )*x_size + j + 1 ] ) > centre ) ) ) ) |
                   ( ( b21 ) & ( mid[ ( i + 2 )*x_size + j - 1 ] > 7 ) & ( mid[ ( i + 2 )*x_size + j ]
                       > 7 ) & ( mid[ ( i + 2 )*x_size + j + 1 ] > 7 )&
                     ( ( b20 & ( ( 2 * r[ ( i + 1 )*x_size + j + 1 ] ) > centre ) ) | ( b22 & ( (
                           2 * r[ ( i + 1 )*x_size + j - 1 ] ) > centre ) ) ) ) ) {
                mid[ ( i )*x_size + j ] = 100;
                if ( b10 & b20 ) j -= 2;
                if ( b00 | b01 | b02 ) {
                  i--;
                  j -= 2;
                }
              }
            }
            #endif

            if ( ( ( b01 + b12 + b21 + b10 ) == 2 ) && ( ( b10 | b12 ) & ( b01 | b21 ) ) &&
                 ( ( b01 & ( ( mid[ ( i - 2 )*x_size + j - 1 ] < 8 ) | ( mid[ ( i - 2 )*x_size + j +
                             1 ] < 8 ) ) ) | ( b10 & ( ( mid[ ( i - 1 )*x_size + j - 2 ] < 8 ) |
                                              ( mid[ ( i + 1 )*x_size + j - 2 ] < 8 ) ) ) |
                   ( b12 & ( ( mid[ ( i - 1 )*x_size + j + 2 ] < 8 ) | ( mid[ ( i + 1 )*x_size + j +
                             2 ] < 8 ) ) ) | ( b21 & ( ( mid[ ( i + 2 )*x_size + j - 1 ] < 8 ) |
                                              ( mid[ ( i + 2 )*x_size + j + 1 ] < 8 ) ) ) ) ) {
              /* case; clears odd right angles.
                 e.g.; O O O  becomes O O O
                       X X O          X O O
                       O X O          O X O     */
              mid[ ( i )*x_size + j ] = 100;
              i--;               /* jump back */
              j -= 2;
              if ( i < 4 ) i = 4;
              if ( j < 4 ) j = 4;
            }
          }
        }

        /* {{{ n>2 the thinning is done here without breaking connectivity */

        if ( n > 2 ) {
          b01 = mid[ ( i - 1 ) * x_size + j   ] < 8;
          b12 = mid[ ( i  ) * x_size + j + 1 ] < 8;
          b21 = mid[ ( i + 1 ) * x_size + j   ] < 8;
          b10 = mid[ ( i  ) * x_size + j - 1 ] < 8;
          if ( ( b01 + b12 + b21 + b10 ) > 1 ) {
            b00 = mid[ ( i - 1 ) * x_size + j - 1 ] < 8;
            b02 = mid[ ( i - 1 ) * x_size + j + 1 ] < 8;
            b20 = mid[ ( i + 1 ) * x_size + j - 1 ] < 8;
            b22 = mid[ ( i + 1 ) * x_size + j + 1 ] < 8;
            p1 = b00 | b01;
            p2 = b02 | b12;
            p3 = b22 | b21;
            p4 = b20 | b10;

            if ( ( ( p1 + p2 + p3 + p4 ) - ( ( b01 & p2 ) + ( b12 & p3 ) + ( b21 & p4 ) +
                                             ( b10 & p1 ) ) ) < 2 ) {
              mid[ ( i )*x_size + j ] = 100;
              i--;
              j -= 2;
              if ( i < 4 ) i = 4;
              if ( j < 4 ) j = 4;
            }
          }
        }
      }
    }
  }
}



void susan_edges( uchar *in, char *r, uchar *mid, uchar *bp,
                  int max_no, int x_size, int y_size )
{
  float z;
  int   do_symmetry, i, j, m, n, a, b, x, y, w;
  uchar c, *p, *cp;

  susan_wccmemset( r, 0, x_size * y_size );

  _Pragma( "loopbound min 89 max 89" )
  for ( i = 3; i < y_size - 3; i++ ) {
    _Pragma( "loopbound min 70 max 70" )
    for ( j = 3; j < x_size - 3; j++ ) {
      n = 100;
      p = in + ( i - 3 ) * x_size + j - 1;
      cp = bp + in[ i * x_size + j ];

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 3;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 5;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 6;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += 2;
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 6;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 5;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 3;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );

      if ( n <= max_no )
        r[ i * x_size + j ] = max_no - n;
    }
  }

  _Pragma( "loopbound min 87 max 87" )
  for ( i = 4; i < y_size - 4; i++ ) {
    _Pragma( "loopbound min 68 max 68" )
    for ( j = 4; j < x_size - 4; j++ ) {
      if ( r[ i * x_size + j ] > 0 ) {
        m = r[ i * x_size + j ];
        n = max_no - m;
        cp = bp + in[ i * x_size + j ];

        if ( n > 600 ) {
          p = in + ( i - 3 ) * x_size + j - 1;
          x = 0;
          y = 0;

          c = *( cp - *p++ );
          x -= c;
          y -= 3 * c;
          c = *( cp - *p++ );
          y -= 3 * c;
          c = *( cp - *p );
          x += c;
          y -= 3 * c;
          p += x_size - 3;

          c = *( cp - *p++ );
          x -= 2 * c;
          y -= 2 * c;
          c = *( cp - *p++ );
          x -= c;
          y -= 2 * c;
          c = *( cp - *p++ );
          y -= 2 * c;
          c = *( cp - *p++ );
          x += c;
          y -= 2 * c;
          c = *( cp - *p );
          x += 2 * c;
          y -= 2 * c;
          p += x_size - 5;

          c = *( cp - *p++ );
          x -= 3 * c;
          y -= c;
          c = *( cp - *p++ );
          x -= 2 * c;
          y -= c;
          c = *( cp - *p++ );
          x -= c;
          y -= c;
          c = *( cp - *p++ );
          y -= c;
          c = *( cp - *p++ );
          x += c;
          y -= c;
          c = *( cp - *p++ );
          x += 2 * c;
          y -= c;
          c = *( cp - *p );
          x += 3 * c;
          y -= c;
          p += x_size - 6;

          c = *( cp - *p++ );
          x -= 3 * c;
          c = *( cp - *p++ );
          x -= 2 * c;
          c = *( cp - *p );
          x -= c;
          p += 2;
          c = *( cp - *p++ );
          x += c;
          c = *( cp - *p++ );
          x += 2 * c;
          c = *( cp - *p );
          x += 3 * c;
          p += x_size - 6;

          c = *( cp - *p++ );
          x -= 3 * c;
          y += c;
          c = *( cp - *p++ );
          x -= 2 * c;
          y += c;
          c = *( cp - *p++ );
          x -= c;
          y += c;
          c = *( cp - *p++ );
          y += c;
          c = *( cp - *p++ );
          x += c;
          y += c;
          c = *( cp - *p++ );
          x += 2 * c;
          y += c;
          c = *( cp - *p );
          x += 3 * c;
          y += c;
          p += x_size - 5;

          c = *( cp - *p++ );
          x -= 2 * c;
          y += 2 * c;
          c = *( cp - *p++ );
          x -= c;
          y += 2 * c;
          c = *( cp - *p++ );
          y += 2 * c;
          c = *( cp - *p++ );
          x += c;
          y += 2 * c;
          c = *( cp - *p );
          x += 2 * c;
          y += 2 * c;
          p += x_size - 3;

          c = *( cp - *p++ );
          x -= c;
          y += 3 * c;
          c = *( cp - *p++ );
          y += 3 * c;
          c = *( cp - *p );
          x += c;
          y += 3 * c;

          z = susan_sqrtf( ( float )( ( x * x ) + ( y * y ) ) );
          if ( z > ( 0.9 * ( float )n ) ) { /* 0.5 */
            do_symmetry = 0;
            if ( x == 0 )
              z = 1000000.0;
            else
              z = ( ( float )y ) / ( ( float )x );
            if ( z < 0 ) {
              z = -z;
              w = -1;
            } else w = 1;
            if ( z < 0.5 ) {
              /* vert_edge */ a = 0;
              b = 1;
            } else {
              if ( z > 2.0 ) {
                /* hor_edge */ a = 1;
                b = 0;
              } else {
                /* diag_edge */ if ( w > 0 ) {
                  a = 1;
                  b = 1;
                } else {
                  a = -1;
                  b = 1;
                }
              }
            }
            if ( ( m > r[ ( i + a )*x_size + j + b ] ) &&
                 ( m >= r[ ( i - a )*x_size + j - b ] ) &&
                 ( m > r[ ( i + ( 2 * a ) )*x_size + j + ( 2 * b ) ] ) &&
                 ( m >= r[ ( i - ( 2 * a ) )*x_size + j - ( 2 * b ) ] ) )
              mid[ i * x_size + j ] = 1;
          } else
            do_symmetry = 1;
        } else
          do_symmetry = 1;

        if ( do_symmetry == 1 ) {
          p = in + ( i - 3 ) * x_size + j - 1;
          x = 0;
          y = 0;
          w = 0;

          /*   |      \
               y  -x-  w
               |        \   */

          c = *( cp - *p++ );
          x += c;
          y += 9 * c;
          w += 3 * c;
          c = *( cp - *p++ );
          y += 9 * c;
          c = *( cp - *p );
          x += c;
          y += 9 * c;
          w -= 3 * c;
          p += x_size - 3;

          c = *( cp - *p++ );
          x += 4 * c;
          y += 4 * c;
          w += 4 * c;
          c = *( cp - *p++ );
          x += c;
          y += 4 * c;
          w += 2 * c;
          c = *( cp - *p++ );
          y += 4 * c;
          c = *( cp - *p++ );
          x += c;
          y += 4 * c;
          w -= 2 * c;
          c = *( cp - *p );
          x += 4 * c;
          y += 4 * c;
          w -= 4 * c;
          p += x_size - 5;

          c = *( cp - *p++ );
          x += 9 * c;
          y += c;
          w += 3 * c;
          c = *( cp - *p++ );
          x += 4 * c;
          y += c;
          w += 2 * c;
          c = *( cp - *p++ );
          x += c;
          y += c;
          w += c;
          c = *( cp - *p++ );
          y += c;
          c = *( cp - *p++ );
          x += c;
          y += c;
          w -= c;
          c = *( cp - *p++ );
          x += 4 * c;
          y += c;
          w -= 2 * c;
          c = *( cp - *p );
          x += 9 * c;
          y += c;
          w -= 3 * c;
          p += x_size - 6;

          c = *( cp - *p++ );
          x += 9 * c;
          c = *( cp - *p++ );
          x += 4 * c;
          c = *( cp - *p );
          x += c;
          p += 2;
          c = *( cp - *p++ );
          x += c;
          c = *( cp - *p++ );
          x += 4 * c;
          c = *( cp - *p );
          x += 9 * c;
          p += x_size - 6;

          c = *( cp - *p++ );
          x += 9 * c;
          y += c;
          w -= 3 * c;
          c = *( cp - *p++ );
          x += 4 * c;
          y += c;
          w -= 2 * c;
          c = *( cp - *p++ );
          x += c;
          y += c;
          w -= c;
          c = *( cp - *p++ );
          y += c;
          c = *( cp - *p++ );
          x += c;
          y += c;
          w += c;
          c = *( cp - *p++ );
          x += 4 * c;
          y += c;
          w += 2 * c;
          c = *( cp - *p );
          x += 9 * c;
          y += c;
          w += 3 * c;
          p += x_size - 5;

          c = *( cp - *p++ );
          x += 4 * c;
          y += 4 * c;
          w -= 4 * c;
          c = *( cp - *p++ );
          x += c;
          y += 4 * c;
          w -= 2 * c;
          c = *( cp - *p++ );
          y += 4 * c;
          c = *( cp - *p++ );
          x += c;
          y += 4 * c;
          w += 2 * c;
          c = *( cp - *p );
          x += 4 * c;
          y += 4 * c;
          w += 4 * c;
          p += x_size - 3;

          c = *( cp - *p++ );
          x += c;
          y += 9 * c;
          w -= 3 * c;
          c = *( cp - *p++ );
          y += 9 * c;
          c = *( cp - *p );
          x += c;
          y += 9 * c;
          w += 3 * c;

          if ( y == 0 )
            z = 1000000.0;
          else
            z = ( ( float )x ) / ( ( float )y );
          if ( z < 0.5 ) {
            /* vertical */ a = 0;
            b = 1;
          } else {
            if ( z > 2.0 ) {
              /* horizontal */ a = 1;
              b = 0;
            } else {
              /* diagonal */ if ( w > 0 ) {
                a = -1;
                b = 1;
              } else {
                a = 1;
                b = 1;
              }
            }
          }
          if ( ( m > r[ ( i + a )*x_size + j + b ] ) &&
               ( m >= r[ ( i - a )*x_size + j - b ] ) &&
               ( m > r[ ( i + ( 2 * a ) )*x_size + j + ( 2 * b ) ] ) &&
               ( m >= r[ ( i - ( 2 * a ) )*x_size + j - ( 2 * b ) ] ) )
            mid[ i * x_size + j ] = 2;
        }
      }
    }
  }
}


void susan_edges_small( uchar *in, char *r, uchar *mid, uchar *bp,
                        int max_no, int x_size, int y_size )
{
  float z;
  int   do_symmetry, i, j, m, n, a, b, x, y, w;
  uchar c, *p, *cp;

  susan_wccmemset( r, 0, x_size * y_size );

  _Pragma( "loopbound min 0 max 0" )
  for ( i = 1; i < y_size - 1; i++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( j = 1; j < x_size - 1; j++ ) {
      n = 100;
      p = in + ( i - 1 ) * x_size + j - 1;
      cp = bp + in[ i * x_size + j ];

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 2;

      n += *( cp - *p );
      p += 2;
      n += *( cp - *p );
      p += x_size - 2;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );

      if ( n <= max_no )
        r[ i * x_size + j ] = max_no - n;
    }
  }

  _Pragma( "loopbound min 0 max 0" )
  for ( i = 2; i < y_size - 2; i++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( j = 2; j < x_size - 2; j++ ) {
      if ( r[ i * x_size + j ] > 0 ) {
        m = r[ i * x_size + j ];
        n = max_no - m;
        cp = bp + in[ i * x_size + j ];

        if ( n > 250 ) {
          p = in + ( i - 1 ) * x_size + j - 1;
          x = 0;
          y = 0;

          c = *( cp - *p++ );
          x -= c;
          y -= c;
          c = *( cp - *p++ );
          y -= c;
          c = *( cp - *p );
          x += c;
          y -= c;
          p += x_size - 2;

          c = *( cp - *p );
          x -= c;
          p += 2;
          c = *( cp - *p );
          x += c;
          p += x_size - 2;

          c = *( cp - *p++ );
          x -= c;
          y += c;
          c = *( cp - *p++ );
          y += c;
          c = *( cp - *p );
          x += c;
          y += c;

          z = susan_sqrtf( ( float )( ( x * x ) + ( y * y ) ) );
          if ( z > ( 0.4 * ( float )n ) ) { /* 0.6 */
            do_symmetry = 0;
            if ( x == 0 )
              z = 1000000.0;
            else
              z = ( ( float )y ) / ( ( float )x );
            if ( z < 0 ) {
              z = -z;
              w = -1;
            } else w = 1;
            if ( z < 0.5 ) {
              /* vert_edge */ a = 0;
              b = 1;
            } else {
              if ( z > 2.0 ) {
                /* hor_edge */ a = 1;
                b = 0;
              } else {
                /* diag_edge */ if ( w > 0 ) {
                  a = 1;
                  b = 1;
                } else {
                  a = -1;
                  b = 1;
                }
              }
            }
            if ( ( m > r[ ( i + a )*x_size + j + b ] ) &&
                 ( m >= r[ ( i - a )*x_size + j - b ] ) )
              mid[ i * x_size + j ] = 1;
          } else
            do_symmetry = 1;
        } else
          do_symmetry = 1;

        if ( do_symmetry == 1 ) {
          p = in + ( i - 1 ) * x_size + j - 1;
          x = 0;
          y = 0;
          w = 0;

          /*   |      \
               y  -x-  w
               |        \   */

          c = *( cp - *p++ );
          x += c;
          y += c;
          w += c;
          c = *( cp - *p++ );
          y += c;
          c = *( cp - *p );
          x += c;
          y += c;
          w -= c;
          p += x_size - 2;

          c = *( cp - *p );
          x += c;
          p += 2;
          c = *( cp - *p );
          x += c;
          p += x_size - 2;

          c = *( cp - *p++ );
          x += c;
          y += c;
          w -= c;
          c = *( cp - *p++ );
          y += c;
          c = *( cp - *p );
          x += c;
          y += c;
          w += c;

          if ( y == 0 )
            z = 1000000.0;
          else
            z = ( ( float )x ) / ( ( float )y );
          if ( z < 0.5 ) {
            /* vertical */ a = 0;
            b = 1;
          } else {
            if ( z > 2.0 ) {
              /* horizontal */ a = 1;
              b = 0;
            } else {
              /* diagonal */ if ( w > 0 ) {
                a = -1;
                b = 1;
              } else {
                a = 1;
                b = 1;
              }
            }
          }
          if ( ( m > r[ ( i + a )*x_size + j + b ] ) &&
               ( m >= r[ ( i - a )*x_size + j - b ] ) )
            mid[ i * x_size + j ] = 2;
        }
      }
    }
  }
}

#ifdef PROGRAM_ON_CCM2
__attribute__((section(".ccmram")))
#endif
void susan_corner_draw( uchar *in, CORNER_LIST corner_list,
                        int x_size, int drawing_mode )
{
  uchar *p;
  int   n = 0;

  _Pragma( "loopbound min 0 max 0" )
  while ( corner_list[ n ].info != 7 ) {
    if ( drawing_mode == 0 ) {
      p = in + ( corner_list[ n ].y - 1 ) * x_size + corner_list[ n ].x - 1;
      *p++ = 255;
      *p++ = 255;
      *p = 255;
      p += x_size - 2;
      *p++ = 255;
      *p++ = 0;
      *p = 255;
      p += x_size - 2;
      *p++ = 255;
      *p++ = 255;
      *p = 255;
      n++;
    } else {
      p = in + corner_list[ n ].y * x_size + corner_list[ n ].x;
      *p = 0;
      n++;
    }
  }
}

#ifdef PROGRAM_ON_CCM2
__attribute__((section(".ccmram")))
#endif
void susan_corners( uchar *in, char *r, uchar *bp,
                    int max_no, CORNER_LIST corner_list, int x_size, int y_size )
{
  int   n, x, y, sq, xx, yy,
        i, j;
  float divide;
  uchar c, *p, *cp;
  char  *cgx, *cgy;

  susan_wccmemset( r, 0, x_size * y_size );

  cgx = ( char * )susan_wccmalloc( x_size * y_size );
  cgy = ( char * )susan_wccmalloc( x_size * y_size );

  _Pragma( "loopbound min 85 max 85" )
  for ( i = 5; i < y_size - 5; i++ ) {
    _Pragma( "loopbound min 66 max 66" )
    for ( j = 5; j < x_size - 5; j++ ) {
      n = 100;
      p = in + ( i - 3 ) * x_size + j - 1;
      cp = bp + in[ i * x_size + j ];

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 3;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 5;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 6;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      if ( n < max_no ) { /* do this test early and often ONLY to save wasted computation */
        p += 2;
        n += *( cp - *p++ );
        if ( n < max_no ) {
          n += *( cp - *p++ );
          if ( n < max_no ) {
            n += *( cp - *p );
            if ( n < max_no ) {
              p += x_size - 6;

              n += *( cp - *p++ );
              if ( n < max_no ) {
                n += *( cp - *p++ );
                if ( n < max_no ) {
                  n += *( cp - *p++ );
                  if ( n < max_no ) {
                    n += *( cp - *p++ );
                    if ( n < max_no ) {
                      n += *( cp - *p++ );
                      if ( n < max_no ) {
                        n += *( cp - *p++ );
                        if ( n < max_no ) {
                          n += *( cp - *p );
                          if ( n < max_no ) {
                            p += x_size - 5;

                            n += *( cp - *p++ );
                            if ( n < max_no ) {
                              n += *( cp - *p++ );
                              if ( n < max_no ) {
                                n += *( cp - *p++ );
                                if ( n < max_no ) {
                                  n += *( cp - *p++ );
                                  if ( n < max_no ) {
                                    n += *( cp - *p );
                                    if ( n < max_no ) {
                                      p += x_size - 3;

                                      n += *( cp - *p++ );
                                      if ( n < max_no ) {
                                        n += *( cp - *p++ );
                                        if ( n < max_no ) {
                                          n += *( cp - *p );

                                          if ( n < max_no ) {
                                            x = 0;
                                            y = 0;
                                            p = in + ( i - 3 ) * x_size + j - 1;

                                            c = *( cp - *p++ );
                                            x -= c;
                                            y -= 3 * c;
                                            c = *( cp - *p++ );
                                            y -= 3 * c;
                                            c = *( cp - *p );
                                            x += c;
                                            y -= 3 * c;
                                            p += x_size - 3;

                                            c = *( cp - *p++ );
                                            x -= 2 * c;
                                            y -= 2 * c;
                                            c = *( cp - *p++ );
                                            x -= c;
                                            y -= 2 * c;
                                            c = *( cp - *p++ );
                                            y -= 2 * c;
                                            c = *( cp - *p++ );
                                            x += c;
                                            y -= 2 * c;
                                            c = *( cp - *p );
                                            x += 2 * c;
                                            y -= 2 * c;
                                            p += x_size - 5;

                                            c = *( cp - *p++ );
                                            x -= 3 * c;
                                            y -= c;
                                            c = *( cp - *p++ );
                                            x -= 2 * c;
                                            y -= c;
                                            c = *( cp - *p++ );
                                            x -= c;
                                            y -= c;
                                            c = *( cp - *p++ );
                                            y -= c;
                                            c = *( cp - *p++ );
                                            x += c;
                                            y -= c;
                                            c = *( cp - *p++ );
                                            x += 2 * c;
                                            y -= c;
                                            c = *( cp - *p );
                                            x += 3 * c;
                                            y -= c;
                                            p += x_size - 6;

                                            c = *( cp - *p++ );
                                            x -= 3 * c;
                                            c = *( cp - *p++ );
                                            x -= 2 * c;
                                            c = *( cp - *p );
                                            x -= c;
                                            p += 2;
                                            c = *( cp - *p++ );
                                            x += c;
                                            c = *( cp - *p++ );
                                            x += 2 * c;
                                            c = *( cp - *p );
                                            x += 3 * c;
                                            p += x_size - 6;

                                            c = *( cp - *p++ );
                                            x -= 3 * c;
                                            y += c;
                                            c = *( cp - *p++ );
                                            x -= 2 * c;
                                            y += c;
                                            c = *( cp - *p++ );
                                            x -= c;
                                            y += c;
                                            c = *( cp - *p++ );
                                            y += c;
                                            c = *( cp - *p++ );
                                            x += c;
                                            y += c;
                                            c = *( cp - *p++ );
                                            x += 2 * c;
                                            y += c;
                                            c = *( cp - *p );
                                            x += 3 * c;
                                            y += c;
                                            p += x_size - 5;

                                            c = *( cp - *p++ );
                                            x -= 2 * c;
                                            y += 2 * c;
                                            c = *( cp - *p++ );
                                            x -= c;
                                            y += 2 * c;
                                            c = *( cp - *p++ );
                                            y += 2 * c;
                                            c = *( cp - *p++ );
                                            x += c;
                                            y += 2 * c;
                                            c = *( cp - *p );
                                            x += 2 * c;
                                            y += 2 * c;
                                            p += x_size - 3;

                                            c = *( cp - *p++ );
                                            x -= c;
                                            y += 3 * c;
                                            c = *( cp - *p++ );
                                            y += 3 * c;
                                            c = *( cp - *p );
                                            x += c;
                                            y += 3 * c;

                                            xx = x * x;
                                            yy = y * y;
                                            sq = xx + yy;
                                            if ( sq > ( ( n * n ) / 2 ) ) {
                                              if ( yy < xx ) {
                                                divide = ( float )y / ( float )abs( x );
                                                sq = abs( x ) / x;
                                                sq = *( cp - in[ ( i + FTOI( divide ) ) * x_size + j + sq ] ) +
                                                     *( cp - in[ ( i + FTOI( 2 * divide ) ) * x_size + j + 2 * sq ] ) +
                                                     *( cp - in[ ( i + FTOI( 3 * divide ) ) * x_size + j + 3 * sq ] );
                                              } else {
                                                divide = ( float )x / ( float )abs( y );
                                                sq = abs( y ) / y;
                                                sq = *( cp - in[ ( i + sq ) * x_size + j + FTOI( divide ) ] ) +
                                                     *( cp - in[ ( i + 2 * sq ) * x_size + j + FTOI( 2 * divide ) ] ) +
                                                     *( cp - in[ ( i + 3 * sq ) * x_size + j + FTOI( 3 * divide ) ] );
                                              }

                                              if ( sq > 290 ) {
                                                r[ i * x_size + j ] = max_no - n;
                                                cgx[ i * x_size + j ] = ( 51 * x ) / n;
                                                cgy[ i * x_size + j ] = ( 51 * y ) / n;
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /* to locate the local maxima */
  n = 0;
  _Pragma( "loopbound min 85 max 85" )
  for ( i = 5; i < y_size - 5; i++ ) {
    _Pragma( "loopbound min 66 max 66" )
    for ( j = 5; j < x_size - 5; j++ ) {
      x = r[ i * x_size + j ];
      if ( x > 0 )  {
        /* 5x5 mask */
        #ifdef FIVE_SUPP
        if ( ( x > r[ ( i - 1 )*x_size + j + 2 ] ) &&
             ( x > r[ ( i  )*x_size + j + 1 ] ) &&
             ( x > r[ ( i  )*x_size + j + 2 ] ) &&
             ( x > r[ ( i + 1 )*x_size + j - 1 ] ) &&
             ( x > r[ ( i + 1 )*x_size + j   ] ) &&
             ( x > r[ ( i + 1 )*x_size + j + 1 ] ) &&
             ( x > r[ ( i + 1 )*x_size + j + 2 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j - 2 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j - 1 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j   ] ) &&
             ( x > r[ ( i + 2 )*x_size + j + 1 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j + 2 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j - 2 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j - 1 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j   ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j + 1 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j + 2 ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j - 2 ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j - 1 ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j   ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j + 1 ] ) &&
             ( x >= r[ ( i  )*x_size + j - 2 ] ) &&
             ( x >= r[ ( i  )*x_size + j - 1 ] ) &&
             ( x >= r[ ( i + 1 )*x_size + j - 2 ] ) )
        #endif
        #ifdef SEVEN_SUPP
          if ( ( x > r[ ( i - 3 )*x_size + j - 3 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j - 2 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j - 1 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j   ] ) &&
               ( x > r[ ( i - 3 )*x_size + j + 1 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j + 2 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j + 3 ] ) &&

               ( x > r[ ( i - 2 )*x_size + j - 3 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j - 2 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j - 1 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j   ] ) &&
               ( x > r[ ( i - 2 )*x_size + j + 1 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j + 2 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j + 3 ] ) &&

               ( x > r[ ( i - 1 )*x_size + j - 3 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j - 2 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j - 1 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j   ] ) &&
               ( x > r[ ( i - 1 )*x_size + j + 1 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j + 2 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j + 3 ] ) &&

               ( x > r[ ( i )*x_size + j - 3 ] ) &&
               ( x > r[ ( i )*x_size + j - 2 ] ) &&
               ( x > r[ ( i )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i )*x_size + j + 3 ] ) &&

               ( x >= r[ ( i + 1 )*x_size + j - 3 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j - 2 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j   ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j + 3 ] ) &&

               ( x >= r[ ( i + 2 )*x_size + j - 3 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j - 2 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j   ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j + 3 ] ) &&

               ( x >= r[ ( i + 3 )*x_size + j - 3 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j - 2 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j   ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j + 3 ] ) )
        #endif
          {
            corner_list[ n ].info = 0;
            corner_list[ n ].x = j;
            corner_list[ n ].y = i;
            corner_list[ n ].dx = cgx[ i * x_size + j ];
            corner_list[ n ].dy = cgy[ i * x_size + j ];
            corner_list[ n ].I = in[ i * x_size + j ];
            n++;
            if ( n == MAX_CORNERS ) {
              /*  "Too many corners." */
            }
          }
      }
    }
  }
  corner_list[ n ].info = 7;
}


void susan_corners_quick( uchar *in, char *r, uchar *bp,
                          int max_no, CORNER_LIST corner_list, int x_size, int y_size )
{
  int   n, x, y, i, j;
  uchar *p, *cp;

  susan_wccmemset( r, 0, x_size * y_size );

  _Pragma( "loopbound min 0 max 0" )
  for ( i = 7; i < y_size - 7; i++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( j = 7; j < x_size - 7; j++ ) {
      n = 100;
      p = in + ( i - 3 ) * x_size + j - 1;
      cp = bp + in[ i * x_size + j ];

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 3;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 5;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      p += x_size - 6;

      n += *( cp - *p++ );
      n += *( cp - *p++ );
      n += *( cp - *p );
      if ( n < max_no ) {
        p += 2;
        n += *( cp - *p++ );
        if ( n < max_no ) {
          n += *( cp - *p++ );
          if ( n < max_no ) {
            n += *( cp - *p );
            if ( n < max_no ) {
              p += x_size - 6;

              n += *( cp - *p++ );
              if ( n < max_no ) {
                n += *( cp - *p++ );
                if ( n < max_no ) {
                  n += *( cp - *p++ );
                  if ( n < max_no ) {
                    n += *( cp - *p++ );
                    if ( n < max_no ) {
                      n += *( cp - *p++ );
                      if ( n < max_no ) {
                        n += *( cp - *p++ );
                        if ( n < max_no ) {
                          n += *( cp - *p );
                          if ( n < max_no ) {
                            p += x_size - 5;

                            n += *( cp - *p++ );
                            if ( n < max_no ) {
                              n += *( cp - *p++ );
                              if ( n < max_no ) {
                                n += *( cp - *p++ );
                                if ( n < max_no ) {
                                  n += *( cp - *p++ );
                                  if ( n < max_no ) {
                                    n += *( cp - *p );
                                    if ( n < max_no ) {
                                      p += x_size - 3;

                                      n += *( cp - *p++ );
                                      if ( n < max_no ) {
                                        n += *( cp - *p++ );
                                        if ( n < max_no ) {
                                          n += *( cp - *p );

                                          if ( n < max_no )
                                            r[ i * x_size + j ] = max_no - n;
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /* to locate the local maxima */
  n = 0;
  _Pragma( "loopbound min 0 max 0" )
  for ( i = 7; i < y_size - 7; i++ ) {
    _Pragma( "loopbound min 0 max 0" )
    for ( j = 7; j < x_size - 7; j++ ) {
      x = r[ i * x_size + j ];
      if ( x > 0 )  {
        /* 5x5 mask */
        #ifdef FIVE_SUPP
        if ( ( x > r[ ( i - 1 )*x_size + j + 2 ] ) &&
             ( x > r[ ( i  )*x_size + j + 1 ] ) &&
             ( x > r[ ( i  )*x_size + j + 2 ] ) &&
             ( x > r[ ( i + 1 )*x_size + j - 1 ] ) &&
             ( x > r[ ( i + 1 )*x_size + j   ] ) &&
             ( x > r[ ( i + 1 )*x_size + j + 1 ] ) &&
             ( x > r[ ( i + 1 )*x_size + j + 2 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j - 2 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j - 1 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j   ] ) &&
             ( x > r[ ( i + 2 )*x_size + j + 1 ] ) &&
             ( x > r[ ( i + 2 )*x_size + j + 2 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j - 2 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j - 1 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j   ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j + 1 ] ) &&
             ( x >= r[ ( i - 2 )*x_size + j + 2 ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j - 2 ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j - 1 ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j   ] ) &&
             ( x >= r[ ( i - 1 )*x_size + j + 1 ] ) &&
             ( x >= r[ ( i  )*x_size + j - 2 ] ) &&
             ( x >= r[ ( i  )*x_size + j - 1 ] ) &&
             ( x >= r[ ( i + 1 )*x_size + j - 2 ] ) )
        #endif
        #ifdef SEVEN_SUPP
          if ( ( x > r[ ( i - 3 )*x_size + j - 3 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j - 2 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j - 1 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j   ] ) &&
               ( x > r[ ( i - 3 )*x_size + j + 1 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j + 2 ] ) &&
               ( x > r[ ( i - 3 )*x_size + j + 3 ] ) &&

               ( x > r[ ( i - 2 )*x_size + j - 3 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j - 2 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j - 1 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j   ] ) &&
               ( x > r[ ( i - 2 )*x_size + j + 1 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j + 2 ] ) &&
               ( x > r[ ( i - 2 )*x_size + j + 3 ] ) &&

               ( x > r[ ( i - 1 )*x_size + j - 3 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j - 2 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j - 1 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j   ] ) &&
               ( x > r[ ( i - 1 )*x_size + j + 1 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j + 2 ] ) &&
               ( x > r[ ( i - 1 )*x_size + j + 3 ] ) &&

               ( x > r[ ( i )*x_size + j - 3 ] ) &&
               ( x > r[ ( i )*x_size + j - 2 ] ) &&
               ( x > r[ ( i )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i )*x_size + j + 3 ] ) &&

               ( x >= r[ ( i + 1 )*x_size + j - 3 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j - 2 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j   ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i + 1 )*x_size + j + 3 ] ) &&

               ( x >= r[ ( i + 2 )*x_size + j - 3 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j - 2 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j   ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i + 2 )*x_size + j + 3 ] ) &&

               ( x >= r[ ( i + 3 )*x_size + j - 3 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j - 2 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j - 1 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j   ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j + 1 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j + 2 ] ) &&
               ( x >= r[ ( i + 3 )*x_size + j + 3 ] ) )
        #endif
          {
            corner_list[ n ].info = 0;
            corner_list[ n ].x = j;
            corner_list[ n ].y = i;
            x = in[ ( i - 2 ) * x_size + j - 2 ] + in[ ( i - 2 ) * x_size + j - 1 ] + in[ ( i -
                2 ) * x_size + j ] + in[ ( i - 2 ) * x_size + j + 1 ] + in[ ( i - 2 ) * x_size + j +
                    2 ] +
                in[ ( i - 1 ) * x_size + j - 2 ] + in[ ( i - 1 ) * x_size + j - 1 ] + in[ ( i - 1 ) *
                    x_size + j ] + in[ ( i - 1 ) * x_size + j + 1 ] + in[ ( i - 1 ) * x_size + j + 2 ] +
                in[ ( i  ) * x_size + j - 2 ] + in[ ( i  ) * x_size + j - 1 ] + in[ ( i  ) * x_size +
                    j ] + in[ ( i  ) * x_size + j + 1 ] + in[ ( i  ) * x_size + j + 2 ] +
                in[ ( i + 1 ) * x_size + j - 2 ] + in[ ( i + 1 ) * x_size + j - 1 ] + in[ ( i + 1 ) *
                    x_size + j ] + in[ ( i + 1 ) * x_size + j + 1 ] + in[ ( i + 1 ) * x_size + j + 2 ] +
                in[ ( i + 2 ) * x_size + j - 2 ] + in[ ( i + 2 ) * x_size + j - 1 ] + in[ ( i + 2 ) *
                    x_size + j ] + in[ ( i + 2 ) * x_size + j + 1 ] + in[ ( i + 2 ) * x_size + j + 2 ];

            corner_list[ n ].I = x / 25;
            /*corner_list[ n ].I=in[ i*x_size+j ];*/
            x = in[ ( i - 2 ) * x_size + j + 2 ] + in[ ( i - 1 ) * x_size + j + 2 ] + in[ ( i ) *
                x_size + j + 2 ] + in[ ( i + 1 ) * x_size + j + 2 ] + in[ ( i + 2 ) * x_size + j +
                    2 ] -
                ( in[ ( i - 2 ) * x_size + j - 2 ] + in[ ( i - 1 ) * x_size + j - 2 ] + in[ ( i ) *
                    x_size + j - 2 ] + in[ ( i + 1 ) * x_size + j - 2 ] + in[ ( i + 2 ) * x_size + j -
                        2 ] );
            x += x + in[ ( i - 2 ) * x_size + j + 1 ] + in[ ( i - 1 ) * x_size + j + 1 ] +
                 in[ ( i ) * x_size + j + 1 ] + in[ ( i + 1 ) * x_size + j + 1 ] + in[ ( i + 2 ) *
                     x_size + j + 1 ] -
                 ( in[ ( i - 2 ) * x_size + j - 1 ] + in[ ( i - 1 ) * x_size + j - 1 ] + in[ ( i ) *
                     x_size + j - 1 ] + in[ ( i + 1 ) * x_size + j - 1 ] + in[ ( i + 2 ) * x_size + j -
                         1 ] );

            y = in[ ( i + 2 ) * x_size + j - 2 ] + in[ ( i + 2 ) * x_size + j - 1 ] + in[ ( i +
                2 ) * x_size + j ] + in[ ( i + 2 ) * x_size + j + 1 ] + in[ ( i + 2 ) * x_size + j +
                    2 ] -
                ( in[ ( i - 2 ) * x_size + j - 2 ] + in[ ( i - 2 ) * x_size + j - 1 ] + in[ ( i - 2 )
                    * x_size + j ] + in[ ( i - 2 ) * x_size + j + 1 ] + in[ ( i - 2 ) * x_size + j +
                        2 ] );
            y += y + in[ ( i + 1 ) * x_size + j - 2 ] + in[ ( i + 1 ) * x_size + j - 1 ] +
                 in[ ( i + 1 ) * x_size + j ] + in[ ( i + 1 ) * x_size + j + 1 ] + in[ ( i + 1 ) *
                     x_size + j + 2 ] -
                 ( in[ ( i - 1 ) * x_size + j - 2 ] + in[ ( i - 1 ) * x_size + j - 1 ] + in[ ( i - 1 )
                     * x_size + j ] + in[ ( i - 1 ) * x_size + j + 1 ] + in[ ( i - 1 ) * x_size + j +
                         2 ] );
            corner_list[ n ].dx = x / 15;
            corner_list[ n ].dy = y / 15;
            n++;
            if ( n == MAX_CORNERS ) {
              /*  "Too many corners." */
            }
          }
      }
    }
  }
  corner_list[ n ].info = 7;
}

#ifdef PROGRAM_ON_CCM1
__attribute__((section(".ccmram")))
#endif
void susan_call_susan( struct wccFILE *inputFile, int mode )
{
  uchar  *in, *bp, *mid;
  int   x_size, y_size;
  char  *r;
  CORNER_LIST corner_list;

  susan_get_image( inputFile, &in, &x_size, &y_size );

  if ( susan_dt < 0 ) susan_three_by_three = 1;
  if ( ( susan_principle_conf == 1 ) && ( mode == 0 ) )
    mode = 1;

  switch ( mode ) {
    case 0:
      /* {{{ smoothing */

      susan_setup_brightness_lut( &bp, susan_bt, 2 );
      susan_smoothing( susan_three_by_three, in, susan_dt, x_size, y_size, bp );

      break;
    case 1:
      /* {{{ edges */

      r   = ( char * ) susan_wccmalloc( x_size * y_size );
      susan_setup_brightness_lut( &bp, susan_bt, 6 );

      if ( susan_principle_conf ) {
        if ( susan_three_by_three )
          susan_principle_small( in, r, bp, susan_max_no_edges, x_size, y_size );
        else
          susan_principle( in, r, bp, susan_max_no_edges, x_size, y_size );
        susan_int_to_uchar( r, in, x_size * y_size );
      } else {
        mid = ( uchar * )susan_wccmalloc( x_size * y_size );
        susan_wccmemset( mid, 100, x_size * y_size ); /* note not set to zero */

        if ( susan_three_by_three )
          susan_edges_small( in, r, mid, bp, susan_max_no_edges, x_size, y_size );
        else
          susan_edges( in, r, mid, bp, susan_max_no_edges, x_size, y_size );
        if ( susan_thin_post_proc )
          susan_thin( r, mid, x_size, y_size );
        susan_edge_draw( in, mid, x_size, y_size, susan_drawing_mode );
      }

      break;

    case 2:
      /* {{{ corners */

      r   = ( char * ) susan_wccmalloc( x_size * y_size );
      susan_setup_brightness_lut( &bp, susan_bt, 6 );


      if ( susan_principle_conf ) {
        susan_principle( in, r, bp, susan_max_no_corners, x_size, y_size );
        susan_int_to_uchar( r, in, x_size * y_size );
      } else {
        if ( susan_susan_quick ){
          susan_corners_quick( in, r, bp, susan_max_no_corners, corner_list, x_size,
                               y_size );
      } else {//comes inside
          susan_corners( in, r, bp, susan_max_no_corners, corner_list, x_size, y_size );
      }
        susan_corner_draw( in, corner_list, x_size, susan_drawing_mode );
      }

      //The size of the corner list
      break;
  }

  susan_put_image( x_size, y_size );
}


#ifdef PROGRAM_ON_CCM1
__attribute__((section(".ccmram")))
#endif
void susan_init( void )
{
  volatile int a = 0;
  susan_file.data = susan_input;
  susan_file.size = 7292;
  susan_file.size += a;
  susan_file.cur_pos = 0;
  susan_file.cur_pos += a;

  susan_dt = 4.0;
  susan_dt += a;
  susan_bt = 20;
  susan_bt += a;
  susan_principle_conf = 0;
  susan_principle_conf += a;
  susan_thin_post_proc = 1;
  susan_thin_post_proc += a;
  susan_three_by_three = 0;
  susan_three_by_three += a;
  susan_drawing_mode = 0;
  susan_drawing_mode += a;
  susan_susan_quick = 0;
  susan_susan_quick += a;
  susan_max_no_corners = 50;
  susan_max_no_corners += a;
  susan_max_no_edges = 50;
  susan_max_no_edges += a;

  // mode=0; /* Smoothing mode */
  // mode=1; /* Edges mode */
  // mode=2; /* Corners mode */

  // principle=1; /* Output initial enhancement image only; corners or edges mode (default is edges mode) */
  // thin_post_proc=0; /* No post-processing on the binary edge map (runs much faster); edges mode */
  // drawing_mode=1; /* Mark corners/edges with single black points instead of black with white border; corners or edges mode */
  // three_by_three=1; /* Use flat 3x3 mask, edges or smoothing mode */
  // susan_quick=1; /* Use faster (and usually stabler) corner mode; edge-like corner suppression not carried out; corners mode */
  // dt=10.0; /* Distance threshold, smoothing mode, (default=4) */
  // bt=50; /* Brightness threshold, all modes, (default=20) */
}

void susan_main( void )
{
//  susan_call_susan( &susan_file, 0 );
//  susan_wccfreeall();
//  susan_call_susan( &susan_file, 1 );
//  susan_wccfreeall();
  susan_call_susan( &susan_file, 2 );
  susan_wccfreeall();
}

int susan_return( void )
{
  return 0;
}
//const void * const pointers[10] = {&pointers[1], &pointers[2], &pointers[3], &pointers[4], &pointers[5], &pointers[6], &pointers[7], &pointers[8], &pointers[9], NULL };

// the generation script is in generate_random_pointer_array.ipynb

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
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
    int time1 __attribute__((aligned (4))) = 0;
    int time2 __attribute__((aligned (4))) = 0;

  //void **this_pp = p0;
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
//  int n = 1000;
//  TIM2->CR1&=~TIM_CR1_DIR;

  // enable update interrupts
  TIM2->DIER|=TIM_DIER_UIE;
  //NVIC_EnableIRQ(TIM2_IRQn);



  //start the timer
  HAL_TIM_Base_Start(&htim2);
  //set the over flow counter to 0
  overflow_cnt = 0;

  time1 = TIM2->CNT;
  //execution time is in ms

  // MAX_CORNERS now is 1000
  // the original was 15000 (15KB which causes a stack overflow)
  // Even if we set it to 1500 it will cause a stack overflow
  susan_init();
  susan_main();

  //41.099 on CCM
  //


  time2 = TIM2->CNT;
  tim_cnt = time2 -time1;

  execution_time = overflow_cnt*1000 + (double)tim_cnt/(1000);
  uint8_t msg[40] = {'\0'};

  sprintf(msg,"\n\rSUSAN\n\r%d", (int)(execution_time*1000));
  HAL_UART_Transmit(&huart1, msg, sizeof(msg), 0xffff);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  //measure 5 times
  // no CCM: 45.921
  // CCM1: 45.811
  // CCM2: 44.396
  int i = 0;
  while (i<50)
  {

	  MX_TIM2_Init();
	  	  TIM2->DIER|=TIM_DIER_UIE;
	  	  //start the timer
	  	  HAL_TIM_Base_Start(&htim2);
	  	  overflow_cnt = 0;

	  	  time1 = TIM2->CNT;

	  	  susan_init();
	  	  susan_main();

	  	  time2 = TIM2->CNT;
	  	  tim_cnt = time2 -time1;

	  	  execution_time = overflow_cnt*1000 + (double)tim_cnt/(1000);
	  	  uint8_t msg[40] = {'\0'};

	  	  sprintf(msg,"\n\r%d", (int)(execution_time*1000));
	  	  HAL_UART_Transmit(&huart1, msg, sizeof(msg), 0xffff);
	  	  i++;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  return 0;
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL3;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
  htim2.Init.Prescaler = (24-1);
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = (1000000-1);
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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
  huart1.Init.Parity = UART_PARITY_NONE;
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

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	overflow_cnt++;

}



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
