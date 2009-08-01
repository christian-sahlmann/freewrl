/*
=INSERT_TEMPLATE_HERE=

$Id: MPEG_Utils.c,v 1.8 2009/08/01 09:45:39 couannette Exp $

???

*/

#ifndef ARCH_PPC /* JAS - compile problem on OSX ppc */

#include <config.h>
#include <system.h>
#ifndef WIN32
#include <system_net.h>
#endif
#include <display.h>
#include <internal.h>

#include <libFreeWRL.h>

#include "../vrml_parser/Structs.h"
#include "../main/headers.h"

/*
 * Copyright (c) 1995 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*
 * Portions of this software Copyright (c) 1995 Brown University.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement
 * is hereby granted, provided that the above copyright notice and the
 * following two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BROWN
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * BROWN UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 * BASIS, AND BROWN UNIVERSITY HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "mpeg.h"

#define NO_SANITY_CHECKS
extern const int zigzag_direct[];

/* global return values */
int *frameCount;
int *ySize;
int *xSize;
int *pictureDepth;
char *dataPointer;

/* Decoding table for macroblock_address_increment */
mb_addr_inc_entry     mb_addr_inc[2048];

/* Decoding table for macroblock_type in predictive-coded pictures */
mb_type_entry         mb_type_P[64];

/* Decoding table for macroblock_type in bidirectionally-coded pictures */
mb_type_entry         mb_type_B[64];

/* Decoding table for motion vectors */
motion_vectors_entry  motion_vectors[2048];

/* Decoding table for coded_block_pattern */

coded_block_pattern_entry coded_block_pattern[512] =
{ {(unsigned int)ERROR, 0}, {(unsigned int)ERROR, 0}, {39, 9}, {27, 9}, {59, 9}, {55, 9}, {47, 9}, {31, 9},
    {58, 8}, {58, 8}, {54, 8}, {54, 8}, {46, 8}, {46, 8}, {30, 8}, {30, 8},
    {57, 8}, {57, 8}, {53, 8}, {53, 8}, {45, 8}, {45, 8}, {29, 8}, {29, 8},
    {38, 8}, {38, 8}, {26, 8}, {26, 8}, {37, 8}, {37, 8}, {25, 8}, {25, 8},
    {43, 8}, {43, 8}, {23, 8}, {23, 8}, {51, 8}, {51, 8}, {15, 8}, {15, 8},
    {42, 8}, {42, 8}, {22, 8}, {22, 8}, {50, 8}, {50, 8}, {14, 8}, {14, 8},
    {41, 8}, {41, 8}, {21, 8}, {21, 8}, {49, 8}, {49, 8}, {13, 8}, {13, 8},
    {35, 8}, {35, 8}, {19, 8}, {19, 8}, {11, 8}, {11, 8}, {7, 8}, {7, 8},
    {34, 7}, {34, 7}, {34, 7}, {34, 7}, {18, 7}, {18, 7}, {18, 7}, {18, 7},
    {10, 7}, {10, 7}, {10, 7}, {10, 7}, {6, 7}, {6, 7}, {6, 7}, {6, 7},
    {33, 7}, {33, 7}, {33, 7}, {33, 7}, {17, 7}, {17, 7}, {17, 7}, {17, 7},
    {9, 7}, {9, 7}, {9, 7}, {9, 7}, {5, 7}, {5, 7}, {5, 7}, {5, 7},
    {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6}, {63, 6},
    {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6}, {3, 6},
    {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6}, {36, 6},
    {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6}, {24, 6},
    {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5},
    {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5}, {62, 5},
    {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5},
    {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5}, {2, 5},
    {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5},
    {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5}, {61, 5},
    {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5},
    {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5}, {1, 5},
    {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5},
    {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5}, {56, 5},
    {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5},
    {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5}, {52, 5},
    {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5},
    {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5}, {44, 5},
    {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5},
    {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5}, {28, 5},
    {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5},
    {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5}, {40, 5},
    {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5},
    {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5}, {20, 5},
    {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5},
    {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5}, {48, 5},
    {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5},
    {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5}, {12, 5},
    {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4},
    {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4},
    {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4},
    {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4}, {32, 4},
    {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4},
    {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4},
    {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4},
    {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4}, {16, 4},
    {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4},
    {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4},
    {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4},
    {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4}, {8, 4},
    {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
    {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
    {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
    {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3},
    {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}, {60, 3}
};

/* Decoding tables for dct_dc_size_luminance */
dct_dc_size_entry dct_dc_size_luminance[32] =
{   {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
    {4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}, {(unsigned int)ERROR, 0}
};

dct_dc_size_entry dct_dc_size_luminance1[16] =
{   {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
    {8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10, 9}, {11, 9}
};

/* Decoding table for dct_dc_size_chrominance */
dct_dc_size_entry dct_dc_size_chrominance[32] =
{   {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}, {(unsigned int)ERROR, 0}
};

dct_dc_size_entry dct_dc_size_chrominance1[32] =
{   {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
    {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
    {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7},
    {8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10, 10}, {11, 10}
};

/* DCT coeff tables. */

unsigned short int dct_coeff_tbl_0[256] =
{
0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff,
0xffff, 0xffff, 0xffff, 0xffff,
0x052f, 0x051f, 0x050f, 0x04ff,
0x183f, 0x402f, 0x3c2f, 0x382f,
0x342f, 0x302f, 0x2c2f, 0x7c1f,
0x781f, 0x741f, 0x701f, 0x6c1f,
0x028e, 0x028e, 0x027e, 0x027e,
0x026e, 0x026e, 0x025e, 0x025e,
0x024e, 0x024e, 0x023e, 0x023e,
0x022e, 0x022e, 0x021e, 0x021e,
0x020e, 0x020e, 0x04ee, 0x04ee,
0x04de, 0x04de, 0x04ce, 0x04ce,
0x04be, 0x04be, 0x04ae, 0x04ae,
0x049e, 0x049e, 0x048e, 0x048e,
0x01fd, 0x01fd, 0x01fd, 0x01fd,
0x01ed, 0x01ed, 0x01ed, 0x01ed,
0x01dd, 0x01dd, 0x01dd, 0x01dd,
0x01cd, 0x01cd, 0x01cd, 0x01cd,
0x01bd, 0x01bd, 0x01bd, 0x01bd,
0x01ad, 0x01ad, 0x01ad, 0x01ad,
0x019d, 0x019d, 0x019d, 0x019d,
0x018d, 0x018d, 0x018d, 0x018d,
0x017d, 0x017d, 0x017d, 0x017d,
0x016d, 0x016d, 0x016d, 0x016d,
0x015d, 0x015d, 0x015d, 0x015d,
0x014d, 0x014d, 0x014d, 0x014d,
0x013d, 0x013d, 0x013d, 0x013d,
0x012d, 0x012d, 0x012d, 0x012d,
0x011d, 0x011d, 0x011d, 0x011d,
0x010d, 0x010d, 0x010d, 0x010d,
0x282c, 0x282c, 0x282c, 0x282c,
0x282c, 0x282c, 0x282c, 0x282c,
0x242c, 0x242c, 0x242c, 0x242c,
0x242c, 0x242c, 0x242c, 0x242c,
0x143c, 0x143c, 0x143c, 0x143c,
0x143c, 0x143c, 0x143c, 0x143c,
0x0c4c, 0x0c4c, 0x0c4c, 0x0c4c,
0x0c4c, 0x0c4c, 0x0c4c, 0x0c4c,
0x085c, 0x085c, 0x085c, 0x085c,
0x085c, 0x085c, 0x085c, 0x085c,
0x047c, 0x047c, 0x047c, 0x047c,
0x047c, 0x047c, 0x047c, 0x047c,
0x046c, 0x046c, 0x046c, 0x046c,
0x046c, 0x046c, 0x046c, 0x046c,
0x00fc, 0x00fc, 0x00fc, 0x00fc,
0x00fc, 0x00fc, 0x00fc, 0x00fc,
0x00ec, 0x00ec, 0x00ec, 0x00ec,
0x00ec, 0x00ec, 0x00ec, 0x00ec,
0x00dc, 0x00dc, 0x00dc, 0x00dc,
0x00dc, 0x00dc, 0x00dc, 0x00dc,
0x00cc, 0x00cc, 0x00cc, 0x00cc,
0x00cc, 0x00cc, 0x00cc, 0x00cc,
0x681c, 0x681c, 0x681c, 0x681c,
0x681c, 0x681c, 0x681c, 0x681c,
0x641c, 0x641c, 0x641c, 0x641c,
0x641c, 0x641c, 0x641c, 0x641c,
0x601c, 0x601c, 0x601c, 0x601c,
0x601c, 0x601c, 0x601c, 0x601c,
0x5c1c, 0x5c1c, 0x5c1c, 0x5c1c,
0x5c1c, 0x5c1c, 0x5c1c, 0x5c1c,
0x581c, 0x581c, 0x581c, 0x581c,
0x581c, 0x581c, 0x581c, 0x581c,
};

unsigned short int dct_coeff_tbl_1[16] =
{
0x00bb, 0x202b, 0x103b, 0x00ab,
0x084b, 0x1c2b, 0x541b, 0x501b,
0x009b, 0x4c1b, 0x481b, 0x045b,
0x0c3b, 0x008b, 0x182b, 0x441b,
};

unsigned short int dct_coeff_tbl_2[4] =
{
0x4019, 0x1429, 0x0079, 0x0839,
};

unsigned short int dct_coeff_tbl_3[4] =
{
0x0449, 0x3c19, 0x3819, 0x1029,
};

unsigned short int dct_coeff_next[256] =
{
0xffff, 0xffff, 0xffff, 0xffff,
0xf7d5, 0xf7d5, 0xf7d5, 0xf7d5,
0x0826, 0x0826, 0x2416, 0x2416,
0x0046, 0x0046, 0x2016, 0x2016,
0x1c15, 0x1c15, 0x1c15, 0x1c15,
0x1815, 0x1815, 0x1815, 0x1815,
0x0425, 0x0425, 0x0425, 0x0425,
0x1415, 0x1415, 0x1415, 0x1415,
0x3417, 0x0067, 0x3017, 0x2c17,
0x0c27, 0x0437, 0x0057, 0x2817,
0x0034, 0x0034, 0x0034, 0x0034,
0x0034, 0x0034, 0x0034, 0x0034,
0x1014, 0x1014, 0x1014, 0x1014,
0x1014, 0x1014, 0x1014, 0x1014,
0x0c14, 0x0c14, 0x0c14, 0x0c14,
0x0c14, 0x0c14, 0x0c14, 0x0c14,
0x0023, 0x0023, 0x0023, 0x0023,
0x0023, 0x0023, 0x0023, 0x0023,
0x0023, 0x0023, 0x0023, 0x0023,
0x0023, 0x0023, 0x0023, 0x0023,
0x0813, 0x0813, 0x0813, 0x0813,
0x0813, 0x0813, 0x0813, 0x0813,
0x0813, 0x0813, 0x0813, 0x0813,
0x0813, 0x0813, 0x0813, 0x0813,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0xfbe1, 0xfbe1, 0xfbe1, 0xfbe1,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
0x0011, 0x0011, 0x0011, 0x0011,
};

unsigned short int dct_coeff_first[256] =
{
0xffff, 0xffff, 0xffff, 0xffff,
0xf7d5, 0xf7d5, 0xf7d5, 0xf7d5,
0x0826, 0x0826, 0x2416, 0x2416,
0x0046, 0x0046, 0x2016, 0x2016,
0x1c15, 0x1c15, 0x1c15, 0x1c15,
0x1815, 0x1815, 0x1815, 0x1815,
0x0425, 0x0425, 0x0425, 0x0425,
0x1415, 0x1415, 0x1415, 0x1415,
0x3417, 0x0067, 0x3017, 0x2c17,
0x0c27, 0x0437, 0x0057, 0x2817,
0x0034, 0x0034, 0x0034, 0x0034,
0x0034, 0x0034, 0x0034, 0x0034,
0x1014, 0x1014, 0x1014, 0x1014,
0x1014, 0x1014, 0x1014, 0x1014,
0x0c14, 0x0c14, 0x0c14, 0x0c14,
0x0c14, 0x0c14, 0x0c14, 0x0c14,
0x0023, 0x0023, 0x0023, 0x0023,
0x0023, 0x0023, 0x0023, 0x0023,
0x0023, 0x0023, 0x0023, 0x0023,
0x0023, 0x0023, 0x0023, 0x0023,
0x0813, 0x0813, 0x0813, 0x0813,
0x0813, 0x0813, 0x0813, 0x0813,
0x0813, 0x0813, 0x0813, 0x0813,
0x0813, 0x0813, 0x0813, 0x0813,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0412, 0x0412, 0x0412, 0x0412,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
0x0010, 0x0010, 0x0010, 0x0010,
};

/* Macro for filling up the decoding table for mb_addr_inc */
#define ASSIGN1(start, end, step, val, num) \
  for (i = start; i < end; i+= step) { \
    for (j = 0; j < step; j++) { \
      mb_addr_inc[i+j].value = val; \
      mb_addr_inc[i+j].num_bits = num; \
    } \
    val--; \
    }




/*
 *--------------------------------------------------------------
 *
 * init_mb_addr_inc --
 *
 *	Initialize the VLC decoding table for macro_block_address_increment
 *
 * Results:
 *	The decoding table for macro_block_address_increment will
 *      be filled; illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array mb_addr_inc will be filled.
 *
 *--------------------------------------------------------------
 */
static void
init_mb_addr_inc()
{
  int i, j, val;

  for (i = 0; i < 8; i++) {
    mb_addr_inc[i].value = ERROR;
    mb_addr_inc[i].num_bits = 0;
  }

  mb_addr_inc[8].value = MACRO_BLOCK_ESCAPE;
  mb_addr_inc[8].num_bits = 11;

  for (i = 9; i < 15; i++) {
    mb_addr_inc[i].value = ERROR;
    mb_addr_inc[i].num_bits = 0;
  }

  mb_addr_inc[15].value = MACRO_BLOCK_STUFFING;
  mb_addr_inc[15].num_bits = 11;

  for (i = 16; i < 24; i++) {
    mb_addr_inc[i].value = ERROR;
    mb_addr_inc[i].num_bits = 0;
  }

  val = 33;

  ASSIGN1(24, 36, 1, val, 11);
  ASSIGN1(36, 48, 2, val, 10);
  ASSIGN1(48, 96, 8, val, 8);
  ASSIGN1(96, 128, 16, val, 7);
  ASSIGN1(128, 256, 64, val, 5);
  ASSIGN1(256, 512, 128, val, 4);
  ASSIGN1(512, 1024, 256, val, 3);
  ASSIGN1(1024, 2048, 1024, val, 1);
}


/* Macro for filling up the decoding table for mb_type */
#define ASSIGN2(start, end, quant, motion_forward, motion_backward, pattern, intra, num, mb_type) \
  for (i = start; i < end; i ++) { \
    mb_type[i].mb_quant = quant; \
    mb_type[i].mb_motion_forward = motion_forward; \
    mb_type[i].mb_motion_backward = motion_backward; \
    mb_type[i].mb_pattern = pattern; \
    mb_type[i].mb_intra = intra; \
    mb_type[i].num_bits = num; \
  }



/*
 *--------------------------------------------------------------
 *
 * init_mb_type_P --
 *
 *	Initialize the VLC decoding table for macro_block_type in
 *      predictive-coded pictures.
 *
 * Results:
 *	The decoding table for macro_block_type in predictive-coded
 *      pictures will be filled; illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array mb_type_P will be filled.
 *
 *--------------------------------------------------------------
 */
static void
init_mb_type_P()
{
  int i;

  mb_type_P[0].mb_quant = mb_type_P[0].mb_motion_forward
    = mb_type_P[0].mb_motion_backward = mb_type_P[0].mb_pattern
      = mb_type_P[0].mb_intra = (unsigned int) ERROR;
  mb_type_P[0].num_bits = 0;

  ASSIGN2(1, 2, 1, 0, 0, 0, 1, 6, mb_type_P)
  ASSIGN2(2, 4, 1, 0, 0, 1, 0, 5, mb_type_P)
  ASSIGN2(4, 6, 1, 1, 0, 1, 0, 5, mb_type_P);
  ASSIGN2(6, 8, 0, 0, 0, 0, 1, 5, mb_type_P);
  ASSIGN2(8, 16, 0, 1, 0, 0, 0, 3, mb_type_P);
  ASSIGN2(16, 32, 0, 0, 0, 1, 0, 2, mb_type_P);
  ASSIGN2(32, 64, 0, 1, 0, 1, 0, 1, mb_type_P);
}




/*
 *--------------------------------------------------------------
 *
 * init_mb_type_B --
 *
 *	Initialize the VLC decoding table for macro_block_type in
 *      bidirectionally-coded pictures.
 *
 * Results:
 *	The decoding table for macro_block_type in bidirectionally-coded
 *      pictures will be filled; illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array mb_type_B will be filled.
 *
 *--------------------------------------------------------------
 */
static void
init_mb_type_B()
{
  int i;

  mb_type_B[0].mb_quant = mb_type_B[0].mb_motion_forward
    = mb_type_B[0].mb_motion_backward = mb_type_B[0].mb_pattern
      = mb_type_B[0].mb_intra = (unsigned int) ERROR;
  mb_type_B[0].num_bits = 0;

  ASSIGN2(1, 2, 1, 0, 0, 0, 1, 6, mb_type_B);
  ASSIGN2(2, 3, 1, 0, 1, 1, 0, 6, mb_type_B);
  ASSIGN2(3, 4, 1, 1, 0, 1, 0, 6, mb_type_B);
  ASSIGN2(4, 6, 1, 1, 1, 1, 0, 5, mb_type_B);
  ASSIGN2(6, 8, 0, 0, 0, 0, 1, 5, mb_type_B);
  ASSIGN2(8, 12, 0, 1, 0, 0, 0, 4, mb_type_B);
  ASSIGN2(12, 16, 0, 1, 0, 1, 0, 4, mb_type_B);
  ASSIGN2(16, 24, 0, 0, 1, 0, 0, 3, mb_type_B);
  ASSIGN2(24, 32, 0, 0, 1, 1, 0, 3, mb_type_B);
  ASSIGN2(32, 48, 0, 1, 1, 0, 0, 2, mb_type_B);
  ASSIGN2(48, 64, 0, 1, 1, 1, 0, 2, mb_type_B);
}


/* Macro for filling up the decoding tables for motion_vectors */
#define ASSIGN3(start, end, step, val, num) \
  for (i = start; i < end; i+= step) { \
    for (j = 0; j < step / 2; j++) { \
      motion_vectors[i+j].code = val; \
      motion_vectors[i+j].num_bits = num; \
    } \
    for (j = step / 2; j < step; j++) { \
      motion_vectors[i+j].code = -val; \
      motion_vectors[i+j].num_bits = num; \
    } \
    val--; \
  }



/*
 *--------------------------------------------------------------
 *
 * init_motion_vectors --
 *
 *	Initialize the VLC decoding table for the various motion
 *      vectors, including motion_horizontal_forward_code,
 *      motion_vertical_forward_code, motion_horizontal_backward_code,
 *      and motion_vertical_backward_code.
 *
 * Results:
 *	The decoding table for the motion vectors will be filled;
 *      illegal values will be filled as ERROR.
 *
 * Side effects:
 *	The global array motion_vector will be filled.
 *
 *--------------------------------------------------------------
 */
static void
init_motion_vectors()
{
  int i, j, val = 16;

  for (i = 0; i < 24; i++) {
    motion_vectors[i].code = ERROR;
    motion_vectors[i].num_bits = 0;
  }

  ASSIGN3(24, 36, 2, val, 11);
  ASSIGN3(36, 48, 4, val, 10);
  ASSIGN3(48, 96, 16, val, 8);
  ASSIGN3(96, 128, 32, val, 7);
  ASSIGN3(128, 256, 128, val, 5);
  ASSIGN3(256, 512, 256, val, 4);
  ASSIGN3(512, 1024, 512, val, 3);
  ASSIGN3(1024, 2048, 1024, val, 1);
}



extern void init_pre_idct();


/*
 *--------------------------------------------------------------
 *
 * init_tables --
 *
 *	Initialize all the tables for VLC decoding; this must be
 *      called when the system is set up before any decoding can
 *      take place.
 *
 * Results:
 *	All the decoding tables will be filled accordingly.
 *
 * Side effects:
 *	The corresponding global array for each decoding table
 *      will be filled.
 *
 *--------------------------------------------------------------
 */
void
init_tables()
{

  init_mb_addr_inc();
  init_mb_type_P();
  init_mb_type_B();
  init_motion_vectors();

#ifdef FLOATDCT
  if (qualityFlag)
    init_float_idct();
  else
#endif
    init_pre_idct();

}

#if defined(OLDCODE)

/*
 *--------------------------------------------------------------
 *
 * DecodeDCTDCSizeLum --
 *
 *	Huffman Decoder for dct_dc_size_luminance; location where
 *      the result of decoding will be placed is passed as argument.
 *      The decoded values are obtained by doing a table lookup on
 *      dct_dc_size_luminance.
 *
 * Results:
 *	The decoded value for dct_dc_size_luminance or ERROR for
 *      unbound values will be placed in the location specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
void
decodeDCTDCSizeLum(value)
unsigned int *value;
{
  unsigned int index;

  show_bits5(index);

  if (index < 31) {
  	*value = dct_dc_size_luminance[index].value;
  	flush_bits(dct_dc_size_luminance[index].num_bits);
  }
  else {
	show_bits9(index);
	index -= 0x1f0;
	*value = dct_dc_size_luminance1[index].value;
	flush_bits(dct_dc_size_luminance1[index].num_bits);
  }
}




/*
 *--------------------------------------------------------------
 *
 * DecodeDCTDCSizeChrom --
 *
 *	Huffman Decoder for dct_dc_size_chrominance; location where
 *      the result of decoding will be placed is passed as argument.
 *      The decoded values are obtained by doing a table lookup on
 *      dct_dc_size_chrominance.
 *
 * Results:
 *	The decoded value for dct_dc_size_chrominance or ERROR for
 *      unbound values will be placed in the location specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
void
decodeDCTDCSizeChrom(value)
unsigned int *value;
{
  unsigned int index;

  show_bits5(index);

  if (index < 31) {
  	*value = dct_dc_size_chrominance[index].value;
  	flush_bits(dct_dc_size_chrominance[index].num_bits);
  }
  else {
	show_bits10(index);
	index -= 0x3e0;
	*value = dct_dc_size_chrominance1[index].value;
	flush_bits(dct_dc_size_chrominance1[index].num_bits);
  }
}



/*
 *--------------------------------------------------------------
 *
 * decodeDCTCoeff --
 *
 *	Huffman Decoder for dct_coeff_first and dct_coeff_next;
 *      locations where the results of decoding: run and level, are to
 *      be placed and also the type of DCT coefficients, either
 *      dct_coeff_first or dct_coeff_next, are being passed as argument.
 *
 *      The decoder first examines the next 8 bits in the input stream,
 *      and perform according to the following cases:
 *
 *      '0000 0000' - examine 8 more bits (i.e. 16 bits total) and
 *                    perform a table lookup on dct_coeff_tbl_0.
 *                    One more bit is then examined to determine the sign
 *                    of level.
 *
 *      '0000 0001' - examine 4 more bits (i.e. 12 bits total) and
 *                    perform a table lookup on dct_coeff_tbl_1.
 *                    One more bit is then examined to determine the sign
 *                    of level.
 *
 *      '0000 0010' - examine 2 more bits (i.e. 10 bits total) and
 *                    perform a table lookup on dct_coeff_tbl_2.
 *                    One more bit is then examined to determine the sign
 *                    of level.
 *
 *      '0000 0011' - examine 2 more bits (i.e. 10 bits total) and
 *                    perform a table lookup on dct_coeff_tbl_3.
 *                    One more bit is then examined to determine the sign
 *                    of level.
 *
 *      otherwise   - perform a table lookup on dct_coeff_tbl. If the
 *                    value of run is not ESCAPE, extract one more bit
 *                    to determine the sign of level; otherwise 6 more
 *                    bits will be extracted to obtain the actual value
 *                    of run , and then 8 or 16 bits to get the value of level.
 *
 *
 *
 * Results:
 *	The decoded values of run and level or ERROR for unbound values
 *      are placed in the locations specified.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
static void
decodeDCTCoeff(dct_coeff_tbl, run, level)
unsigned short int *dct_coeff_tbl;
unsigned int *run;
int *level;
{
  unsigned int temp, index /*, num_bits */;
  unsigned int value, next32bits, flushed;

  /*
   * Grab the next 32 bits and use it to improve performance of
   * getting the bits to parse. Thus, calls are translated as:
   *
   *	show_bitsX  <-->   next32bits >> (32-X)
   *	get_bitsX   <-->   val = next32bits >> (32-flushed-X);
   *			   flushed += X;
   *			   next32bits &= bitMask[flushed];
   *	flush_bitsX <-->   flushed += X;
   *			   next32bits &= bitMask[flushed];
   *
   */
  show_bits32(next32bits);
  flushed = 0;

  /* show_bits8(index); */
  index = next32bits >> 24;

  if (index > 3) {
    value = dct_coeff_tbl[index];
    *run = (value & RUN_MASK) >> RUN_SHIFT;
    if (*run == END_OF_BLOCK) {
      *level = END_OF_BLOCK;
    }
    else {
      /* num_bits = (value & NUM_MASK) + 1; */
      /* flush_bits(num_bits); */
      flushed = (value & NUM_MASK) + 1;
      next32bits &= bitMask[flushed];
      if (*run != ESCAPE) {
         *level = (value & LEVEL_MASK) >> LEVEL_SHIFT;
	 /* get_bits1(value); */
	 /* if (value) *level = -*level; */
	 if (next32bits >> (31-flushed)) *level = -*level;
	 flushed++;
	 /* next32bits &= bitMask[flushed];  last op before update */
       }
       else {    /* *run == ESCAPE */
         /* get_bits14(temp); */
	 temp = next32bits >> (18-flushed);
	 flushed += 14;
	 next32bits &= bitMask[flushed];
	 *run = temp >> 8;
	 temp &= 0xff;
	 if (temp == 0) {
            /* get_bits8(*level); */
	    *level = next32bits >> (24-flushed);
	    flushed += 8;
	    /* next32bits &= bitMask[flushed];  last op before update */
 	    assert(*level >= 128);
	 } else if (temp != 128) {
	    /* Grab sign bit */
	    *level = ((int) (temp << 24)) >> 24;
	 } else {
            /* get_bits8(*level); */
	    *level = next32bits >> (24-flushed);
	    flushed += 8;
	    /* next32bits &= bitMask[flushed];  last op before update */
	    *level = *level - 256;
	    assert(*level <= -128 && *level >= -255);
	 }
       }
       /* Update bitstream... */
       flush_bits(flushed);
    }
  }
  else {
    if (index == 2) {
      /* show_bits10(index); */
      index = next32bits >> 22;
      value = dct_coeff_tbl_2[index & 3];
    }
    else if (index == 3) {
      /* show_bits10(index); */
      index = next32bits >> 22;
      value = dct_coeff_tbl_3[index & 3];
    }
    else if (index) {	/* index == 1 */
      /* show_bits12(index); */
      index = next32bits >> 20;
      value = dct_coeff_tbl_1[index & 15];
    }
    else {   /* index == 0 */
      /* show_bits16(index); */
      index = next32bits >> 16;
      value = dct_coeff_tbl_0[index & 255];
    }
    *run = (value & RUN_MASK) >> RUN_SHIFT;
    *level = (value & LEVEL_MASK) >> LEVEL_SHIFT;

    /*
     * Fold these operations together to make it fast...
     */
    /* num_bits = (value & NUM_MASK) + 1; */
    /* flush_bits(num_bits); */
    /* get_bits1(value); */
    /* if (value) *level = -*level; */

    flushed = (value & NUM_MASK) + 2;
    if ((next32bits >> (32-flushed)) & 0x1) *level = -*level;

    /* Update bitstream ... */
    flush_bits(flushed);
  }
}


/*
 *--------------------------------------------------------------
 *
 * decodeDCTCoeffFirst --
 *
 *	Huffman Decoder for dct_coeff_first. Locations for the
 *      decoded results: run and level, are being passed as
 *      arguments. Actual work is being done by calling DecodeDCTCoeff,
 *      with the table dct_coeff_first.
 *
 * Results:
 *	The decoded values of run and level for dct_coeff_first or
 *      ERROR for unbound values are placed in the locations given.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
void
decodeDCTCoeffFirst(run, level)
unsigned int *run;
int *level;
{
  decodeDCTCoeff(dct_coeff_first, run, level);
}




/*
 *--------------------------------------------------------------
 *
 * decodeDCTCoeffNext --
 *
 *	Huffman Decoder for dct_coeff_first. Locations for the
 *      decoded results: run and level, are being passed as
 *      arguments. Actual work is being done by calling DecodeDCTCoeff,
 *      with the table dct_coeff_next.
 *
 * Results:
 *	The decoded values of run and level for dct_coeff_next or
 *      ERROR for unbound values are placed in the locations given.
 *
 * Side effects:
 *	Bit stream is irreversibly parsed.
 *
 *--------------------------------------------------------------
 */
void
decodeDCTCoeffNext(run, level)
unsigned int *run;
int *level;
{
  decodeDCTCoeff(dct_coeff_next, run, level);
}
#endif

static int number_of_bits_set(unsigned long a)
/* unsigned long a;*/
{
    if(!a) return 0;
    if(a & 1) return 1 + number_of_bits_set(a >> 1);
    return(number_of_bits_set(a >> 1));
}

/*
 * How many 0 bits are there at least significant end of longword.
 * Low performance, do not call often.
 */
static int
free_bits_at_bottom(unsigned long a)
/* unsigned long a;*/
{
      /* assume char is 8 bits */
    if(!a) return sizeof(unsigned long) * 8;
    if(((long)a) & 1l) return 0;
    return 1 + free_bits_at_bottom ( a >> 1);
}

static int *L_tab=NULL, *Cr_r_tab=NULL, *Cr_g_tab=NULL, *Cb_g_tab=NULL,
	   *Cb_b_tab=NULL;

/*
 * We define tables that convert a color value between -256 and 512
 * into the R, G and B parts of the pixel. The normal range is 0-255.
 */

static long *r_2_pix=NULL;
static long *g_2_pix=NULL;
static long *b_2_pix=NULL;
static long *r_2_pix_alloc=NULL;
static long *g_2_pix_alloc=NULL;
static long *b_2_pix_alloc=NULL;


/*
 *--------------------------------------------------------------
 *
 * InitColor16Dither --
 *
 *	To get rid of the multiply and other conversions in color
 *	dither, we use a lookup table.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The lookup tables are initialized.
 *
 *--------------------------------------------------------------
 */

void
InitColorDither(int thirty2)
/* int thirty2;*/
{
    /*
     * misuse of the wpixel array for the pixel masks. Note that this
     * implies that the window is created before this routine is called
     */
    unsigned long red_mask = 0xff;
    unsigned long green_mask = 0xff00;
    unsigned long blue_mask = 0xff0000;

    int CR, CB, i;

    if (L_tab==NULL)
       L_tab = (int *)MALLOC(256*sizeof(int));
    if (Cr_r_tab==NULL)
       Cr_r_tab = (int *)MALLOC(256*sizeof(int));
    if (Cr_g_tab==NULL)
       Cr_g_tab = (int *)MALLOC(256*sizeof(int));
    if (Cb_g_tab==NULL)
       Cb_g_tab = (int *)MALLOC(256*sizeof(int));
    if (Cb_b_tab==NULL)
       Cb_b_tab = (int *)MALLOC(256*sizeof(int));

    if (r_2_pix_alloc==NULL)
       r_2_pix_alloc = (long *)MALLOC(768*sizeof(long));
    if (g_2_pix_alloc==NULL)
       g_2_pix_alloc = (long *)MALLOC(768*sizeof(long));
    if (b_2_pix_alloc==NULL)
       b_2_pix_alloc = (long *)MALLOC(768*sizeof(long));


    for (i=0; i<256; i++) {
      L_tab[i] = i;
      CB = CR = i;

      CB -= 128; CR -= 128;
      Cr_r_tab[i] = (int) (0.419/0.299) * CR;
      Cr_g_tab[i] = - (int)(0.299/0.419) * CR;
      Cb_g_tab[i] = - (int)(0.114/0.331) * CB;
      Cb_b_tab[i] = (int)(0.587/0.331) * CB;
    }

    /*
     * Set up entries 0-255 in rgb-to-pixel value tables.
     */
    for (i = 0; i < 256; i++) {
      r_2_pix_alloc[i + 256] = i >> (8 - number_of_bits_set(red_mask));
      r_2_pix_alloc[i + 256] <<= free_bits_at_bottom(red_mask);
      g_2_pix_alloc[i + 256] = i >> (8 - number_of_bits_set(green_mask));
      g_2_pix_alloc[i + 256] <<= free_bits_at_bottom(green_mask);
      b_2_pix_alloc[i + 256] = i >> (8 - number_of_bits_set(blue_mask));
      b_2_pix_alloc[i + 256] <<= free_bits_at_bottom(blue_mask);
      /*
       * If we have 16-bit output depth, then we double the value
       * in the top word. This means that we can write out both
       * pixels in the pixel doubling mode with one op. It is
       * harmless in the normal case as storing a 32-bit value
       * through a short pointer will lose the top bits anyway.
       * A similar optimisation for Alpha for 64 bit has been
       * prepared for, but is not yet implemented.
       */
    }

    /*
     * Spread out the values we have to the rest of the array so that
     * we do not need to check for overflow.
     */
    for (i = 0; i < 256; i++) {
      r_2_pix_alloc[i] = r_2_pix_alloc[256];
      r_2_pix_alloc[i+ 512] = r_2_pix_alloc[511];
      g_2_pix_alloc[i] = g_2_pix_alloc[256];
      g_2_pix_alloc[i+ 512] = g_2_pix_alloc[511];
      b_2_pix_alloc[i] = b_2_pix_alloc[256];
      b_2_pix_alloc[i+ 512] = b_2_pix_alloc[511];
    }

    r_2_pix = r_2_pix_alloc + 256;
    g_2_pix = g_2_pix_alloc + 256;
    b_2_pix = b_2_pix_alloc + 256;

}


/*
 *--------------------------------------------------------------
 *
 * Color32DitherImage --
 *
 *	Converts image into 32 bit color (or 24-bit non-packed).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

/*
 * This is a copysoft version of the function above with ints instead
 * of shorts to cause a 4-byte pixel size
 */

void
Color32DitherImage(
  unsigned char *lum,
  unsigned char *cr,
  unsigned char *cb,
  unsigned char *out,
  int cols, int rows )
{

    int L, CR, CB;
    unsigned int *row1, *row2;
    unsigned char *lum2;
    int x, y;
    int cr_r;
    int cr_g;
    int cb_g;
    int cb_b;
    int cols_2 = cols / 2;

    row1 = (unsigned int *)out;
    row2 = row1 + cols_2 + cols_2;
    lum2 = lum + cols_2 + cols_2;
    for (y=0; y<rows; y+=2) {
	for (x=0; x<cols_2; x++) {
	    int R, G, B;

	    CR = *cr++;
	    CB = *cb++;
	    cr_r = Cr_r_tab[CR];
	    cr_g = Cr_g_tab[CR];
	    cb_g = Cb_g_tab[CB];
	    cb_b = Cb_b_tab[CB];

            L = L_tab[(int) *lum++];

	    R = L + cr_r;
	    G = L + cr_g + cb_g;
	    B = L + cb_b;

	    *row1++ = (r_2_pix[R] | g_2_pix[G] | b_2_pix[B]);

            L = L_tab[(int) *lum++];

	    R = L + cr_r;
	    G = L + cr_g + cb_g;
	    B = L + cb_b;

	    *row1++ = (r_2_pix[R] | g_2_pix[G] | b_2_pix[B]);

	    /*
	     * Now, do second row.
	     */

	    L = L_tab [(int) *lum2++];
	    R = L + cr_r;
	    G = L + cr_g + cb_g;
	    B = L + cb_b;

	    *row2++ = (r_2_pix[R] | g_2_pix[G] | b_2_pix[B]);

	    L = L_tab [(int) *lum2++];
	    R = L + cr_r;
	    G = L + cr_g + cb_g;
	    B = L + cb_b;

	    *row2++ = (r_2_pix[R] | g_2_pix[G] | b_2_pix[B]);
	}
	lum += cols_2 + cols_2;
	lum2 += cols_2 + cols_2;
	row1 += cols_2 + cols_2;
	row2 += cols_2 + cols_2;
    }
}

/* Array that remaps color numbers to actual pixel values used by X server. */

unsigned char pixel[256];
#ifndef WIN32
extern char *strrchr();
#endif
#define PPM_BITS 8


/*
 *--------------------------------------------------------------
 *
 * ExecuteTexture --
 *
 *	Write out a display plane as a OpenGL texture.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

void
ExecuteTexture(
     mpeg_VidStream *vid_stream )
{
  unsigned int *p;
  unsigned int r, g, b;
  unsigned int i,j;
  int hsize;
  int blockSize;
  char *tmpptr;

  /* v_size = height, h_size = width (vertical, horizontal) */
#ifdef WIN32
  GLubyte *Image;
  Image = MALLOC(vid_stream->v_size * vid_stream->h_size * 3);
#else
  GLubyte Image[vid_stream->v_size][vid_stream->h_size][3];
#endif

  /* if the hsize is a power of 2. */
  hsize = vid_stream->mb_width * 16;

  for (i=0; i<vid_stream->v_size; i++) {
	  p = (unsigned int *) vid_stream->current->display + i*hsize;
	  for (j=0; j<vid_stream->h_size; j++){

    		r = *p & 0xff;
    		g = (*p >> PPM_BITS) & 0xff;
    		b = (*p >> (2*PPM_BITS)) & 0xff;
#ifdef WIN32
			/* p.839 strustrup C++  [j][j][k] or [(i*dim2 +j)*dim3 +k] */
    		Image [((vid_stream->v_size-i-1)*vid_stream->h_size + j)*3 + 0]=r;
    		Image [((vid_stream->v_size-i-1)*vid_stream->h_size + j)*3 + 1]=g;
    		Image [((vid_stream->v_size-i-1)*vid_stream->h_size + j)*3 + 2]=b;
#else
    		Image [vid_stream->v_size-i-1][j][0]=r;
    		Image [vid_stream->v_size-i-1][j][1]=g;
    		Image [vid_stream->v_size-i-1][j][2]=b;
#endif
		p++;
	}
  }

  /* store this frame for later binding in the correct thread */
	*xSize= vid_stream->h_size;
	*ySize = vid_stream->v_size;

  	blockSize = sizeof(GLubyte) * 3 * vid_stream->v_size * vid_stream->h_size;
        dataPointer = (char *)REALLOC(dataPointer,blockSize * (*frameCount));
	tmpptr = dataPointer + (blockSize * ((*frameCount)-1));

        memcpy (tmpptr, Image, blockSize);
	(*frameCount)++;
#ifdef WIN32
	FREE_IF_NZ(Image);
#endif

}
/* Bit masks used by bit i/o operations. */
unsigned int nBitMask[] = { 0x00000000, 0x80000000, 0xc0000000, 0xe0000000,
				0xf0000000, 0xf8000000, 0xfc000000, 0xfe000000,
				0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
				0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000,
				0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000,
				0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
				0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
				0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe};

unsigned int bitMask[] = {  0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff,
			    0x0fffffff, 0x07ffffff, 0x03ffffff, 0x01ffffff,
			    0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff,
			    0x000fffff, 0x0007ffff, 0x0003ffff, 0x0001ffff,
			    0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff,
			    0x00000fff, 0x000007ff, 0x000003ff, 0x000001ff,
			    0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
			    0x0000000f, 0x00000007, 0x00000003, 0x00000001};

unsigned int rBitMask[] = { 0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
			    0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
			    0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
			    0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
			    0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
			    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
			    0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
			    0xf0000000, 0xe0000000, 0xc0000000, 0x80000000};

unsigned int bitTest[] = {  0x80000000, 0x40000000, 0x20000000, 0x10000000,
			    0x08000000, 0x04000000, 0x02000000, 0x01000000,
			    0x00800000, 0x00400000, 0x00200000, 0x00100000,
			    0x00080000, 0x00040000, 0x00020000, 0x00010000,
			    0x00008000, 0x00004000, 0x00002000, 0x00001000,
			    0x00000800, 0x00000400, 0x00000200, 0x00000100,
			    0x00000080, 0x00000040, 0x00000020, 0x00000010,
			    0x00000008, 0x00000004, 0x00000002, 0x00000001};


/*
 *--------------------------------------------------------------
 *
 * correct_underflow --
 *
 *	Called when buffer does not have sufficient data to
 *      satisfy request for bits.
 *      Calls get_more_data, an application specific routine
 *      required to fill the buffer with more data.
 *
 * Results:
 *      None really.
 *
 * Side effects:
 *	buf_length and buffer fields may be changed.
 *
 *--------------------------------------------------------------
 */

void
correct_underflow(
   mpeg_VidStream *vid_stream )
{

  int status;

  status = get_more_data(vid_stream);

  if (status  < 0) {
      fprintf (stderr, "\n");
      perror("Unexpected read error.");
  }
  else if ((status == 0) && (vid_stream->buf_length < 1)) {
      printf("\nImproper or missing sequence end code.\n");
    vid_stream->film_has_ended=TRUE;
      clear_data_stream(vid_stream);
    return;
  }
#ifdef UTIL2
  vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;
#else
  vid_stream->curBits = *vid_stream->buffer;
#endif

}


/*
 *--------------------------------------------------------------
 *
 * next_bits --
 *
 *	Compares next num bits to low order position in mask.
 *      Buffer pointer is NOT advanced.
 *
 * Results:
 *	TRUE, FALSE, or error code.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

int next_bits(
int num,
unsigned int mask,
mpeg_VidStream *vid_stream )
{
  unsigned int stream;
  int ret_value;

  /* If no current stream, return error. */

  if (vid_stream == NULL)
    return NO_VID_STREAM;

  /* Get next num bits, no buffer pointer advance. */

  show_bitsn(num, stream);

  /* Compare bit stream and mask. Set return value toTRUE if equal, FALSE if
     differs.
  */

  if (mask == stream) {
    ret_value = TRUE;
  } else ret_value = FALSE;

  /* Return return value. */
  return ret_value;
}


/*
 *--------------------------------------------------------------
 *
 * get_ext_data --
 *
 *	Assumes that bit stream is at begining of extension
 *      data. Parses off extension data into dynamically
 *      allocated space until start code is hit.
 *
 * Results:
 *	Pointer to dynamically allocated memory containing
 *      extension data.
 *
 * Side effects:
 *	Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

char *get_ext_data (
   mpeg_VidStream *vid_stream)
{
  unsigned int size, marker;
  char *dataPtr;
  unsigned int data;

  /* Set initial ext data buffer size. */

  size = EXT_BUF_SIZE;

  /* Allocate ext data buffer. */

  dataPtr = (char *) MALLOC(size);

  /* Initialize marker to keep place in ext data buffer. */

  marker = 0;

  /* While next data is not start code... */
  while (!next_bits(24, 0x000001, vid_stream)) {

    /* Get next byte of ext data. */

    get_bits8(data);

    /* Put ext data into ext data buffer. Advance marker. */

    dataPtr[marker] = (char) data;
    marker++;

    /* If end of ext data buffer reached, resize data buffer. */

    if (marker == size) {
      size += EXT_BUF_SIZE;
      dataPtr = (char *) REALLOC(dataPtr, size);
    }
  }

  /* Realloc data buffer to free any extra space. */

  dataPtr = (char *) REALLOC(dataPtr, marker);

  /* Return pointer to ext data buffer. */
  return dataPtr;
}


/*
 *--------------------------------------------------------------
 *
 * next_start_code --
 *
 *	Parses off bitstream until start code reached. When done
 *      next 4 bytes of bitstream will be start code. Bit offset
 *      reset to 0.
 *
 * Results:
 *	Status code.
 *
 * Side effects:
 *	Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

int next_start_code(
   mpeg_VidStream *vid_stream)
{
  int state;
  int byteoff;
  unsigned int data;


  /* If no current stream, return error. */

  if (vid_stream== NULL)
    return NO_VID_STREAM;

  /* If insufficient buffer length, correct underflow. */

  if (vid_stream->buf_length < 4) {
    correct_underflow(vid_stream);
  }

  /* If bit offset not zero, reset and advance buffer pointer. */

  byteoff = vid_stream->bit_offset % 8;

  if (byteoff != 0) {
    flush_bits((8-byteoff));
  }

  /* Set state = 0. */

  state = 0;

  /* While buffer has data ... */

  while(vid_stream->buf_length > 0) {

    /* If insufficient data exists, correct underflow. */


    if (vid_stream->buf_length < 4) {
      correct_underflow(vid_stream);
    }

    /* If next byte is zero... */

    get_bits8(data);

    if (data == 0) {

      /* If state < 2, advance state. */

      if (state < 2) state++;
    }

    /* If next byte is one... */

    else if (data == 1) {

      /* If state == 2, advance state (i.e. start code found). */

      if (state == 2) state++;

      /* Otherwise, reset state to zero. */

      else state = 0;
    }

    /* Otherwise byte is neither 1 or 0, reset state to 0. */

    else {
      state = 0;
    }

    /* If state == 3 (i.e. start code found)... */

    if (state == 3) {

      /* Set buffer pointer back and reset length & bit offsets so
       * next bytes will be beginning of start code.
       */

      vid_stream->bit_offset = vid_stream->bit_offset - 24;

      if (vid_stream->bit_offset < 0) {
        vid_stream->bit_offset = 32 + vid_stream->bit_offset;
        vid_stream->buf_length++;
        vid_stream->buffer--;
#ifdef UTIL2
        vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;
#else
        vid_stream->curBits = *vid_stream->buffer;
#endif
      }
      else {
#ifdef UTIL2
        vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;
#else
        vid_stream->curBits = *vid_stream->buffer;
#endif
      }

      /* Return success. */
      return OK;
    }
  }

  /* Return underflow error. */
  return STREAM_UNDERFLOW;
}


/*
 *--------------------------------------------------------------
 *
 * get_extra_bit_info --
 *
 *	Parses off extra bit info stream into dynamically
 *      allocated memory. Extra bit info is indicated by
 *      a flag bit set to 1, followed by 8 bits of data.
 *      This continues until the flag bit is zero. Assumes
 *      that bit stream set to first flag bit in extra
 *      bit info stream.
 *
 * Results:
 *	Pointer to dynamically allocated memory with extra
 *      bit info in it. Flag bits are NOT included.
 *
 * Side effects:
 *	Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

char *get_extra_bit_info(
mpeg_VidStream *vid_stream)
{
  unsigned int size, marker;
  char *dataPtr;
  unsigned int data;

  /* Get first flag bit. */
  get_bits1(data);

  /* If flag is false, return NULL pointer (i.e. no extra bit info). */

  if (!data) return NULL;

  /* Initialize size of extra bit info buffer and allocate. */

  size = EXT_BUF_SIZE;
  dataPtr = (char *) MALLOC(size);

  /* Reset marker to hold place in buffer. */

  marker = 0;

  /* While flag bit is true. */

  while (data) {

    /* Get next 8 bits of data. */
    get_bits8(data);

    /* Place in extra bit info buffer. */

    dataPtr[marker] = (char) data;
    marker++;

    /* If buffer is full, REALLOCate. */

    if (marker == size) {
      size += EXT_BUF_SIZE;
      dataPtr = (char *) REALLOC(dataPtr, size);
    }

    /* Get next flag bit. */
    get_bits1(data);
  }

  /* Reallocate buffer to free extra space. */
  dataPtr = (char *) REALLOC(dataPtr, marker);

  /* Return pointer to extra bit info buffer. */
  return dataPtr;
}

#ifdef DCPREC
extern int dcprec;
#endif

/* Macro for returning 1 if num is positive, -1 if negative, 0 if 0. */

#define Sign(num) ((num > 0) ? 1 : ((num == 0) ? 0 : -1))


/*
 *--------------------------------------------------------------
 *
 * ParseReconBlock --
 *
 *    Parse values for block structure from bitstream.
 *      n is an indication of the position of the block within
 *      the macroblock (i.e. 0-5) and indicates the type of
 *      block (i.e. luminance or chrominance). Reconstructs
 *      coefficients from values parsed and puts in
 *      block.dct_recon array in vid stream structure.
 *      sparseFlag is set when the block contains only one
 *      coeffictient and is used by the IDCT.
 *
 * Results:
 *
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

#define DCT_recon blockPtr->dct_recon
#define DCT_dc_y_past blockPtr->dct_dc_y_past
#define DCT_dc_cr_past blockPtr->dct_dc_cr_past
#define DCT_dc_cb_past blockPtr->dct_dc_cb_past

#define DECODE_DCT_COEFF_FIRST DecodeDCTCoeffFirst
#define DECODE_DCT_COEFF_NEXT DecodeDCTCoeffNext

void
ParseReconBlock(
     int n,
     mpeg_VidStream *vid_stream)
{
#ifdef RISC
  unsigned int temp_curBits;
  int temp_bitOffset;
  int temp_bufLength;
  unsigned int *temp_bitBuffer;
#endif

  Block *blockPtr = &vid_stream->block;
  int coeffCount=0;

  if (vid_stream->buf_length < 100)
    correct_underflow(vid_stream);

#ifdef RISC
  temp_curBits = vid_stream->curBits;
  temp_bitOffset = vid_stream->buf_offset;
  temp_bufLength = vid_stream->buf_length;
  temp_bitBuffer = bitBuffer;
#endif

  {
    /*
     * Copy the mpeg_VidStream fields curBits, bitOffset, and bitBuffer
     * into local variables with the same names, so the macros use the
     * local variables instead.  This allows register allocation and
     * can provide 1-2 fps speedup.  On machines with not so many registers,
     * don't do this.
     */
#ifdef RISC
    register unsigned int curBits = temp_curBits;
    register int bitOffset = temp_bitOffset;
    register int bufLength = temp_bufLength;
    register unsigned int *bitBuffer = temp_bitBuffer;
#endif

    int diff;
    int size, level=0, i, run, pos, coeff;
    short int *reconptr;
    unsigned char *iqmatrixptr, *niqmatrixptr;
    int qscale;

    reconptr = DCT_recon[0];

    /*
     * Hand coded version of memset that's a little faster...
     * Old call:
     *    memset((char *) DCT_recon, 0, 64*sizeof(short int));
     */
    {
      int *p;
      p = (int *) reconptr;

      p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = p[6] = p[7] = p[8] = p[9] =
      p[10] = p[11] = p[12] = p[13] = p[14] = p[15] = p[16] = p[17] = p[18] =
      p[19] = p[20] = p[21] = p[22] = p[23] = p[24] = p[25] = p[26] = p[27] =
      p[28] = p[29] = p[30] = p[31] = 0;

    }

    if (vid_stream->mblock.mb_intra) {

      if (n < 4) {

    /*
     * Get the luminance bits.  This code has been hand optimized to
     * get by the normal bit parsing routines.  We get some speedup
     * by grabbing the next 16 bits and parsing things locally.
     * Thus, calls are translated as:
     *
     *    show_bitsX  <-->   next16bits >> (16-X)
     *    get_bitsX   <-->   val = next16bits >> (16-flushed-X);
     *               flushed += X;
     *               next16bits &= bitMask[flushed];
     *    flush_bitsX <-->   flushed += X;
     *               next16bits &= bitMask[flushed];
     *
     * I've streamlined the code a lot, so that we don't have to mask
     * out the low order bits and a few of the extra adds are removed.
     *    bsmith
     */
    unsigned int next16bits, index, flushed;

        show_bits16(next16bits);
        index = next16bits >> (16-5);
        if (index < 31) {
          size = dct_dc_size_luminance[index].value;
          flushed = dct_dc_size_luminance[index].num_bits;
        } else {
          index = next16bits >> (16-9);
          index -= 0x1f0;
          size = dct_dc_size_luminance1[index].value;
          flushed = dct_dc_size_luminance1[index].num_bits;
        }
        next16bits &= bitMask[16+flushed];

        if (size != 0) {
          flushed += size;
          diff = next16bits >> (16-flushed);
          if (!(diff & bitTest[32-size])) {
            diff = rBitMask[size] | (diff + 1);
          }
        } else {
          diff = 0;
        }
        flush_bits(flushed);

        if (n == 0) {
          coeff = diff << 3;
          if (vid_stream->mblock.mb_address -
              vid_stream->mblock.past_intra_addr > 1) {
            coeff += 1024;
          } else {
		    coeff += DCT_dc_y_past;
          }
          DCT_dc_y_past = coeff;
        } else {
          coeff = DCT_dc_y_past + (diff << 3);
          DCT_dc_y_past = coeff;
        }

      } else { /* n = 4 or 5 */

    /*
     * Get the chrominance bits.  This code has been hand optimized to
     * as described above
     */

    unsigned int next16bits, index, flushed;

        show_bits16(next16bits);
        index = next16bits >> (16-5);
        if (index < 31) {
          size = dct_dc_size_chrominance[index].value;
          flushed = dct_dc_size_chrominance[index].num_bits;
        } else {
          index = next16bits >> (16-10);
          index -= 0x3e0;
          size = dct_dc_size_chrominance1[index].value;
          flushed = dct_dc_size_chrominance1[index].num_bits;
        }
        next16bits &= bitMask[16+flushed];

        if (size != 0) {
          flushed += size;
          diff = next16bits >> (16-flushed);
          if (!(diff & bitTest[32-size])) {
            diff = rBitMask[size] | (diff + 1);
          }
        } else {
          diff = 0;
        }
        flush_bits(flushed);

      /* We test 5 first; a result of the mixup of Cr and Cb */
        if (n == 5) {
          coeff = diff << 3;

          if (vid_stream->mblock.mb_address -
              vid_stream->mblock.past_intra_addr > 1) {
            coeff += 1024;
          } else {
            coeff += DCT_dc_cr_past;
          }
          DCT_dc_cr_past = coeff;
        } else {
          coeff = diff << 3;
          if (vid_stream->mblock.mb_address -
              vid_stream->mblock.past_intra_addr > 1) {
            coeff += 1024;
          } else {
            coeff += DCT_dc_cb_past;
          }
          DCT_dc_cb_past = coeff;
        }
      }

      *reconptr = coeff;
      i = 0;
      pos = 0;
      coeffCount = (coeff != 0);

      if (vid_stream->picture.code_type != 4) {

        qscale = vid_stream->slice.quant_scale;
        iqmatrixptr = vid_stream->intra_quant_matrix[0];

        while(1) {

          DECODE_DCT_COEFF_NEXT(run, level);

          if (run >= END_OF_BLOCK) break;

          i = i + run + 1;
          pos = zigzag_direct[i];

          /* quantizes and oddifies each coefficient */
          if (level < 0) {
            coeff = ((level<<1) * qscale *
                     ((int) (iqmatrixptr[pos]))) / 16;
            coeff += (1 - (coeff & 1));
          } else {
            coeff = ((level<<1) * qscale *
                     ((int) (*(iqmatrixptr+pos)))) >> 4;
            coeff -= (1 - (coeff & 1));
          }
          reconptr[pos] = coeff;
          coeffCount++;

        }

        flush_bits(2);
		goto end;
      }
    } else { /* non-intra-coded macroblock */

      niqmatrixptr = vid_stream->non_intra_quant_matrix[0];
      qscale = vid_stream->slice.quant_scale;

      DECODE_DCT_COEFF_FIRST(run, level);
      i = run;

      pos = zigzag_direct[i];

        /* quantizes and oddifies each coefficient */
      if (level < 0) {
        coeff = (((level<<1) - 1) * qscale *
                 ((int) (niqmatrixptr[pos]))) / 16;
	if ((coeff & 1) == 0) {coeff = coeff + 1;}
      } else {
        coeff = (((level<<1) + 1) * qscale *
                 ((int) (*(niqmatrixptr+pos)))) >> 4;
	coeff = (coeff-1) | 1; /* equivalent to: if ((coeff&1)==0) coeff = coeff - 1; */
      }

      reconptr[pos] = coeff;
      if (coeff) {
          coeffCount = 1;
      }

      if (vid_stream->picture.code_type != 4) {

        while(1) {

          DECODE_DCT_COEFF_NEXT(run, level);

          if (run >= END_OF_BLOCK) {
	       	break;
          }

          i = i+run+1;
          pos = zigzag_direct[i];
          if (level < 0) {
            coeff = (((level<<1) - 1) * qscale *
                     ((int) (niqmatrixptr[pos]))) / 16;
            if ((coeff & 1) == 0) {coeff = coeff + 1;}
          } else {
            coeff = (((level<<1) + 1) * qscale *
                     ((int) (*(niqmatrixptr+pos)))) >> 4;
            coeff = (coeff-1) | 1; /* equivalent to: if ((coeff&1)==0) coeff = coeff - 1; */
          }
          reconptr[pos] = coeff;
          coeffCount++;
        } /* end while */

        flush_bits(2);

        goto end;
      } /* end if (vid_stream->picture.code_type != 4) */
    }

  end:

    if (coeffCount == 1) {
      j_rev_dct_sparse (reconptr, pos);
    }
    else {
#ifdef FLOATDCT
      if (qualityFlag) {
     	float_idct(reconptr);
      } else {
#endif
        j_rev_dct(reconptr);
#ifdef FLOATDCT
      }
#endif
    }

#ifdef RISC
    temp_curBits = vid_stream->curBits;
    temp_bitOffset = vid_stream->bit_offset;
    temp_bufLength = vid_stream->buf_length;
    temp_bitBuffer = bitBuffer;
#endif

  }

#ifdef RISC
  vid_stream->curBits = temp_curBits;
  vid_stream->bitOffset = temp_bitOffset;
  vid_stream->buf_length = temp_bufLength;
  bitBuffer = temp_bitBuffer;
#endif
}

#undef DCT_recon
#undef DCT_dc_y_past
#undef DCT_dc_cr_past
#undef DCT_dc_cb_past


/*
 *--------------------------------------------------------------
 *
 * ParseAwayBlock --
 *
 *    Parses off block values, throwing them away.
 *      Used with grayscale dithering.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

void
ParseAwayBlock(
     int n,
     mpeg_VidStream *vid_stream)
{
  unsigned int diff;
  unsigned int size, run;
  int level;

  if (vid_stream->buf_length < 100)
    correct_underflow(vid_stream);

  if (vid_stream->mblock.mb_intra) {

    /* If the block is a luminance block... */

    if (n < 4) {

      /* Parse and decode size of first coefficient. */

      DecodeDCTDCSizeLum(size);

      /* Parse first coefficient. */

      if (size != 0) {
        get_bitsn(size, diff);
      }
    }

    /* Otherwise, block is chrominance block... */

    else {

      /* Parse and decode size of first coefficient. */

      DecodeDCTDCSizeChrom(size);

      /* Parse first coefficient. */

      if (size != 0) {
        get_bitsn(size, diff);
      }
    }
  }

  /* Otherwise, block is not intracoded... */

  else {

    /* Decode and set first coefficient. */

    DECODE_DCT_COEFF_FIRST(run, level);
  }

  /* If picture is not D type (i.e. I, P, or B)... */

  if (vid_stream->picture.code_type != 4) {

    /* While end of macroblock has not been reached... */

    while (1) {

      /* Get the dct_coeff_next */

      DECODE_DCT_COEFF_NEXT(run, level);

      if (run >= END_OF_BLOCK) break;
    }

    /* End_of_block */

    flush_bits(2);
  }
}
/* Declarations of functions. */
static void ReconIMBlock(mpeg_VidStream *vid_stream,int bnum);
static void ReconPMBlock(mpeg_VidStream *vid_stream,int bnum,int recon_right_for,int recon_down_for,int zflag);
static void ReconBMBlock(mpeg_VidStream *vid_stream,int bnum,int recon_right_back,int recon_down_back,int zflag);
static void ReconBiMBlock( mpeg_VidStream *vid_stream,int bnum,int recon_right_for,int recon_down_for,int recon_right_back,int recon_down_back,int zflag);
static void ReconSkippedBlock( unsigned char *source,unsigned char *dest,int row,int col,int row_size,int right,int down,int right_half,int down_half,int width);
static void DoPictureDisplay(mpeg_VidStream *vid_stream);
static int ParseSeqHead( mpeg_VidStream *vid_stream );
static int ParseGOP( mpeg_VidStream *vid_stream );
static int ParsePicture( mpeg_VidStream *vid_stream, TimeStamp time_stamp);
static int ParseSlice(mpeg_VidStream *vid_stream);
static int ParseMacroBlock( mpeg_VidStream *vid_stream );
static void ProcessSkippedPFrameMBlocks(mpeg_VidStream *vid_stream);
static void ProcessSkippedBFrameMBlocks(mpeg_VidStream *vid_stream);

/* Macro for returning 1 if num is positive, -1 if negative, 0 if 0. */

#define Sign(num) ((num > 0) ? 1 : ((num == 0) ? 0 : -1))

/* Set up array for fast conversion from zig zag order to row/column
   coordinates.
*/

const int zigzag[64][2] = {
	{0, 0}, {1, 0}, {0, 1}, {0, 2}, {1, 1}, {2, 0}, {3, 0}, {2, 1},
	{1, 2}, {0, 3}, {0, 4}, {1, 3}, {2, 2}, {3, 1}, {4, 0}, {5, 0},
	{4, 1}, {3, 2}, {2, 3}, {1, 4}, {0, 5}, {0, 6}, {1, 5}, {2, 4},
	{3, 3}, {4, 2}, {5, 1}, {6, 0}, {7, 0}, {6, 1}, {5, 2}, {4, 3},
	{3, 4}, {2, 5}, {1, 6}, {0, 7}, {1, 7}, {2, 6}, {3, 5}, {4, 4},
	{5, 3}, {6, 2}, {7, 1}, {7, 2}, {6, 3}, {5, 4}, {4, 5}, {3, 6},
	{2, 7}, {3, 7}, {4, 6}, {5, 5}, {6, 4}, {7, 3}, {7, 4}, {6, 5},
	{5, 6}, {4, 7}, {5, 7}, {6, 6}, {7, 5}, {7, 6}, {6, 7}, {7, 7}};
/* Array mapping zigzag to array pointer offset. */

const int zigzag_direct[64] = {
	0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12,
	19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28, 35,
	42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};
/* Set up array for fast conversion from row/column coordinates to
   zig zag order.
*/

const int scan[8][8] = {
	{0, 1, 5, 6, 14, 15, 27, 28},
	{2, 4, 7, 13, 16, 26, 29, 42},
	{3, 8, 12, 17, 25, 30, 41, 43},
	{9, 11, 18, 24, 31, 40, 44, 53},
	{10, 19, 23, 32, 39, 45, 52, 54},
	{20, 22, 33, 38, 46, 51, 55, 60},
	{21, 34, 37, 47, 50, 56, 59, 61},
	{35, 36, 48, 49, 57, 58, 62, 63}};
/* Initialize P and B skip flags. */

static int No_P_Flag = FALSE;
static int No_B_Flag = FALSE;

/* Max lum, chrom indices for illegal block checking. */


/*
 * We use a lookup table to make sure values stay in the 0..255 range.
 * Since this is cropping (ie, x = (x < 0)?0:(x>255)?255:x; ), wee call this
 * table the "crop table".
 * MAX_NEG_CROP is the maximum neg/pos value we can handle.
 */

#define MAX_NEG_CROP 2048
#define NUM_CROP_ENTRIES (2048+2*MAX_NEG_CROP)
static unsigned char cropTbl[NUM_CROP_ENTRIES];



/*
 *--------------------------------------------------------------
 *
 * ReadSysClock --
 *
 *	Computes the current time according to the system clock.
 *
 * Results:
 *  The current time according to the system clock.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

double
ReadSysClock()
{
/* #ifdef WIN32 */
/*   return Time1970sec(); */
/* #else */
  struct timeval tv;
  (void) gettimeofday(&tv, (struct timezone *)NULL);
  return (tv.tv_sec + tv.tv_usec / 1000000.0);
/* #endif */
}


/*
 *--------------------------------------------------------------
 *
 * InitCrop --
 *
 *      Initializes cropTbl - this was taken from newmpeg_VidStream so
 *      that it wasn't done for each new video stream
 *
 * Results:
 *	None
 *
 * Side effects:
 *      cropTbl will be initialized
 *
 *--------------------------------------------------------------
 */
void
InitCrop()
{
  int i;

  /* Initialize crop table. */

  for (i = (-MAX_NEG_CROP); i < NUM_CROP_ENTRIES - MAX_NEG_CROP; i++) {
    if (i <= 0) {
      cropTbl[i + MAX_NEG_CROP] = 0;
#ifdef TWELVE_BITS
	} else if (i >= 2047) {
      cropTbl[i + MAX_NEG_CROP] = 2047;
#endif
    } else if (i >= 255) {
      cropTbl[i + MAX_NEG_CROP] = 255;
    } else {
      cropTbl[i + MAX_NEG_CROP] = i;
    }
  }

}



/*
 *--------------------------------------------------------------
 *
 * mpg_NewVidStream --
 *
 *	Allocates and initializes a mpeg_VidStream structure. Takes
 *      as parameter requested size for buffer length.
 *
 * Results:
 *	A pointer to the new mpeg_VidStream structure.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

mpeg_VidStream *
mpg_NewVidStream(
  unsigned int buffer_len)
{
  int i, j;
  mpeg_VidStream *newstream;
  static const unsigned char default_intra_matrix[64] = {
    8, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
  27, 29, 35, 38, 46, 56, 69, 83};

  /* Check for legal buffer length. */

  if (buffer_len < 4)
    return NULL;

  /* Make buffer length multiple of 4. */

  buffer_len = (buffer_len + 3) >> 2;

  /* Allocate memory for new structure. */

  newstream = (mpeg_VidStream *) MALLOC(sizeof(mpeg_VidStream));

  /* Initialize pointers to extension and user data. */

  newstream->group.ext_data = NULL;
  newstream->group.user_data = NULL;
  newstream->picture.extra_info = NULL;
  newstream->picture.user_data = NULL;
  newstream->picture.ext_data = NULL;
  newstream->slice.extra_info = NULL;
  newstream->ext_data = NULL;
  newstream->user_data = NULL;

  /* Copy default intra matrix. */

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      newstream->intra_quant_matrix[i][j] = default_intra_matrix[i * 8 + j];
    }
  }

  /* Initialize non intra quantization matrix. */

  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      newstream->non_intra_quant_matrix[i][j] = 16;
    }
  }

  /* Initialize pointers to image spaces. */

  newstream->current = newstream->past = newstream->future = NULL;
  for (i = 0; i < RING_BUF_SIZE; i++) {
    newstream->ring[i] = NULL;
  }

  /* Create buffer. */


  newstream->buf_start = (unsigned int *) MALLOC(buffer_len * 4);

  /*
   * Set max_buf_length to one less than actual length to deal with messy
   * data without proper seq. end codes.
   */

  newstream->max_buf_length = buffer_len - 1;

  /* Initialize bitstream i/o fields. */

  newstream->bit_offset = 0;
  newstream->buf_length = 0;
  newstream->buffer = newstream->buf_start;

  /* Initialize fields that used to be global */
  newstream->film_has_ended = FALSE;
  newstream->filename = NULL;
  newstream->EOF_flag = FALSE;

  /* Return structure. */

  return newstream;
}



/*
 *--------------------------------------------------------------
 *
 * Destroympeg_VidStream --
 *
 *	Deallocates a mpeg_VidStream structure.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
Destroympeg_VidStream(
  mpeg_VidStream *astream )
{
  int i;

  if (astream->ext_data != NULL)
    FREE_IF_NZ (astream->ext_data);

  if (astream->user_data != NULL)
    FREE_IF_NZ (astream->user_data);

  if (astream->group.ext_data != NULL)
    FREE_IF_NZ (astream->group.ext_data);

  if (astream->group.user_data != NULL)
    FREE_IF_NZ (astream->group.user_data);

  if (astream->picture.extra_info != NULL)
    FREE_IF_NZ (astream->picture.extra_info);

  if (astream->picture.ext_data != NULL)
    FREE_IF_NZ (astream->picture.ext_data);

  if (astream->picture.user_data != NULL)
    FREE_IF_NZ (astream->picture.user_data);

  if (astream->slice.extra_info != NULL)
    FREE_IF_NZ (astream->slice.extra_info);

  if (astream->buf_start != NULL)
    FREE_IF_NZ (astream->buf_start);

  for (i = 0; i < RING_BUF_SIZE; i++) {
    if (astream->ring[i] != NULL) {
      DestroyPictImage(astream->ring[i]);
      astream->ring[i] = NULL;
    }
  }

  FREE_IF_NZ (astream);

}




/*
 *--------------------------------------------------------------
 *
 * NewPictImage --
 *
 *	Allocates and initializes a PictImage structure.
 *      The width and height of the image space are passed in
 *      as parameters.
 *
 * Results:
 *	A pointer to the new PictImage structure.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

PictImage *
NewPictImage( mpeg_VidStream *vid_stream )
{
  PictImage *newimage;
  unsigned int width=vid_stream->mb_width * 16;
  unsigned int height=vid_stream->mb_height * 16;

  /* Allocate memory space for new structure. */

  newimage = (PictImage *) MALLOC(sizeof(PictImage));


  /* Allocate memory for image spaces. */

    {
    int temp_sz;
    int factor;
    temp_sz = vid_stream->matched_depth >> 3;
    if(!temp_sz) temp_sz = 1;
    if(temp_sz == 3) temp_sz = 4;
    factor = 1;

    newimage->display = (unsigned char *) MALLOC(width * height * temp_sz *
                                                            factor * factor);
    }
  newimage->luminance = (unsigned char *) MALLOC(width * height);
  newimage->Cr = (unsigned char *) MALLOC(width * height / 4);
  newimage->Cb = (unsigned char *) MALLOC(width * height / 4);

  /* Reset locked flag. */

  newimage->locked = 0;

  /* Return pointer to new structure. */

  return newimage;
}



/*
 *--------------------------------------------------------------
 *
 * DestroyPictImage --
 *
 *	Deallocates a PictImage structure.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
DestroyPictImage(
  PictImage *apictimage)
{
  if (apictimage->luminance != NULL) {
    FREE_IF_NZ (apictimage->luminance);
  }
  if (apictimage->Cr != NULL) {
    FREE_IF_NZ (apictimage->Cr);
  }
  if (apictimage->Cb != NULL) {
    FREE_IF_NZ (apictimage->Cb);
  }
  if (apictimage->display != NULL) {
    FREE_IF_NZ (apictimage->display);
  }
  FREE_IF_NZ (apictimage);
}


/*
 *--------------------------------------------------------------
 *
 * mpegVidRsrc --
 *
 *      Parses bit stream until MB_QUANTUM number of
 *      macroblocks have been decoded or current slice or
 *      picture ends, whichever comes first. If the start
 *      of a frame is encountered, the frame is time stamped
 *      with the value passed in time_stamp. If the value
 *      passed in buffer is not null, the video stream buffer
 *      is set to buffer and the length of the buffer is
 *      expected in value passed in through length. The current
 *      video stream is set to vid_stream. If vid_stream
 *      is passed as NULL, a new mpeg_VidStream structure is created
 *      and initialized and used as the current video stream.
 *
 * Results:
 *      A pointer to the video stream structure used.
 *
 * Side effects:
 *      Bit stream is irreversibly parsed. If a picture is completed,
 *      a function is called to display the frame at the correct time.
 *
 *--------------------------------------------------------------
 */

mpeg_VidStream *
mpegVidRsrc(
  TimeStamp  time_stamp,
  mpeg_VidStream *vid_stream,
  int first )
{
  unsigned int data;
  int i, status;

  /* If vid_stream is null, create new mpeg_VidStream structure. */

  if (vid_stream == NULL) {
    return NULL;
  }

  /*
   * If called for the first time, find start code, make sure it is a
   * sequence start code.
   */

  if (first) {
    vid_stream->sys_layer=-1;
    vid_stream->num_left=0;
    vid_stream->leftover_bytes=0;
    vid_stream->Parse_done=FALSE;

    next_start_code(vid_stream);  /* sets curBits */

    show_bits32(data);
    if (data != SEQ_START_CODE) {
      printf("This is not an MPEG video stream. (%x)\n",data);
      Destroympeg_VidStream(vid_stream);
      return NULL;
    }
  } else {
#ifdef UTIL2
    vid_stream->curBits = *vid_stream->buffer << vid_stream->bit_offset;
#else
    vid_stream->curBits = *vid_stream->buffer;
#endif
  }

  /* Get next 32 bits (size of start codes). */
  show_bits32(data);

  /*
   * Process according to start code (or parse macroblock if not a start code
   * at all).
   */

  switch (data) {

  case SEQ_END_CODE:
  case 0x000001b9:   /*  handle ISO_11172_END_CODE too */

    /* Display last frame. */

    if (vid_stream->future != NULL) {
      vid_stream->current = vid_stream->future;
    }

    vid_stream->film_has_ended=TRUE;
    Destroympeg_VidStream(vid_stream);
    vid_stream=NULL;
    goto done;
    break;

  case SEQ_START_CODE:
    /* Sequence start code. Parse sequence header. */

    if (ParseSeqHead(vid_stream) != PARSE_OK)
      goto error;
    goto done;

  case GOP_START_CODE:
    /* Group of Pictures start code. Parse gop header. */

    if (ParseGOP(vid_stream) != PARSE_OK)
      goto error;
    goto done;

  case PICTURE_START_CODE:
    /* Picture start code. Parse picture header and first slice header. */

    status = ParsePicture(vid_stream, time_stamp);

    if (status == SKIP_PICTURE) {
      next_start_code(vid_stream);
      while (!next_bits(32, PICTURE_START_CODE, vid_stream)) {
        if (next_bits(32, GOP_START_CODE, vid_stream))
          break;
        else if (next_bits(32, SEQ_END_CODE, vid_stream))
          break;
        flush_bits(24);
        next_start_code(vid_stream);
      }
      goto done;
    } else if (status != PARSE_OK)
      goto error;


    if (ParseSlice(vid_stream) != PARSE_OK)
      goto error;
    break;

  case SEQUENCE_ERROR_CODE:
    flush_bits32;
    next_start_code(vid_stream);
    goto done;

  default:

    /* Check for slice start code. */

    if ((data >= SLICE_MIN_START_CODE) && (data <= SLICE_MAX_START_CODE)) {

      /* Slice start code. Parse slice header. */

      if (ParseSlice(vid_stream) != PARSE_OK)
        goto error;
    }
    break;
  }

  /* Parse next MB_QUANTUM macroblocks. */

  for (i = 0; i < MB_QUANTUM; i++) {

    /* Check to see if actually a startcode and not a macroblock. */

    if (!next_bits(23, 0x00000000, vid_stream)) {

      /* Not start code. Parse Macroblock. */

      if (ParseMacroBlock(vid_stream) != PARSE_OK)
        goto error;

    } else {

      /* Not macroblock, actually start code. Get start code. */

      next_start_code(vid_stream);
      show_bits32(data);

      /*
       * If start code is outside range of slice start codes, frame is
       * complete, display frame.
       */

      if (((data < SLICE_MIN_START_CODE) || (data > SLICE_MAX_START_CODE)) &&
	  (data != SEQUENCE_ERROR_CODE)) {

	DoPictureDisplay(vid_stream);
      }
      goto done;
    }
  }

  /* Check if we just finished a picture on the MB_QUANTUMth macroblock */
  if (next_bits(23, 0x00000000, vid_stream)) {
    next_start_code(vid_stream);
    show_bits32(data);
    if ((data < SLICE_MIN_START_CODE) || (data > SLICE_MAX_START_CODE)) {
	DoPictureDisplay(vid_stream);
    }
  }

  /* Return pointer to video stream structure. */

  goto done;

error:
  vid_stream->film_has_ended=TRUE;
  Destroympeg_VidStream(vid_stream);
  goto done;

done:
  return vid_stream;
}




/*
 *--------------------------------------------------------------
 *
 * ParseSeqHead --
 *
 *      Assumes bit stream is at the begining of the sequence
 *      header start code. Parses off the sequence header.
 *
 * Results:
 *      Fills the vid_stream structure with values derived and
 *      decoded from the sequence header. Allocates the pict image
 *      structures based on the dimensions of the image space
 *      found in the sequence header.
 *
 * Side effects:
 *      Bit stream irreversibly parsed off.
 *
 *--------------------------------------------------------------
 */
static int
ParseSeqHead(
  mpeg_VidStream *vid_stream)
{
  unsigned int data;
  int i;

  /* Flush off sequence start code. */

  flush_bits32;

  /* Get horizontal size of image space. */

  get_bits12(data);
  vid_stream->h_size = data;

  /* Get vertical size of image space. */

  get_bits12(data);
  vid_stream->v_size = data;

  /* Calculate macroblock width and height of image space. */

  vid_stream->mb_width = (vid_stream->h_size + 15) / 16;
  vid_stream->mb_height = (vid_stream->v_size + 15) / 16;

  /*
   * Initialize ring buffer of pict images now that dimensions of image space
   * are known.
   */

  if (vid_stream->ring[0] == NULL) {
    for (i = 0; i < RING_BUF_SIZE; i++) {
      vid_stream->ring[i] = NewPictImage(vid_stream);
    }
  }

  /* Parse of aspect ratio code. */

  get_bits4(data);
  vid_stream->aspect_ratio = (unsigned char) data;

  /* Parse off picture rate code. */

  get_bits4(data);
  vid_stream->picture_rate = (unsigned char) data;

  /* Parse off bit rate. */

  get_bits18(data);
  vid_stream->bit_rate = data;

  /* Flush marker bit. */

  flush_bits(1);

  /* Parse off vbv buffer size. */

  get_bits10(data);
  vid_stream->vbv_buffer_size = data;

  /* Parse off contrained parameter flag. */

  get_bits1(data);
  if (data) {
    vid_stream->const_param_flag = TRUE;
  } else
    vid_stream->const_param_flag = FALSE;

  /*
   * If intra_quant_matrix_flag set, parse off intra quant matrix values.
   */

  get_bits1(data);
  if (data) {
    for (i = 0; i < 64; i++) {
      get_bits8(data);

      vid_stream->intra_quant_matrix[zigzag[i][1]][zigzag[i][0]] =
	(unsigned char) data;
    }
  }
  /*
   * If non intra quant matrix flag set, parse off non intra quant matrix
   * values.
   */

  get_bits1(data);
  if (data) {
    for (i = 0; i < 64; i++) {
      get_bits8(data);

      vid_stream->non_intra_quant_matrix[zigzag[i][1]][zigzag[i][0]] =
	(unsigned char) data;
    }
  }
  /* Go to next start code. */

  next_start_code(vid_stream);

  /*
   * If next start code is extension start code, parse off extension data.
   */

  if (next_bits(32, EXT_START_CODE, vid_stream)) {
    flush_bits32;
    if (vid_stream->ext_data != NULL) {
      FREE_IF_NZ (vid_stream->ext_data);
      vid_stream->ext_data = NULL;
    }
    vid_stream->ext_data = get_ext_data(vid_stream);
  }
  /* If next start code is user start code, parse off user data. */

  if (next_bits(32, USER_START_CODE, vid_stream)) {
    flush_bits32;
    if (vid_stream->user_data != NULL) {
      FREE_IF_NZ (vid_stream->user_data);
      vid_stream->user_data = NULL;
    }
    vid_stream->user_data = get_ext_data(vid_stream);
  }
  return PARSE_OK;
}



/*
 *--------------------------------------------------------------
 *
 * ParseGOP --
 *
 *      Parses of group of pictures header from bit stream
 *      associated with vid_stream.
 *
 * Results:
 *      Values in gop header placed into video stream structure.
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

static int
ParseGOP(
  mpeg_VidStream *vid_stream)
{
  unsigned int data;

  /* Flush group of pictures start code. */

  flush_bits32;

  /* Parse off drop frame flag. */

  get_bits1(data);
  if (data) {
    vid_stream->group.drop_flag = TRUE;
  } else
    vid_stream->group.drop_flag = FALSE;

  /* Parse off hour component of time code. */

  get_bits5(data);
  vid_stream->group.tc_hours = data;

  /* Parse off minute component of time code. */

  get_bits6(data);
  vid_stream->group.tc_minutes = data;

  /* Flush marker bit. */

  flush_bits(1);

  /* Parse off second component of time code. */

  get_bits6(data);
  vid_stream->group.tc_seconds = data;

  /* Parse off picture count component of time code. */

  get_bits6(data);
  vid_stream->group.tc_pictures = data;

  /* Parse off closed gop and broken link flags. */

  get_bits2(data);
  if (data > 1) {
    vid_stream->group.closed_gop = TRUE;
    if (data > 2) {
      vid_stream->group.broken_link = TRUE;
    } else
      vid_stream->group.broken_link = FALSE;
  } else {
    vid_stream->group.closed_gop = FALSE;
    if (data) {
      vid_stream->group.broken_link = TRUE;
    } else
      vid_stream->group.broken_link = FALSE;
  }

  /* Goto next start code. */

  next_start_code(vid_stream);

  /* If next start code is extension data, parse off extension data. */

  if (next_bits(32, EXT_START_CODE, vid_stream)) {
    flush_bits32;
    if (vid_stream->group.ext_data != NULL) {
      FREE_IF_NZ (vid_stream->group.ext_data);
      vid_stream->group.ext_data = NULL;
    }
    vid_stream->group.ext_data = get_ext_data(vid_stream);
  }
  /* If next start code is user data, parse off user data. */

  if (next_bits(32, USER_START_CODE,vid_stream)) {
    flush_bits32;
    if (vid_stream->group.user_data != NULL) {
      FREE_IF_NZ (vid_stream->group.user_data);
      vid_stream->group.user_data = NULL;
    }
    vid_stream->group.user_data = get_ext_data(vid_stream);
  }
  return PARSE_OK;
}



/*
 *--------------------------------------------------------------
 *
 * ParsePicture --
 *
 *      Parses picture header. Marks picture to be presented
 *      at particular time given a time stamp.
 *
 * Results:
 *      Values from picture header put into video stream structure.
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

static int
ParsePicture(
  mpeg_VidStream *vid_stream,
  TimeStamp time_stamp)
{
  unsigned int data;
  int i;

  /* Flush header start code. */
  flush_bits32;

  /* Parse off temporal reference. */
  get_bits10(data);
  vid_stream->picture.temp_ref = data;

  /* Parse of picture type. */
  get_bits3(data);
  vid_stream->picture.code_type = data;

  if ((vid_stream->picture.code_type == B_TYPE) &&
      (/* No_B_Flag || */
       (vid_stream->future == NULL) ||
       ((vid_stream->past == NULL) && !(vid_stream->group.closed_gop))))
    /* According to 2-D.5.1 (p D-18) this is ok, if the refereneces are OK */
    return SKIP_PICTURE;

  if ((vid_stream->picture.code_type == P_TYPE) &&
      (No_P_Flag || (vid_stream->future == NULL)))
    return SKIP_PICTURE;

  /* Parse off vbv buffer delay value. */
  get_bits16(data);
  vid_stream->picture.vbv_delay = data;

  /* If P or B type frame... */

  if ((vid_stream->picture.code_type == P_TYPE) ||
      (vid_stream->picture.code_type == B_TYPE)) {

    /* Parse off forward vector full pixel flag. */
    get_bits1(data);
    if (data) {
      vid_stream->picture.full_pel_forw_vector = TRUE;
    } else {
      vid_stream->picture.full_pel_forw_vector = FALSE;
    }

    /* Parse of forw_r_code. */
    get_bits3(data);

    /* Decode forw_r_code into forw_r_size and forw_f. */

    vid_stream->picture.forw_r_size = data - 1;
    vid_stream->picture.forw_f = (1 << vid_stream->picture.forw_r_size);
  }
  /* If B type frame... */

  if (vid_stream->picture.code_type == B_TYPE) {

    /* Parse off back vector full pixel flag. */
    get_bits1(data);
    if (data)
      vid_stream->picture.full_pel_back_vector = TRUE;
    else
      vid_stream->picture.full_pel_back_vector = FALSE;

    /* Parse off back_r_code. */
    get_bits3(data);

    /* Decode back_r_code into back_r_size and back_f. */

    vid_stream->picture.back_r_size = data - 1;
    vid_stream->picture.back_f = (1 << vid_stream->picture.back_r_size);
  }
  /* Get extra bit picture info. */

  if (vid_stream->picture.extra_info != NULL) {
    FREE_IF_NZ (vid_stream->picture.extra_info);
    vid_stream->picture.extra_info = NULL;
  }
  vid_stream->picture.extra_info = get_extra_bit_info(vid_stream);

  /* Goto next start code. */
  next_start_code(vid_stream);

  /* If start code is extension start code, parse off extension data. */

  if (next_bits(32, EXT_START_CODE, vid_stream)) {
    flush_bits32;

    if (vid_stream->picture.ext_data != NULL) {
      FREE_IF_NZ (vid_stream->picture.ext_data);
      vid_stream->picture.ext_data = NULL;
    }
    vid_stream->picture.ext_data = get_ext_data(vid_stream);
  }
  /* If start code is user start code, parse off user data. */

  if (next_bits(32, USER_START_CODE, vid_stream)) {
    flush_bits32;

    if (vid_stream->picture.user_data != NULL) {
      FREE_IF_NZ (vid_stream->picture.user_data);
      vid_stream->picture.user_data = NULL;
    }
    vid_stream->picture.user_data = get_ext_data(vid_stream);
  }
  /* Find a pict image structure in ring buffer not currently locked. */

  i = 0;

  while (vid_stream->ring[i]->locked != 0) {
    if (++i >= RING_BUF_SIZE) {
      perror("Fatal error. Ring buffer full.");
    }
  }

  /* Set current pict image structure to the one just found in ring. */

  vid_stream->current = vid_stream->ring[i];

  /* Set time stamp. */

  vid_stream->current->show_time = time_stamp;

  /* Reset past macroblock address field. */

  vid_stream->mblock.past_mb_addr = -1;

  return PARSE_OK;
}



/*
 *--------------------------------------------------------------
 *
 * ParseSlice --
 *
 *      Parses off slice header.
 *
 * Results:
 *      Values found in slice header put into video stream structure.
 *
 * Side effects:
 *      Bit stream irreversibly parsed.
 *
 *--------------------------------------------------------------
 */

static int
ParseSlice(
  mpeg_VidStream *vid_stream)
{
  unsigned int data;

  /* Flush slice start code. */

  flush_bits(24);

  /* Parse off slice vertical position. */

  get_bits8(data);
  vid_stream->slice.vert_pos = data;

  /* Parse off quantization scale. */

  get_bits5(data);
  vid_stream->slice.quant_scale = data;

  /* Parse off extra bit slice info. */

  if (vid_stream->slice.extra_info != NULL) {
    FREE_IF_NZ (vid_stream->slice.extra_info);
    vid_stream->slice.extra_info = NULL;
  }
  vid_stream->slice.extra_info = get_extra_bit_info(vid_stream);

  /* Reset past intrablock address. */

  vid_stream->mblock.past_intra_addr = -2;

  /* Reset previous recon motion vectors. */

  vid_stream->mblock.recon_right_for_prev = 0;
  vid_stream->mblock.recon_down_for_prev = 0;
  vid_stream->mblock.recon_right_back_prev = 0;
  vid_stream->mblock.recon_down_back_prev = 0;

  /* Reset macroblock address. */

  vid_stream->mblock.mb_address = ((vid_stream->slice.vert_pos - 1) *
				   vid_stream->mb_width) - 1;

  /* Reset past dct dc y, cr, and cb values. */

  vid_stream->block.dct_dc_y_past = 1024 << 3;
  vid_stream->block.dct_dc_cr_past = 1024 << 3;
  vid_stream->block.dct_dc_cb_past = 1024 << 3;

  return PARSE_OK;
}



/*
 *--------------------------------------------------------------
 *
 * ParseMacroBlock --
 *
 *      Parseoff macroblock. Reconstructs DCT values. Applies
 *      inverse DCT, reconstructs motion vectors, calculates and
 *      set pixel values for macroblock in current pict image
 *      structure.
 *
 * Results:
 *      Here's where everything really happens. Welcome to the
 *      heart of darkness.
 *
 * Side effects:
 *      Bit stream irreversibly parsed off.
 *
 *--------------------------------------------------------------
 */

static int
ParseMacroBlock(
  mpeg_VidStream *vid_stream)
{
  int addr_incr;
  unsigned int data;
  int mask, i, recon_right_for, recon_down_for, recon_right_back,
      recon_down_back;
  int zero_block_flag;
  int mb_quant = 0, mb_motion_forw = 0, mb_motion_back = 0,
      mb_pattern = 0;

  /*
   * Parse off macroblock address increment and add to macroblock address.
   */
  do {
    unsigned int ind;
    show_bits11(ind);
    DecodeMBAddrInc(addr_incr);
    if (mb_addr_inc[ind].num_bits==0) {
      addr_incr = 1;
    }
    if (addr_incr == MB_ESCAPE) {
      vid_stream->mblock.mb_address += 33;
      addr_incr = MB_STUFFING;
    }
  } while (addr_incr == MB_STUFFING);
  vid_stream->mblock.mb_address += addr_incr;

  if (vid_stream->mblock.mb_address > (vid_stream->mb_height *
				       vid_stream->mb_width - 1))
    return SKIP_TO_START_CODE;

  /*
   * If macroblocks have been skipped, process skipped macroblocks.
   */

  if (vid_stream->mblock.mb_address - vid_stream->mblock.past_mb_addr > 1) {
    if (vid_stream->picture.code_type == P_TYPE)
      ProcessSkippedPFrameMBlocks(vid_stream);
    else if (vid_stream->picture.code_type == B_TYPE)
      ProcessSkippedBFrameMBlocks(vid_stream);
  }
  /* Set past macroblock address to current macroblock address. */
  vid_stream->mblock.past_mb_addr = vid_stream->mblock.mb_address;

  /* Based on picture type decode macroblock type. */
  switch (vid_stream->picture.code_type) {
  case I_TYPE:
    DecodeMBTypeI(mb_quant, mb_motion_forw, mb_motion_back, mb_pattern,
		  vid_stream->mblock.mb_intra);
    break;

  case P_TYPE:
    DecodeMBTypeP(mb_quant, mb_motion_forw, mb_motion_back, mb_pattern,
		  vid_stream->mblock.mb_intra);
    break;

  case B_TYPE:
    DecodeMBTypeB(mb_quant, mb_motion_forw, mb_motion_back, mb_pattern,
		  vid_stream->mblock.mb_intra);
    break;
  case D_TYPE:
    ConsoleMessage("ERROR:  MPEG-1 Streams with D-frames are not supported\n");
	exit(1);
  }

  /* If quantization flag set, parse off new quantization scale. */

  if (mb_quant == TRUE) {
    get_bits5(data);
    vid_stream->slice.quant_scale = data;
  }
  /* If forward motion vectors exist... */
  if (mb_motion_forw == TRUE) {

    /* Parse off and decode horizontal forward motion vector. */
    DecodeMotionVectors(vid_stream->mblock.motion_h_forw_code);

    /* If horiz. forward r data exists, parse off. */

    if ((vid_stream->picture.forw_f != 1) &&
	(vid_stream->mblock.motion_h_forw_code != 0)) {
      get_bitsn(vid_stream->picture.forw_r_size, data);
      vid_stream->mblock.motion_h_forw_r = data;
    }
    /* Parse off and decode vertical forward motion vector. */
    DecodeMotionVectors(vid_stream->mblock.motion_v_forw_code);

    /* If vert. forw. r data exists, parse off. */

    if ((vid_stream->picture.forw_f != 1) &&
	(vid_stream->mblock.motion_v_forw_code != 0)) {
      get_bitsn(vid_stream->picture.forw_r_size, data);
      vid_stream->mblock.motion_v_forw_r = data;
    }
  }
  /* If back motion vectors exist... */
  if (mb_motion_back == TRUE) {

    /* Parse off and decode horiz. back motion vector. */
    DecodeMotionVectors(vid_stream->mblock.motion_h_back_code);

    /* If horiz. back r data exists, parse off. */

    if ((vid_stream->picture.back_f != 1) &&
	(vid_stream->mblock.motion_h_back_code != 0)) {
      get_bitsn(vid_stream->picture.back_r_size, data);
      vid_stream->mblock.motion_h_back_r = data;
    }
    /* Parse off and decode vert. back motion vector. */
    DecodeMotionVectors(vid_stream->mblock.motion_v_back_code);

    /* If vert. back r data exists, parse off. */

    if ((vid_stream->picture.back_f != 1) &&
	(vid_stream->mblock.motion_v_back_code != 0)) {
      get_bitsn(vid_stream->picture.back_r_size, data);
      vid_stream->mblock.motion_v_back_r = data;
    }
  }

  /* If mblock pattern flag set, parse and decode CBP (code block pattern). */
  if (mb_pattern == TRUE) {
    DecodeCBP(vid_stream->mblock.cbp);
  }
  /* Otherwise, set CBP to zero. */
  else
    vid_stream->mblock.cbp = 0;


  /* Reconstruct motion vectors depending on picture type. */
  if (vid_stream->picture.code_type == P_TYPE) {

    /*
     * If no forw motion vectors, reset previous and current vectors to 0.
     */

    if (!mb_motion_forw) {
      recon_right_for = 0;
      recon_down_for = 0;
      vid_stream->mblock.recon_right_for_prev = 0;
      vid_stream->mblock.recon_down_for_prev = 0;
    }
    /*
     * Otherwise, compute new forw motion vectors. Reset previous vectors to
     * current vectors.
     */

    else {
      ComputeForwVector(&recon_right_for, &recon_down_for, vid_stream);
    }
  }
  if (vid_stream->picture.code_type == B_TYPE) {

    /* Reset prev. and current vectors to zero if mblock is intracoded. */

    if (vid_stream->mblock.mb_intra) {
      vid_stream->mblock.recon_right_for_prev = 0;
      vid_stream->mblock.recon_down_for_prev = 0;
      vid_stream->mblock.recon_right_back_prev = 0;
      vid_stream->mblock.recon_down_back_prev = 0;
    } else {

      /* If no forw vectors, current vectors equal prev. vectors. */

      if (!mb_motion_forw) {
	recon_right_for = vid_stream->mblock.recon_right_for_prev;
	recon_down_for = vid_stream->mblock.recon_down_for_prev;
      }
      /*
       * Otherwise compute forw. vectors. Reset prev vectors to new values.
       */

      else {
	ComputeForwVector(&recon_right_for, &recon_down_for, vid_stream);
      }

      /* If no back vectors, set back vectors to prev back vectors. */

      if (!mb_motion_back) {
        recon_right_back = vid_stream->mblock.recon_right_back_prev;
        recon_down_back = vid_stream->mblock.recon_down_back_prev;
      }
      /* Otherwise compute new vectors and reset prev. back vectors. */

      else {
        ComputeBackVector(&recon_right_back, &recon_down_back, vid_stream);
      }

      /*
       * Store vector existence flags in structure for possible skipped
       * macroblocks to follow.
       */

      vid_stream->mblock.bpict_past_forw = mb_motion_forw;
      vid_stream->mblock.bpict_past_back = mb_motion_back;
    }
  }

      for (mask = 32, i = 0; i < 6; mask >>= 1, i++) {

	/* If block exists... */
	if ((vid_stream->mblock.mb_intra) || (vid_stream->mblock.cbp & mask)) {
	  zero_block_flag = 0;
	  ParseReconBlock(i, vid_stream);
	} else {
	  zero_block_flag = 1;
	}

	/* If macroblock is intra coded... */
	if (vid_stream->mblock.mb_intra) {
	  ReconIMBlock(vid_stream, i);
	} else if (mb_motion_forw && mb_motion_back) {
	  ReconBiMBlock(vid_stream, i, recon_right_for, recon_down_for,
			recon_right_back, recon_down_back, zero_block_flag);
	} else if (mb_motion_forw || (vid_stream->picture.code_type == P_TYPE)) {
	  ReconPMBlock(vid_stream, i, recon_right_for, recon_down_for,
		       zero_block_flag);
	} else if (mb_motion_back) {
	  ReconBMBlock(vid_stream, i, recon_right_back, recon_down_back,
		       zero_block_flag);
	}
      }

  /* If D Type picture, flush marker bit. */
  if (vid_stream->picture.code_type == 4)
    flush_bits(1);

  /* If macroblock was intracoded, set macroblock past intra address. */
  if (vid_stream->mblock.mb_intra)
    vid_stream->mblock.past_intra_addr =
      vid_stream->mblock.mb_address;

  return PARSE_OK;
}



/*
 *--------------------------------------------------------------
 *
 * ReconIMBlock --
 *
 *	Reconstructs intra coded macroblock.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

#if defined(FW_MPEG_DEBUG)
/* If people really want to see such things, check 'em */
/* #define myassert(x,expression)\ */
/*   if (!(expression)) {\ */
/*   fprintf (stderr,"Bad crop value (%d) at line %d\n", x, __LINE__);\ */
/*   next_start_code(vid_stream); return;} */
#define assertCrop(x)	ASSERT(((x) >= -MAX_NEG_CROP) && \
			       ((x) <= 2048+MAX_NEG_CROP))
#else
#define assertCrop(x)
#endif

static void
ReconIMBlock(
  mpeg_VidStream *vid_stream,
  int bnum)
{
  int mb_row, mb_col, row, col, row_size, rr;
  unsigned char *dest;

  /* Calculate macroblock row and column from address. */

  mb_row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  mb_col = vid_stream->mblock.mb_address % vid_stream->mb_width;


  /* If block is luminance block... */

  if (bnum < 4) {

    /* Calculate row and col values for upper left pixel of block. */

    row = mb_row * 16;
    col = mb_col * 16;
    if (bnum > 1)
      row += 8;
    if (bnum % 2)
      col += 8;

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    /* Establish row size. */

    row_size = vid_stream->mb_width * 16;
  }
  /* Otherwise if block is Cr block... */
  /* Cr first because of the earlier mixup */

  else if (bnum == 5) {

    /* Set dest to Cr plane of current pict image. */

    dest = vid_stream->current->Cr;

    /* Establish row size. */

    row_size = vid_stream->mb_width * 8;

    /* Calculate row,col for upper left pixel of block. */

    row = mb_row * 8;
    col = mb_col * 8;
  }
  /* Otherwise block is Cb block, and ... */

  else {

    /* Set dest to Cb plane of current pict image. */

    dest = vid_stream->current->Cb;

    /* Establish row size. */

    row_size = vid_stream->mb_width * 8;

    /* Calculate row,col for upper left pixel value of block. */

    row = mb_row * 8;
    col = mb_col * 8;
  }

  /*
   * For each pixel in block, set to cropped reconstructed value from inverse
   * dct.
   */
  {
    short *sp = &vid_stream->block.dct_recon[0][0];
    unsigned char *cm = cropTbl + MAX_NEG_CROP;
    dest += row * row_size + col;
    for (rr = 0; rr < 4; rr++, sp += 16, dest += row_size) {
      dest[0] = cm[sp[0]];
	  assertCrop(sp[0]);
      dest[1] = cm[sp[1]];
      assertCrop(sp[1]);
      dest[2] = cm[sp[2]];
      assertCrop(sp[2]);
      dest[3] = cm[sp[3]];
      assertCrop(sp[3]);
      dest[4] = cm[sp[4]];
      assertCrop(sp[4]);
      dest[5] = cm[sp[5]];
      assertCrop(sp[5]);
      dest[6] = cm[sp[6]];
      assertCrop(sp[6]);
      dest[7] = cm[sp[7]];
      assertCrop(sp[7]);

      dest += row_size;
      dest[0] = cm[sp[8]];
      assertCrop(sp[8]);
      dest[1] = cm[sp[9]];
      assertCrop(sp[9]);
      dest[2] = cm[sp[10]];
      assertCrop(sp[10]);
      dest[3] = cm[sp[11]];
      assertCrop(sp[11]);
      dest[4] = cm[sp[12]];
      assertCrop(sp[12]);
      dest[5] = cm[sp[13]];
      assertCrop(sp[13]);
      dest[6] = cm[sp[14]];
      assertCrop(sp[14]);
      dest[7] = cm[sp[15]];
      assertCrop(sp[15]);
    }
  }
}



/*
 *--------------------------------------------------------------
 *
 * ReconPMBlock --
 *
 *	Reconstructs forward predicted macroblocks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void
ReconPMBlock(
  mpeg_VidStream *vid_stream,
  int bnum,
  int recon_right_for,
  int recon_down_for,
  int zflag)
{
  int mb_row, mb_col, row, col, row_size, rr;
  unsigned char *dest, *past;
  unsigned char *rindex1, *rindex2;
  unsigned char *index;
  short int *blockvals;

  /* Calculate macroblock row and column from address. */

  mb_row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  mb_col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  if (bnum < 4) {

    /* Calculate right_for, down_for motion vectors. */

    vid_stream->right_for = recon_right_for >> 1;
    vid_stream->down_for = recon_down_for >> 1;
    vid_stream->right_half_for = recon_right_for & 0x1;
    vid_stream->down_half_for = recon_down_for & 0x1;

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    if (vid_stream->picture.code_type == B_TYPE) {
      if (vid_stream->past != NULL)
       past = vid_stream->past->luminance;
    } else {

      /* Set predictive frame to current future frame. */

      if (vid_stream->future != NULL)
        past = vid_stream->future->luminance;
    }

    /* Establish row size. */

    row_size = vid_stream->mb_width << 4;

    /* Calculate row,col of upper left pixel in block. */

    row = mb_row << 4;
    col = mb_col << 4;
    if (bnum > 1)
      row += 8;
    if (bnum % 2)
      col += 8;
  }
  /* Otherwise, block is NOT luminance block, ... */

  else {

    /* Construct motion vectors. */

    recon_right_for /= 2;
    recon_down_for /= 2;
    vid_stream->right_for = recon_right_for >> 1;
    vid_stream->down_for = recon_down_for >> 1;
    vid_stream->right_half_for = recon_right_for & 0x1;
    vid_stream->down_half_for = recon_down_for & 0x1;

    /* Establish row size. */

    row_size = vid_stream->mb_width << 3;

    /* Calculate row,col of upper left pixel in block. */

    row = mb_row << 3;
    col = mb_col << 3;

    /* If block is Cr block... */
    /* 5 first because order was mixed up in earlier versions */

    if (bnum == 5) {

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

      if (vid_stream->picture.code_type == B_TYPE) {

    if (vid_stream->past != NULL)
      past = vid_stream->past->Cr;
      } else {
    if (vid_stream->future != NULL)
      past = vid_stream->future->Cr;
      }
    }
    /* Otherwise, block is Cb block... */

    else {

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;

      if (vid_stream->picture.code_type == B_TYPE) {
        if (vid_stream->past != NULL)
          past = vid_stream->past->Cb;
      } else {
        if (vid_stream->future != NULL)
          past = vid_stream->future->Cb;
      }
    }
  }

  /* For each pixel in block... */
    index = dest + (row * row_size) + col;
    rindex1 = past + (row + vid_stream->down_for) * row_size
	      + col + vid_stream->right_for;

    blockvals = &(vid_stream->block.dct_recon[0][0]);

    /*
     * Calculate predictive pixel value based on motion vectors and copy to
     * dest plane.
     */

    if ((!vid_stream->down_half_for) && (!vid_stream->right_half_for)) {
      unsigned char *cm = cropTbl + MAX_NEG_CROP;
      if (!zflag)
        for (rr = 0; rr < 4; rr++) {
          index[0] = cm[(int) rindex1[0] + (int) blockvals[0]];
          index[1] = cm[(int) rindex1[1] + (int) blockvals[1]];
          index[2] = cm[(int) rindex1[2] + (int) blockvals[2]];
          index[3] = cm[(int) rindex1[3] + (int) blockvals[3]];
          index[4] = cm[(int) rindex1[4] + (int) blockvals[4]];
          index[5] = cm[(int) rindex1[5] + (int) blockvals[5]];
          index[6] = cm[(int) rindex1[6] + (int) blockvals[6]];
          index[7] = cm[(int) rindex1[7] + (int) blockvals[7]];
          index += row_size;
          rindex1 += row_size;

          index[0] = cm[(int) rindex1[0] + (int) blockvals[8]];
          index[1] = cm[(int) rindex1[1] + (int) blockvals[9]];
          index[2] = cm[(int) rindex1[2] + (int) blockvals[10]];
          index[3] = cm[(int) rindex1[3] + (int) blockvals[11]];
          index[4] = cm[(int) rindex1[4] + (int) blockvals[12]];
          index[5] = cm[(int) rindex1[5] + (int) blockvals[13]];
          index[6] = cm[(int) rindex1[6] + (int) blockvals[14]];
          index[7] = cm[(int) rindex1[7] + (int) blockvals[15]];
          blockvals += 16;
          index += row_size;
          rindex1 += row_size;
        }
      else {
        if (vid_stream->right_for & 0x1) {
          /* No alignment, use bye copy */
          for (rr = 0; rr < 4; rr++) {
            index[0] = rindex1[0];
            index[1] = rindex1[1];
            index[2] = rindex1[2];
            index[3] = rindex1[3];
            index[4] = rindex1[4];
            index[5] = rindex1[5];
            index[6] = rindex1[6];
            index[7] = rindex1[7];
            index += row_size;
            rindex1 += row_size;

            index[0] = rindex1[0];
            index[1] = rindex1[1];
            index[2] = rindex1[2];
            index[3] = rindex1[3];
            index[4] = rindex1[4];
            index[5] = rindex1[5];
            index[6] = rindex1[6];
            index[7] = rindex1[7];
            index += row_size;
            rindex1 += row_size;
          }
        } else if (vid_stream->right_for & 0x2) {
          /* Half-word bit aligned, use 16 bit copy */
          short *src = (short *)rindex1;
          short *dest = (short *)index;
          row_size >>= 1;
          for (rr = 0; rr < 4; rr++) {
            dest[0] = src[0];
            dest[1] = src[1];
            dest[2] = src[2];
            dest[3] = src[3];
            dest += row_size;
            src += row_size;

            dest[0] = src[0];
            dest[1] = src[1];
            dest[2] = src[2];
            dest[3] = src[3];
            dest += row_size;
            src += row_size;
          }
        } else {
          /* Word aligned, use 32 bit copy */
          int *src = (int *)rindex1;
          int *dest = (int *)index;
          row_size >>= 2;
          for (rr = 0; rr < 4; rr++) {
            dest[0] = src[0];
            dest[1] = src[1];
            dest += row_size;
            src += row_size;

            dest[0] = src[0];
            dest[1] = src[1];
            dest += row_size;
            src += row_size;
          }
        }
      }
    } else {
      unsigned char *cm = cropTbl + MAX_NEG_CROP;
      rindex2 = rindex1 + vid_stream->right_half_for
		+ (vid_stream->down_half_for * row_size);

      /* if one of the two is zero, then quality makes no difference */

        if (!zflag) {
          for (rr = 0; rr < 4; rr++) {
            index[0] = cm[((int) (rindex1[0] + rindex2[0] + 1) >> 1) + blockvals[0]];
            index[1] = cm[((int) (rindex1[1] + rindex2[1] + 1) >> 1) + blockvals[1]];
            index[2] = cm[((int) (rindex1[2] + rindex2[2] + 1) >> 1) + blockvals[2]];
            index[3] = cm[((int) (rindex1[3] + rindex2[3] + 1) >> 1) + blockvals[3]];
            index[4] = cm[((int) (rindex1[4] + rindex2[4] + 1) >> 1) + blockvals[4]];
            index[5] = cm[((int) (rindex1[5] + rindex2[5] + 1) >> 1) + blockvals[5]];
            index[6] = cm[((int) (rindex1[6] + rindex2[6] + 1) >> 1) + blockvals[6]];
            index[7] = cm[((int) (rindex1[7] + rindex2[7] + 1) >> 1) + blockvals[7]];
            index += row_size;
            rindex1 += row_size;
            rindex2 += row_size;

            index[0] = cm[((int) (rindex1[0] + rindex2[0] + 1) >> 1) + blockvals[8]];
            index[1] = cm[((int) (rindex1[1] + rindex2[1] + 1) >> 1) + blockvals[9]];
            index[2] = cm[((int) (rindex1[2] + rindex2[2] + 1) >> 1) + blockvals[10]];
            index[3] = cm[((int) (rindex1[3] + rindex2[3] + 1) >> 1) + blockvals[11]];
            index[4] = cm[((int) (rindex1[4] + rindex2[4] + 1) >> 1) + blockvals[12]];
            index[5] = cm[((int) (rindex1[5] + rindex2[5] + 1) >> 1) + blockvals[13]];
            index[6] = cm[((int) (rindex1[6] + rindex2[6] + 1) >> 1) + blockvals[14]];
            index[7] = cm[((int) (rindex1[7] + rindex2[7] + 1) >> 1) + blockvals[15]];
            blockvals += 16;
            index += row_size;
            rindex1 += row_size;
            rindex2 += row_size;
          }
        } else { /* zflag */
          for (rr = 0; rr < 8; rr++) {
            index[0] = (int) (rindex1[0] + rindex2[0] + 1) >> 1;
            index[1] = (int) (rindex1[1] + rindex2[1] + 1) >> 1;
            index[2] = (int) (rindex1[2] + rindex2[2] + 1) >> 1;
            index[3] = (int) (rindex1[3] + rindex2[3] + 1) >> 1;
            index[4] = (int) (rindex1[4] + rindex2[4] + 1) >> 1;
            index[5] = (int) (rindex1[5] + rindex2[5] + 1) >> 1;
            index[6] = (int) (rindex1[6] + rindex2[6] + 1) >> 1;
            index[7] = (int) (rindex1[7] + rindex2[7] + 1) >> 1;
            index += row_size;
            rindex1 += row_size;
            rindex2 += row_size;
          }
        } /* zflag */

    }
}


/*
 *--------------------------------------------------------------
 *
 * ReconBMBlock --
 *
 *	Reconstructs back predicted macroblocks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void
ReconBMBlock(
  mpeg_VidStream *vid_stream,
  int bnum,
  int recon_right_back,
  int recon_down_back,
  int zflag)
{
  int mb_row, mb_col, row, col, row_size, rr;
  unsigned char *dest, *future;
  int right_back, down_back, right_half_back, down_half_back;
  unsigned char *rindex1, *rindex2;
  unsigned char *index;
  short int *blockvals;

  /* Calculate macroblock row and column from address. */

  mb_row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  mb_col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  /* If block is luminance block... */

  if (bnum < 4) {

    /* Calculate right_back, down_back motion vectors. */

    right_back = recon_right_back >> 1;
    down_back = recon_down_back >> 1;
    right_half_back = recon_right_back & 0x1;
    down_half_back = recon_down_back & 0x1;

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    /*
     * If future frame exists, set future to luminance plane of future frame.
     */

    if (vid_stream->future != NULL)
      future = vid_stream->future->luminance;

    /* Establish row size. */

    row_size = vid_stream->mb_width << 4;

    /* Calculate row,col of upper left pixel in block. */

    row = mb_row << 4;
    col = mb_col << 4;
    if (bnum > 1)
      row += 8;
    if (bnum % 2)
      col += 8;

  }
  /* Otherwise, block is NOT luminance block, ... */

  else {

    /* Construct motion vectors. */

    recon_right_back /= 2;
    recon_down_back /= 2;
    right_back = recon_right_back >> 1;
    down_back = recon_down_back >> 1;
    right_half_back = recon_right_back & 0x1;
    down_half_back = recon_down_back & 0x1;

    /* Establish row size. */

    row_size = vid_stream->mb_width << 3;

    /* Calculate row,col of upper left pixel in block. */

    row = mb_row << 3;
    col = mb_col << 3;

    /* If block is Cr block... */
    /* They were switched earlier, so 5 is first - eyhung */

    if (bnum == 5) {

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

      /*
       * If future frame exists, set future to Cr plane of future image.
       */

      if (vid_stream->future != NULL)
	future = vid_stream->future->Cr;
    }
    /* Otherwise, block is Cb block... */

    else {

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;

      /*
       * If future frame exists, set future to Cb plane of future frame.
       */

      if (vid_stream->future != NULL)
	future = vid_stream->future->Cb;
    }
  }

  /* For each pixel in block do... */

    index = dest + (row * row_size) + col;
    rindex1 = future + (row + down_back) * row_size + col + right_back;

    blockvals = &(vid_stream->block.dct_recon[0][0]);

    if ((!right_half_back) && (!down_half_back)) {
      unsigned char *cm = cropTbl + MAX_NEG_CROP;
      if (!zflag)
	for (rr = 0; rr < 4; rr++) {
	  index[0] = cm[(int) rindex1[0] + (int) blockvals[0]];
	  index[1] = cm[(int) rindex1[1] + (int) blockvals[1]];
	  index[2] = cm[(int) rindex1[2] + (int) blockvals[2]];
	  index[3] = cm[(int) rindex1[3] + (int) blockvals[3]];
	  index[4] = cm[(int) rindex1[4] + (int) blockvals[4]];
	  index[5] = cm[(int) rindex1[5] + (int) blockvals[5]];
	  index[6] = cm[(int) rindex1[6] + (int) blockvals[6]];
	  index[7] = cm[(int) rindex1[7] + (int) blockvals[7]];
	  index += row_size;
	  rindex1 += row_size;

	  index[0] = cm[(int) rindex1[0] + (int) blockvals[8]];
	  index[1] = cm[(int) rindex1[1] + (int) blockvals[9]];
	  index[2] = cm[(int) rindex1[2] + (int) blockvals[10]];
	  index[3] = cm[(int) rindex1[3] + (int) blockvals[11]];
	  index[4] = cm[(int) rindex1[4] + (int) blockvals[12]];
	  index[5] = cm[(int) rindex1[5] + (int) blockvals[13]];
	  index[6] = cm[(int) rindex1[6] + (int) blockvals[14]];
	  index[7] = cm[(int) rindex1[7] + (int) blockvals[15]];
	  blockvals += 16;
	  index += row_size;
	  rindex1 += row_size;
	}
      else {
	if (right_back & 0x1) {
	  /* No alignment, use bye copy */
	  for (rr = 0; rr < 4; rr++) {
	    index[0] = rindex1[0];
	    index[1] = rindex1[1];
	    index[2] = rindex1[2];
	    index[3] = rindex1[3];
	    index[4] = rindex1[4];
	    index[5] = rindex1[5];
	    index[6] = rindex1[6];
	    index[7] = rindex1[7];
	    index += row_size;
	    rindex1 += row_size;

	    index[0] = rindex1[0];
	    index[1] = rindex1[1];
	    index[2] = rindex1[2];
	    index[3] = rindex1[3];
	    index[4] = rindex1[4];
	    index[5] = rindex1[5];
	    index[6] = rindex1[6];
	    index[7] = rindex1[7];
	    index += row_size;
	    rindex1 += row_size;
	  }
	} else if (right_back & 0x2) {
	  /* Half-word bit aligned, use 16 bit copy */
	  short *src = (short *)rindex1;
	  short *dest = (short *)index;
	  row_size >>= 1;
	  for (rr = 0; rr < 4; rr++) {
	    dest[0] = src[0];
	    dest[1] = src[1];
	    dest[2] = src[2];
	    dest[3] = src[3];
	    dest += row_size;
	    src += row_size;

	    dest[0] = src[0];
	    dest[1] = src[1];
	    dest[2] = src[2];
	    dest[3] = src[3];
	    dest += row_size;
	    src += row_size;
	  }
	} else {
	  /* Word aligned, use 32 bit copy */
	  int *src = (int *)rindex1;
	  int *dest = (int *)index;
	  row_size >>= 2;
	  for (rr = 0; rr < 4; rr++) {
	    dest[0] = src[0];
	    dest[1] = src[1];
	    dest += row_size;
	    src += row_size;

	    dest[0] = src[0];
	    dest[1] = src[1];
	    dest += row_size;
	    src += row_size;
	  }
	}
      }
    } else {
      unsigned char *cm = cropTbl + MAX_NEG_CROP;
      rindex2 = rindex1 + right_half_back + (down_half_back * row_size);
        if (!zflag) {
          for (rr = 0; rr < 4; rr++) {
            index[0] = cm[((int) (rindex1[0] + rindex2[0] + 1) >> 1) + blockvals[0]];
            index[1] = cm[((int) (rindex1[1] + rindex2[1] + 1) >> 1) + blockvals[1]];
            index[2] = cm[((int) (rindex1[2] + rindex2[2] + 1) >> 1) + blockvals[2]];
            index[3] = cm[((int) (rindex1[3] + rindex2[3] + 1) >> 1) + blockvals[3]];
            index[4] = cm[((int) (rindex1[4] + rindex2[4] + 1) >> 1) + blockvals[4]];
            index[5] = cm[((int) (rindex1[5] + rindex2[5] + 1) >> 1) + blockvals[5]];
            index[6] = cm[((int) (rindex1[6] + rindex2[6] + 1) >> 1) + blockvals[6]];
            index[7] = cm[((int) (rindex1[7] + rindex2[7] + 1) >> 1) + blockvals[7]];
            index += row_size;
            rindex1 += row_size;
            rindex2 += row_size;

            index[0] = cm[((int) (rindex1[0] + rindex2[0] + 1) >> 1) + blockvals[8]];
            index[1] = cm[((int) (rindex1[1] + rindex2[1] + 1) >> 1) + blockvals[9]];
            index[2] = cm[((int) (rindex1[2] + rindex2[2] + 1) >> 1) + blockvals[10]];
            index[3] = cm[((int) (rindex1[3] + rindex2[3] + 1) >> 1) + blockvals[11]];
            index[4] = cm[((int) (rindex1[4] + rindex2[4] + 1) >> 1) + blockvals[12]];
            index[5] = cm[((int) (rindex1[5] + rindex2[5] + 1) >> 1) + blockvals[13]];
            index[6] = cm[((int) (rindex1[6] + rindex2[6] + 1) >> 1) + blockvals[14]];
            index[7] = cm[((int) (rindex1[7] + rindex2[7] + 1) >> 1) + blockvals[15]];
            blockvals += 16;
            index += row_size;
            rindex1 += row_size;
            rindex2 += row_size;
          }
        } else { /* zflag */
          for (rr = 0; rr < 8; rr++) {
            index[0] = (int) (rindex1[0] + rindex2[0] + 1) >> 1;
            index[1] = (int) (rindex1[1] + rindex2[1] + 1) >> 1;
            index[2] = (int) (rindex1[2] + rindex2[2] + 1) >> 1;
            index[3] = (int) (rindex1[3] + rindex2[3] + 1) >> 1;
            index[4] = (int) (rindex1[4] + rindex2[4] + 1) >> 1;
            index[5] = (int) (rindex1[5] + rindex2[5] + 1) >> 1;
            index[6] = (int) (rindex1[6] + rindex2[6] + 1) >> 1;
            index[7] = (int) (rindex1[7] + rindex2[7] + 1) >> 1;
            index += row_size;
            rindex1 += row_size;
            rindex2 += row_size;
          }
        }
    }
}


/*
 *--------------------------------------------------------------
 *
 * ReconBiMBlock --
 *
 *	Reconstructs bidirectionally predicted macroblocks.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

static void
ReconBiMBlock(
  mpeg_VidStream *vid_stream,
  int bnum,
  int recon_right_for,
  int recon_down_for,
  int recon_right_back,
  int recon_down_back,
  int zflag)
{
  int mb_row, mb_col, row, col, row_size, rr;
  unsigned char *dest, *past=NULL, *future=NULL;
  int right_for, down_for, right_half_for, down_half_for;
  int right_back, down_back, right_half_back, down_half_back;
  unsigned char *index, *rindex1, *bindex1;
  short int *blockvals;
  int forw_row_start, back_row_start, forw_col_start, back_col_start;

  /* Calculate macroblock row and column from address. */

  mb_row = vid_stream->mblock.mb_address / vid_stream->mb_width;
  mb_col = vid_stream->mblock.mb_address % vid_stream->mb_width;

  /* If block is luminance block... */

  if (bnum < 4) {

    /*
     * Calculate right_for, down_for, right_half_for, down_half_for,
     * right_back, down_bakc, right_half_back, and down_half_back, motion
     * vectors.
     */

    right_for = recon_right_for >> 1;
    down_for = recon_down_for >> 1;
    right_half_for = recon_right_for & 0x1;
    down_half_for = recon_down_for & 0x1;

    right_back = recon_right_back >> 1;
    down_back = recon_down_back >> 1;
    right_half_back = recon_right_back & 0x1;
    down_half_back = recon_down_back & 0x1;

    /* Set dest to luminance plane of current pict image. */

    dest = vid_stream->current->luminance;

    /* If past frame exists, set past to luminance plane of past frame. */

    if (vid_stream->past != NULL)
      past = vid_stream->past->luminance;

    /*
     * If future frame exists, set future to luminance plane of future frame.
     */

    if (vid_stream->future != NULL)
      future = vid_stream->future->luminance;

    /* Establish row size. */

    row_size = (vid_stream->mb_width << 4);

    /* Calculate row,col of upper left pixel in block. */

    row = (mb_row << 4);
    col = (mb_col << 4);
    if (bnum > 1)
      row += 8;
    if (bnum & 0x01)
      col += 8;

    forw_col_start = col + right_for;
    forw_row_start = row + down_for;

    back_col_start = col + right_back;
    back_row_start = row + down_back;

  }
  /* Otherwise, block is NOT luminance block, ... */

  else {

    /* Construct motion vectors. */

    recon_right_for /= 2;
    recon_down_for /= 2;
    right_for = recon_right_for >> 1;
    down_for = recon_down_for >> 1;
    right_half_for = recon_right_for & 0x1;
    down_half_for = recon_down_for & 0x1;

    recon_right_back /= 2;
    recon_down_back /= 2;
    right_back = recon_right_back >> 1;
    down_back = recon_down_back >> 1;
    right_half_back = recon_right_back & 0x1;
    down_half_back = recon_down_back & 0x1;

    /* Establish row size. */

    row_size = (vid_stream->mb_width << 3);

    /* Calculate row,col of upper left pixel in block. */

    row = (mb_row << 3);
    col = (mb_col << 3);

    forw_col_start = col + right_for;
    forw_row_start = row + down_for;

    back_col_start = col + right_back;
    back_row_start = row + down_back;

    /* If block is Cr block... */
	/* Switched earlier, so we test Cr first - eyhung */

    if (bnum == 5) {

      /* Set dest to Cr plane of current pict image. */

      dest = vid_stream->current->Cr;

      /* If past frame exists, set past to Cr plane of past image. */

      if (vid_stream->past != NULL)
	past = vid_stream->past->Cr;

      /*
       * If future frame exists, set future to Cr plane of future image.
       */

      if (vid_stream->future != NULL)
	future = vid_stream->future->Cr;
    }
    /* Otherwise, block is Cb block... */

    else {

      /* Set dest to Cb plane of current pict image. */

      dest = vid_stream->current->Cb;

      /* If past frame exists, set past to Cb plane of past frame. */

      if (vid_stream->past != NULL)
	past = vid_stream->past->Cb;

      /*
       * If future frame exists, set future to Cb plane of future frame.
       */

      if (vid_stream->future != NULL)
	future = vid_stream->future->Cb;
    }
  }

  /* For each pixel in block... */

  index = dest + (row * row_size) + col;

    rindex1 = past + forw_row_start  * row_size + forw_col_start;

    bindex1 = future + back_row_start * row_size + back_col_start;

  blockvals = (short int *) &(vid_stream->block.dct_recon[0][0]);

  {
  unsigned char *cm = cropTbl + MAX_NEG_CROP;
  if (!zflag)
    for (rr = 0; rr < 4; rr++) {
      index[0] = cm[((int) (rindex1[0] + bindex1[0]) >> 1) + blockvals[0]];
      index[1] = cm[((int) (rindex1[1] + bindex1[1]) >> 1) + blockvals[1]];
      index[2] = cm[((int) (rindex1[2] + bindex1[2]) >> 1) + blockvals[2]];
      index[3] = cm[((int) (rindex1[3] + bindex1[3]) >> 1) + blockvals[3]];
      index[4] = cm[((int) (rindex1[4] + bindex1[4]) >> 1) + blockvals[4]];
      index[5] = cm[((int) (rindex1[5] + bindex1[5]) >> 1) + blockvals[5]];
      index[6] = cm[((int) (rindex1[6] + bindex1[6]) >> 1) + blockvals[6]];
      index[7] = cm[((int) (rindex1[7] + bindex1[7]) >> 1) + blockvals[7]];
      index += row_size;
      rindex1 += row_size;
      bindex1 += row_size;

      index[0] = cm[((int) (rindex1[0] + bindex1[0]) >> 1) + blockvals[8]];
      index[1] = cm[((int) (rindex1[1] + bindex1[1]) >> 1) + blockvals[9]];
      index[2] = cm[((int) (rindex1[2] + bindex1[2]) >> 1) + blockvals[10]];
      index[3] = cm[((int) (rindex1[3] + bindex1[3]) >> 1) + blockvals[11]];
      index[4] = cm[((int) (rindex1[4] + bindex1[4]) >> 1) + blockvals[12]];
      index[5] = cm[((int) (rindex1[5] + bindex1[5]) >> 1) + blockvals[13]];
      index[6] = cm[((int) (rindex1[6] + bindex1[6]) >> 1) + blockvals[14]];
      index[7] = cm[((int) (rindex1[7] + bindex1[7]) >> 1) + blockvals[15]];
      blockvals += 16;
      index += row_size;
      rindex1 += row_size;
      bindex1 += row_size;
    }

  else
    for (rr = 0; rr < 4; rr++) {
      index[0] = (int) (rindex1[0] + bindex1[0]) >> 1;
      index[1] = (int) (rindex1[1] + bindex1[1]) >> 1;
      index[2] = (int) (rindex1[2] + bindex1[2]) >> 1;
      index[3] = (int) (rindex1[3] + bindex1[3]) >> 1;
      index[4] = (int) (rindex1[4] + bindex1[4]) >> 1;
      index[5] = (int) (rindex1[5] + bindex1[5]) >> 1;
      index[6] = (int) (rindex1[6] + bindex1[6]) >> 1;
      index[7] = (int) (rindex1[7] + bindex1[7]) >> 1;
      index += row_size;
      rindex1 += row_size;
      bindex1 += row_size;

      index[0] = (int) (rindex1[0] + bindex1[0]) >> 1;
      index[1] = (int) (rindex1[1] + bindex1[1]) >> 1;
      index[2] = (int) (rindex1[2] + bindex1[2]) >> 1;
      index[3] = (int) (rindex1[3] + bindex1[3]) >> 1;
      index[4] = (int) (rindex1[4] + bindex1[4]) >> 1;
      index[5] = (int) (rindex1[5] + bindex1[5]) >> 1;
      index[6] = (int) (rindex1[6] + bindex1[6]) >> 1;
      index[7] = (int) (rindex1[7] + bindex1[7]) >> 1;
      index += row_size;
      rindex1 += row_size;
      bindex1 += row_size;
    }
  }
}


/*
 *--------------------------------------------------------------
 *
 * ProcessSkippedPFrameMBlocks --
 *
 *	Processes skipped macroblocks in P frames.
 *
 * Results:
 *	Calculates pixel values for luminance, Cr, and Cb planes
 *      in current pict image for skipped macroblocks.
 *
 * Side effects:
 *	Pixel values in pict image changed.
 *
 *--------------------------------------------------------------
 */

static void
ProcessSkippedPFrameMBlocks(
  mpeg_VidStream *vid_stream)
{
  int row_size, half_row, mb_row, mb_col, row, col, rr;
  int addr, row_incr, half_row_incr, crow, ccol;
  int *dest, *src, *dest1, *src1;

  /* Calculate row sizes for luminance and Cr/Cb macroblock areas. */

  row_size = vid_stream->mb_width << 4;
  half_row = (row_size >> 1);
  row_incr = row_size >> 2;
  half_row_incr = half_row >> 2;

  /* For each skipped macroblock, do... */

  for (addr = vid_stream->mblock.past_mb_addr + 1;
       addr < vid_stream->mblock.mb_address; addr++) {

    /* Calculate macroblock row and col. */

    mb_row = addr / vid_stream->mb_width;
    mb_col = addr % vid_stream->mb_width;

    /* Calculate upper left pixel row,col for luminance plane. */

    row = mb_row << 4;
    col = mb_col << 4;


    /* For each row in macroblock luminance plane... */

    dest = (int *)(vid_stream->current->luminance + (row * row_size) + col);
    src = (int *)(vid_stream->future->luminance + (row * row_size) + col);

    for (rr = 0; rr < 8; rr++) {

      /* Copy pixel values from last I or P picture. */

      dest[0] = src[0];
      dest[1] = src[1];
      dest[2] = src[2];
      dest[3] = src[3];
      dest += row_incr;
      src += row_incr;

      dest[0] = src[0];
      dest[1] = src[1];
      dest[2] = src[2];
      dest[3] = src[3];
      dest += row_incr;
      src += row_incr;
    }

    /*
     * Divide row,col to get upper left pixel of macroblock in Cr and Cb
     * planes.
     */

    crow = row >> 1;
    ccol = col >> 1;

    /* For each row in Cr, and Cb planes... */

    dest = (int *)(vid_stream->current->Cr + (crow * half_row) + ccol);
    src = (int *)(vid_stream->future->Cr + (crow * half_row) + ccol);
    dest1 = (int *)(vid_stream->current->Cb + (crow * half_row) + ccol);
    src1 = (int *)(vid_stream->future->Cb + (crow * half_row) + ccol);

    for (rr = 0; rr < 4; rr++) {

      /* Copy pixel values from last I or P picture. */

      dest[0] = src[0];
      dest[1] = src[1];

      dest1[0] = src1[0];
      dest1[1] = src1[1];

      dest += half_row_incr;
      src += half_row_incr;
      dest1 += half_row_incr;
      src1 += half_row_incr;

      dest[0] = src[0];
      dest[1] = src[1];

      dest1[0] = src1[0];
      dest1[1] = src1[1];

      dest += half_row_incr;
      src += half_row_incr;
      dest1 += half_row_incr;
      src1 += half_row_incr;
    }

  }

  vid_stream->mblock.recon_right_for_prev = 0;
  vid_stream->mblock.recon_down_for_prev = 0;
}





/*
 *--------------------------------------------------------------
 *
 * ProcessSkippedBFrameMBlocks --
 *
 *	Processes skipped macroblocks in B frames.
 *
 * Results:
 *	Calculates pixel values for luminance, Cr, and Cb planes
 *      in current pict image for skipped macroblocks.
 *
 * Side effects:
 *	Pixel values in pict image changed.
 *
 *--------------------------------------------------------------
 */

static void
ProcessSkippedBFrameMBlocks(
  mpeg_VidStream *vid_stream)
{
  int row_size, half_row, mb_row, mb_col, row, col, rr;
  int right_half_for = 0, down_half_for = 0;
  int c_right_half_for = 0, c_down_half_for = 0;
  int right_half_back = 0, down_half_back = 0;
  int c_right_half_back = 0, c_down_half_back = 0;
  int addr, right_for = 0, down_for = 0;
  int recon_right_for, recon_down_for;
  int recon_right_back, recon_down_back;
  int right_back = 0, down_back = 0;
  int c_right_for = 0, c_down_for = 0;
  int c_right_back = 0, c_down_back = 0;
  unsigned char forw_lum[256];
  unsigned char forw_cr[64], forw_cb[64];
  unsigned char back_lum[256], back_cr[64], back_cb[64];
  int row_incr, half_row_incr;
  int ccol, crow;

  /* Calculate row sizes for luminance and Cr/Cb macroblock areas. */

  row_size = vid_stream->mb_width << 4;
  half_row = (row_size >> 1);
  row_incr = row_size >> 2;
  half_row_incr =  half_row >> 2;

  /* Establish motion vector codes based on full pixel flag. */

  if (vid_stream->picture.full_pel_forw_vector) {
    recon_right_for = vid_stream->mblock.recon_right_for_prev << 1;
    recon_down_for = vid_stream->mblock.recon_down_for_prev << 1;
  } else {
    recon_right_for = vid_stream->mblock.recon_right_for_prev;
    recon_down_for = vid_stream->mblock.recon_down_for_prev;
  }

  if (vid_stream->picture.full_pel_back_vector) {
    recon_right_back = vid_stream->mblock.recon_right_back_prev << 1;
    recon_down_back = vid_stream->mblock.recon_down_back_prev << 1;
  } else {
    recon_right_back = vid_stream->mblock.recon_right_back_prev;
    recon_down_back = vid_stream->mblock.recon_down_back_prev;
  }


  /* If only one motion vector, do display copy, else do full
     calculation.
  */

  /* Calculate motion vectors. */

  if (vid_stream->mblock.bpict_past_forw) {
    right_for = recon_right_for >> 1;
    down_for = recon_down_for >> 1;
    right_half_for = recon_right_for & 0x1;
    down_half_for = recon_down_for & 0x1;

    recon_right_for /= 2;
    recon_down_for /= 2;
    c_right_for = recon_right_for >> 1;
    c_down_for = recon_down_for >> 1;
    c_right_half_for = recon_right_for & 0x1;
    c_down_half_for = recon_down_for & 0x1;

  }
  if (vid_stream->mblock.bpict_past_back) {
    right_back = recon_right_back >> 1;
    down_back = recon_down_back >> 1;
    right_half_back = recon_right_back & 0x1;
    down_half_back = recon_down_back & 0x1;

    recon_right_back /= 2;
    recon_down_back /= 2;
    c_right_back = recon_right_back >> 1;
    c_down_back = recon_down_back >> 1;
    c_right_half_back = recon_right_back & 0x1;
    c_down_half_back = recon_down_back & 0x1;

  }
  /* For each skipped macroblock, do... */

  for (addr = vid_stream->mblock.past_mb_addr + 1;
       addr < vid_stream->mblock.mb_address; addr++) {

    /* Calculate macroblock row and col. */

    mb_row = addr / vid_stream->mb_width;
    mb_col = addr % vid_stream->mb_width;

    /* Calculate upper left pixel row,col for luminance plane. */

    row = mb_row << 4;
    col = mb_col << 4;
    crow = row / 2;
    ccol = col / 2;

    /* If forward predicted, calculate prediction values. */

    if (vid_stream->mblock.bpict_past_forw) {

      ReconSkippedBlock(vid_stream->past->luminance, forw_lum,
			row, col, row_size, right_for, down_for,
			right_half_for, down_half_for, 16);
      ReconSkippedBlock(vid_stream->past->Cr, forw_cr, crow,
			ccol, half_row,
			c_right_for, c_down_for, c_right_half_for, c_down_half_for, 8);
      ReconSkippedBlock(vid_stream->past->Cb, forw_cb, crow,
			ccol, half_row,
			c_right_for, c_down_for, c_right_half_for, c_down_half_for, 8);
    }
    /* If back predicted, calculate prediction values. */

    if (vid_stream->mblock.bpict_past_back) {
      ReconSkippedBlock(vid_stream->future->luminance, back_lum,
			row, col, row_size, right_back, down_back,
			right_half_back, down_half_back, 16);
      ReconSkippedBlock(vid_stream->future->Cr, back_cr, crow,
			ccol, half_row,
			c_right_back, c_down_back,
			c_right_half_back, c_down_half_back, 8);
      ReconSkippedBlock(vid_stream->future->Cb, back_cb, crow,
			ccol, half_row,
			c_right_back, c_down_back,
			c_right_half_back, c_down_half_back, 8);
    }
    if (vid_stream->mblock.bpict_past_forw &&
	!vid_stream->mblock.bpict_past_back) {

      int *dest, *dest1;
      int *src, *src1;
      dest = (int *)(vid_stream->current->luminance + (row * row_size) + col);
      src = (int *)forw_lum;

      for (rr = 0; rr < 16; rr++) {

	/* memcpy(dest, forw_lum+(rr<<4), 16);  */
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
	dest += row_incr;
	src += 4;
      }

      dest = (int *)(vid_stream->current->Cr + (crow * half_row) + ccol);
      dest1 = (int *)(vid_stream->current->Cb + (crow * half_row) + ccol);
      src = (int *)forw_cr;
      src1 = (int *)forw_cb;

      for (rr = 0; rr < 8; rr++) {
	/*
	 * memcpy(dest, forw_cr+(rr<<3), 8); memcpy(dest1, forw_cb+(rr<<3),
	 * 8);
	 */

	dest[0] = src[0];
	dest[1] = src[1];

	dest1[0] = src1[0];
	dest1[1] = src1[1];

	dest += half_row_incr;
	dest1 += half_row_incr;
	src += 2;
	src1 += 2;
      }
    } else if (vid_stream->mblock.bpict_past_back &&
	       !vid_stream->mblock.bpict_past_forw) {

      int *src, *src1;
      int *dest, *dest1;
      dest = (int *)(vid_stream->current->luminance + (row * row_size) + col);
      src = (int *)back_lum;

      for (rr = 0; rr < 16; rr++) {
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
	dest += row_incr;
	src += 4;
      }


      dest = (int *)(vid_stream->current->Cr + (crow * half_row) + ccol);
      dest1 = (int *)(vid_stream->current->Cb + (crow * half_row) + ccol);
      src = (int *)back_cr;
      src1 = (int *)back_cb;

      for (rr = 0; rr < 8; rr++) {
	/*
	 * memcpy(dest, back_cr+(rr<<3), 8); memcpy(dest1, back_cb+(rr<<3),
	 * 8);
	 */

	dest[0] = src[0];
	dest[1] = src[1];

	dest1[0] = src1[0];
	dest1[1] = src1[1];

	dest += half_row_incr;
	dest1 += half_row_incr;
	src += 2;
	src1 += 2;
      }
    } else {

      unsigned char *src1, *src2, *src1a, *src2a;
      unsigned char *dest, *dest1;
      dest = vid_stream->current->luminance + (row * row_size) + col;
      src1 = forw_lum;
      src2 = back_lum;

      for (rr = 0; rr < 16; rr++) {
        dest[0] = (int) (src1[0] + src2[0]) >> 1;
        dest[1] = (int) (src1[1] + src2[1]) >> 1;
        dest[2] = (int) (src1[2] + src2[2]) >> 1;
        dest[3] = (int) (src1[3] + src2[3]) >> 1;
        dest[4] = (int) (src1[4] + src2[4]) >> 1;
        dest[5] = (int) (src1[5] + src2[5]) >> 1;
        dest[6] = (int) (src1[6] + src2[6]) >> 1;
        dest[7] = (int) (src1[7] + src2[7]) >> 1;
        dest[8] = (int) (src1[8] + src2[8]) >> 1;
        dest[9] = (int) (src1[9] + src2[9]) >> 1;
        dest[10] = (int) (src1[10] + src2[10]) >> 1;
        dest[11] = (int) (src1[11] + src2[11]) >> 1;
        dest[12] = (int) (src1[12] + src2[12]) >> 1;
        dest[13] = (int) (src1[13] + src2[13]) >> 1;
        dest[14] = (int) (src1[14] + src2[14]) >> 1;
        dest[15] = (int) (src1[15] + src2[15]) >> 1;
        dest += row_size;
        src1 += 16;
        src2 += 16;
      }


      dest = vid_stream->current->Cr + (crow * half_row) + ccol;
      dest1 = vid_stream->current->Cb + (crow * half_row) + ccol;
      src1 = forw_cr;
      src2 = back_cr;
      src1a = forw_cb;
      src2a = back_cb;

      for (rr = 0; rr < 8; rr++) {
        dest[0] = (int) (src1[0] + src2[0]) >> 1;
        dest[1] = (int) (src1[1] + src2[1]) >> 1;
        dest[2] = (int) (src1[2] + src2[2]) >> 1;
        dest[3] = (int) (src1[3] + src2[3]) >> 1;
        dest[4] = (int) (src1[4] + src2[4]) >> 1;
        dest[5] = (int) (src1[5] + src2[5]) >> 1;
        dest[6] = (int) (src1[6] + src2[6]) >> 1;
        dest[7] = (int) (src1[7] + src2[7]) >> 1;
        dest += half_row;
        src1 += 8;
        src2 += 8;

        dest1[0] = (int) (src1a[0] + src2a[0]) >> 1;
        dest1[1] = (int) (src1a[1] + src2a[1]) >> 1;
        dest1[2] = (int) (src1a[2] + src2a[2]) >> 1;
        dest1[3] = (int) (src1a[3] + src2a[3]) >> 1;
        dest1[4] = (int) (src1a[4] + src2a[4]) >> 1;
        dest1[5] = (int) (src1a[5] + src2a[5]) >> 1;
        dest1[6] = (int) (src1a[6] + src2a[6]) >> 1;
        dest1[7] = (int) (src1a[7] + src2a[7]) >> 1;
        dest1 += half_row;
        src1a += 8;
        src2a += 8;
      }
    }

  }
}





/*
 *--------------------------------------------------------------
 *
 * ReconSkippedBlock --
 *
 *	Reconstructs predictive block for skipped macroblocks
 *      in B Frames.
 *
 * Results:
 *	No return values.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

static void
ReconSkippedBlock(
  unsigned char *source,
  unsigned char *dest,
  int row,
  int col,
  int row_size,
  int right,
  int down,
  int right_half,
  int down_half,
  int width)
{
  int rr;
  unsigned char *source2;

  source += ((row + down) * row_size) + col + right;

  if (width == 16) {
    if ((!right_half) && (!down_half)) {
	if (right & 0x1) {
	  /* No alignment, use bye copy */
	  for (rr = 0; rr < 16; rr++) {
	    dest[0] = source[0];
	    dest[1] = source[1];
	    dest[2] = source[2];
	    dest[3] = source[3];
	    dest[4] = source[4];
	    dest[5] = source[5];
	    dest[6] = source[6];
	    dest[7] = source[7];
	    dest[8] = source[8];
	    dest[9] = source[9];
	    dest[10] = source[10];
	    dest[11] = source[11];
	    dest[12] = source[12];
	    dest[13] = source[13];
	    dest[14] = source[14];
	    dest[15] = source[15];
	    dest += 16;
	    source += row_size;
	  }
	} else if (right & 0x2) {
	  /* Half-word bit aligned, use 16 bit copy */
	  short *src = (short *)source;
	  short *d = (short *)dest;
	  row_size >>= 1;
	  for (rr = 0; rr < 16; rr++) {
	    d[0] = src[0];
	    d[1] = src[1];
	    d[2] = src[2];
	    d[3] = src[3];
	    d[4] = src[4];
	    d[5] = src[5];
	    d[6] = src[6];
	    d[7] = src[7];
	    d += 8;
	    src += row_size;
	  }
	} else {
	  /* Word aligned, use 32 bit copy */
	  int *src = (int *)source;
	  int *d = (int *)dest;
	  row_size >>= 2;
	  for (rr = 0; rr < 16; rr++) {
	    d[0] = src[0];
	    d[1] = src[1];
	    d[2] = src[2];
	    d[3] = src[3];
	    d += 4;
	    src += row_size;
	  }
	}
    } else {
      source2 = source + right_half + (row_size * down_half);
      for (rr = 0; rr < width; rr++) {
        dest[0] = (int) (source[0] + source2[0]) >> 1;
        dest[1] = (int) (source[1] + source2[1]) >> 1;
        dest[2] = (int) (source[2] + source2[2]) >> 1;
        dest[3] = (int) (source[3] + source2[3]) >> 1;
        dest[4] = (int) (source[4] + source2[4]) >> 1;
        dest[5] = (int) (source[5] + source2[5]) >> 1;
        dest[6] = (int) (source[6] + source2[6]) >> 1;
        dest[7] = (int) (source[7] + source2[7]) >> 1;
        dest[8] = (int) (source[8] + source2[8]) >> 1;
        dest[9] = (int) (source[9] + source2[9]) >> 1;
        dest[10] = (int) (source[10] + source2[10]) >> 1;
        dest[11] = (int) (source[11] + source2[11]) >> 1;
        dest[12] = (int) (source[12] + source2[12]) >> 1;
        dest[13] = (int) (source[13] + source2[13]) >> 1;
        dest[14] = (int) (source[14] + source2[14]) >> 1;
        dest[15] = (int) (source[15] + source2[15]) >> 1;
        dest += width;
        source += row_size;
        source2 += row_size;
      }
    }
  } else {			/* (width == 8) */
    assert(width == 8);
    if ((!right_half) && (!down_half)) {
      if (right & 0x1) {
	for (rr = 0; rr < width; rr++) {
	  dest[0] = source[0];
	  dest[1] = source[1];
	  dest[2] = source[2];
	  dest[3] = source[3];
	  dest[4] = source[4];
	  dest[5] = source[5];
	  dest[6] = source[6];
	  dest[7] = source[7];
	  dest += 8;
	  source += row_size;
	}
      } else if (right & 0x02) {
	short *d = (short *)dest;
	short *src = (short *)source;
	row_size >>= 1;
	for (rr = 0; rr < width; rr++) {
	  d[0] = src[0];
	  d[1] = src[1];
	  d[2] = src[2];
	  d[3] = src[3];
	  d += 4;
	  src += row_size;
	}
      } else {
	int *d = (int *)dest;
	int *src = (int *)source;
	row_size >>= 2;
	for (rr = 0; rr < width; rr++) {
	  d[0] = src[0];
	  d[1] = src[1];
	  d += 2;
	  src += row_size;
	}
      }
    } else {
      source2 = source + right_half + (row_size * down_half);
      for (rr = 0; rr < width; rr++) {
        dest[0] = (int) (source[0] + source2[0]) >> 1;
        dest[1] = (int) (source[1] + source2[1]) >> 1;
        dest[2] = (int) (source[2] + source2[2]) >> 1;
        dest[3] = (int) (source[3] + source2[3]) >> 1;
        dest[4] = (int) (source[4] + source2[4]) >> 1;
        dest[5] = (int) (source[5] + source2[5]) >> 1;
        dest[6] = (int) (source[6] + source2[6]) >> 1;
        dest[7] = (int) (source[7] + source2[7]) >> 1;
        dest += width;
        source += row_size;
        source2 += row_size;
      }
    }
  }
}




/*
 *--------------------------------------------------------------
 *
 * DoPictureDisplay --
 *
 *	Converts image from Lum, Cr, Cb to colormap space. Puts
 *      image in lum plane. Updates past and future frame
 *      pointers. Dithers image. Sends to display mechanism.
 *
 * Results:
 *	Pict image structure locked if displaying or if frame
 *      is needed as past or future reference.
 *
 * Side effects:
 *	Lum plane pummelled.
 *
 *--------------------------------------------------------------
 */

static void
DoPictureDisplay(
  mpeg_VidStream *vid_stream)
{

  if (No_B_Flag && (vid_stream->picture.code_type == B_TYPE)) return;

  /* Convert to colormap space and dither. */
 Color32DitherImage(vid_stream->current->luminance,
		vid_stream->current->Cr,
		vid_stream->current->Cb,
		vid_stream->current->display,
		vid_stream->mb_height * 16,
		vid_stream->mb_width * 16);



  /* Update past and future references if needed. */

  if ((vid_stream->picture.code_type == I_TYPE) || (vid_stream->picture.code_type == P_TYPE)) {
    if (vid_stream->future == NULL) {
      vid_stream->future = vid_stream->current;
      vid_stream->future->locked |= FUTURE_LOCK;
    } else {
      if (vid_stream->past != NULL) {
        vid_stream->past->locked &= ~PAST_LOCK;
      }
      vid_stream->past = vid_stream->future;
      vid_stream->past->locked &= ~FUTURE_LOCK;
      vid_stream->past->locked |= PAST_LOCK;
      vid_stream->future = vid_stream->current;
      vid_stream->future->locked |= FUTURE_LOCK;
      vid_stream->current = vid_stream->past;
      ExecuteTexture(vid_stream);
    }
  } else {
    ExecuteTexture(vid_stream);
  }
}




#define GLOBAL			/* a function referenced thru EXTERNs */

/* We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an int quantity.
 * It is only applied with constant shift counts.  SHIFT_TEMPS must be
 * included in the variables of any routine using RIGHT_SHIFT.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS	int shift_temp;
#define RIGHT_SHIFT(x,shft)  \
	((shift_temp = (x)) < 0 ? \
	 (shift_temp >> (shft)) | ((~((int) 0)) << (32-(shft))) : \
	 (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif

/*
 * This routine is specialized to the case DCTSIZE = 8.
 */

#if DCTSIZE != 8
  Sorry, this code only copes with 8x8 DCTs. /* deliberate syntax err */
#endif


/*
 * A 2-D IDCT can be done by 1-D IDCT on each row followed by 1-D IDCT
 * on each column.  Direct algorithms are also available, but they are
 * much more complex and seem not to be any faster when reduced to code.
 *
 * The poop on this scaling stuff is as follows:
 *
 * Each 1-D IDCT step produces outputs which are a factor of sqrt(N)
 * larger than the true IDCT outputs.  The final outputs are therefore
 * a factor of N larger than desired; since N=8 this can be cured by
 * a simple right shift at the end of the algorithm.  The advantage of
 * this arrangement is that we save two multiplications per 1-D IDCT,
 * because the y0 and y4 inputs need not be divided by sqrt(N).
 *
 * We have to do addition and subtraction of the integer inputs, which
 * is no problem, and multiplication by fractional constants, which is
 * a problem to do in integer arithmetic.  We multiply all the constants
 * by CONST_SCALE and convert them to integer constants (thus retaining
 * CONST_BITS bits of precision in the constants).  After doing a
 * multiplication we have to divide the product by CONST_SCALE, with proper
 * rounding, to produce the correct output.  This division can be done
 * cheaply as a right shift of CONST_BITS bits.  We postpone shifting
 * as long as possible so that partial sums can be added together with
 * full fractional precision.
 *
 * The outputs of the first pass are scaled up by PASS1_BITS bits so that
 * they are represented to better-than-integral precision.  These outputs
 * require BITS_IN_JSAMPLE + PASS1_BITS + 3 bits; this fits in a 16-bit word
 * with the recommended scaling.  (To scale up 12-bit sample data further, an
 * intermediate int array would be needed.)
 *
 * To avoid overflow of the 32-bit intermediate results in pass 2, we must
 * have BITS_IN_JSAMPLE + CONST_BITS + PASS1_BITS <= 26.  Error analysis
 * shows that the values given below are the most effective.
 */

#ifdef EIGHT_BIT_SAMPLES
#define PASS1_BITS  2
#else
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

#define ONE	((int) 1)

#define CONST_SCALE (ONE << CONST_BITS)

/* Convert a positive real constant to an integer scaled by CONST_SCALE.
 * IMPORTANT: if your compiler doesn't do this arithmetic at compile time,
 * you will pay a significant penalty in run time.  In that case, figure
 * the correct integer constant values and insert them by hand.
 */

#define FIX(x)	((int) ((x) * CONST_SCALE + 0.5))

/* When adding two opposite-signed fixes, the 0.5 cancels */
#define FIX2(x)	((int) ((x) * CONST_SCALE))

/* Descale and correctly round an int value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

/* Multiply an int variable by an INT32 constant to yield an INT32 result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply;
 * this provides a useful speedup on many machines.
 * There is no way to specify a 16x16->32 multiply in portable C, but
 * some C compilers will do the right thing if you provide the correct
 * combination of casts.
 * NB: for 12-bit samples, a full 32-bit multiplication will be needed.
 */

#ifdef EIGHT_BIT_SAMPLES
#ifdef SHORTxSHORT_32		/* may work if 'int' is 32 bits */
#define MULTIPLY(var,const)  (((INT16) (var)) * ((INT16) (const)))
#endif
#ifdef SHORTxLCONST_32		/* known to work with Microsoft C 6.0 */
#define MULTIPLY(var,const)  (((INT16) (var)) * ((int) (const)))
#endif
#endif

#define MULTIPLY(var,const)  ((var) * (const))

#define SPARSE_SCALE_FACTOR  8

/* Precomputed idct value arrays. */

static DCTELEM PreIDCT[64][64];


/*
 *--------------------------------------------------------------
 *
 * init_pre_idct --
 *
 *  Pre-computes singleton coefficient IDCT values.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
init_pre_idct() {
  int i;

  for (i=0; i<64; i++) {
    memset((char *) PreIDCT[i], 0, 64*sizeof(DCTELEM));
    PreIDCT[i][i] = 1 << SPARSE_SCALE_FACTOR;
    j_rev_dct(PreIDCT[i]);
  }
}


/*
 *--------------------------------------------------------------
 *
 * j_rev_dct_sparse --
 *
 *  Performs the inverse DCT on one block of coefficients.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

void
j_rev_dct_sparse (
     DCTBLOCK data,
     int pos)
{
  short int val;
  register int *dp;
  register int v;
  int quant;

#ifdef SPARSE_AC
  register DCTELEM *dataptr;
  DCTELEM *ndataptr;
  int coeff, rr;
  DCTBLOCK tmpdata, tmp2data;
  DCTELEM *tmpdataptr, *tmp2dataptr;
  int printFlag = 1;
#endif


  /* If DC Coefficient. */

  if (pos == 0) {
    dp = (int *)data;
    v = *data;
    quant = 8;

    /* Compute 32 bit value to assign.  This speeds things up a bit */
    if (v < 0) {
        val = -v;
        val += (quant >> 1);
        val /= quant;
        val = -val;
    }
    else {
        val = (v + (quant >> 1)) / quant;
    }

    v = ((val & 0xffff) | (val << 16));

    dp[0] = v;      dp[1] = v;      dp[2] = v;      dp[3] = v;
    dp[4] = v;      dp[5] = v;      dp[6] = v;      dp[7] = v;
    dp[8] = v;      dp[9] = v;      dp[10] = v;      dp[11] = v;
    dp[12] = v;      dp[13] = v;      dp[14] = v;      dp[15] = v;
    dp[16] = v;      dp[17] = v;      dp[18] = v;      dp[19] = v;
    dp[20] = v;      dp[21] = v;      dp[22] = v;      dp[23] = v;
    dp[24] = v;      dp[25] = v;      dp[26] = v;      dp[27] = v;
    dp[28] = v;      dp[29] = v;      dp[30] = v;      dp[31] = v;

    return;
  }

  /* Some other coefficient. */

#ifdef SPARSE_AC
  dataptr = (DCTELEM *)data;
  coeff = dataptr[pos];
  ndataptr = PreIDCT[pos];

  printf ("\n");
  printf ("COEFFICIENT = %3d, POSITION = %2d\n", coeff, pos);

  for (v=0; v<64; v++) {
    memcpy((char *) tmpdata, data, 64*sizeof(DCTELEM));
  }
  tmpdataptr = (DCTELEM *)tmpdata;

  for (v=0; v<64; v++) {
    memcpy((char *) tmp2data, data, 64*sizeof(DCTELEM));
  }
  tmp2dataptr = (DCTELEM *)tmp2data;

  for (rr=0; rr<4; rr++) {
    dataptr[0] = (ndataptr[0] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[1] = (ndataptr[1] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[2] = (ndataptr[2] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[3] = (ndataptr[3] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[4] = (ndataptr[4] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[5] = (ndataptr[5] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[6] = (ndataptr[6] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[7] = (ndataptr[7] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[8] = (ndataptr[8] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[9] = (ndataptr[9] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[10] = (ndataptr[10] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[11] = (ndataptr[11] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[12] = (ndataptr[12] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[13] = (ndataptr[13] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[14] = (ndataptr[14] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr[15] = (ndataptr[15] * coeff) >> SPARSE_SCALE_FACTOR;
    dataptr += 16;
    ndataptr += 16;
  }

  dataptr = (DCTELEM *) data;

#else  /* NO_SPARSE_AC */
#ifdef FLOATDCT
  if (qualityFlag)
	float_idct(data);
  else
#endif /* FLOATDCT */
	j_rev_dct(data);
#endif /* SPARSE_AC */
  return;

}


/*
 *--------------------------------------------------------------
 *
 * j_rev_dct --
 *
 *  The inverse DCT function.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
j_rev_dct (
     DCTBLOCK data )
{
  int tmp0, tmp1, tmp2, tmp3;
  int tmp10, tmp11, tmp12, tmp13;
  int z1, z2, z3, z4, z5;
  int d0, d1, d2, d3, d4, d5, d6, d7;
  register DCTELEM *dataptr;
  int rowctr;
  SHIFT_TEMPS

  /* Pass 1: process rows. */
  /* Note results are scaled up by sqrt(8) compared to a true IDCT; */
  /* furthermore, we scale the results by 2**PASS1_BITS. */

  dataptr = data;

  for (rowctr = DCTSIZE-1; rowctr >= 0; rowctr--) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any row in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * row DCT calculations can be simplified this way.
     */

    register int *idataptr = (int*)dataptr;
    d0 = dataptr[0];
    d1 = dataptr[1];
    if ((d1 == 0) && (idataptr[1] | idataptr[2] | idataptr[3]) == 0) {
      /* AC terms all zero */
      if (d0) {
	  /* Compute a 32 bit value to assign. */
	  DCTELEM dcval = (DCTELEM) (d0 << PASS1_BITS);
	  register int v = (dcval & 0xffff) | (dcval << 16);

	  idataptr[0] = v;
	  idataptr[1] = v;
	  idataptr[2] = v;
	  idataptr[3] = v;
      }

      dataptr += DCTSIZE;	/* advance pointer to next row */
      continue;
    }
    d2 = dataptr[2];
    d3 = dataptr[3];
    d4 = dataptr[4];
    d5 = dataptr[5];
    d6 = dataptr[6];
    d7 = dataptr[7];

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */
    if (d6) {
	if (d4) {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 != 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp0 = (d0 + d4) << CONST_BITS;
		    tmp1 = (d0 - d4) << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp1 + tmp2;
		    tmp12 = tmp1 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 != 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp0 = d4 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp2 - tmp0;
		    tmp12 = -(tmp0 + tmp2);
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 != 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp0 = (d0 + d4) << CONST_BITS;
		    tmp1 = (d0 - d4) << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp1 + tmp2;
		    tmp12 = tmp1 - tmp2;
		} else {
		    /* d0 == 0, d2 == 0, d4 != 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp0 = d4 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp2 - tmp0;
		    tmp12 = -(tmp0 + tmp2);
		}
	    }
	} else {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 == 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp0 = d0 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp0 + tmp2;
		    tmp12 = tmp0 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 == 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp10 = tmp3;
		    tmp13 = -tmp3;
		    tmp11 = tmp2;
		    tmp12 = -tmp2;
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 == 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp0 = d0 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp0 + tmp2;
		    tmp12 = tmp0 - tmp2;
		} else {
		    /* d0 == 0, d2 == 0, d4 == 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp10 = tmp3;
		    tmp13 = -tmp3;
		    tmp11 = tmp2;
		    tmp12 = -tmp2;
		}
	    }
	}
    } else {
	if (d4) {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 != 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp0 = (d0 + d4) << CONST_BITS;
		    tmp1 = (d0 - d4) << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp1 + tmp2;
		    tmp12 = tmp1 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 != 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp0 = d4 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp2 - tmp0;
		    tmp12 = -(tmp0 + tmp2);
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 != 0, d6 == 0 */
		    tmp10 = tmp13 = (d0 + d4) << CONST_BITS;
		    tmp11 = tmp12 = (d0 - d4) << CONST_BITS;
		} else {
		    /* d0 == 0, d2 == 0, d4 != 0, d6 == 0 */
		    tmp10 = tmp13 = d4 << CONST_BITS;
		    tmp11 = tmp12 = -tmp10;
		}
	    }
	} else {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 == 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp0 = d0 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp0 + tmp2;
		    tmp12 = tmp0 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 == 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp10 = tmp3;
		    tmp13 = -tmp3;
		    tmp11 = tmp2;
		    tmp12 = -tmp2;
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 == 0, d6 == 0 */
		    tmp10 = tmp13 = tmp11 = tmp12 = d0 << CONST_BITS;
		} else {
		    /* d0 == 0, d2 == 0, d4 == 0, d6 == 0 */
		    tmp10 = tmp13 = tmp11 = tmp12 = 0;
		}
	    }
	}
    }


    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */

    if (d7) {
	if (d5) {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 != 0, d7 != 0 */
		    z1 = d7 + d1;
		    z2 = d5 + d3;
		    z3 = d7 + d3;
		    z4 = d5 + d1;
		    z5 = MULTIPLY(z3 + z4, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(z1, - FIX(0.899976223));
		    z2 = MULTIPLY(z2, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX(1.961570560));
		    z4 = MULTIPLY(z4, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 != 0, d5 != 0, d7 != 0 */
		    z2 = d5 + d3;
		    z3 = d7 + d3;
		    z5 = MULTIPLY(z3 + d5, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    z1 = MULTIPLY(d7, - FIX(0.899976223));
		    z2 = MULTIPLY(z2, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX(1.961570560));
		    z4 = MULTIPLY(d5, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 = z1 + z4;
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 != 0, d7 != 0 */
		    z1 = d7 + d1;
		    z4 = d5 + d1;
		    z5 = MULTIPLY(d7 + z4, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(z1, - FIX(0.899976223));
		    z2 = MULTIPLY(d5, - FIX(2.562915447));
		    z3 = MULTIPLY(d7, - FIX(1.961570560));
		    z4 = MULTIPLY(z4, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 = z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 == 0, d5 != 0, d7 != 0 */
		    z5 = MULTIPLY(d7 + d5, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, - FIX2(0.601344887));
		    tmp1 = MULTIPLY(d5, - FIX2(0.509795578));
		    z1 = MULTIPLY(d7, - FIX(0.899976223));
		    z3 = MULTIPLY(d7, - FIX(1.961570560));
		    z2 = MULTIPLY(d5, - FIX(2.562915447));
		    z4 = MULTIPLY(d5, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z3;
		    tmp1 += z4;
		    tmp2 = z2 + z3;
		    tmp3 = z1 + z4;
		}
	    }
	} else {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 == 0, d7 != 0 */
		    z1 = d7 + d1;
		    z3 = d7 + d3;
		    z5 = MULTIPLY(z3 + d1, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(z1, - FIX(0.899976223));
		    z2 = MULTIPLY(d3, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX(1.961570560));
		    z4 = MULTIPLY(d1, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 = z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 != 0, d5 == 0, d7 != 0 */
		    z3 = d7 + d3;
		    z5 = MULTIPLY(z3, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, - FIX2(0.601344887));
		    tmp2 = MULTIPLY(d3, FIX(0.509795579));
		    z1 = MULTIPLY(d7, - FIX(0.899976223));
		    z2 = MULTIPLY(d3, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX2(0.785694958));

		    tmp0 += z3;
		    tmp1 = z2 + z5;
		    tmp2 += z3;
		    tmp3 = z1 + z5;
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 == 0, d7 != 0 */
		    z1 = d7 + d1;
		    z5 = MULTIPLY(z1, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, - FIX2(1.662939224));
		    tmp3 = MULTIPLY(d1, FIX2(1.111140466));
		    z1 = MULTIPLY(z1, FIX2(0.275899379));
		    z3 = MULTIPLY(d7, - FIX(1.961570560));
		    z4 = MULTIPLY(d1, - FIX(0.390180644));

		    tmp0 += z1;
		    tmp1 = z4 + z5;
		    tmp2 = z3 + z5;
		    tmp3 += z1;
		} else {
		    /* d1 == 0, d3 == 0, d5 == 0, d7 != 0 */
		    tmp0 = MULTIPLY(d7, - FIX2(1.387039845));
		    tmp1 = MULTIPLY(d7, FIX(1.175875602));
		    tmp2 = MULTIPLY(d7, - FIX2(0.785694958));
		    tmp3 = MULTIPLY(d7, FIX2(0.275899379));
		}
	    }
	}
    } else {
	if (d5) {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 != 0, d7 == 0 */
		    z2 = d5 + d3;
		    z4 = d5 + d1;
		    z5 = MULTIPLY(d3 + z4, FIX(1.175875602));

		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(d1, - FIX(0.899976223));
		    z2 = MULTIPLY(z2, - FIX(2.562915447));
		    z3 = MULTIPLY(d3, - FIX(1.961570560));
		    z4 = MULTIPLY(z4, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 = z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 != 0, d5 != 0, d7 == 0 */
		    z2 = d5 + d3;
		    z5 = MULTIPLY(z2, FIX(1.175875602));

		    tmp1 = MULTIPLY(d5, FIX2(1.662939225));
		    tmp2 = MULTIPLY(d3, FIX2(1.111140466));
		    z2 = MULTIPLY(z2, - FIX2(1.387039845));
		    z3 = MULTIPLY(d3, - FIX(1.961570560));
		    z4 = MULTIPLY(d5, - FIX(0.390180644));

		    tmp0 = z3 + z5;
		    tmp1 += z2;
		    tmp2 += z2;
		    tmp3 = z4 + z5;
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 != 0, d7 == 0 */
		    z4 = d5 + d1;
		    z5 = MULTIPLY(z4, FIX(1.175875602));

		    tmp1 = MULTIPLY(d5, - FIX2(0.509795578));
		    tmp3 = MULTIPLY(d1, FIX2(0.601344887));
		    z1 = MULTIPLY(d1, - FIX(0.899976223));
		    z2 = MULTIPLY(d5, - FIX(2.562915447));
		    z4 = MULTIPLY(z4, FIX2(0.785694958));

		    tmp0 = z1 + z5;
		    tmp1 += z4;
		    tmp2 = z2 + z5;
		    tmp3 += z4;
		} else {
		    /* d1 == 0, d3 == 0, d5 != 0, d7 == 0 */
		    tmp0 = MULTIPLY(d5, FIX(1.175875602));
		    tmp1 = MULTIPLY(d5, FIX2(0.275899380));
		    tmp2 = MULTIPLY(d5, - FIX2(1.387039845));
		    tmp3 = MULTIPLY(d5, FIX2(0.785694958));
		}
	    }
	} else {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 == 0, d7 == 0 */
		    z5 = d3 + d1;

		    tmp2 = MULTIPLY(d3, - FIX(1.451774981));
		    tmp3 = MULTIPLY(d1, (FIX(0.211164243) - 1));
		    z1 = MULTIPLY(d1, FIX(1.061594337));
		    z2 = MULTIPLY(d3, - FIX(2.172734803));
		    z4 = MULTIPLY(z5, FIX(0.785694958));
		    z5 = MULTIPLY(z5, FIX(1.175875602));

		    tmp0 = z1 - z4;
		    tmp1 = z2 + z4;
		    tmp2 += z5;
		    tmp3 += z5;
		} else {
		    /* d1 == 0, d3 != 0, d5 == 0, d7 == 0 */
		    tmp0 = MULTIPLY(d3, - FIX2(0.785694958));
		    tmp1 = MULTIPLY(d3, - FIX2(1.387039845));
		    tmp2 = MULTIPLY(d3, - FIX2(0.275899379));
		    tmp3 = MULTIPLY(d3, FIX(1.175875602));
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 == 0, d7 == 0 */
		    tmp0 = MULTIPLY(d1, FIX2(0.275899379));
		    tmp1 = MULTIPLY(d1, FIX2(0.785694958));
		    tmp2 = MULTIPLY(d1, FIX(1.175875602));
		    tmp3 = MULTIPLY(d1, FIX2(1.387039845));
		} else {
		    /* d1 == 0, d3 == 0, d5 == 0, d7 == 0 */
		    tmp0 = tmp1 = tmp2 = tmp3 = 0;
		}
	    }
	}
    }

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    dataptr[0] = (DCTELEM) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = (DCTELEM) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (DCTELEM) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = (DCTELEM) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = (DCTELEM) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = (DCTELEM) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = (DCTELEM) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = (DCTELEM) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += DCTSIZE;		/* advance pointer to next row */
  }

  /* Pass 2: process columns. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  dataptr = data;
  for (rowctr = DCTSIZE-1; rowctr >= 0; rowctr--) {
    /* Columns of zeroes can be exploited in the same way as we did with rows.
     * However, the row calculation has created many nonzero AC terms, so the
     * simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */

    d0 = dataptr[DCTSIZE*0];
    d1 = dataptr[DCTSIZE*1];
    d2 = dataptr[DCTSIZE*2];
    d3 = dataptr[DCTSIZE*3];
    d4 = dataptr[DCTSIZE*4];
    d5 = dataptr[DCTSIZE*5];
    d6 = dataptr[DCTSIZE*6];
    d7 = dataptr[DCTSIZE*7];

    /* Even part: reverse the even part of the forward DCT. */
    /* The rotator is sqrt(2)*c(-6). */
    if (d6) {
	if (d4) {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 != 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp0 = (d0 + d4) << CONST_BITS;
		    tmp1 = (d0 - d4) << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp1 + tmp2;
		    tmp12 = tmp1 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 != 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp0 = d4 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp2 - tmp0;
		    tmp12 = -(tmp0 + tmp2);
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 != 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp0 = (d0 + d4) << CONST_BITS;
		    tmp1 = (d0 - d4) << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp1 + tmp2;
		    tmp12 = tmp1 - tmp2;
		} else {
		    /* d0 == 0, d2 == 0, d4 != 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, -FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp0 = d4 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp2 - tmp0;
		    tmp12 = -(tmp0 + tmp2);
		}
	    }
	} else {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 == 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp0 = d0 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp0 + tmp2;
		    tmp12 = tmp0 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 == 0, d6 != 0 */
		    z1 = MULTIPLY(d2 + d6, FIX(0.541196100));
		    tmp2 = z1 + MULTIPLY(d6, - FIX(1.847759065));
		    tmp3 = z1 + MULTIPLY(d2, FIX(0.765366865));

		    tmp10 = tmp3;
		    tmp13 = -tmp3;
		    tmp11 = tmp2;
		    tmp12 = -tmp2;
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 == 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp0 = d0 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp0 + tmp2;
		    tmp12 = tmp0 - tmp2;
		} else {
		    /* d0 == 0, d2 == 0, d4 == 0, d6 != 0 */
		    tmp2 = MULTIPLY(d6, - FIX2(1.306562965));
		    tmp3 = MULTIPLY(d6, FIX(0.541196100));

		    tmp10 = tmp3;
		    tmp13 = -tmp3;
		    tmp11 = tmp2;
		    tmp12 = -tmp2;
		}
	    }
	}
    } else {
	if (d4) {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 != 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp0 = (d0 + d4) << CONST_BITS;
		    tmp1 = (d0 - d4) << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp1 + tmp2;
		    tmp12 = tmp1 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 != 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp0 = d4 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp2 - tmp0;
		    tmp12 = -(tmp0 + tmp2);
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 != 0, d6 == 0 */
		    tmp10 = tmp13 = (d0 + d4) << CONST_BITS;
		    tmp11 = tmp12 = (d0 - d4) << CONST_BITS;
		} else {
		    /* d0 == 0, d2 == 0, d4 != 0, d6 == 0 */
		    tmp10 = tmp13 = d4 << CONST_BITS;
		    tmp11 = tmp12 = -tmp10;
		}
	    }
	} else {
	    if (d2) {
		if (d0) {
		    /* d0 != 0, d2 != 0, d4 == 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp0 = d0 << CONST_BITS;

		    tmp10 = tmp0 + tmp3;
		    tmp13 = tmp0 - tmp3;
		    tmp11 = tmp0 + tmp2;
		    tmp12 = tmp0 - tmp2;
		} else {
		    /* d0 == 0, d2 != 0, d4 == 0, d6 == 0 */
		    tmp2 = MULTIPLY(d2, FIX(0.541196100));
		    tmp3 = (int)MULTIPLY(d2, (FIX(1.306562965) + .5));

		    tmp10 = tmp3;
		    tmp13 = -tmp3;
		    tmp11 = tmp2;
		    tmp12 = -tmp2;
		}
	    } else {
		if (d0) {
		    /* d0 != 0, d2 == 0, d4 == 0, d6 == 0 */
		    tmp10 = tmp13 = tmp11 = tmp12 = d0 << CONST_BITS;
		} else {
		    /* d0 == 0, d2 == 0, d4 == 0, d6 == 0 */
		    tmp10 = tmp13 = tmp11 = tmp12 = 0;
		}
	    }
	}
    }

    /* Odd part per figure 8; the matrix is unitary and hence its
     * transpose is its inverse.  i0..i3 are y7,y5,y3,y1 respectively.
     */
    if (d7) {
	if (d5) {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 != 0, d7 != 0 */
		    z1 = d7 + d1;
		    z2 = d5 + d3;
		    z3 = d7 + d3;
		    z4 = d5 + d1;
		    z5 = MULTIPLY(z3 + z4, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(z1, - FIX(0.899976223));
		    z2 = MULTIPLY(z2, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX(1.961570560));
		    z4 = MULTIPLY(z4, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 != 0, d5 != 0, d7 != 0 */
		    z2 = d5 + d3;
		    z3 = d7 + d3;
		    z5 = MULTIPLY(z3 + d5, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    z1 = MULTIPLY(d7, - FIX(0.899976223));
		    z2 = MULTIPLY(z2, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX(1.961570560));
		    z4 = MULTIPLY(d5, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 = z1 + z4;
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 != 0, d7 != 0 */
		    z1 = d7 + d1;
		    z4 = d5 + d1;
		    z5 = MULTIPLY(d7 + z4, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(z1, - FIX(0.899976223));
		    z2 = MULTIPLY(d5, - FIX(2.562915447));
		    z3 = MULTIPLY(d7, - FIX(1.961570560));
		    z4 = MULTIPLY(z4, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 = z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 == 0, d5 != 0, d7 != 0 */
		    z5 = MULTIPLY(d5 + d7, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, - FIX2(0.601344887));
		    tmp1 = MULTIPLY(d5, - FIX2(0.509795578));
		    z1 = MULTIPLY(d7, - FIX(0.899976223));
		    z3 = MULTIPLY(d7, - FIX(1.961570560));
		    z2 = MULTIPLY(d5, - FIX(2.562915447));
		    z4 = MULTIPLY(d5, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z3;
		    tmp1 += z4;
		    tmp2 = z2 + z3;
		    tmp3 = z1 + z4;
		}
	    }
	} else {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 == 0, d7 != 0 */
		    z1 = d7 + d1;
		    z3 = d7 + d3;
		    z5 = MULTIPLY(z3 + d1, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, FIX(0.298631336));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(z1, - FIX(0.899976223));
		    z2 = MULTIPLY(d3, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX(1.961570560));
		    z4 = MULTIPLY(d1, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 += z1 + z3;
		    tmp1 = z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 != 0, d5 == 0, d7 != 0 */
		    z3 = d7 + d3;
		    z5 = MULTIPLY(z3, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, - FIX2(0.601344887));
		    z1 = MULTIPLY(d7, - FIX(0.899976223));
		    tmp2 = MULTIPLY(d3, FIX(0.509795579));
		    z2 = MULTIPLY(d3, - FIX(2.562915447));
		    z3 = MULTIPLY(z3, - FIX2(0.785694958));

		    tmp0 += z3;
		    tmp1 = z2 + z5;
		    tmp2 += z3;
		    tmp3 = z1 + z5;
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 == 0, d7 != 0 */
		    z1 = d7 + d1;
		    z5 = MULTIPLY(z1, FIX(1.175875602));

		    tmp0 = MULTIPLY(d7, - FIX2(1.662939224));
		    tmp3 = MULTIPLY(d1, FIX2(1.111140466));
		    z1 = MULTIPLY(z1, FIX2(0.275899379));
		    z3 = MULTIPLY(d7, - FIX(1.961570560));
		    z4 = MULTIPLY(d1, - FIX(0.390180644));

		    tmp0 += z1;
		    tmp1 = z4 + z5;
		    tmp2 = z3 + z5;
		    tmp3 += z1;
		} else {
		    /* d1 == 0, d3 == 0, d5 == 0, d7 != 0 */
		    tmp0 = MULTIPLY(d7, - FIX2(1.387039845));
		    tmp1 = MULTIPLY(d7, FIX(1.175875602));
		    tmp2 = MULTIPLY(d7, - FIX2(0.785694958));
		    tmp3 = MULTIPLY(d7, FIX2(0.275899379));
		}
	    }
	}
    } else {
	if (d5) {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 != 0, d7 == 0 */
		    z2 = d5 + d3;
		    z4 = d5 + d1;
		    z5 = MULTIPLY(d3 + z4, FIX(1.175875602));

		    tmp1 = MULTIPLY(d5, FIX(2.053119869));
		    tmp2 = MULTIPLY(d3, FIX(3.072711026));
		    tmp3 = MULTIPLY(d1, FIX(1.501321110));
		    z1 = MULTIPLY(d1, - FIX(0.899976223));
		    z2 = MULTIPLY(z2, - FIX(2.562915447));
		    z3 = MULTIPLY(d3, - FIX(1.961570560));
		    z4 = MULTIPLY(z4, - FIX(0.390180644));

		    z3 += z5;
		    z4 += z5;

		    tmp0 = z1 + z3;
		    tmp1 += z2 + z4;
		    tmp2 += z2 + z3;
		    tmp3 += z1 + z4;
		} else {
		    /* d1 == 0, d3 != 0, d5 != 0, d7 == 0 */
		    z2 = d5 + d3;
		    z5 = MULTIPLY(z2, FIX(1.175875602));

		    tmp1 = MULTIPLY(d5, FIX2(1.662939225));
		    tmp2 = MULTIPLY(d3, FIX2(1.111140466));
		    z2 = MULTIPLY(z2, - FIX2(1.387039845));
		    z3 = MULTIPLY(d3, - FIX(1.961570560));
		    z4 = MULTIPLY(d5, - FIX(0.390180644));

		    tmp0 = z3 + z5;
		    tmp1 += z2;
		    tmp2 += z2;
		    tmp3 = z4 + z5;
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 != 0, d7 == 0 */
		    z4 = d5 + d1;
		    z5 = MULTIPLY(z4, FIX(1.175875602));

		    tmp1 = MULTIPLY(d5, - FIX2(0.509795578));
		    tmp3 = MULTIPLY(d1, FIX2(0.601344887));
		    z1 = MULTIPLY(d1, - FIX(0.899976223));
		    z2 = MULTIPLY(d5, - FIX(2.562915447));
		    z4 = MULTIPLY(z4, FIX2(0.785694958));

		    tmp0 = z1 + z5;
		    tmp1 += z4;
		    tmp2 = z2 + z5;
		    tmp3 += z4;
		} else {
		    /* d1 == 0, d3 == 0, d5 != 0, d7 == 0 */
		    tmp0 = MULTIPLY(d5, FIX(1.175875602));
		    tmp1 = MULTIPLY(d5, FIX2(0.275899380));
		    tmp2 = MULTIPLY(d5, - FIX2(1.387039845));
		    tmp3 = MULTIPLY(d5, FIX2(0.785694958));
		}
	    }
	} else {
	    if (d3) {
		if (d1) {
		    /* d1 != 0, d3 != 0, d5 == 0, d7 == 0 */
		    z5 = d3 + d1;

		    tmp2 = MULTIPLY(d3, - FIX(1.451774981));
		    tmp3 = MULTIPLY(d1, (FIX(0.211164243) - 1));
		    z1 = MULTIPLY(d1, FIX(1.061594337));
		    z2 = MULTIPLY(d3, - FIX(2.172734803));
		    z4 = MULTIPLY(z5, FIX(0.785694958));
		    z5 = MULTIPLY(z5, FIX(1.175875602));

		    tmp0 = z1 - z4;
		    tmp1 = z2 + z4;
		    tmp2 += z5;
		    tmp3 += z5;
		} else {
		    /* d1 == 0, d3 != 0, d5 == 0, d7 == 0 */
		    tmp0 = MULTIPLY(d3, - FIX2(0.785694958));
		    tmp1 = MULTIPLY(d3, - FIX2(1.387039845));
		    tmp2 = MULTIPLY(d3, - FIX2(0.275899379));
		    tmp3 = MULTIPLY(d3, FIX(1.175875602));
		}
	    } else {
		if (d1) {
		    /* d1 != 0, d3 == 0, d5 == 0, d7 == 0 */
		    tmp0 = MULTIPLY(d1, FIX2(0.275899379));
		    tmp1 = MULTIPLY(d1, FIX2(0.785694958));
		    tmp2 = MULTIPLY(d1, FIX(1.175875602));
		    tmp3 = MULTIPLY(d1, FIX2(1.387039845));
		} else {
		    /* d1 == 0, d3 == 0, d5 == 0, d7 == 0 */
		    tmp0 = tmp1 = tmp2 = tmp3 = 0;
		}
	    }
	}
    }

    /* Final output stage: inputs are tmp10..tmp13, tmp0..tmp3 */

    dataptr[DCTSIZE*0] = (DCTELEM) DESCALE(tmp10 + tmp3,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*7] = (DCTELEM) DESCALE(tmp10 - tmp3,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*1] = (DCTELEM) DESCALE(tmp11 + tmp2,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*6] = (DCTELEM) DESCALE(tmp11 - tmp2,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*2] = (DCTELEM) DESCALE(tmp12 + tmp1,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*5] = (DCTELEM) DESCALE(tmp12 - tmp1,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*3] = (DCTELEM) DESCALE(tmp13 + tmp0,
					   CONST_BITS+PASS1_BITS+3);
    dataptr[DCTSIZE*4] = (DCTELEM) DESCALE(tmp13 - tmp0,
					   CONST_BITS+PASS1_BITS+3);

    dataptr++;			/* advance pointer to next column */
  }
}


/*
 *--------------------------------------------------------------
 *
 * ComputeVector --
 *
 *	Computes motion vector given parameters previously parsed
 *      and reconstructed.
 *
 * Results:
 *      Reconstructed motion vector info is put into recon_* parameters
 *      passed to this function. Also updated previous motion vector
 *      information.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

#define ComputeVector(recon_right_ptr, recon_down_ptr, recon_right_prev, recon_down_prev, f, full_pel_vector, motion_h_code, motion_v_code, motion_h_r, motion_v_r)				\
									\
{									\
  int comp_h_r, comp_v_r;						\
  int right_little, right_big, down_little, down_big;			\
  int max, min, new_vector;						\
									\
  /* The following procedure for the reconstruction of motion vectors 	\
     is a direct and simple implementation of the instructions given	\
     in the mpeg December 1991 standard draft. 				\
  */									\
									\
  if (f == 1 || motion_h_code == 0)					\
    comp_h_r = 0;							\
  else 									\
    comp_h_r = f - 1 - motion_h_r;					\
									\
  if (f == 1 || motion_v_code == 0)					\
    comp_v_r = 0;							\
  else 									\
    comp_v_r = f - 1 - motion_v_r;					\
									\
  right_little = motion_h_code * f;					\
  if (right_little == 0)						\
    right_big = 0;							\
  else {								\
    if (right_little > 0) {						\
      right_little = right_little - comp_h_r;				\
      right_big = right_little - 32 * f;				\
    }									\
    else {								\
      right_little = right_little + comp_h_r;				\
      right_big = right_little + 32 * f;				\
    }									\
  }									\
									\
  down_little = motion_v_code * f;					\
  if (down_little == 0)							\
    down_big = 0;							\
  else {								\
    if (down_little > 0) {						\
      down_little = down_little - comp_v_r;				\
      down_big = down_little - 32 * f;					\
    }									\
    else {								\
      down_little = down_little + comp_v_r;				\
      down_big = down_little + 32 * f;					\
    }									\
  }									\
  									\
  max = 16 * f - 1;							\
  min = -16 * f;							\
									\
  new_vector = recon_right_prev + right_little;				\
									\
  if (new_vector <= max && new_vector >= min)				\
    *recon_right_ptr = recon_right_prev + right_little;			\
                      /* just new_vector */				\
  else									\
    *recon_right_ptr = recon_right_prev + right_big;			\
  recon_right_prev = *recon_right_ptr;					\
  if (full_pel_vector)							\
    *recon_right_ptr = *recon_right_ptr << 1;				\
									\
  new_vector = recon_down_prev + down_little;				\
  if (new_vector <= max && new_vector >= min)				\
    *recon_down_ptr = recon_down_prev + down_little;			\
                      /* just new_vector */				\
  else									\
    *recon_down_ptr = recon_down_prev + down_big;			\
  recon_down_prev = *recon_down_ptr;					\
  if (full_pel_vector)							\
    *recon_down_ptr = *recon_down_ptr << 1;				\
}


/*
 *--------------------------------------------------------------
 *
 * ComputeForwVector --
 *
 *	Computes forward motion vector by calling ComputeVector
 *      with appropriate parameters.
 *
 * Results:
 *	Reconstructed motion vector placed in recon_right_for_ptr and
 *      recon_down_for_ptr.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

void
ComputeForwVector(
     int *recon_right_for_ptr,
     int *recon_down_for_ptr,
     mpeg_VidStream *the_stream)
{

  Pict *picture;
  Macroblock *mblock;

  picture = &(the_stream->picture);
  mblock = &(the_stream->mblock);

  ComputeVector(recon_right_for_ptr, recon_down_for_ptr,
		mblock->recon_right_for_prev,
		mblock->recon_down_for_prev,
		(int) picture->forw_f,
		picture->full_pel_forw_vector,
		mblock->motion_h_forw_code, mblock->motion_v_forw_code,
		mblock->motion_h_forw_r, mblock->motion_v_forw_r);
}


/*
 *--------------------------------------------------------------
 *
 * ComputeBackVector --
 *
 *	Computes backward motion vector by calling ComputeVector
 *      with appropriate parameters.
 *
 * Results:
 *	Reconstructed motion vector placed in recon_right_back_ptr and
 *      recon_down_back_ptr.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

void
ComputeBackVector(
     int *recon_right_back_ptr,
     int *recon_down_back_ptr,
     mpeg_VidStream *the_stream)
{
  Pict *picture;
  Macroblock *mblock;

  picture = &(the_stream->picture);
  mblock = &(the_stream->mblock);

  ComputeVector(recon_right_back_ptr, recon_down_back_ptr,
		mblock->recon_right_back_prev,
		mblock->recon_down_back_prev,
		(int) picture->back_f,
		picture->full_pel_back_vector,
		mblock->motion_h_back_code, mblock->motion_v_back_code,
		mblock->motion_h_back_r, mblock->motion_v_back_r);

}



/* Silly Constants.... */
#define PACK_START_CODE             ((unsigned int)0x000001ba)
#define SYSTEM_HEADER_START_CODE    ((unsigned int)0x000001bb)
#define PACKET_START_CODE_MASK      ((unsigned int)0xffffff00)
#define PACKET_START_CODE_PREFIX    ((unsigned int)0x00000100)
#define ISO_11172_END_CODE          ((unsigned int)0x000001b9)

#define PACK_HEADER_SIZE 8

#define STD_AUDIO_STREAM_ID ((unsigned char) 0xb8)
#define STD_VIDEO_STREAM_ID ((unsigned char) 0xb9)
#define MIN_STREAM_ID_ID    ((unsigned char) 0xbc)
#define RESERVED_STREAM_ID  ((unsigned char) 0xbc)
#define PRIVATE_STREAM_1_ID ((unsigned char) 0xbd)
#define PADDING_STREAM_ID   ((unsigned char) 0xbe)
#define PRIVATE_STREAM_2_ID ((unsigned char) 0xbf)

#define STD_SYSTEM_CLOCK_FREQ (unsigned long)90000
#define MUX_RATE_SCALE_FACTOR 50
#define MAX_STREAMS 8
#define NOT_PACKET_ID       ((unsigned char) 0xff)
#define KILL_BUFFER         ((unsigned char) 0xfe)


/*
 *--------------------------------------------------------------
 *
 * get_more_data --
 *
 *	Called by get_more_data to read in more data from
 *      video MPG files (non-system-layer)
 *
 * Results:
 *	Input buffer updated, buffer length updated.
 *      Returns 1 if data read, 0 if EOF, -1 if error.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */
int
get_more_data(
      mpeg_VidStream *vid_stream)
{
  unsigned int **bs_ptr=&vid_stream->buf_start;
  int *max_length=&vid_stream->max_buf_length;
  int *length_ptr=&vid_stream->buf_length;
  unsigned int **buf_ptr=&vid_stream->buffer;
  int ioBytes, data, result;
  unsigned char byte;
  unsigned int *mark;
  int sys_layer= vid_stream->sys_layer;

  if (sys_layer == 0) {
    return pure_get_more_data(*bs_ptr,
			      *max_length,
			       length_ptr,
			       buf_ptr,
			       vid_stream);
  }

  if (sys_layer == -1) {
    /* Time to init ourselves */
    vid_stream->swap = (htonl(1) != 1);
    mark = *bs_ptr;
    ioBytes = fread(&data, 1, 4, vid_stream->input);

    if (ioBytes != 4) {
	  return 0;
    }

    data = ntohl(data);
    if ( (data == PACK_START_CODE) || (data == SYSTEM_HEADER_START_CODE) ) {
    got_sys:
      /* Yow, a System Layer Stream.  Much harder to parse.  Call in the
	     specialist.... */
      /* fprintf(stderr,"This is an MPEG System Layer Stream.  ");*/
      /* fprintf(stderr,"Audio is not played.\n");*/
      vid_stream->sys_layer = 1;
      result = read_sys(vid_stream,(unsigned int) data);
      return result;
    } else if (data ==  SEQ_START_CODE) {
    got_seq:
      /* No system Layer junk, just pretent we didn't peek,
	     and hereafter just call pure_get_more_data */
      vid_stream->sys_layer = 0;
      **bs_ptr = data;
      *length_ptr = 1;
      result = pure_get_more_data(*bs_ptr, *max_length,
				 length_ptr, buf_ptr, vid_stream);
      *buf_ptr = *bs_ptr;
      return result;
    } else {
      int state;

      fprintf(stderr, "Junk at start of stream, searching for start code\n");
      state = 0;
      while (TRUE) {
	if ((ioBytes = fread(&byte, 1, 1, vid_stream->input)) != 1) return 0;
	if (byte == 0) {
	  if (state < 2) state++;
	} else if ((byte == 1) && (state == 2)) {
	  state++;
	} else {
	  state = 0;
	}
	if (state == 3) {
	  if ((ioBytes = fread(&byte, 1, 1, vid_stream->input)) != 1) return 0;
	  data = ((unsigned int) byte + 0x100);
	  switch (data) {
	  case SEQ_START_CODE:
	    goto got_seq;
	  case PACK_START_CODE:
	  case SYSTEM_HEADER_START_CODE:
	    goto got_sys;
	  default:
	    /* keep looking */
	    state=0;
	  }
	}
      }}
  }

  /* A system layer stream (called after the 1st time), call the specialist */
  result = read_sys(vid_stream,0);
  return result;
}


/*
 *-------------------------------------------------------------
 *
 * clear_data_stream
 *
 * Empties out internal buffers
 *
 *-------------------------------------------------------------
 */
void
  clear_data_stream(
     mpeg_VidStream *vid_stream)
{
  /* Only internal buffer is in ReadPacket */
  if (vid_stream->sys_layer) {
    ReadPacket(KILL_BUFFER, vid_stream);
  }
}

/*
 *--------------------------------------------------------------
 *
 * pure_get_more_data --
 *      (get_more_data from ver 2.0 with swap added)
 *
 *	Called by get_more_data to read in more data from
 *      video MPG files (non-system-layer)
 *
 * Results:
 *	Input buffer updated, buffer length updated.
 *      Returns 1 if data read, 0 if EOF, -1 if error.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

int
pure_get_more_data(
     unsigned int *buf_start,
     int max_length,
     int *length_ptr,
     unsigned int **buf_ptr,
     mpeg_VidStream *vid_stream)
{

  int length, num_read, i;
  unsigned int request;
  unsigned char *buffer, *mark;
  unsigned int *lmark;
  int swap=vid_stream->swap;

  if (vid_stream->EOF_flag) return 0;

  length = *length_ptr;
  buffer = (unsigned char *) *buf_ptr;

  if (length > 0) {
	  /*   a known problem - can overwrite - JAS*/
	  /* */
	  if ((unsigned char *)buf_start != buffer) {
    		memcpy((unsigned char *) buf_start, buffer, (unsigned int) (length*4));
	  }
    mark = ((unsigned char *) (buf_start + length));
  }
  else {
    mark = (unsigned char *) buf_start;
    length = 0;
  }

  request = (max_length-length)*4;


  num_read = fread(mark, 1, request, vid_stream->input);

  /* Paulo Villegas - 26/1/1993: Correction for 4-byte alignment */
  {
    int num_read_rounded;
    unsigned char *index;

    num_read_rounded = 4*(num_read/4);

    /* this can happen only if num_read<request; i.e. end of file reached */
    if ( num_read_rounded < num_read ) {
 	  num_read_rounded = 4*( num_read/4+1 );

 	    /* fill in with zeros */
 	  for( index=mark+num_read; index<mark+num_read_rounded; *(index++)=0 );

 	  /* advance to the next 4-byte boundary */
 	  num_read = num_read_rounded;
    }
  }

  if (num_read < 0) {
    return -1;
  } else if (num_read == 0) {
    *buf_ptr = buf_start;

    /* Make 32 bits after end equal to 0 and 32
     * bits after that equal to seq end code
     * in order to prevent messy data from infinite
     * recursion.
     */

    *(buf_start + length) = 0x0;
    *(buf_start + length+1) = SEQ_END_CODE;

    vid_stream->EOF_flag = 1;
    return 0;
  }

  lmark = (unsigned int *) mark;

  num_read = num_read/4;

  if (swap) {
    for (i = 0; i < num_read; i++) {
      *lmark = htonl(*lmark);
      lmark++;
    }
  }

  *buf_ptr = buf_start;
  *length_ptr = length + num_read;

  return 1;
}

/*
 *----------------------------------------------------------
 *
 *  read_sys
 *
 *      Parse out a packet of the system layer MPEG file.
 *
 *  Results:  Returns 0 if error or EOF
 *            Returns 1 if more data read (could be just one int)
 *
 *  Side Effects:  ReadPacket can change *bs_ptr to be a new buffer
 *                 buf_ptr will remain pointing at *length_ptr (at input)
 *                         into the buffer
 *                 *length_ptr will be changed to the new size
 *                 *max_length can be changed if a new buffer is alloc'd
 *
 *----------------------------------------------------------
 */
int read_sys(
     mpeg_VidStream *vid_stream,
     unsigned int start)
     /* start is either a start code or 0 to indicate continued parsing */
{
  unsigned int startCode;
  int errorCode, PacketReply;
  unsigned char packetID;
  double systemClockTime;
  unsigned long muxRate;
  /* Statistics */
  int match;

  if (!start) {
    errorCode = ReadStartCode(&startCode,vid_stream);
    if (vid_stream->EOF_flag) return 0;
    if (errorCode != 0) {
      fprintf(stderr, "Unable to read initial pack start code\n");
      return 0;
    }}
  else {
    errorCode = 0;
    startCode = start;
  }

  while (1) {
    match=FALSE;
    if (startCode == PACK_START_CODE) {
      match = TRUE;
      errorCode = ReadPackHeader( &systemClockTime, &muxRate, vid_stream);
      if (errorCode != 0) {
        fprintf(stderr, "Error in reading pack header\n");
        return 0;
      }
      errorCode = ReadStartCode( &startCode, vid_stream );
      if (errorCode != 0) {
        fprintf(stderr, "Error in reading start code\n");
        return 0;
      }
    }
    if (startCode == SYSTEM_HEADER_START_CODE) {
      match = TRUE;
      errorCode = ReadSystemHeader(vid_stream);
      if (errorCode != 0) {
        fprintf(stderr, "Error in reading system header\n");
        return 0;
      }
      errorCode = ReadStartCode( &startCode, vid_stream );
      if (errorCode != 0) {
        fprintf(stderr,"Error in reading start code after system header\n");
        return 0;
      }
    }
    packetID = startCode & 0xff;
    while (((startCode & PACKET_START_CODE_MASK) == PACKET_START_CODE_PREFIX) &&
	   (packetID >= 0xbc)) {
      match = TRUE;
      packetID = startCode & 0xff;
      PacketReply = ReadPacket(packetID, vid_stream);
      switch (PacketReply) {
      case 2:
        return 1;
      case 1:
        return 0;
      default: /* do nothing */
        break;
      }
      errorCode = ReadStartCode( &startCode, vid_stream );
      if (errorCode != 0) {
        fprintf(stderr,"Error in start code after packet\n");
        return 0;
      }
      if (startCode == PACK_START_CODE || startCode == ISO_11172_END_CODE) {
        break;
      }
    }

    if (startCode == ISO_11172_END_CODE) {
      match = TRUE;
      if (vid_stream->Parse_done) {
	return 1;
      }
      ReadPacket(NOT_PACKET_ID, vid_stream);
      vid_stream->Parse_done = TRUE;
      return 1;
    }
    if (errorCode != 0)
      return 1;
    if (! match) {
      fprintf(stderr,"\nNo match found for start code %08x in system layer, skipping\n",startCode);
      startCode = find_start_code(vid_stream->input);
      if (startCode == EOF) {
        vid_stream->EOF_flag = 1;
        return 0;
      }
    }
  }
}


/*
 *-----------------------------------------------------------
 *
 *  ReadStartCode
 *
 *      Parses a start code out of the stream
 *
 *  Results/Side Effects:  Sets *startCode to the code, returns
 *     1 on error, 0 on success
 *
 *-----------------------------------------------------------
 */
int ReadStartCode(
     unsigned int *startCode,
     mpeg_VidStream *vid_stream)
{
  int numRead;

  numRead = fread((unsigned char *)startCode, 1, 4, vid_stream->input);
  *startCode = htonl(*startCode);

  if (numRead < 4) {
    vid_stream->EOF_flag = 1;
    return 1;
  }

  if ((*startCode&0xfffffe00) != 0) {
    fprintf(stderr,"Problem with system layer parse, skipping to start code\n");
    *startCode = find_start_code(vid_stream->input);
    if (*startCode == EOF) {
       vid_stream->EOF_flag = TRUE;
      return 0;
    }
  }

  return 0;
}


/*
 *-----------------------------------------------------------
 *
 *  find_start_code
 *
 *      Parses a start code out of the stream by tossing bytes until it gets one
 *
 *  Results/Side Effects:  Parses bytes of the stream, returns code
 *                         Returns EOF in case of end of file
 *
 *-----------------------------------------------------------
 */
int find_start_code(
    FILE *input)
{
 NO_ZEROS:
  switch(fgetc(input)) {
  case 0:    goto ONE_ZERO;
  case EOF:  goto EOF_FOUND;
  default:   goto NO_ZEROS;
  }

 ONE_ZERO:
  switch(fgetc(input)) {
  case 0:    goto TWO_ZEROS;
  case EOF:  goto EOF_FOUND;
  default:   goto NO_ZEROS;
  }

 TWO_ZEROS:
  switch(fgetc(input)) {
  case 0x01:  goto CODE_FOUND;
  case 0x00:  goto TWO_ZEROS;
  case EOF:  goto EOF_FOUND;
  default:    goto NO_ZEROS;
  }

 CODE_FOUND:
  return 0x00000100+fgetc(input);

 EOF_FOUND:   /* received EOF */
  return EOF;
}




/*
 *-----------------------------------------------------------------
 *
 *  ReadPackHeader
 *
 *      Parses out the PACK header
 *
 *  Returns: 1 on error, 0 on success
 *
 *-------------------------------------------------------------------
 */
int ReadPackHeader(
     double *systemClockTime,
     unsigned long *muxRate,
     mpeg_VidStream *vid_stream)
{
  int numRead;
  unsigned char inputBuffer[PACK_HEADER_SIZE];
  unsigned long systemClockRef;
  unsigned char systemClockRefHiBit;
  int errorCode;

  numRead = fread(inputBuffer, 1, PACK_HEADER_SIZE, vid_stream->input);
  if (numRead < PACK_HEADER_SIZE) {
    vid_stream->EOF_flag = 1;
    return 1;
  }
  ReadTimeStamp(inputBuffer, &systemClockRefHiBit, &systemClockRef);
  errorCode = MakeFloatClockTime(systemClockRefHiBit, systemClockRef,
				 systemClockTime);
  ReadRate(&inputBuffer[5], muxRate);
  *muxRate *= MUX_RATE_SCALE_FACTOR;
  return 0;
}


/*
 *------------------------------------------------------------------
 *
 *   ReadSystemHeader
 *
 *      Parse out the system header, setup out stream IDs for parsing packets
 *
 *   Results:  Returns 1 on error, 0 on success.
 *             Sets gAudioStreamID and gVideoStreamID
 *
 *------------------------------------------------------------------
 */
int ReadSystemHeader(
   mpeg_VidStream *vid_stream)
{
  unsigned char *inputBuffer = NULL;
  int numRead;
  int pos;
  unsigned short headerSize;
  unsigned char streamID;

  numRead = fread((char *)&headerSize, 1, 2, vid_stream->input);
  headerSize = ntohs(headerSize);
  if (numRead != 2) {
    vid_stream->EOF_flag = 1;
    return 1;
  }
  inputBuffer = (unsigned char *) MALLOC((unsigned int) headerSize+1);
  if (inputBuffer == NULL) {
    return 1;
  }
  inputBuffer[headerSize]=0;
  numRead = fread(inputBuffer, 1, headerSize, vid_stream->input);
  /* Brown - get rid of Ansi C complaints */
  if (numRead < (int) headerSize) {
    vid_stream->EOF_flag = 1;
    return 1;
  }

  pos = 6;
  while ((inputBuffer[pos] & 0x80) == 0x80) {
    streamID = inputBuffer[pos];
    switch (streamID) {
    case STD_VIDEO_STREAM_ID:
      break;
    case STD_AUDIO_STREAM_ID:
      break;
    case RESERVED_STREAM_ID:
      break;
    case PADDING_STREAM_ID:
      break;
    case PRIVATE_STREAM_1_ID:
      break;
    case PRIVATE_STREAM_2_ID:
      break;
    default:
      if (streamID < MIN_STREAM_ID_ID) {
	return 1;
      }
      switch (streamID >> 4) {
      case 0xc:
      case 0xd:
	vid_stream->gAudioStreamID = streamID;
	break;
      case 0xe:
	if ((vid_stream->gVideoStreamID != 0) &&
	    (vid_stream->gVideoStreamID!=streamID)) {
	  break;
	}
	vid_stream->gVideoStreamID = streamID;
	break;
      case 0xf:
/*Brown - deglobalized gReservedStreamID */
	vid_stream->gReservedStreamID = streamID;
	break;
      }
      break;
    }
    pos += 3;
  }
  if (inputBuffer != NULL)
    FREE_IF_NZ (inputBuffer);
  return 0;
}


/*
 *-----------------------------------------------------------------
 *
 *  ReadPacket
 *
 *      Reads a single packet out of the stream, and puts it in the
 *      buffer if it is video.
 *
 *  Results:
 *      Changes the value of *length_ptr to be the new length (plus old)
 *      If the buffer is too small, can change *bs_ptr, *max_length, and
 *      buf_ptr to be correct for a newly allocated buffer.
 *
 *  State:
 *      The buffer is in ints, but the packets can be an arbitrary number
 *      of bytes, so leftover bytes are kept in the mpeg_VidStream variable and
 *      are added on the next call.
 *
 *-----------------------------------------------------------------
 */
#ifdef __STDC__
int ReadPacket(unsigned char packetID, mpeg_VidStream *vid_stream)
#else
int ReadPacket(
     unsigned char packetID,
     mpeg_VidStream *vid_stream)
#endif

     /* Returns:
	0 - no error, but not video packet we want
	1 - error
	2 - got video packet into buffer
	*/
{

	/* to get around compiler warnings, fread return val always caught */
	size_t fret;

  unsigned int **bs_ptr=&vid_stream->buf_start;
  int *max_length = &vid_stream->max_buf_length;
  int *length_ptr=&vid_stream->buf_length;
  unsigned int **buf_ptr=&vid_stream->buffer;
  int ioBytes;
  unsigned char nextByte;
  unsigned short packetLength;
  unsigned char *packetBuffer = NULL;
  int pos;
  int numStuffBytes = 0;
  unsigned int packetDataLength;
  int byte_length;
  unsigned char scratch[10];
  /* Leftovers from previous video packets */

  if (packetID == NOT_PACKET_ID) {
    /* Gross hack to handle unread bytes before end of stream */
    if (vid_stream->num_left != 0) {
      /* Sigh, deal with previous leftovers */
      *(*buf_ptr+*length_ptr) = vid_stream->leftover_bytes;
      *(*buf_ptr+*length_ptr+1) = ISO_11172_END_CODE;
      *length_ptr += 2;
    } else {
      *(*buf_ptr+*length_ptr) = ISO_11172_END_CODE;
      *length_ptr += 1;
    }
    return 1;
  } else if (packetID==KILL_BUFFER) {
    vid_stream->num_left=0;
    vid_stream->leftover_bytes=0;
    return 0;
  }

  ioBytes = fread(&packetLength, 1, 2, vid_stream->input);
  packetLength = htons(packetLength);
  if (ioBytes < 2) {
    return 1;
  }
  if (packetID == vid_stream->gAudioStreamID) {
  }
  else if (packetID == vid_stream->gVideoStreamID) {
  }
  else {
    switch (packetID) {
    case PADDING_STREAM_ID:
    case RESERVED_STREAM_ID:
    case PRIVATE_STREAM_1_ID:
    case PRIVATE_STREAM_2_ID:
      break;
    default:
      fprintf(stderr, "\nUnknown packet type encountered. P'bly audio? (%x) at %d\n",
	      packetID,(int) ftell(vid_stream->input));
    }
    if (packetID != vid_stream->gVideoStreamID) {/* changed by jim */
      fseek(vid_stream->input, packetLength, 1);
      return 0;
    }
  }

  fret = fread(&nextByte,1,1,vid_stream->input);
  pos = 0;
  while (nextByte & 0x80) {
    ++numStuffBytes;
    ++pos;
    fret = fread(&nextByte,1,1,vid_stream->input);
  }
  if ((nextByte >> 6) == 0x01) {
    pos += 2;
    fret = fread(&nextByte,1,1,vid_stream->input);
    fret = fread(&nextByte,1,1,vid_stream->input);
  }
  if ((nextByte >> 4) == 0x02) {
    scratch[0] = nextByte;                      /* jim */
    fret = fread(&scratch[1],1,4,vid_stream->input);   /* jim */
    fret = fread(&nextByte,1,1,vid_stream->input);
    pos += 5;
  }
  else if ((nextByte >> 4) == 0x03) {
    scratch[0] = nextByte;                      /* jim */
    fret = fread(&scratch[1],1,9,vid_stream->input);   /* jim */
    fret = fread(&nextByte,1,1,vid_stream->input);
    pos += 10;
  }
  else {
    fret = fread(&nextByte,1,1,vid_stream->input);
    pos += 1;
  }
  /* Read all the headers, now make room for packet */
  if (*bs_ptr + *max_length < *buf_ptr+ packetLength/4 + *length_ptr) {
     /* Brown - get rid of Ansi C complaints */
    if (*max_length - *length_ptr < (int) packetLength/4) {
      /* Buffer too small for a packet (plus whats there),
	   * time to enlarge it!
	   */
      unsigned int *old = *bs_ptr;
      *max_length = *length_ptr + packetLength/2;
      *bs_ptr=(unsigned int *)MALLOC(*max_length*4);
      if (*bs_ptr == NULL) {
        return 1;
      }
      memcpy((unsigned char *)*bs_ptr, *buf_ptr, (unsigned int) *length_ptr*4);
      FREE_IF_NZ (old);
      *buf_ptr = *bs_ptr;
    } else {
      memcpy((unsigned char *)*bs_ptr, *buf_ptr, (unsigned int) *length_ptr*4);
      *buf_ptr = *bs_ptr;
    }}
  byte_length = *length_ptr*4;
  if (vid_stream->num_left != 0) {
    /* Sigh, deal with previous leftovers */
    byte_length += vid_stream->num_left;
    *(*buf_ptr+*length_ptr) = vid_stream->leftover_bytes;
  }
  packetBuffer=((unsigned char *)*buf_ptr)+byte_length;
  packetDataLength = packetLength - pos;
  *packetBuffer++ = nextByte;
/* Brown - deglobalize gVideoStreamID */
  if (packetID == vid_stream->gVideoStreamID) {
    ioBytes = fread(packetBuffer, 1, packetDataLength-1, vid_stream->input);
    if (ioBytes != packetDataLength-1) {
      vid_stream->EOF_flag = 1;
      return 1;
    }
    if (1 != ntohl(1)) {
      unsigned int *mark = *buf_ptr+*length_ptr;
      unsigned int i;

      for (i=0; i < ((packetDataLength+
			 vid_stream->num_left)&0xfffffffc); i+=4) {
        *mark=ntohl(*mark);
        mark++;
      }
    }
    byte_length = byte_length + packetDataLength;
    vid_stream->num_left = byte_length % 4;
    *length_ptr = byte_length / 4;
    vid_stream->leftover_bytes = *(*buf_ptr + *length_ptr);
    return 2;
  }
  else if (packetID == vid_stream->gAudioStreamID) {
    packetBuffer = (unsigned char *)(*buf_ptr + *length_ptr + 1);
    fret = fread(packetBuffer, 1, packetDataLength - 1, vid_stream->input);
  }
  else /* Donno what it is, just nuke it */ {
    /* This code should be unreachable */
    packetBuffer = (unsigned char *)(*buf_ptr + *length_ptr + 1);
    fret = fread(packetBuffer, 1, packetDataLength - 1, vid_stream->input);
  }
  return 0;
}


/*
 * The remaining procedures are formatting utility procedures.
 */
void ReadTimeStamp(
     unsigned char *inputBuffer,
     unsigned char *hiBit,
     unsigned long *low4Bytes)
{
  *hiBit = ((unsigned long)inputBuffer[0] >> 3) & 0x01;
  *low4Bytes = (((unsigned long)inputBuffer[0] >> 1) & 0x03) << 30;
  *low4Bytes |= (unsigned long)inputBuffer[1] << 22;
  *low4Bytes |= ((unsigned long)inputBuffer[2] >> 1) << 15;
  *low4Bytes |= (unsigned long)inputBuffer[3] << 7;
  *low4Bytes |= ((unsigned long)inputBuffer[4]) >> 1;
}

void ReadSTD(
unsigned char *inputBuffer,
unsigned char *stdBufferScale,
unsigned long *stdBufferSize)
{
  /* Brown - get rid of ANSI C complaints */
  *stdBufferScale = ((int)(inputBuffer[0] & 0x20) >> 5);
  *stdBufferSize = ((unsigned long)inputBuffer[0] & 0x1f) << 8;
  *stdBufferSize |= (unsigned long)inputBuffer[1];
}


void ReadRate(
     unsigned char *inputBuffer,
     unsigned long *rate)
{
  *rate = (inputBuffer[0] & 0x7f) << 15;
  *rate |= inputBuffer[1] << 7;
  /* Brown - get rid of ANSI C complaints */
  *rate |= (int) (inputBuffer[2] & 0xfe) >> 1;
}

#define FLOAT_0x10000 (double)((unsigned long)1 << 16)

#ifdef __STDC__
int MakeFloatClockTime(unsigned char hiBit, unsigned long low4Bytes,
		       double * floatClockTime)
#else
int MakeFloatClockTime(
     unsigned char hiBit,
     unsigned long low4Bytes,
     double *floatClockTime)
#endif
{
  if (hiBit != 0 && hiBit != 1) {
    *floatClockTime = 0.0;
    return 1;
  }
  *floatClockTime
    = (double)hiBit*FLOAT_0x10000*FLOAT_0x10000 + (double)low4Bytes;
  *floatClockTime /= (double)STD_SYSTEM_CLOCK_FREQ;
  return 0;
}



/* Define buffer length. */
#define BUF_LENGTH 80000

#   define P(s) ()
void int_handler P((void ));
void bad_handler P((void ));

/* Global file pointer to incoming data. */
FILE *mpegfile;

/*
 *--------------------------------------------------------------
 *
 * main --
 *
 *        Parses command line, starts decoding and displaying.
 *
 * Results:
 *        None.
 *
 * Side effects:
 *        None.
 *
 *--------------------------------------------------------------
 */


void mpg_main(char *fname, int *x,int *y,int *depth,int *fc,void **ptr) {

  mpeg_VidStream *theStream;
  int ppm_width = -1,  ppm_height = -1, ppm_modulus = -1;

  /* save pointers */
  frameCount = fc;
  *frameCount = 1;
  ySize = y;
  xSize = x;
  *depth = 3;  /* always this depth... */
  dataPointer = NULL;

  theStream = NULL;

  fflush(stdout);
  mpegfile=fopen(fname, "r");

  if (mpegfile == NULL) {
    printf("Could not open MovieTexture file %s\n", fname);
    *frameCount = 0;
    return;
  }

  init_tables();

  InitColorDither(1);

  InitCrop();
    theStream = mpg_NewVidStream((unsigned int) BUF_LENGTH);
    theStream->ppm_width = ppm_width;
    theStream->ppm_height = ppm_height;
    theStream->ppm_modulus = ppm_modulus;
    theStream->input = mpegfile;
    theStream->filename = fname;
    theStream->matched_depth = 24;

    if (mpegVidRsrc(0, theStream, 1)==NULL) {
       /* stream has already been destroyed */
       printf("Skipping movie \"%s\" - not an MPEG stream\n",
	  fname);
      if (theStream!=NULL) {
	      printf ("theStream != NULL, destroying, part1\n");
	      Destroympeg_VidStream(theStream);
      }
    }

  /* Start time for each movie - do after windows are mapped */
     if (theStream != NULL) theStream->realTimeStart = ReadSysClock();
       if (theStream != NULL) {
         while (theStream->film_has_ended == FALSE) {
             mpegVidRsrc(0, theStream, 0);
         }
       }
  if (L_tab!=NULL)
       FREE_IF_NZ (L_tab);
    if (Cr_r_tab!=NULL)
       FREE_IF_NZ (Cr_r_tab);
    if (Cr_g_tab!=NULL)
       FREE_IF_NZ (Cr_g_tab);
    if (Cb_g_tab!=NULL)
       FREE_IF_NZ (Cb_g_tab);
    if (Cb_b_tab!=NULL)
       FREE_IF_NZ (Cb_b_tab);

    if (r_2_pix_alloc!=NULL)
       FREE_IF_NZ (r_2_pix_alloc);
    if (g_2_pix_alloc!=NULL)
       FREE_IF_NZ (g_2_pix_alloc);
    if (b_2_pix_alloc!=NULL)
       FREE_IF_NZ (b_2_pix_alloc);

    /* zero these in case we have another mpg file to do */
    L_tab = NULL; Cr_r_tab = NULL; Cr_g_tab = NULL; Cb_g_tab = NULL;Cb_b_tab = NULL;
    r_2_pix_alloc = NULL; g_2_pix_alloc= NULL;b_2_pix_alloc = NULL;

      fclose(mpegfile);
	*ptr =  dataPointer;

	/* take 1 from the frame count */
	*frameCount = (*frameCount) -1;

	/* tell the calling program what the frame count is */
	*fc = *frameCount;
}
#else
void mpg_main(char *fname, int *x,int *y,int *depth,int *fc,void **ptr) {
ConsoleMessage ("problem with mpeg files on OSX PPC Arch");
  *x=0; *y=0; *depth=0; 
  *fc = 0;
}
#endif

