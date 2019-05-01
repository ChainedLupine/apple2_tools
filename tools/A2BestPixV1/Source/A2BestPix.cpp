// A2BestPix.cpp : Defines the entry point for the console application.
//

// .bmp (140x192)	-> rgbToRgb_Stretched (stretchX, stretchY)				-> _1_eff.bmp (560x384) Effective Resolution
//					-> rgbToBlock -> blockToRgb_Actual (stretchY)			-> _1_act.bmp (560x384) Actual Resolution - Other methods
//					-> rgbToBlock -> blockToDhgr							-> _1.dhgr
//
// .bmp (560x192)	-> rgbToRgb_Stretched (stretchX, stretchY)				-> _2_img.bmp (560x384) Imaginary Resolution
//					-> rgbToBlock_Convert -> blockToRgb_Actual (stretchY)	-> _2_img.bmp (560x384) Actual Resolution - bmpEnhance method
//					-> rgbToBlock_Convert -> blockToDhgr					-> _2.dhgr
#define M_PI   3.14159265358979323846264338327950288

#include <stdio.h>
//#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
//#include <time.h>
//#include <assert.h>

#include <algorithm> /* For std::sort() */
#include <vector>
#include <map>       /* For associative container, std::map<> */

static unsigned baseaddr[192];
static unsigned char grbuf[65536];
static char nbuf[1024];

// 0 = no
// 1 = yes
int optNextBlockCalculate = 0;

// 0 = left to right
// 1 = right to left
// 2 = serpentine start from left
// 3 = serpentine start from right
int optProcessDir = 1;

// 0 = nextBlock equals first block from nextBlockList
// 1 = nextBlock equals current block
int optNextBlockGuess = 1;

// 0 = no
// 1 = yes
int optLimitToPrevOrNextBlock = 0;

// 0 = Closest block to the effective block list ie list of 16
// 1 = Closest block to the actual block list ie list of 159  
int optClosestBlock = 1;

// 0 = Closest colour compare - Less than
// 1 = Closest colour comapre - Less than or equal to  
int optClosestLessThan = 0;

// 1 = Euclidean distance
// 2 = 3-4-3
// 3 = ?
// 4 = CIE76
// 5 = Luminance-weighted RGB
// 6 = CIEDE2000
int optColourDifference = 5;

// 0 = tohgr						IIe
// 1 = ?							IIe
// 2 = AppleWin
// 3 = Wiki							IIe			preview IIe 1
// 4 = tohgr NTSC					IIe
// 5 - Kegs32 RGB								preview IIGS 2
// 6 - CiderPress RGB
// 7 - New AppleWin NTSC
// 8 - Jace NTSC					IIe
// 9 - Cybernesto-Munafo NTSC		IIe
// 10 - Pseudo Palette				IIe
// 11 - tohgr NTSC HGR				IIe	IIGS
// 12 - Test1							IIGS
// 13 - Test2
// 14 - Photo IIGS								preview IIGS 1
// 15 - Photo IIe					IIe			preview IIe 2
int optProcessPallet = 4;
int optPreviewPallet = 4; //(when optPreviewPalletEquProcessPallet == 0)

// 0 = no
// 1 = yes
int optPreviewPalletEquProcessPallet = 0;

int	aDoubleHiResBlock [16][16][16];	// index1 = ColourPrev, index2 = Colour, index3 = ColourNext, value = 0x0 to 0xFFFF (A2Colour for Bit1, A2Colour for Bit2, A2Colour for Bit3, A2Colour for Bit4)

int	aDoubleHiResBlockFrom [16][16] = {
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0000,0x1110,0x0000,0x1110,0x0000,0x1110,0x0000,0x1110,0x0000,0x1110,0x0000,0x1110,0x0000,0x1110,0x0000,0x1110,
    0x0000,0x3300,0x2200,0x3300,0x0000,0x3300,0x2200,0x3300,0x0000,0x3300,0x2200,0x3300,0x0000,0x3300,0x2200,0x3300,
    0x0000,0x3300,0x2200,0x3300,0x0000,0x3300,0x2200,0x3300,0x0000,0x3300,0x2200,0x3300,0x0000,0x3300,0x2200,0x3300,
    0x0400,0x5500,0x6400,0x7500,0x4400,0x5500,0x6400,0x7500,0x0400,0x5500,0x6400,0x7500,0x4400,0x5500,0x6400,0x7500,
    0x0500,0x5500,0x6500,0x7500,0x4500,0x5500,0x6500,0x7500,0x0500,0x5500,0x6500,0x7500,0x4500,0x5500,0x6500,0x7500,
    0x0600,0x7700,0x6600,0x7700,0x4600,0x7700,0x6600,0x7700,0x0600,0x7700,0x6600,0x7700,0x4600,0x7700,0x6600,0x7700,
    0x0700,0x7700,0x6700,0x7700,0x4700,0x7700,0x6700,0x7700,0x0700,0x7700,0x6700,0x7700,0x4700,0x7700,0x6700,0x7700,
    0x8000,0x9000,0xA000,0xB000,0x8000,0x9000,0xA000,0xB000,0x8000,0x9000,0xA000,0xB000,0x8000,0x9000,0xA000,0xB000,
    0x8990,0x9990,0xA990,0xB990,0x8990,0x9990,0xA990,0xB990,0x8990,0x9990,0xA990,0xB990,0x8990,0x9990,0xA990,0xB990,
    0xAAA0,0xBBA0,0xAAA0,0xBBA0,0xAAA0,0xBBA0,0xAAA0,0xBBA0,0xAAA0,0xBBA0,0xAAA0,0xBBA0,0xAAA0,0xBBA0,0xAAA0,0xBBA0,
    0xABB0,0xBBB0,0xABB0,0xBBB0,0xABB0,0xBBB0,0xABB0,0xBBB0,0xABB0,0xBBB0,0xABB0,0xBBB0,0xABB0,0xBBB0,0xABB0,0xBBB0,
    0xCC00,0xDD00,0xEC00,0xFD00,0xCC00,0xDD00,0xEC00,0xFD00,0xCC00,0xDD00,0xEC00,0xFD00,0xCC00,0xDD00,0xEC00,0xFD00,
    0xCDD0,0xDDD0,0xEDD0,0xFDD0,0xCDD0,0xDDD0,0xEDD0,0xFDD0,0xCDD0,0xDDD0,0xEDD0,0xFDD0,0xCDD0,0xDDD0,0xEDD0,0xFDD0,
    0xEEE0,0xFFE0,0xEEE0,0xFFE0,0xEEE0,0xFFE0,0xEEE0,0xFFE0,0xEEE0,0xFFE0,0xEEE0,0xFFE0,0xEEE0,0xFFE0,0xEEE0,0xFFE0,
    0xEFF0,0xFFF0,0xEFF0,0xFFF0,0xEFF0,0xFFF0,0xEFF0,0xFFF0,0xEFF0,0xFFF0,0xEFF0,0xFFF0,0xEFF0,0xFFF0,0xEFF0,0xFFF0
};

int	aDoubleHiResBlockTo [16][16] = {
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
    0x0001,0x0001,0x0001,0x0001,0x0005,0x0005,0x0005,0x0005,0x0009,0x0009,0x0009,0x0009,0x000D,0x000D,0x000D,0x000D,
    0x0020,0x0020,0x0022,0x0022,0x0026,0x0026,0x0026,0x0026,0x00AA,0x00AA,0x00AA,0x00AA,0x00AE,0x00AE,0x00AE,0x00AE,
    0x0033,0x0033,0x0033,0x0033,0x0037,0x0037,0x0037,0x0037,0x00BB,0x00BB,0x00BB,0x00BB,0x00BF,0x00BF,0x00BF,0x00BF,
    0x0000,0x0000,0x0000,0x0000,0x0044,0x0044,0x0044,0x0044,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,
    0x0055,0x0055,0x0055,0x0055,0x0055,0x0055,0x0055,0x0055,0x00DD,0x00DD,0x00DD,0x00DD,0x00DD,0x00DD,0x00DD,0x00DD,
    0x0060,0x0060,0x0062,0x0062,0x0066,0x0066,0x0066,0x0066,0x00EE,0x00EE,0x00EE,0x00EE,0x00EE,0x00EE,0x00EE,0x00EE,
    0x0077,0x0077,0x0077,0x0077,0x0077,0x0077,0x0077,0x0077,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,
    0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0888,0x0888,0x0888,0x0888,0x0888,0x0888,0x0888,0x0888,
    0x0001,0x0001,0x0001,0x0001,0x0005,0x0005,0x0005,0x0005,0x0009,0x0009,0x0009,0x0009,0x000D,0x000D,0x000D,0x000D,
    0x0000,0x0000,0x0002,0x0002,0x0006,0x0006,0x0006,0x0006,0x000A,0x000A,0x000A,0x000A,0x000E,0x000E,0x000E,0x000E,
    0x0003,0x0003,0x0003,0x0003,0x0007,0x0007,0x0007,0x0007,0x000B,0x000B,0x000B,0x000B,0x000F,0x000F,0x000F,0x000F,
    0x0000,0x0000,0x0000,0x0000,0x0044,0x0044,0x0044,0x0044,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,0x00CC,
    0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x0005,0x000D,0x000D,0x000D,0x000D,0x000D,0x000D,0x000D,0x000D,
    0x0000,0x0000,0x0002,0x0002,0x0006,0x0006,0x0006,0x0006,0x000E,0x000E,0x000E,0x000E,0x000E,0x000E,0x000E,0x000E,
    0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,0x0007,0x000F,0x000F,0x000F,0x000F,0x000F,0x000F,0x000F,0x000F,
};

struct BlockRelation {
	int pixelBlock, prevBlockList, currBlock, nextBlockList, matchAmount, valid;
};

struct BlockRelation actBlockList[159] = {
	{0x0000,0, 0, 0,0,0}, //000
	{0x0001,1, 1,11,0,0}, //001
	{0x0005,1, 1,12,0,0}, //002
	{0x0009,1, 1,13,0,0}, //003
	{0x000D,1, 1,14,0,0}, //004
	{0x1111,2, 1,11,0,0}, //005
	{0x1115,2, 1,12,0,0}, //006
	{0x1119,2, 1,13,0,0}, //007
	{0x111D,2, 1,14,0,0}, //008
	{0x0020,3, 2,15,0,0}, //009
	{0x0022,3, 2,16,0,0}, //010
	{0x0026,3, 2,12,0,0}, //011
	{0x00AA,3, 2,13,0,0}, //012
	{0x00AE,3, 2,14,0,0}, //013
	{0x2220,5, 2,15,0,0}, //014
	{0x2222,5, 2,16,0,0}, //015
	{0x2226,5, 2,12,0,0}, //016
	{0x22AA,5, 2,13,0,0}, //017
	{0x22AE,5, 2,14,0,0}, //018
	{0x3320,2, 2,15,0,0}, //019
	{0x3322,2, 2,16,0,0}, //020
	{0x3326,2, 2,12,0,0}, //021
	{0x33AA,2, 2,13,0,0}, //022
	{0x33AE,2, 2,14,0,0}, //023
	{0x0033,3, 3,11,0,0}, //024
	{0x0037,3, 3,12,0,0}, //025
	{0x00BB,3, 3,13,0,0}, //026
	{0x00BF,3, 3,14,0,0}, //027
	{0x2233,5, 3,11,0,0}, //028
	{0x2237,5, 3,12,0,0}, //029
	{0x22BB,5, 3,13,0,0}, //030
	{0x22BF,5, 3,14,0,0}, //031
	{0x3333,2, 3,11,0,0}, //032
	{0x3337,2, 3,12,0,0}, //033
	{0x33BB,2, 3,13,0,0}, //034
	{0x33BF,2, 3,14,0,0}, //035
	{0x0400,7, 4,11,0,0}, //036
	{0x0444,7, 4,12,0,0}, //037
	{0x04CC,7, 4,10,0,0}, //038
	{0x4400,8, 4,11,0,0}, //039
	{0x4444,8, 4,12,0,0}, //040
	{0x44CC,8, 4,10,0,0}, //041
	{0x5500,4, 4,11,0,0}, //042
	{0x5544,4, 4,12,0,0}, //043
	{0x55CC,4, 4,10,0,0}, //044
	{0x6400,5, 4,11,0,0}, //045
	{0x6444,5, 4,12,0,0}, //046
	{0x64CC,5, 4,10,0,0}, //047
	{0x7500,6, 4,11,0,0}, //048
	{0x7544,6, 4,12,0,0}, //049
	{0x75CC,6, 4,10,0,0}, //050
	{0x0555,7, 5, 9,0,0}, //051
	{0x05DD,7, 5,10,0,0}, //052
	{0x4555,8, 5, 9,0,0}, //053
	{0x45DD,8, 5,10,0,0}, //054
	{0x5555,4, 5, 9,0,0}, //055
	{0x55DD,4, 5,10,0,0}, //056
	{0x6555,5, 5, 9,0,0}, //057
	{0x65DD,5, 5,10,0,0}, //058
	{0x7555,6, 5, 9,0,0}, //059
	{0x75DD,6, 5,10,0,0}, //060
	{0x0660,7, 6,15,0,0}, //061
	{0x0662,7, 6,16,0,0}, //062
	{0x0666,7, 6,12,0,0}, //063
	{0x06EE,7, 6,10,0,0}, //064
	{0x4660,8, 6,15,0,0}, //065
	{0x4662,8, 6,16,0,0}, //066
	{0x4666,8, 6,12,0,0}, //067
	{0x46EE,8, 6,10,0,0}, //068
	{0x6660,5, 6,15,0,0}, //069
	{0x6662,5, 6,16,0,0}, //070
	{0x6666,5, 6,12,0,0}, //071
	{0x66EE,5, 6,10,0,0}, //072
	{0x7760,2, 6,15,0,0}, //073
	{0x7762,2, 6,16,0,0}, //074
	{0x7766,2, 6,12,0,0}, //075
	{0x77EE,2, 6,10,0,0}, //076
	{0x0777,7, 7, 9,0,0}, //077
	{0x07FF,7, 7,10,0,0}, //078
	{0x4777,8, 7, 9,0,0}, //079
	{0x47FF,8, 7,10,0,0}, //080
	{0x6777,5, 7, 9,0,0}, //081
	{0x67FF,5, 7,10,0,0}, //082
	{0x7777,2, 7, 9,0,0}, //083
	{0x77FF,2, 7,10,0,0}, //084
	{0x8000,3, 8, 9,0,0}, //085
	{0x8888,3, 8,10,0,0}, //086
	{0x9000,4, 8, 9,0,0}, //087
	{0x9888,4, 8,10,0,0}, //088
	{0xA000,5, 8, 9,0,0}, //089
	{0xA888,5, 8,10,0,0}, //090
	{0xB000,6, 8, 9,0,0}, //091
	{0xB888,6, 8,10,0,0}, //092
	{0x8991,3, 9,11,0,0}, //093
	{0x8995,3, 9,12,0,0}, //094
	{0x8999,3, 9,13,0,0}, //095
	{0x899D,3, 9,14,0,0}, //096
	{0x9991,4, 9,11,0,0}, //097
	{0x9995,4, 9,12,0,0}, //098
	{0x9999,4, 9,13,0,0}, //099
	{0x999D,4, 9,14,0,0}, //100
	{0xA991,5, 9,11,0,0}, //101
	{0xA995,5, 9,12,0,0}, //102
	{0xA999,5, 9,13,0,0}, //103
	{0xA99D,5, 9,14,0,0}, //104
	{0xB991,6, 9,11,0,0}, //105
	{0xB995,6, 9,12,0,0}, //106
	{0xB999,6, 9,13,0,0}, //107
	{0xB99D,6, 9,14,0,0}, //108
	{0xAAA0,1,10,15,0,0}, //109
	{0xAAA2,1,10,16,0,0}, //110
	{0xAAA6,1,10,12,0,0}, //111
	{0xAAAA,1,10,13,0,0}, //112
	{0xAAAE,1,10,14,0,0}, //113
	{0xBBA0,2,10,15,0,0}, //114
	{0xBBA2,2,10,16,0,0}, //115
	{0xBBA6,2,10,12,0,0}, //116
	{0xBBAA,2,10,13,0,0}, //117
	{0xBBAE,2,10,14,0,0}, //118
	{0xABB3,1,11,11,0,0}, //119
	{0xABB7,1,11,12,0,0}, //120
	{0xABBB,1,11,13,0,0}, //121
	{0xABBF,1,11,14,0,0}, //122
	{0xBBB3,2,11,11,0,0}, //123
	{0xBBB7,2,11,12,0,0}, //124
	{0xBBBB,2,11,13,0,0}, //125
	{0xBBBF,2,11,14,0,0}, //126
	{0xCC00,3,12,11,0,0}, //127
	{0xCC44,3,12,12,0,0}, //128
	{0xCCCC,3,12,10,0,0}, //129
	{0xDD00,4,12,11,0,0}, //130
	{0xDD44,4,12,12,0,0}, //131
	{0xDDCC,4,12,10,0,0}, //132
	{0xEC00,5,12,11,0,0}, //133
	{0xEC44,5,12,12,0,0}, //134
	{0xECCC,5,12,10,0,0}, //135
	{0xFD00,6,12,11,0,0}, //136
	{0xFD44,6,12,12,0,0}, //137
	{0xFDCC,6,12,10,0,0}, //138
	{0xCDD5,3,13, 9,0,0}, //139
	{0xCDDD,3,13,10,0,0}, //140
	{0xDDD5,4,13, 9,0,0}, //141
	{0xDDDD,4,13,10,0,0}, //142
	{0xEDD5,5,13, 9,0,0}, //143
	{0xEDDD,5,13,10,0,0}, //144
	{0xFDD5,6,13, 9,0,0}, //145
	{0xFDDD,6,13,10,0,0}, //146
	{0xEEE0,1,14,15,0,0}, //147
	{0xEEE2,1,14,16,0,0}, //148
	{0xEEE6,1,14,12,0,0}, //149
	{0xEEEE,1,14,10,0,0}, //150
	{0xFFE0,2,14,15,0,0}, //151
	{0xFFE2,2,14,16,0,0}, //152
	{0xFFE6,2,14,12,0,0}, //153
	{0xFFEE,2,14,10,0,0}, //154
	{0xEFF7,1,15, 9,0,0}, //155
	{0xEFFF,1,15,10,0,0}, //156
	{0xFFF7,2,15, 9,0,0}, //157
	{0xFFFF,2,15,10,0,0}  //158
};

int adjBlockList[17][8] = {
	{ 0,-1, 0, 0, 0, 0, 0, 0},	//black
	{ 0, 2, 4, 6, 8,10,12,14},	//prev 1
	{ 1, 3, 5, 7, 9,11,13,15},	//prev 2
	{ 0, 4, 8,12,-1, 0, 0, 0},	//prev 3
	{ 1, 5, 9,13,-1, 0, 0, 0},	//prev 4
	{ 2, 6,10,14,-1, 0, 0, 0},	//prev 5
	{ 3, 7,11,15,-1, 0, 0, 0},	//prev 6
	{ 0, 8,-1, 0, 0, 0, 0, 0},	//prev 7
	{ 4,12,-1, 0, 0, 0, 0, 0},	//prev 8
	{ 0, 1, 2, 3, 4, 5, 6, 7},	//next 9
	{ 8, 9,10,11,12,13,14,15},	//next 10
	{ 0, 1, 2, 3,-1, 0, 0, 0},	//next 11
	{ 4, 5, 6, 7,-1, 0, 0, 0},	//next 12
	{ 8, 9,10,11,-1, 0, 0, 0},	//next 13
	{12,13,14,15,-1, 0, 0, 0},	//next 14
	{ 0, 1,-1, 0, 0, 0, 0, 0},	//next 15
	{ 2, 3,-1, 0, 0, 0, 0, 0}	//next 16
};


static void initDoubleHiResBlock ()
{
	int iFrom, iCurrent, iTo;

	for (iFrom = 0; iFrom < 16; iFrom++) {
		for (iCurrent = 0; iCurrent < 16; iCurrent++) {
			for (iTo = 0; iTo < 16; iTo++) {
				aDoubleHiResBlock[iFrom][iCurrent][iTo] = aDoubleHiResBlockFrom[iCurrent][iFrom] + aDoubleHiResBlockTo[iCurrent][iTo];
			}
		}
	}
}

struct pixel {
	unsigned char r, g, b;
};

struct pixelerror {
	double r, g, b;
};

struct image {
	unsigned int h, w;
	struct pixel * p;
};

struct blockImage {
	unsigned int h, w;
	int * b;
};

typedef struct pixel Pixel;
typedef struct pixelerror PixelError;
typedef struct pixel * PixelRef;
typedef struct image * ImageRef;
typedef struct blockImage * BlockImageRef;

static ImageRef im_try;
static Pixel palletlist[17][16] = {
	{	// 0 - tohgr
		{   1,   4,   8},	// 0 - Black
		{ 148,	12, 125},	// 1 - Magenta
		{  99,  77,   0},	// 2 - Brown
		{ 249,  86,  29},	// 3 - Orange
		{  51, 111,   0},	// 4 - Dark Green
		{ 126, 126, 126},	// 5 - Gray
		{  67, 200,   0},	// 6 - Green
		{ 221, 206,  23},	// 7 - Yellow
		{  32,  54, 212},	// 8 - Dark Blue
		{ 188,  55, 255},	// 9 - Violet
		{ 126, 126, 126},	// A - Grey
		{ 255, 129, 236},	// B - Pink
		{   7, 168, 225},	// C - Blue
		{ 158, 172, 255},	// D - Light Blue
		{  93, 248, 133},	// E - Aqua
		{ 248, 250, 244}	// F - White
	},
	{	// 1 - ?
		{   0,   0,   0},	// 0 - Black
		{ 167,  11,  64},	// 1 - Magenta
		{  64,  99,   0},	// 2 - Brown
		{ 230, 111,   0},	// 3 - Orange
		{   0, 116,  64},	// 4 - Dark Green
		{ 128, 128, 128},	// 5 - Gray
		{  25, 215,   0},	// 6 - Green
		{ 191, 227,   8},	// 7 - Yellow
		{  64,  28, 247},	// 8 - Dark Blue
		{ 230,  40, 255},	// 9 - Violet
		{ 128, 128, 128},	// A - Grey
		{ 255, 139, 191},	// B - Pink
		{  25, 144, 255},	// C - Blue
		{ 191, 156, 255},	// D - Light Blue
		{  88, 244, 191},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 2 - AppleWin
		{   0,   0,   0},	// 0 - Black
		{ 208,	 0,  48},	// 1 - Magenta
		{ 128,  80,   0},	// 2 - Brown
		{ 255, 128,   0},	// 3 - Orange
		{	0, 128,   0},	// 4 - Dark Green
		{ 128, 128, 128},	// 5 - Gray
		{   0, 255,   0},	// 6 - Green
		{ 255, 255,   0},	// 7 - Yellow { 255, 255,   8}?
		{   0,   0, 128},	// 8 - Dark Blue
		{ 255,   0, 255},	// 9 - Violet
		{ 192, 192, 192},	// A - Grey
		{ 255, 144, 128},	// B - Pink
		{   0,   0, 255},	// C - Blue
		{  96, 160, 255},	// D - Light Blue
		{  64, 255, 144},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 3 - Wiki
		{   0,   0,   0},	// 0 - Black
		{ 114,	38,  64},	// 1 - Magenta
		{  64,  76,   0},	// 2 - Brown
		{ 228, 101,   1},	// 3 - Orange
		{  14,  89,  64},	// 4 - Dark Green
		{ 128, 128, 128},	// 5 - Gray
		{  27, 203,   1},	// 6 - Green
		{ 191, 204, 128},	// 7 - Yellow
		{  64,  51, 127},	// 8 - Dark Blue
		{ 228,  52, 254},	// 9 - Violet
		{ 128, 128, 128},	// A - Grey
		{ 241, 166, 191},	// B - Pink
		{  27, 154, 254},	// C - Blue
		{ 191, 179, 255},	// D - Light Blue
		{ 141, 217, 191},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 4 - tohgr NTSC
		{   0,   0,   0},	// 0 - Black
		{ 148,	12, 125},	// 1 - Magenta
		{  99,  77,   0},	// 2 - Brown
		{ 249,  86,  29},	// 3 - Orange
		{  51, 111,   0},	// 4 - Dark Green
		{ 126, 126, 126},	// 5 - Gray
		{  67, 200,   0},	// 6 - Green
		{ 221, 206,  23},	// 7 - Yellow
		{  32,  54, 212},	// 8 - Dark Blue
		{ 188,  55, 255},	// 9 - Violet
		{ 126, 126, 126},	// A - Grey
		{ 255, 129, 236},	// B - Pink
		{   7, 168, 225},	// C - Blue
		{ 158, 172, 255},	// D - Light Blue
		{  93, 248, 133},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 5 - Kegs32 RGB
		{   0,   0,   0},	// 0 - Black
		{ 221,	 0,  51},	// 1 - Magenta
		{ 136,  85,  34},	// 2 - Brown
		{ 255, 102,   0},	// 3 - Orange
		{   0, 119,   0},	// 4 - Dark Green
		{  85,  85,  85},	// 5 - Gray
		{   0, 221,   0},	// 6 - Green
		{ 255, 255,   0},	// 7 - Yellow
		{   0,   0, 153},	// 8 - Dark Blue
		{ 221,   0, 221},	// 9 - Violet
		{ 170, 170, 170},	// A - Grey
		{ 255, 153, 136},	// B - Pink
		{  34,  34, 255},	// C - Blue
		{ 102, 170, 255},	// D - Light Blue
		{   0, 255, 153},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 6 - CiderPress RGB
		{   0,   0,   0},	// 0 - Black
		{ 221,	 0,  51},	// 1 - Magenta
		{ 136,  85,   0},	// 2 - Brown
		{ 255, 102,   0},	// 3 - Orange
		{   0, 119,  34},	// 4 - Dark Green
		{  85,  85,  85},	// 5 - Gray
		{  17, 221,   0},	// 6 - Green
		{ 255, 255,   0},	// 7 - Yellow
		{   0,   0, 153},	// 8 - Dark Blue
		{ 221,  34, 221},	// 9 - Violet
		{ 170, 170, 170},	// A - Grey
		{ 255, 153, 136},	// B - Pink
		{  34,  34, 255},	// C - Blue
		{ 102, 170, 255},	// D - Light Blue
		{  68, 255, 153},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 7 - New AppleWin NTSC
		{   0,   0,   0},	// 0 - Black
		{ 157,	 9, 102},	// 1 - Magenta
		{  85,  85,   0},	// 2 - Brown
		{ 242,  94,   0},	// 3 - Orange
		{	0, 118,  26},	// 4 - Dark Green
		{ 128, 128, 128},	// 5 - Gray
		{  56, 203,   0},	// 6 - Green
		{ 213, 213,  26},	// 7 - Yellow
		{  42,  42, 229},	// 8 - Dark Blue
		{ 199,  52, 255},	// 9 - Violet
		{ 192, 192, 192},	// A - Grey
		{ 255, 137, 229},	// B - Pink
		{  13, 161, 255},	// C - Blue
		{ 170, 170, 255},	// D - Light Blue
		{  98, 246, 153},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 8 - Jace NTSC
		{   0,   0,   0},	// 0 - Black
		{ 177,	 0,  93},	// 1 - Magenta
		{  94,  86,   0},	// 2 - Brown
		{ 255,  86,   0},	// 3 - Orange
		{	0, 127,  34},	// 4 - Dark Green
		{ 127, 127, 127},	// 5 - Gray
		{  44, 213,   0},	// 6 - Green
		{ 222, 213,   0},	// 7 - Yellow
		{  32,  41, 255},	// 8 - Dark Blue
		{ 210,  41, 255},	// 9 - Violet
		{ 127, 127, 127},	// A - Grey
		{ 255, 127, 220},	// B - Pink
		{   0, 168, 255},	// C - Blue
		{ 160, 168, 255},	// D - Light Blue
		{  77, 255, 161},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 9 - Cybernesto-Munafo NTSC
		{   0,   0,   0},	// 0 - Black
		{ 227,	30,  96},	// 1 - Magenta
		{  96, 114,   3},	// 2 - Brown
		{ 255, 106,  60},	// 3 - Orange
		{	0, 163,  96},	// 4 - Dark Green
		{ 156, 156, 156},	// 5 - Gray
		{  20, 245,  60},	// 6 - Green
		{ 208, 221, 141},	// 7 - Yellow
		{  96,  78, 189},	// 8 - Dark Blue
		{ 255,  68, 253},	// 9 - Violet
		{ 156, 156, 156},	// A - Grey
		{ 255, 160, 208},	// B - Pink
		{  20, 207, 253},	// C - Blue
		{ 208, 195, 255},	// D - Light Blue
		{ 114, 255, 208},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 10 - Pseudo Palette
		{   0,   0,   0},	// 0 - Black
		{ 184,	 6,  88},	// 1 - Magenta
		{ 117,  81,  17},	// 2 - Brown
		{ 252,  94,  14},	// 3 - Orange
		{  25, 115,   0},	// 4 - Dark Green
		{ 105, 105, 105},	// 5 - Gray
		{  33, 210,   0},	// 6 - Green
		{ 238, 230,  11},	// 7 - Yellow
		{  16,  27, 182},	// 8 - Dark Blue
		{ 204,  27, 238},	// 9 - Violet
		{ 148, 148, 148},	// A - Grey
		{ 255, 141, 186},	// B - Pink
		{  20, 101, 240},	// C - Blue
		{ 130, 171, 255},	// D - Light Blue
		{  46, 251, 143},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 11 - tohgr NTSC HGR
		{   0,   0,   0},	// 0 - Black
		{ 173,	24,  40},	// 1 - Magenta
		{  51, 111,   0},	// 2 - Brown
		{ 208, 129,   1},	// 3 - Orange
		{   1, 115,  99},	// 4 - Dark Green
		{ 126, 130, 127},	// 5 - Gray
		{  29, 214,   9},	// 6 - Green
		{ 174, 234,  34},	// 7 - Yellow
		{  85,  27, 225},	// 8 - Dark Blue
		{ 232,  44, 248},	// 9 - Violet - 232,44,248 - default hgr overlay color
		{ 127, 126, 119},	// A - Grey
		{ 254, 147, 163},	// B - Pink
		{  52, 133, 252},	// C - Blue - 52,133,252 - alternate HGR overlay color
		{ 209, 149, 255},	// D - Light Blue
		{  91, 235, 217},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 12 - Test1
		{   0,   0,   0},	// 0 - Black
		{ 208,	 0,  48},	// 1 - Magenta
		{ 128,  80,   0},	// 2 - Brown
		{ 212, 118,   0},	// 3 - Orange
		{   3, 100,  40},	// 4 - Dark Green
		{ 128, 128, 128},	// 5 - Gray
		{   0, 255,   0},	// 6 - Green
		{ 249, 206,   8},	// 7 - Yellow
		{   0,   0, 128},	// 8 - Dark Blue
		{ 192,   0, 255},	// 9 - Violet
		{ 192, 192, 192},	// A - Grey
		{ 245, 160, 189},	// B - Pink
		{   0,   0, 255},	// C - Blue
		{  96, 160, 255},	// D - Light Blue
		{  64, 255, 144},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 13 - Test2
		{   0,   0,   0},	// 0 - Black
		{ 255,	 0,   0},	// 1 - Magenta
		{ 118,  53,   0},	// 2 - Brown
		{ 228, 101,   1},	// 3 - Orange
		{   0, 110,  40},	// 4 - Dark Green
		{ 128, 128, 128},	// 5 - Gray
		{   0, 255,   0},	// 6 - Green
		{ 207, 185,   0},	// 7 - Yellow
		{  12,  60, 150},	// 8 - Dark Blue
		{ 192,   0, 255},	// 9 - Violet
		{ 192, 192, 192},	// A - Grey
		{ 255, 158, 204},	// B - Pink
		{   0,  30, 250},	// C - Blue
		{  96, 160, 255},	// D - Light Blue
		{  64, 255, 144},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 14 - Photo IIGS
		{   0,   0,   0},	// 0 - Black
		{ 217,	58,  49},	// 1 - Magenta
		{ 116,  63,  13},	// 2 - Brown
		{ 235, 110,  35},	// 3 - Orange
		{  26,  75,  25},	// 4 - Dark Green
		{  50,  48,  43},	// 5 - Gray
		{  63, 196,  69},	// 6 - Green
		{ 214, 197,  58},	// 7 - Yellow
		{   7,  18, 127},	// 8 - Dark Blue
		{ 178,  31, 179},	// 9 - Violet
		{ 152, 155, 161},	// A - Grey
		{ 225, 166, 143},	// B - Pink
		{   2,  20, 220},	// C - Blue
		{  54, 147, 213},	// D - Light Blue
		{  80, 231, 179},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 15 - Photo IIe
		{   0,   0,   0},	// 0 - Black
		{ 130,	36,  46},	// 1 - Magenta
		{  97,  48,  16},	// 2 - Brown
		{ 221,  97,  43},	// 3 - Orange
		{  33,  71,  52},	// 4 - Dark Green
		{ 127, 125, 126},	// 5 - Gray
		{  84, 157,  42},	// 6 - Green
		{ 230, 188, 101},	// 7 - Yellow
		{   0,   8, 141},	// 8 - Dark Blue
		{ 139,  32, 201},	// 9 - Violet
		{ 127, 120, 127},	// A - Grey
		{ 249, 158, 185},	// B - Pink
		{   3, 112, 209},	// C - Blue
		{ 106, 167, 254},	// D - Light Blue
		{ 101, 208, 189},	// E - Aqua
		{ 255, 255, 255},	// F - White
	},
	{	// 16 - tohgr NTSC with 3D
		{   0,   0,   0},	// 0 - Black
		{ 148,	12, 125},	// 1 - Magenta
		{  99,  77,   0},	// 2 - Brown
		{ 249,  86,  29},	// 3 - Orange
		{  51, 111,   0},	// 4 - Dark Green
		{ 127, 127, 127},	// 5 - Gray
		{  67, 200,   0},	// 6 - Green
		{ 221, 206,  23},	// 7 - Yellow
		{  32,  54, 212},	// 8 - Dark Blue
		{ 188,  55, 255},	// 9 - Violet
		{ 126, 126, 126},	// A - Grey
		{ 255, 129, 236},	// B - Pink
		{   7, 168, 225},	// C - Blue
		{  93, 253, 249},	// D - Light Blue *
		{  93, 248, 133},	// E - Aqua
		{ 255, 255, 255},	// F - White
	}
};

//		{ 103, 112, 167},	// 9 - Violet *

//	{	// 4 - tohgr NTSC
//		{   0,   0,   0},	// 0 - Black
//		{ 148,	12, 125},	// 1 - Magenta
//		{  99,  77,   0},	// 2 - Brown
//		{ 249,  86,  29},	// 3 - Orange
//		{  51, 111,   0},	// 4 - Dark Green
//		{ 126, 126, 126},	// 5 - Gray
//		{  67, 200,   0},	// 6 - Green
//		{ 221, 206,  23},	// 7 - Yellow
//		{  32,  54, 212},	// 8 - Dark Blue
//		{ 188,  55, 255},	// 9 - Violet
//		{ 126, 126, 126},	// A - Grey
//		{ 255, 129, 236},	// B - Pink
//		{   7, 168, 225},	// C - Blue
//		{ 158, 172, 255},	// D - Light Blue
//		{  93, 248, 133},	// E - Aqua
//		{ 255, 255, 255},	// F - White
//	},


static Pixel pal[] = {
	{   0,   0,   0},	// 0 - Black
	{   0,	 0,   0},	// 1 - Magenta
	{   0,   0,   0},	// 2 - Brown
	{   0,   0,   0},	// 3 - Orange
	{   0,   0,   0},	// 4 - Dark Green
	{   0,   0,   0},	// 5 - Gray
	{   0,   0,   0},	// 6 - Green
	{   0,   0,   0},	// 7 - Yellow
	{   0,   0,   0},	// 8 - Dark Blue
	{   0,   0,   0},	// 9 - Violet
	{   0,   0,   0},	// A - Grey
	{   0,   0,   0},	// B - Pink
	{   0,   0,   0},	// C - Blue
	{   0,   0,   0},	// D - Light Blue
	{   0,   0,   0},	// E - Aqua
	{   0,   0,   0},	// F - White
};

static Pixel previewpal[] = {
	{   0,   0,   0},	// 0 - Black
	{   0,	 0,   0},	// 1 - Magenta
	{   0,   0,   0},	// 2 - Brown
	{   0,   0,   0},	// 3 - Orange
	{   0,   0,   0},	// 4 - Dark Green
	{   0,   0,   0},	// 5 - Gray
	{   0,   0,   0},	// 6 - Green
	{   0,   0,   0},	// 7 - Yellow
	{   0,   0,   0},	// 8 - Dark Blue
	{   0,   0,   0},	// 9 - Violet
	{   0,   0,   0},	// A - Grey
	{   0,   0,   0},	// B - Pink
	{   0,   0,   0},	// C - Blue
	{   0,   0,   0},	// D - Light Blue
	{   0,   0,   0},	// E - Aqua
	{   0,   0,   0},	// F - White
};

/*
// tohgr NTSC adjusted

static Pixel preview_pal[] = {
	{   0,   0,   0},	// 0 - Black
	{ 148,	12, 125},	// 1 - Magenta
	{  99,  77,   0},	// 2 - Brown
	{ 249,  86,  29},	// 3 - Orange
	{  51, 111,   0},	// 4 - Dark Green
	{ 126, 126, 126},	// 5 - Gray
	{  67, 200,   0},	// 6 - Green
	{ 221, 206,  23},	// 7 - Yellow
	{  32,  54, 212},	// 8 - Dark Blue
	{ 188,  55, 253},	// 9 - Violet
	{ 192, 192, 192},	// A - Grey
	{ 255, 129, 236},	// B - Pink
	{   7, 186, 225},	// C - Blue
	{ 158, 172, 255},	// D - Light Blue
	{  93, 248, 133},	// E - Aqua
	{ 255, 255, 255},	// F - White
};
*/

const char * const error_diffusion_list[] = { 
	"00_OstromoukhovC",			// 0 = Ostromoukhov - Coarse
	"01_OstromoukhovF1",		// 1 = Ostromoukhov - Fine1
	"02_OstromoukhovF2",		// 2 = Ostromoukhov - Fine2
	"03_FloydSteinbergCA",		// 3 = Floyd-Steinberg - Coarse - Average
	"04_FloydSteinbergCO",		// 4 = Floyd-Steinberg - Coarse - Ordered
	"05_FloydSteinbergCS",		// 5 = Floyd-Steinberg - Coarse - Swapped
	"06_FloydSteinbergF1",		// 6 = Floyd-Steinberg - Fine1
	"07_FloydSteinbergF2",		// 7 = Floyd-Steinberg - Fine2
	"08_BuckelsCA",				// 8 = Buckels - Coarse - Average
	"09_BuckelsCS",				// 9 = Buckels - Coarse - Swapped
	"10_BuckelsF",				// 10 = Buckels - Fine
	"11_JarvisJudiceNinkeCA",	// 11 = Jarvis, Judice and Ninke - Coarse - Average
	"12_FloydSteinbergS2A",		// 12 = Floyd-Steinberg - Spread2A
	"13_FloydSteinbergS2B",		// 13 = Floyd-Steinberg - Spread2B
	"14_FloydSteinbergS3",		// 14 = Floyd-Steinberg - Spread3
	"15_FloydSteinbergF3",		// 15 = Floyd-Steinberg - Fine3
	"16_FloydSteinbergF4"	};	// 16 = Floyd-Steinberg - Fine4

//CCIR 601
int lumaRED = 299, lumaGREEN = 587, lumaBLUE = 114;
double dlumaRED = 0.299, dlumaGREEN = 0.587, dlumaBLUE = 0.114;

static void initBaseAddrs (void)
{
	unsigned int i, group_of_eight, line_of_eight, group_of_sixtyfour;
	
	for (i = 0; i < 192; ++i)
	{
		line_of_eight = i % 8;
		group_of_eight = (i % 64) / 8;
		group_of_sixtyfour = i / 64;
		
		baseaddr[i] = line_of_eight * 1024 + group_of_eight * 128 + group_of_sixtyfour * 40;
	}
}

static Pixel imageGetPixel (ImageRef i, unsigned int x, unsigned int y)
{
	Pixel z = {0,0,0};
	if (x >= i->w) return z;
	if (y >= i->h) return z;
	return i->p[(i->w * y) + x];
}

static int blockImageGetBlock (BlockImageRef i, unsigned int x, unsigned int y)
{
	int z = 0;
	if (x >= i->w) return z;
	if (y >= i->h) return z;
	return i->b[(i->w * y) + x];
}

static void imagePutPixel (ImageRef i, unsigned int x, unsigned int y, Pixel p)
{
	if (x >= i->w) return;
	if (y >= i->h) return;
	i->p[(i->w * y) + x] = p;
}

static void blockImagePutBlock (BlockImageRef i, unsigned int x, unsigned int y, int b)
{
	if (x >= i->w) return;
	if (y >= i->h) return;
	i->b[(i->w * y) + x] = b;
}

static ImageRef imageNew (unsigned int w, unsigned int h)
{
	ImageRef ip;
	ip = (ImageRef) malloc(sizeof *ip);
	if (!ip) return 0;
	ip->w = w;
	ip->h = h;
	ip->p = (pixel *) malloc(ip->h * ip->w * sizeof ip->p[0]);
	if (ip->p) return ip;
	free(ip);
	return 0;
}

static BlockImageRef blockImageNew (unsigned int w, unsigned int h)
{
	BlockImageRef ib;
	ib = (BlockImageRef) malloc(sizeof *ib);
	if (!ib) return 0;
	ib->w = w;
	ib->h = h;
	ib->b = (int *) malloc(ib->h * ib->w * sizeof ib->b[0]);
	if (ib->b) return ib;
	free(ib);
	return 0;
}

// from:http://stackoverflow.com/questions/9018016/how-to-compare-two-colors
/**
 * Computes the difference between two RGB colors by converting them to the L*a*b scale and
 * comparing them using the CIE76 algorithm { http://en.wikipedia.org/wiki/Color_difference#CIE76}
 */
//static int lab[3];
//static int* ColorToLab(Pixel p)
static double lab[3];
static double* ColorToLab(Pixel p)
{

	// http://www.brucelindbloom.com

	double r, g, b, X, Y, Z, fx, fy, fz, xr, yr, zr;
	double Ls, fas, fbs;
	double eps = 216.0f / 24389.0f;
	double k = 24389.0f / 27.0f;

	double Xr = 0.964221f;  // reference white D50
	double Yr = 1.0f;
	double Zr = 0.825211f;

	// RGB to XYZ
	r = p.r / 255.0f; //R 0..1
	g = p.g / 255.0f; //G 0..1
	b = p.b / 255.0f; //B 0..1

	// assuming sRGB (D65)
	if (r <= 0.04045) r = r / 12;
	else r = (double)pow((r + 0.055) / 1.055, 2.4);

	if (g <= 0.04045) g = g / 12;
	else g = (double)pow((g + 0.055) / 1.055, 2.4);

	if (b <= 0.04045) b = b / 12;
	else b = (double)pow((b + 0.055) / 1.055, 2.4);

	X = 0.436052025f * r + 0.385081593f * g + 0.143087414f * b;
	Y = 0.222491598f * r + 0.71688606f * g + 0.060621486f * b;
	Z = 0.013929122f * r + 0.097097002f * g + 0.71418547f * b;

	// XYZ to Lab
	xr = X / Xr;
	yr = Y / Yr;
	zr = Z / Zr;

	if (xr > eps) fx = (double)pow(xr, 1 / 3.0);
	else fx = (double)((k * xr + 16.0) / 116.0);

	if (yr > eps) fy = (double)pow(yr, 1 / 3.0);
	else fy = (double)((k * yr + 16.0) / 116.0);

	if (zr > eps) fz = (double)pow(zr, 1 / 3.0);
	else fz = (double)((k * zr + 16.0) / 116);

	Ls = (116 * fy) - 16;
	fas = 500 * (fx - fy);
	fbs = 200 * (fy - fz);

//	lab[0] = (int)(2.55 * Ls + 0.5);
//	lab[1] = (int)(fas + 0.5);
//	lab[2] = (int)(fbs + 0.5);
	lab[0] = 2.55 * Ls + 0.5;
	lab[1] = fas + 0.5;
	lab[2] = fbs + 0.5;
	return lab;

}

/* CIE C illuminant */
static const double illum[3*3] =
{ 0.488718, 0.176204, 0.000000,
  0.310680, 0.812985, 0.0102048,
  0.200602, 0.0108109, 0.989795 };
struct LabItem // CIE L*a*b* color value with C and h added.
{
    double L,a,b,C,h;

    LabItem() { }
    LabItem(double R,double G,double B) { Set(R,G,B); }
    void Set(double R,double G,double B)
    {
        const double* const i = illum;
        double X = i[0]*R + i[3]*G + i[6]*B, x = X / (i[0] + i[1] + i[2]);
        double Y = i[1]*R + i[4]*G + i[7]*B, y = Y / (i[3] + i[4] + i[5]);
        double Z = i[2]*R + i[5]*G + i[8]*B, z = Z / (i[6] + i[7] + i[8]);
        const double threshold1 = (6*6*6.0)/(29*29*29.0);
        const double threshold2 = (29*29.0)/(6*6*3.0);
        double x1 = (x > threshold1) ? pow(x, 1.0/3.0) : (threshold2*x)+(4/29.0);
        double y1 = (y > threshold1) ? pow(y, 1.0/3.0) : (threshold2*y)+(4/29.0);
        double z1 = (z > threshold1) ? pow(z, 1.0/3.0) : (threshold2*z)+(4/29.0);
        L = (29*4)*y1 - (4*4);
        a = (500*(x1-y1) );
        b = (200*(y1-z1) );
        C = sqrt(a*a + b+b);
        h = atan2(b, a);
    }
    LabItem(unsigned rgb) { Set(rgb); }
    void Set(unsigned rgb)
    {
        Set( (rgb>>16)/255.0, ((rgb>>8)&0xFF)/255.0, (rgb&0xFF)/255.0 );
    }
    LabItem(Pixel p) { Set(p); }
    void Set(Pixel p)
    {
        Set(p.r/255.0, p.g/255.0, p.b/255.0);
    }
};

/* From the paper "The CIEDE2000 Color-Difference Formula: Implementation Notes, */
/* Supplementary Test Data, and Mathematical Observations", by */
/* Gaurav Sharma, Wencheng Wu and Edul N. Dalal, */
/* Color Res. Appl., vol. 30, no. 1, pp. 21-30, Feb. 2005. */
/* Return the CIEDE2000 Delta E color difference measure squared, for two Lab values */
static double ColorCompare(const LabItem& lab1, const LabItem& lab2)
{
    #define RAD2DEG(xx) (180.0/M_PI * (xx))
    #define DEG2RAD(xx) (M_PI/180.0 * (xx))
    /* Compute Cromanance and Hue angles */
    double C1,C2, h1,h2;
    {
        double Cab = 0.5 * (lab1.C + lab2.C);
        double Cab7 = pow(Cab,7.0);
        double G = 0.5 * (1.0 - sqrt(Cab7/(Cab7 + 6103515625.0)));
        double a1 = (1.0 + G) * lab1.a;
        double a2 = (1.0 + G) * lab2.a;
        C1 = sqrt(a1 * a1 + lab1.b * lab1.b);
        C2 = sqrt(a2 * a2 + lab2.b * lab2.b);

        if (C1 < 1e-9)
            h1 = 0.0;
        else {
            h1 = RAD2DEG(atan2(lab1.b, a1));
            if (h1 < 0.0)
                h1 += 360.0;
        }

        if (C2 < 1e-9)
            h2 = 0.0;
        else {
            h2 = RAD2DEG(atan2(lab2.b, a2));
            if (h2 < 0.0)
                h2 += 360.0;
        }
    }

    /* Compute delta L, C and H */
    double dL = lab2.L - lab1.L, dC = C2 - C1, dH;
    {
        double dh;
        if (C1 < 1e-9 || C2 < 1e-9) {
            dh = 0.0;
        } else {
            dh = h2 - h1;
            /**/ if (dh > 180.0)  dh -= 360.0;
            else if (dh < -180.0) dh += 360.0;
        }

        dH = 2.0 * sqrt(C1 * C2) * sin(DEG2RAD(0.5 * dh));
    }

    double h;
    double L = 0.5 * (lab1.L  + lab2.L);
    double C = 0.5 * (C1 + C2);
    if (C1 < 1e-9 || C2 < 1e-9) {
        h = h1 + h2;
    } else {
        h = h1 + h2;
        if (fabs(h1 - h2) > 180.0) {
            /**/ if (h < 360.0)  h += 360.0;
            else if (h >= 360.0) h -= 360.0;
        }
        h *= 0.5;
    }
    double T = 1.0
      - 0.17 * cos(DEG2RAD(h - 30.0))
      + 0.24 * cos(DEG2RAD(2.0 * h))
      + 0.32 * cos(DEG2RAD(3.0 * h + 6.0))
      - 0.2 * cos(DEG2RAD(4.0 * h - 63.0));
    double hh = (h - 275.0)/25.0;
    double ddeg = 30.0 * exp(-hh * hh);
    double C7 = pow(C,7.0);
    double RC = 2.0 * sqrt(C7/(C7 + 6103515625.0));
    double L50sq = (L - 50.0) * (L - 50.0);
    double SL = 1.0 + (0.015 * L50sq) / sqrt(20.0 + L50sq);
    double SC = 1.0 + 0.045 * C;
    double SH = 1.0 + 0.015 * C * T;
    double RT = -sin(DEG2RAD(2 * ddeg)) * RC;
    double dLsq = dL/SL, dCsq = dC/SC, dHsq = dH/SH;
    return dLsq*dLsq + dCsq*dCsq + dHsq*dHsq + RT*dCsq*dHsq;
#undef RAD2DEG
#undef DEG2RAD
}

static double pixelDist (Pixel p1, Pixel p2)
{
	int d, pd;

	switch (optColourDifference) {
	case 1:

		d = (int)p1.r - (int)p2.r;
		pd = (d * d);
		d = (int)p1.g - (int)p2.g;
		pd = pd + (d * d) ;
		d = (int)p1.b - (int)p2.b;
		return pd + (d * d);
		break;

	case 2:

		d = abs(p1.r - p2.r);
		pd = 3*d;
		d = abs(p1.g - p2.g);
		pd = pd + 4*d;
		d = abs(p1.b - p2.b);
		return pd + 3*d;
		break;

	case 3:

		double cR, cG, cB, uR;
		cR = (double)p1.r - (double)p2.r;
		cG = (double)p1.g - (double)p2.g;
		cB = (double)p1.b - (double)p2.b;
		uR = (double)p1.r + (double)p2.r;
		return (int)(cR*cR*(2+uR/256) + cG*cG*4 + cB*cB*(2+(255-uR)/256));
		break;

	case 4:

	    double lab1[3], lab2[3];
		double *lab;

		lab = ColorToLab(p1);
		lab1[0] = lab[0];
		lab1[1] = lab[1];
		lab1[2] = lab[2];

		lab = ColorToLab(p2);
		lab2[0] = lab[0];
		lab2[1] = lab[1];
		lab2[2] = lab[2];

		return (int) sqrt(pow((double)(lab2[0] - lab1[0]), 2) + pow((double)(lab2[1] - lab1[1]), 2) + pow((double)(lab2[2] - lab1[2]), 2)) / 2.55;
		break;

	case 5:

		double lumadiff, diffR, diffG, diffB, rgbLuma1, rgbLuma2;
		rgbLuma1 = (p1.r*lumaRED + p1.g*lumaGREEN + p1.b*lumaBLUE) / (255.0*1000);
		rgbLuma2 = (p2.r*lumaRED + p2.g*lumaGREEN + p2.b*lumaBLUE) / (255.0*1000);
		lumadiff = rgbLuma1 - rgbLuma2;
		diffR = (p1.r - p2.r)/255.0;
		diffG = (p1.g - p2.g)/255.0;
		diffB = (p1.b - p2.b)/255.0;
		return (diffR*diffR*dlumaRED + diffG*diffG*dlumaGREEN + diffB*diffB*dlumaBLUE)*0.75 + (lumadiff*lumadiff);
		break;

	case 6:

	    // Input color in CIE L*a*b*
		LabItem labi1(p1);
		LabItem labi2(p2);

		return ColorCompare(labi1, labi2);
		break;

	}
}

static Pixel alterPixel (Pixel p, double dr, double dg, double db)
{
	dr += (int)p.r;
	dg += (int)p.g;
	db += (int)p.b;
	if (dr < 0) dr = 0; else if (dr > 255) dr = 255;
	if (dg < 0) dg = 0; else if (dg > 255) dg = 255;
	if (db < 0) db = 0; else if (db > 255) db = 255;
	p.r = (int)dr;
	p.g = (int)dg;
	p.b = (int)db;
	return p;
}

static int rgbToBlock_Pixel (Pixel p)
{
//	int d,i,bd,bi;
	int i,bi;
	double d,bd;

	bd = 0x7fffffff; // big number
	for (i = 0; i < 16; ++i) {
		d = pixelDist(p, pal[i]);
		if (((d < bd)&&(optClosestLessThan == 0)) || ((d <= bd)&&(optClosestLessThan == 1))) {
			bd = d;
			bi = i;
		}
	}
	return bi;
}

static BlockImageRef rgbToBlock (ImageRef im)
{
	int x, y;
	BlockImageRef ib;
	ib = blockImageNew(im->w, im->h);
	if (!ib) goto emalloc;

	for(x=0; x<im->w; x++)
	{
		for(y=0; y<im->h; y++)
		{
			blockImagePutBlock(ib, x, y, rgbToBlock_Pixel(imageGetPixel(im, x, y)));
		}
	}
	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}
/*
static void imageTest (ImageRef im)
{
	int i,j;

	for(i=0; i<int(im->w); i++)
	{
		for(j=0; j<int(im->h); j++)
		{
			if (j>50)
			{
				im->p[(im->w * ((im->h-1)-j)) + i].r = 130; 
				im->p[(im->w * ((im->h-1)-j)) + i].g = 10; 
				im->p[(im->w * ((im->h-1)-j)) + i].b = 30; 
			}
			else
			{
				im->p[(im->w * ((im->h-1)-j)) + i].r = 255; 
				im->p[(im->w * ((im->h-1)-j)) + i].g = 255; 
				im->p[(im->w * ((im->h-1)-j)) + i].b = 255; 
			}
		}
	}
}

static void imageTest (ImageRef im)
{
	int x,y,i,j;
	Pixel p;

	for(i=0; i<int(im->w); i++)
	{
		for(j=0; j<int(im->h); j++)
		{
			x=i; y=(im->h-1)-j;
			p = imageGetPixel(im, x, y);
			p.r = 10; 
			p.g = 100; 
			p.b = 200; 
			imagePutPixel(im, x, y, p);
		}
	}
}
*/
void write_bmp(char *fname, ImageRef im)
{
	int x,y,i,j;
	Pixel p;
	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3 * im->w * im->h;
	img = (unsigned char *)malloc(3 * im->w * im->h);

	for(i=0; i<int(im->w); i++)
	{
		for(j=0; j<int(im->h); j++)
		{
			x=i; y=(im->h-1)-j;
			p = imageGetPixel(im, x, y);
			img[(x+y*im->w)*3+2] = p.r;
			img[(x+y*im->w)*3+1] = p.g;
			img[(x+y*im->w)*3+0] = p.b;
		}
	}

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] = {0,0,0};

	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	bmpinfoheader[ 4] = (unsigned char)(im->w    );
	bmpinfoheader[ 5] = (unsigned char)(im->w>> 8);
	bmpinfoheader[ 6] = (unsigned char)(im->w>>16);
	bmpinfoheader[ 7] = (unsigned char)(im->w>>24);
	
	bmpinfoheader[ 8] = (unsigned char)(im->h    );
	bmpinfoheader[ 9] = (unsigned char)(im->h>> 8);
	bmpinfoheader[10] = (unsigned char)(im->h>>16);
	bmpinfoheader[11] = (unsigned char)(im->h>>24);

	f = fopen(fname,"wb");
	fwrite(bmpfileheader,1,14,f);
	fwrite(bmpinfoheader,1,40,f);
	for(i=0; i<int(im->h); i++)
	{
		fwrite(img+(im->w*(im->h-i-1)*3),3,im->w,f);
		fwrite(bmppad,1,(4-(im->w*3)%4)%4,f);
	}
	fclose(f);
}

static ImageRef imageFromBMP (FILE *fp, int *fullResolution)
{
	unsigned int w, h, headersize, bits, index;
	ImageRef ip = 0;
	Pixel p = {0,0,0};
	unsigned char bmpfileheader[14];
	unsigned char bmpinfoheader[40];
	int x,y,i,j;
	unsigned char *img = NULL;
	unsigned int offset, filesize, colour, pad, compress;
	unsigned int bitmapsize;

	/* open file and test for it being a supported bmp */
	fread(bmpfileheader, 1, 14, fp);
	if (bmpfileheader[0] != 'B' || bmpfileheader[1] != 'M')
	{
		printf("[read_bmp_file] File is not recognized as a supported BMP file\n");
		goto emalloc2;
	}
	filesize = (bmpfileheader[5] * 256 * 256 * 256) + (bmpfileheader[4] * 256 * 256) + (bmpfileheader[3] * 256) + bmpfileheader[2];
//	printf("filesize %d\n", filesize);

	offset = (bmpfileheader[13] * 256 * 256 * 256) + (bmpfileheader[12] * 256 * 256) + (bmpfileheader[11] * 256) + bmpfileheader[10];
//	printf("offset %d\n", offset);

	fread(bmpinfoheader, 1, 40, fp);
	headersize = (bmpinfoheader[3] * 256 * 256 * 256) + (bmpinfoheader[2] * 256 * 256) + (bmpinfoheader[1] * 256) + bmpinfoheader[0];
//	printf("headersize %d\n", headersize);

	w = (bmpinfoheader[7] * 256 * 256 * 256) + (bmpinfoheader[6] * 256 * 256) + (bmpinfoheader[5] * 256) + bmpinfoheader[4];
	h = (bmpinfoheader[11] * 256 * 256 * 256) + (bmpinfoheader[10] * 256 * 256) + (bmpinfoheader[9] * 256) + bmpinfoheader[8];

	colour = (bmpinfoheader[13] * 256) + bmpinfoheader[12];
//	printf("colour %d\n", colour);

	bits = (bmpinfoheader[15] * 256) + bmpinfoheader[14];
//	printf("bits %d\n", bits);

	compress = (bmpinfoheader[19] * 256 * 256 * 256) + (bmpinfoheader[18] * 256 * 256) + (bmpinfoheader[17] * 256) + bmpinfoheader[16];
//	printf("compress %d\n", compress);

	bitmapsize = (bmpinfoheader[23] * 256 * 256 * 256) + (bmpinfoheader[22] * 256 * 256) + (bmpinfoheader[21] * 256) + bmpinfoheader[20];
//	printf("bitmapsize %d\n", bitmapsize);

	pad = 0;
	while ((w*3+pad)%4!=0) pad++;
//	printf("pad %d\n", pad);

	if ((w != 140 && w != 560) || h != 192)
	{
		printf("[read_bmp_file] Only resolutions of 140x192 or 560x192 are currently supported\n");
		goto emalloc2;
	}

	if ((bits != 4) && (bits != 24))
	{
		printf("[read_bmp_file] Only the 4bit and 24bit formats are currently supported\n");
		goto emalloc2;
	}

	*fullResolution = (int)(w == 560);

	ip = imageNew(w, h);
	if (!ip) goto emalloc;

	if (headersize - 40 > 0)
	{ 
		img = (unsigned char *)malloc(headersize - 40);
		fread(img, 1, headersize - 40, fp);
		free(img);
	}
	
	if (bits == 4)
	{
		img = (unsigned char *)malloc(64);
		fread(img, 1, 64, fp);
		for (i=0; i<16; i++)
		{
			pal[i].b = img[i*4+0];
			pal[i].g = img[i*4+1];
			pal[i].r = img[i*4+2];
		}
		free(img);
	}

	// w is not always equal to ip->w. The width of 140 tends to be stored as length 144 but width of 560 tends to be stored as length of 560.
//	printf("Width %d\n", (bitmapsize/h)*2);
	if (bitmapsize == 0)
	{
		w = ip->w+4;
	}
	else
	{
		w = (bitmapsize/h)*2;
	}

	if (bits == 4)
	{
		img = (unsigned char *)malloc((w/2) * ip->h);
		fread(img, 1, ((w/2) * ip->h), fp);
		for(j=0; j<ip->h; j++)
		{
			for(i=0; i<(ip->w)/2; i++)
			{
				x=i*2; y=(ip->h-1)-j;
				index = (unsigned char)img[i+j*(w/2)];
				p = pal[index>>4];
				imagePutPixel(ip, x, y, p);
				p = pal[index&15];
				imagePutPixel(ip, x+1, y, p);
			}
		}
		if (img) free(img);
	} 
	else
	{
		img = (unsigned char *)malloc((w*3) * ip->h);
		fread(img, 1, ((w*3) * ip->h), fp);
		for(j=0; j<ip->h; j++)
		{
			for(i=0; i<ip->w; i++)
			{
				x=i; y=(ip->h-1)-j;
				p.b = (unsigned char)img[(i*3)+(j*w)/2];
				p.g = (unsigned char)img[(i*3)+1+(j*w)/2];
				p.r = (unsigned char)img[(i*3)+2+(j*w)/2];
				imagePutPixel(ip, x, y, p);
			}
		}
		if (img) free(img);
	}

	return ip;

emalloc:
	if (ip->p) free(ip->p);
emalloc2:
	if (ip) free(ip);
	if (img) free(img);
	return 0;
}

static ImageRef blockToRgb_Actual(BlockImageRef ib, int stretchY)
{
	ImageRef id;
	unsigned int actBlockColour;
	Pixel p;
	int i, j, k, l;
	int	z1, z2, z3;

	id = imageNew(ib->w*4, ib->h*stretchY);
	if (!id) goto emalloc;

	for(j=0; j<ib->h; j++)
	{
		for(i=0; i<ib->w; i++)
		{
			z1 = blockImageGetBlock(ib, i-1, j);
			z2 = blockImageGetBlock(ib, i, j);
			z3 = blockImageGetBlock(ib, i+1, j);
			actBlockColour = aDoubleHiResBlock[z1][z2][z3];

//			actBlockColour = aDoubleHiResBlock[blockImageGetBlock(ib, i-1, j)][blockImageGetBlock(ib, i, j)][blockImageGetBlock(ib, i+1, j)];

//			printf("test1 %d %d,%d,%d,%d\n", actBlockColour, (unsigned char)((actBlockColour>>(4*0))&15), (unsigned char)((actBlockColour>>(4*1))&15), (unsigned char)((actBlockColour>>(4*2))&15), (unsigned char)((actBlockColour>>(4*3))&15));
//			printf("test1 %d, %d, %d\n", blockImageGetBlock(ib, i-1, j), blockImageGetBlock(ib, i, j), blockImageGetBlock(ib, i+1, j));

			for (k=0; k<4; k++)
			{
				p = previewpal[(unsigned char)((actBlockColour>>(4*(3-k)))&15)];
				for (l=0; l<stretchY; l++)
				{
					imagePutPixel(id, i*4 + k, j*stretchY + l, p);
				}
			}
		}
	}

	return id;

emalloc:
	if (id->p) free(id->p);
	if (id) free(id);
	return 0;

}

static ImageRef blockToRgb_Storage(BlockImageRef ib, int stretchY)
{
	ImageRef id;
	Pixel p;
	int i, j, k, l;
	int	z;

	id = imageNew(ib->w*4, ib->h*stretchY);
	if (!id) goto emalloc;

	for(j=0; j<ib->h; j++)
	{
		for(i=0; i<ib->w; i++)
		{
			z = blockImageGetBlock(ib, i, j);
			for (k=0; k<4; k++)
			{
				p = pal[(unsigned char)z];
				for (l=0; l<stretchY; l++)
				{
					imagePutPixel(id, i*4 + k, j*stretchY + l, p);
				}
			}
		}
	}

	return id;

emalloc:
	if (id->p) free(id->p);
	if (id) free(id);
	return 0;

}

static ImageRef rgbToRgb_Stretched(ImageRef im, int stretchX, int stretchY)
{
	ImageRef id;
	Pixel p;
	int i, j, k, l;

	id = imageNew(im->w*stretchX, im->h*stretchY);
	if (!id) goto emalloc;

	for(i=0; i<im->w; i++)
	{
		for(j=0; j<im->h; j++)
		{
			for (k=0; k<stretchX; k++)
			{
				p = imageGetPixel(im, i, j);
				for (l=0; l<stretchY; l++)
				{
					imagePutPixel(id, i*stretchX + k, j*stretchY + l, p);
				}
			}
		}
	}

	return id;

emalloc:
	if (id->p) free(id->p);
	if (id) free(id);
	return 0;
}

static ImageRef rgbToRgb_16Colour(ImageRef im, int stretchX, int stretchY)
{
	ImageRef id;
	Pixel p;
	int i, j, k, l;

	id = imageNew(im->w*stretchX, im->h*stretchY);
	if (!id) goto emalloc;

	for(i=0; i<im->w; i++)
	{
		for(j=0; j<im->h; j++)
		{
			for (k=0; k<stretchX; k++)
			{
				p = imageGetPixel(im, i, j);
				p = pal[rgbToBlock_Pixel(p)];
				for (l=0; l<stretchY; l++)
				{
					imagePutPixel(id, i*stretchX + k, j*stretchY + l, p);
				}
			}
		}
	}

	return id;

emalloc:
	if (id->p) free(id->p);
	if (id) free(id);
	return 0;
}

// DitherType
// 1 - Floyd Steinberg 80
//
/*
static ImageRef rgbDither(ImageRef im, int ditherType)
{
	ImageRef imd;
	Pixel p;
	int i, j;

	imd = imageNew(im->w, im->h);
	if (!imd) goto emalloc;

	for(i=0; i<im->w; i++)
	{
		for(j=0; j<im->h; j++)
		{
			p = imageGetPixel(im, i, j);
			imagePutPixel(imd, i, j, p);
		}
	}

	return imd;

emalloc:
	if (imd->p) free(imd->p);
	if (imd) free(imd);
	return 0;
}
*/
/*
static BlockImageRef blockDither(BlockImageRef ib, int ditherType)
{
	BlockImageRef ibd;
	int i, j, b;

	ibd = blockImageNew(ib->w, ib->h);
	if (!ibd) goto emalloc;

	for(i=0; i<ib->w; i++)
	{
		for(j=0; j<ib->h; j++)
		{
			b = blockImageGetBlock(ib, i, j);
			blockImagePutBlock(ibd, i, j, b);
		}
	}

	return ibd;

emalloc:
	if (ibd->b) free(ibd->b);
	if (ibd) free(ibd);
	return 0;

}
*/

static int block_GetClosest (ImageRef im, int x, int y, int *actBlockListOffset, int limitPrevBlock, int limitNextBlock)
{
	int i, j, k, prevListIndex, nextListIndex;
	Pixel im_p0, im_p1, im_p2, im_p3, p0, p1, p2, p3;
	int b0, b1, b2, b3;
	int bi;
	double d, pd, bd;

	im_p0 = imageGetPixel(im, x, y);
	im_p1 = imageGetPixel(im, x+1, y);
	im_p2 = imageGetPixel(im, x+2, y);
	im_p3 = imageGetPixel(im, x+3, y);

	bd = 0x7fffffff; // big number;
	bi = 0;

	if (optClosestBlock) {

		// Find the closest storage block.
		for(i=0; i<159; i++)
		{
			b0 = (actBlockList[i].pixelBlock & 0xF000) >> 12;
			b1 = (actBlockList[i].pixelBlock & 0x0F00) >> 8;
			b2 = (actBlockList[i].pixelBlock & 0x00F0) >> 4;
			b3 = (actBlockList[i].pixelBlock & 0x000F);

			p0 = pal[b0];
			p1 = pal[b1];
			p2 = pal[b2];
			p3 = pal[b3];

			pd = 0;
			d = pixelDist(im_p0, p0);
			pd = pd + d;
			d = pixelDist(im_p1, p1);
			pd = pd + d;
			d = pixelDist(im_p2, p2);
			pd = pd + d;
			d = pixelDist(im_p3, p3);
			pd = pd + d;

			actBlockList[i].matchAmount = (int) pd;

			if (((pd < bd)&&(optClosestLessThan == 0)) || ((pd <= bd)&&(optClosestLessThan == 1))) {
//			if ((pd < bd)||(i == 0)) {
				if (((limitPrevBlock < 0)&&(limitNextBlock < 0))||(i == 0)) {
					// No limit from prev or next block
					bd = pd;
					bi = i;
				} else {
					if ((limitPrevBlock >= 0)&&(limitNextBlock >= 0)) {
						// Limit to prev and next block
						prevListIndex = actBlockList[i].prevBlockList;
						j = 0;
						while((j < 8) && (adjBlockList[prevListIndex][j] != -1)) {
							if (adjBlockList[prevListIndex][j] == limitPrevBlock) {
								nextListIndex = actBlockList[i].nextBlockList;
								k = 0;
								while((k < 8) && (adjBlockList[nextListIndex][k] != -1)) {
									if (adjBlockList[nextListIndex][k] == limitNextBlock) {
										bd = pd;
										bi = i;
									}
									k += 1;
								}
							}
							j += 1;
						}
					} else {
						if (limitPrevBlock >= 0) {
							// Limit to prev block
							prevListIndex = actBlockList[i].prevBlockList;
							j = 0;
							while((j < 8) && (adjBlockList[prevListIndex][j] != -1)) {
								if (adjBlockList[prevListIndex][j] == limitPrevBlock) {
									bd = pd;
									bi = i;
								}
								j += 1;
							}
						} else {
							// Limit to next block
							nextListIndex = actBlockList[i].nextBlockList;
							k = 0;
							while((k < 8) && (adjBlockList[nextListIndex][k] != -1)) {
								if (adjBlockList[nextListIndex][k] == limitNextBlock) {
									bd = pd;
									bi = i;
								}
								k += 1;
							}
						}
					}
				}
			}

			*actBlockListOffset = bi;
		}

		return actBlockList[bi].currBlock;

	} else {

		// Find the closest storage block.
		for(i=0; i<16; i++)
		{
			p0 = pal[i];

			pd = 0;
			d = pixelDist(im_p0, p0);
			pd = pd + d;
			d = pixelDist(im_p1, p0);
			pd = pd + d;
			d = pixelDist(im_p2, p0);
			pd = pd + d;
			d = pixelDist(im_p3, p0);
			pd = pd + d;

			if (((pd < bd)&&(optClosestLessThan == 0)) || ((pd <= bd)&&(optClosestLessThan == 1))) {
				bd = pd;
				bi = i;
			}

			*actBlockListOffset = 0;
		}
		return bi;

	}
}


/***************************************************************************************************************************/
/* Ostromoukhov */

	static int const table[][3] =
	{
	     {13, 0, 5}, {13, 0, 5}, {21, 0, 10}, {7, 0, 4},
	     {8, 0, 5}, {47, 3, 28}, {23, 3, 13}, {15, 3, 8},
	     {22, 6, 11}, {43, 15, 20}, {7, 3, 3}, {501, 224, 211},
	     {249, 116, 103}, {165, 80, 67}, {123, 62, 49}, {489, 256, 191},
	     {81, 44, 31}, {483, 272, 181}, {60, 35, 22}, {53, 32, 19},
	     {237, 148, 83}, {471, 304, 161}, {3, 2, 1}, {481, 314, 185},
	     {354, 226, 155}, {1389, 866, 685}, {227, 138, 125}, {267, 158, 163},
	     {327, 188, 220}, {61, 34, 45}, {627, 338, 505}, {1227, 638, 1075},
	     {20, 10, 19}, {1937, 1000, 1767}, {977, 520, 855}, {657, 360, 551},
	     {71, 40, 57}, {2005, 1160, 1539}, {337, 200, 247}, {2039, 1240, 1425},
	     {257, 160, 171}, {691, 440, 437}, {1045, 680, 627}, {301, 200, 171},
	     {177, 120, 95}, {2141, 1480, 1083}, {1079, 760, 513}, {725, 520, 323},
	     {137, 100, 57}, {2209, 1640, 855}, {53, 40, 19}, {2243, 1720, 741},
	     {565, 440, 171}, {759, 600, 209}, {1147, 920, 285}, {2311, 1880, 513},
	     {97, 80, 19}, {335, 280, 57}, {1181, 1000, 171}, {793, 680, 95},
	     {599, 520, 57}, {2413, 2120, 171}, {405, 360, 19}, {2447, 2200, 57},
	     {11, 10, 0}, {158, 151, 3}, {178, 179, 7}, {1030, 1091, 63},
	     {248, 277, 21}, {318, 375, 35}, {458, 571, 63}, {878, 1159, 147},
	     {5, 7, 1}, {172, 181, 37}, {97, 76, 22}, {72, 41, 17},
	     {119, 47, 29}, {4, 1, 1}, {4, 1, 1}, {4, 1, 1},
	     {4, 1, 1}, {4, 1, 1}, {4, 1, 1}, {4, 1, 1},
	     {4, 1, 1}, {4, 1, 1}, {65, 18, 17}, {95, 29, 26},
	     {185, 62, 53}, {30, 11, 9}, {35, 14, 11}, {85, 37, 28},
	     {55, 26, 19}, {80, 41, 29}, {155, 86, 59}, {5, 3, 2},
	     {5, 3, 2}, {5, 3, 2}, {5, 3, 2}, {5, 3, 2},
	     {5, 3, 2}, {5, 3, 2}, {5, 3, 2}, {5, 3, 2},
	     {5, 3, 2}, {5, 3, 2}, {5, 3, 2}, {5, 3, 2},
	     {305, 176, 119}, {155, 86, 59}, {105, 56, 39}, {80, 41, 29},
	     {65, 32, 23}, {55, 26, 19}, {335, 152, 113}, {85, 37, 28},
	     {115, 48, 37}, {35, 14, 11}, {355, 136, 109}, {30, 11, 9},
	     {365, 128, 107}, {185, 62, 53}, {25, 8, 7}, {95, 29, 26},
	     {385, 112, 103}, {65, 18, 17}, {395, 104, 101}, {4, 1, 1}
	};

/***************************************************************************************************************************/

static int image_Dither (ImageRef im, int x, int y, unsigned int actBlockColourCur, int ditherType, int reverse)
{

	Pixel im_p0, im_p1, im_p2, im_p3, p0, p1, p2, p3, i, i0, i1, i2, i3;
	PixelError e, d;
	Pixel c0, c1, c2, c3;
	int bc0, bc1, bc2, bc3;
	int singleline, s;
	PixelError e0, e1, e2, e3, ee0, ee1, ee2, ee3;

	if (im->h == 1) {
		singleline = 1;
	} else {
		singleline = 0;
	}

	im_p0 = imageGetPixel(im, x, y);
	im_p1 = imageGetPixel(im, x+1, y);
	im_p2 = imageGetPixel(im, x+2, y);
	im_p3 = imageGetPixel(im, x+3, y);

	bc0 = (actBlockColourCur & 0xF000) >> 12;
	bc1 = (actBlockColourCur & 0x0F00) >> 8;
	bc2 = (actBlockColourCur & 0x00F0) >> 4;
	bc3 = (actBlockColourCur & 0x000F);

	c0 = pal[bc0];
	c1 = pal[bc1];
	c2 = pal[bc2];
	c3 = pal[bc3];

	switch (ditherType) {
	case 0:

		// Ostromoukhov - Coarse

		// Determine the average block error for each colour component (R/G/B).
		e.r = ((im_p0.r - c0.r) + (im_p1.r - c1.r) + (im_p2.r - c2.r) + (im_p3.r - c3.r))/4;
		e.g = ((im_p0.g - c0.g) + (im_p1.g - c1.g) + (im_p2.g - c2.g) + (im_p3.g - c3.g))/4;
		e.b = ((im_p0.b - c0.b) + (im_p1.b - c1.b) + (im_p2.b - c2.b) + (im_p3.b - c3.b))/4;

		i.r = (im_p0.r + im_p1.r + im_p2.r + im_p3.r)/4;
		if(i.r > 127) i.r = 255 - i.r;
		e.r /= table[i.r][0] + table[i.r][1] + table[i.r][2];

		i.g = (im_p0.g + im_p1.g + im_p2.g + im_p3.g)/4;
	    if(i.g > 127) i.g = 255 - i.g;
	    e.g /= table[i.g][0] + table[i.g][1] + table[i.g][2];

		i.b = (im_p0.b + im_p1.b + im_p2.b + im_p3.b)/4;
	    if(i.b > 127) i.b = 255 - i.b;
	    e.b /= table[i.b][0] + table[i.b][1] + table[i.b][2];

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), e.r*table[i.r][0], e.g*table[i.g][0], e.b*table[i.b][0]));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), e.r*table[i.r][0], e.g*table[i.g][0], e.b*table[i.b][0]));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), e.r*table[i.r][0], e.g*table[i.g][0], e.b*table[i.b][0]));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), e.r*table[i.r][0], e.g*table[i.g][0], e.b*table[i.b][0]));

		if (!singleline) {
			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e.r*table[i.r][2], e.g*table[i.g][2], e.b*table[i.b][2]));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e.r*table[i.r][2], e.g*table[i.g][2], e.b*table[i.b][2]));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e.r*table[i.r][2], e.g*table[i.g][2], e.b*table[i.b][2]));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e.r*table[i.r][2], e.g*table[i.g][2], e.b*table[i.b][2]));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), e.r*table[i.r][1], e.g*table[i.g][1], e.b*table[i.b][1]));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), e.r*table[i.r][1], e.g*table[i.g][1], e.b*table[i.b][1]));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), e.r*table[i.r][1], e.g*table[i.g][1], e.b*table[i.b][1]));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), e.r*table[i.r][1], e.g*table[i.g][1], e.b*table[i.b][1]));
		}
		break;
	case 1:

		// Ostromoukhov - Fine1

		if (!reverse) {
			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			i0.r = im_p0.r;
			if(i0.r > 127) i0.r = 255 - i0.r;
			ee0.r = e0.r/(table[i0.r][0] + table[i0.r][1] + table[i0.r][2]);

			i0.g = im_p0.g;
			if(i0.g > 127) i0.g = 255 - i0.g;
			ee0.g = e0.g/(table[i0.g][0] + table[i0.g][1] + table[i0.g][2]);

			i0.b = im_p0.b;
			if(i0.b > 127) i0.b = 255 - i0.b;
			ee0.b = e0.b/(table[i0.b][0] + table[i0.b][1] + table[i0.b][2]);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), ee0.r*table[i0.r][1], ee0.g*table[i0.g][1], ee0.b*table[i0.b][1]));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), ee0.r*table[i0.r][2], ee0.g*table[i0.g][2], ee0.b*table[i0.b][2]));
			}			

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			i1.r = im_p1.r;
			if(i1.r > 127) i1.r = 255 - i1.r;
			ee1.r = e1.r/(table[i1.r][0] + table[i1.r][1] + table[i1.r][2]);

			i1.g = im_p1.g;
			if(i1.g > 127) i1.g = 255 - i1.g;
			ee1.g = e1.g/(table[i1.g][0] + table[i1.g][1] + table[i1.g][2]);

			i1.b = im_p1.b;
			if(i1.b > 127) i1.b = 255 - i1.b;
			ee1.b = e1.b/(table[i1.b][0] + table[i1.b][1] + table[i1.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), ee1.r*table[i1.r][1], ee1.g*table[i1.g][1], ee1.b*table[i1.b][1]));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee1.r*table[i1.r][2], ee1.g*table[i1.g][2], ee1.b*table[i1.b][2]));
			}

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			i2.r = im_p2.r;
			if(i2.r > 127) i2.r = 255 - i2.r;
			ee2.r = e2.r/(table[i2.r][0] + table[i2.r][1] + table[i2.r][2]);

			i2.g = im_p2.g;
			if(i2.g > 127) i2.g = 255 - i2.g;
			ee2.g = e2.g/(table[i2.g][0] + table[i2.g][1] + table[i2.g][2]);

			i2.b = im_p2.b;
			if(i2.b > 127) i2.b = 255 - i2.b;
			ee2.b = e2.b/(table[i2.b][0] + table[i2.b][1] + table[i2.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee2.r*table[i2.r][1], ee2.g*table[i2.g][1], ee2.b*table[i2.b][1]));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee2.r*table[i2.r][2], ee2.g*table[i2.g][2], ee2.b*table[i2.b][2]));
			}

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			i3.r = im_p3.r;
			if(i3.r > 127) i3.r = 255 - i3.r;
			ee3.r = e3.r/(table[i3.r][0] + table[i3.r][1] + table[i3.r][2]);

			i3.g = im_p3.g;
			if(i3.g > 127) i3.g = 255 - i3.g;
			ee3.g = e3.g/(table[i3.g][0] + table[i3.g][1] + table[i3.g][2]);

			i3.b = im_p3.b;
			if(i3.b > 127) i3.b = 255 - i3.b;
			ee3.b = e3.b/(table[i3.b][0] + table[i3.b][1] + table[i3.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee3.r*table[i3.r][1], ee3.g*table[i3.g][1], ee3.b*table[i3.b][1]));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), ee3.r*table[i3.r][2], ee3.g*table[i3.g][2], ee3.b*table[i3.b][2]));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), ee3.r*table[i3.r][0] + ee2.r*table[i2.r][0] + ee1.r*table[i1.r][0] + ee0.r*table[i0.r][0], ee3.g*table[i3.g][0] + ee2.g*table[i2.g][0] + ee1.g*table[i1.g][0] + ee0.g*table[i0.g][0], ee3.b*table[i3.b][0] + ee2.b*table[i2.b][0] + ee1.b*table[i1.b][0] + ee0.b*table[i0.b][0]));

		} else {

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			i3.r = im_p3.r;
			if(i3.r > 127) i3.r = 255 - i3.r;
			ee3.r = e3.r/(table[i3.r][0] + table[i3.r][1] + table[i3.r][2]);

			i3.g = im_p3.g;
			if(i3.g > 127) i3.g = 255 - i3.g;
			ee3.g = e3.g/(table[i3.g][0] + table[i3.g][1] + table[i3.g][2]);

			i3.b = im_p3.b;
			if(i3.b > 127) i3.b = 255 - i3.b;
			ee3.b = e3.b/(table[i3.b][0] + table[i3.b][1] + table[i3.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), ee3.r*table[i3.r][2], ee3.g*table[i3.g][2], ee3.b*table[i3.b][2]));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), ee3.r*table[i3.r][1], ee3.g*table[i3.g][1], ee3.b*table[i3.b][1]));
			}			

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			i2.r = im_p2.r;
			if(i2.r > 127) i2.r = 255 - i2.r;
			ee2.r = e2.r/(table[i2.r][0] + table[i2.r][1] + table[i2.r][2]);

			i2.g = im_p2.g;
			if(i2.g > 127) i2.g = 255 - i2.g;
			ee2.g = e2.g/(table[i2.g][0] + table[i2.g][1] + table[i2.g][2]);

			i2.b = im_p2.b;
			if(i2.b > 127) i2.b = 255 - i2.b;
			ee2.b = e2.b/(table[i2.b][0] + table[i2.b][1] + table[i2.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee2.r*table[i2.r][2], ee2.g*table[i2.g][2], ee2.b*table[i2.b][2]));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), ee2.r*table[i2.r][1], ee2.g*table[i2.g][1], ee2.b*table[i2.b][1]));
			}

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			i1.r = im_p1.r;
			if(i1.r > 127) i1.r = 255 - i1.r;
			ee1.r = e1.r/(table[i1.r][0] + table[i1.r][1] + table[i1.r][2]);

			i1.g = im_p1.g;
			if(i1.g > 127) i1.g = 255 - i1.g;
			ee1.g = e1.g/(table[i1.g][0] + table[i1.g][1] + table[i1.g][2]);

			i1.b = im_p1.b;
			if(i1.b > 127) i1.b = 255 - i1.b;
			ee1.b = e1.b/(table[i1.b][0] + table[i1.b][1] + table[i1.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee1.r*table[i1.r][2], ee1.g*table[i1.g][2], ee1.b*table[i1.b][2]));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee1.r*table[i1.r][1], ee1.g*table[i1.g][1], ee1.b*table[i1.b][1]));
			}

			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			i0.r = im_p0.r;
			if(i0.r > 127) i0.r = 255 - i0.r;
			ee0.r = e0.r/(table[i0.r][0] + table[i0.r][1] + table[i0.r][2]);

			i0.g = im_p0.g;
			if(i0.g > 127) i0.g = 255 - i0.g;
			ee0.g = e0.g/(table[i0.g][0] + table[i0.g][1] + table[i0.g][2]);

			i0.b = im_p0.b;
			if(i0.b > 127) i0.b = 255 - i0.b;
			ee0.b = e0.b/(table[i0.b][0] + table[i0.b][1] + table[i0.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), ee0.r*table[i0.r][2], ee0.g*table[i0.g][2], ee0.b*table[i0.b][2]));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee0.r*table[i0.r][1], ee0.g*table[i0.g][1], ee0.b*table[i0.b][1]));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), ee3.r*table[i3.r][0] + ee2.r*table[i2.r][0] + ee1.r*table[i1.r][0] + ee0.r*table[i0.r][0], ee3.g*table[i3.g][0] + ee2.g*table[i2.g][0] + ee1.g*table[i1.g][0] + ee0.g*table[i0.g][0], ee3.b*table[i3.b][0] + ee2.b*table[i2.b][0] + ee1.b*table[i1.b][0] + ee0.b*table[i0.b][0]));
		}
		break;
	case 2:

		// Ostromoukhov - Fine2

		if (!reverse) {
			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			i0.r = im_p0.r;
			if(i0.r > 127) i0.r = 255 - i0.r;
			ee0.r = e0.r/(table[i0.r][0] + table[i0.r][1] + table[i0.r][2]);

			i0.g = im_p0.g;
			if(i0.g > 127) i0.g = 255 - i0.g;
			ee0.g = e0.g/(table[i0.g][0] + table[i0.g][1] + table[i0.g][2]);

			i0.b = im_p0.b;
			if(i0.b > 127) i0.b = 255 - i0.b;
			ee0.b = e0.b/(table[i0.b][0] + table[i0.b][1] + table[i0.b][2]);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), ee0.r*table[i0.r][1], ee0.g*table[i0.g][1], ee0.b*table[i0.b][1]));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), ee0.r*table[i0.r][2], ee0.g*table[i0.g][2], ee0.b*table[i0.b][2]));
			}			

			e1.r = (im_p1.r - c1.r) + (ee0.r * table[i0.r][0]);
			e1.g = (im_p1.g - c1.g) + (ee0.g * table[i0.g][0]);
			e1.b = (im_p1.b - c1.b) + (ee0.b * table[i0.b][0]);

			i1.r = im_p1.r;
			if(i1.r > 127) i1.r = 255 - i1.r;
			ee1.r = e1.r/(table[i1.r][0] + table[i1.r][1] + table[i1.r][2]);

			i1.g = im_p1.g;
			if(i1.g > 127) i1.g = 255 - i1.g;
			ee1.g = e1.g/(table[i1.g][0] + table[i1.g][1] + table[i1.g][2]);

			i1.b = im_p1.b;
			if(i1.b > 127) i1.b = 255 - i1.b;
			ee1.b = e1.b/(table[i1.b][0] + table[i1.b][1] + table[i1.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), ee1.r*table[i1.r][1], ee1.g*table[i1.g][1], ee1.b*table[i1.b][1]));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee1.r*table[i1.r][2], ee1.g*table[i1.g][2], ee1.b*table[i1.b][2]));
			}

			e2.r = (im_p2.r - c2.r) + (ee1.r * table[i1.r][0]);
			e2.g = (im_p2.g - c2.g) + (ee1.g * table[i1.g][0]);
			e2.b = (im_p2.b - c2.b) + (ee1.b * table[i1.b][0]);

			i2.r = im_p2.r;
			if(i2.r > 127) i2.r = 255 - i2.r;
			ee2.r = e2.r/(table[i2.r][0] + table[i2.r][1] + table[i2.r][2]);

			i2.g = im_p2.g;
			if(i2.g > 127) i2.g = 255 - i2.g;
			ee2.g = e2.g/(table[i2.g][0] + table[i2.g][1] + table[i2.g][2]);

			i2.b = im_p2.b;
			if(i2.b > 127) i2.b = 255 - i2.b;
			ee2.b = e2.b/(table[i2.b][0] + table[i2.b][1] + table[i2.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee2.r*table[i2.r][1], ee2.g*table[i2.g][1], ee2.b*table[i2.b][1]));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee2.r*table[i2.r][2], ee2.g*table[i2.g][2], ee2.b*table[i2.b][2]));
			}

			e3.r = (im_p3.r - c3.r) + (ee2.r * table[i2.r][0]);
			e3.g = (im_p3.g - c3.g) + (ee2.g * table[i2.g][0]);
			e3.b = (im_p3.b - c3.b) + (ee2.b * table[i2.b][0]);

			i3.r = im_p3.r;
			if(i3.r > 127) i3.r = 255 - i3.r;
			ee3.r = e3.r/(table[i3.r][0] + table[i3.r][1] + table[i3.r][2]);

			i3.g = im_p3.g;
			if(i3.g > 127) i3.g = 255 - i3.g;
			ee3.g = e3.g/(table[i3.g][0] + table[i3.g][1] + table[i3.g][2]);

			i3.b = im_p3.b;
			if(i3.b > 127) i3.b = 255 - i3.b;
			ee3.b = e3.b/(table[i3.b][0] + table[i3.b][1] + table[i3.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee3.r*table[i3.r][1], ee3.g*table[i3.g][1], ee3.b*table[i3.b][1]));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), ee3.r*table[i3.r][2], ee3.g*table[i3.g][2], ee3.b*table[i3.b][2]));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), ee3.r*table[i3.r][0], ee3.g*table[i3.g][0], ee3.b*table[i3.b][0]));

		} else {

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			i3.r = im_p3.r;
			if(i3.r > 127) i3.r = 255 - i3.r;
			ee3.r = e3.r/(table[i3.r][0] + table[i3.r][1] + table[i3.r][2]);

			i3.g = im_p3.g;
			if(i3.g > 127) i3.g = 255 - i3.g;
			ee3.g = e3.g/(table[i3.g][0] + table[i3.g][1] + table[i3.g][2]);

			i3.b = im_p3.b;
			if(i3.b > 127) i3.b = 255 - i3.b;
			ee3.b = e3.b/(table[i3.b][0] + table[i3.b][1] + table[i3.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), ee3.r*table[i3.r][2], ee3.g*table[i3.g][2], ee3.b*table[i3.b][2]));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), ee3.r*table[i3.r][1], ee3.g*table[i3.g][1], ee3.b*table[i3.b][1]));
			}			

			e2.r = (im_p2.r - c2.r) + (ee3.r * table[i3.r][0]);
			e2.g = (im_p2.g - c2.g) + (ee3.g * table[i3.g][0]);
			e2.b = (im_p2.b - c2.b) + (ee3.b * table[i3.b][0]);

			i2.r = im_p2.r;
			if(i2.r > 127) i2.r = 255 - i2.r;
			ee2.r = e2.r/(table[i2.r][0] + table[i2.r][1] + table[i2.r][2]);

			i2.g = im_p2.g;
			if(i2.g > 127) i2.g = 255 - i2.g;
			ee2.g = e2.g/(table[i2.g][0] + table[i2.g][1] + table[i2.g][2]);

			i2.b = im_p2.b;
			if(i2.b > 127) i2.b = 255 - i2.b;
			ee2.b = e2.b/(table[i2.b][0] + table[i2.b][1] + table[i2.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee2.r*table[i2.r][2], ee2.g*table[i2.g][2], ee2.b*table[i2.b][2]));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), ee2.r*table[i2.r][1], ee2.g*table[i2.g][1], ee2.b*table[i2.b][1]));
			}

			e1.r = (im_p1.r - c1.r) + (ee2.r * table[i2.r][0]);
			e1.g = (im_p1.g - c1.g) + (ee2.g * table[i2.g][0]);
			e1.b = (im_p1.b - c1.b) + (ee2.b * table[i2.b][0]);

			i1.r = im_p1.r;
			if(i1.r > 127) i1.r = 255 - i1.r;
			ee1.r = e1.r/(table[i1.r][0] + table[i1.r][1] + table[i1.r][2]);

			i1.g = im_p1.g;
			if(i1.g > 127) i1.g = 255 - i1.g;
			ee1.g = e1.g/(table[i1.g][0] + table[i1.g][1] + table[i1.g][2]);

			i1.b = im_p1.b;
			if(i1.b > 127) i1.b = 255 - i1.b;
			ee1.b = e1.b/(table[i1.b][0] + table[i1.b][1] + table[i1.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee1.r*table[i1.r][2], ee1.g*table[i1.g][2], ee1.b*table[i1.b][2]));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), ee1.r*table[i1.r][1], ee1.g*table[i1.g][1], ee1.b*table[i1.b][1]));
			}

			e0.r = (im_p0.r - c0.r) + (ee1.r * table[i1.r][0]);
			e0.g = (im_p0.g - c0.g) + (ee1.g * table[i1.g][0]);
			e0.b = (im_p0.b - c0.b) + (ee1.b * table[i1.b][0]);

			i0.r = im_p0.r;
			if(i0.r > 127) i0.r = 255 - i0.r;
			ee0.r = e0.r/(table[i0.r][0] + table[i0.r][1] + table[i0.r][2]);

			i0.g = im_p0.g;
			if(i0.g > 127) i0.g = 255 - i0.g;
			ee0.g = e0.g/(table[i0.g][0] + table[i0.g][1] + table[i0.g][2]);

			i0.b = im_p0.b;
			if(i0.b > 127) i0.b = 255 - i0.b;
			ee0.b = e0.b/(table[i0.b][0] + table[i0.b][1] + table[i0.b][2]);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), ee0.r*table[i0.r][2], ee0.g*table[i0.g][2], ee0.b*table[i0.b][2]));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), ee0.r*table[i0.r][1], ee0.g*table[i0.g][1], ee0.b*table[i0.b][1]));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), ee0.r*table[i0.r][0], ee0.g*table[i0.g][0], ee0.b*table[i0.b][0]));

		}
		break;
	case 3:

		// Floyd-Steinberg - Coarse - Average

		// Determine the average block error for each colour component (R/G/B).
		e.r = ((im_p0.r - c0.r) + (im_p1.r - c1.r) + (im_p2.r - c2.r) + (im_p3.r - c3.r))/4;
		e.g = ((im_p0.g - c0.g) + (im_p1.g - c1.g) + (im_p2.g - c2.g) + (im_p3.g - c3.g))/4;
		e.b = ((im_p0.b - c0.b) + (im_p1.b - c1.b) + (im_p2.b - c2.b) + (im_p3.b - c3.b))/4;

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), e.r*7/16, e.g*7/16, e.b*7/16));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), e.r*7/16, e.g*7/16, e.b*7/16));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), e.r*7/16, e.g*7/16, e.b*7/16));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), e.r*7/16, e.g*7/16, e.b*7/16));

		if (!singleline) {
			imagePutPixel(im, x+(s*4)+0, y+1, alterPixel(imageGetPixel(im, x+(s*4)+0, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*4)+1, y+1, alterPixel(imageGetPixel(im, x+(s*4)+1, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*4)+2, y+1, alterPixel(imageGetPixel(im, x+(s*4)+2, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*4)+3, y+1, alterPixel(imageGetPixel(im, x+(s*4)+3, y+1), e.r*1/16, e.g*1/16, e.b*1/16));

			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e.r*5/16, e.g*5/16, e.b*5/16));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e.r*5/16, e.g*5/16, e.b*5/16));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e.r*5/16, e.g*5/16, e.b*5/16));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e.r*5/16, e.g*5/16, e.b*5/16));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), e.r*3/16, e.g*3/16, e.b*3/16));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), e.r*3/16, e.g*3/16, e.b*3/16));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), e.r*3/16, e.g*3/16, e.b*3/16));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), e.r*3/16, e.g*3/16, e.b*3/16));
		}
		break;
	case 4:

		// Floyd-Steinberg - Coarse - Ordered

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), (im_p0.r - c0.r)*7/16, (im_p0.g - c0.g)*7/16, (im_p0.b - c0.b)*7/16));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), (im_p1.r - c1.r)*7/16, (im_p1.g - c1.g)*7/16, (im_p1.b - c1.b)*7/16));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), (im_p2.r - c2.r)*7/16, (im_p2.g - c2.g)*7/16, (im_p2.b - c2.b)*7/16));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), (im_p3.r - c3.r)*7/16, (im_p3.g - c3.g)*7/16, (im_p3.b - c3.b)*7/16));

		if (!singleline) {
			imagePutPixel(im, x+(s*4)+0, y+1, alterPixel(imageGetPixel(im, x+(s*4)+0, y+1), (im_p0.r - c0.r)*1/16, (im_p0.g - c0.g)*1/16, (im_p0.b - c0.b)*1/16));
			imagePutPixel(im, x+(s*4)+1, y+1, alterPixel(imageGetPixel(im, x+(s*4)+1, y+1), (im_p1.r - c1.r)*1/16, (im_p1.g - c1.g)*1/16, (im_p1.b - c1.b)*1/16));
			imagePutPixel(im, x+(s*4)+2, y+1, alterPixel(imageGetPixel(im, x+(s*4)+2, y+1), (im_p2.r - c2.r)*1/16, (im_p2.g - c2.g)*1/16, (im_p2.b - c2.b)*1/16));
			imagePutPixel(im, x+(s*4)+3, y+1, alterPixel(imageGetPixel(im, x+(s*4)+3, y+1), (im_p3.r - c3.r)*1/16, (im_p3.g - c3.g)*1/16, (im_p3.b - c3.b)*1/16));

			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), (im_p0.r - c0.r)*5/16, (im_p0.g - c0.g)*5/16, (im_p0.b - c0.b)*5/16));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), (im_p1.r - c1.r)*5/16, (im_p1.g - c1.g)*5/16, (im_p1.b - c1.b)*5/16));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), (im_p2.r - c2.r)*5/16, (im_p2.g - c2.g)*5/16, (im_p2.b - c2.b)*5/16));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), (im_p3.r - c3.r)*5/16, (im_p3.g - c3.g)*5/16, (im_p3.b - c3.b)*5/16));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), (im_p0.r - c0.r)*3/16, (im_p0.g - c0.g)*3/16, (im_p0.b - c0.b)*3/16));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), (im_p1.r - c1.r)*3/16, (im_p1.g - c1.g)*3/16, (im_p1.b - c1.b)*3/16));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), (im_p2.r - c2.r)*3/16, (im_p2.g - c2.g)*3/16, (im_p2.b - c2.b)*3/16));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), (im_p3.r - c3.r)*3/16, (im_p3.g - c3.g)*3/16, (im_p3.b - c3.b)*3/16));
		}

		break;
	case 5:

		// Floyd-Steinberg - Coarse - Swapped

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), (im_p0.r - c0.r)*7.0/16, (im_p3.g - c3.g)*7.0/16, (im_p2.b - c2.b)*7.0/16));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), (im_p1.r - c1.r)*7.0/16, (im_p0.g - c0.g)*7.0/16, (im_p3.b - c3.b)*7.0/16));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), (im_p2.r - c2.r)*7.0/16, (im_p1.g - c1.g)*7.0/16, (im_p0.b - c0.b)*7.0/16));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), (im_p3.r - c3.r)*7.0/16, (im_p2.g - c2.g)*7.0/16, (im_p1.b - c1.b)*7.0/16));

		if (!singleline) {
			imagePutPixel(im, x+(s*4)+0, y+1, alterPixel(imageGetPixel(im, x+(s*4)+0, y+1), (im_p1.r - c1.r)*1.0/16, (im_p2.g - c2.g)*1.0/16, (im_p3.b - c3.b)*1.0/16));
			imagePutPixel(im, x+(s*4)+1, y+1, alterPixel(imageGetPixel(im, x+(s*4)+1, y+1), (im_p0.r - c0.r)*1.0/16, (im_p3.g - c3.g)*1.0/16, (im_p2.b - c2.b)*1.0/16));
			imagePutPixel(im, x+(s*4)+2, y+1, alterPixel(imageGetPixel(im, x+(s*4)+2, y+1), (im_p3.r - c3.r)*1.0/16, (im_p1.g - c1.g)*1.0/16, (im_p0.b - c0.b)*1.0/16));
			imagePutPixel(im, x+(s*4)+3, y+1, alterPixel(imageGetPixel(im, x+(s*4)+3, y+1), (im_p2.r - c2.r)*1.0/16, (im_p0.g - c0.g)*1.0/16, (im_p1.b - c1.b)*1.0/16));

			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), (im_p3.r - c3.r)*5.0/16, (im_p1.g - c1.g)*5.0/16, (im_p2.b - c2.b)*5.0/16));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), (im_p2.r - c2.r)*5.0/16, (im_p0.g - c0.g)*5.0/16, (im_p3.b - c3.b)*5.0/16));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), (im_p0.r - c0.r)*5.0/16, (im_p3.g - c3.g)*5.0/16, (im_p1.b - c1.b)*5.0/16));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), (im_p1.r - c1.r)*5.0/16, (im_p2.g - c2.g)*5.0/16, (im_p0.b - c0.b)*5.0/16));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), (im_p2.r - c2.r)*3.0/16, (im_p3.g - c3.g)*3.0/16, (im_p1.b - c1.b)*3.0/16));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), (im_p3.r - c3.r)*3.0/16, (im_p2.g - c2.g)*3.0/16, (im_p0.b - c0.b)*3.0/16));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), (im_p1.r - c1.r)*3.0/16, (im_p0.g - c0.g)*3.0/16, (im_p3.b - c3.b)*3.0/16));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), (im_p0.r - c0.r)*3.0/16, (im_p1.g - c1.g)*3.0/16, (im_p2.b - c2.b)*3.0/16));
		}

		break;
	case 6:

		// Floyd-Steinberg - Fine1

		if (!reverse) {
			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*1.0/16, e0.b*1.0/16, e0.b*1.0/16));
			}			

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
			}

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
			}

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {

				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16, e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16, e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16));

		} else {

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
			}			

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
			}

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
			}

			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*1.0/16, e0.g*1.0/16, e0.b*1.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16, e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16, e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16));
		}
		break;
	case 7:

		// Floyd-Steinberg - Fine2

		if (!reverse) {
			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*1.0/16, e0.b*1.0/16, e0.b*1.0/16));
			}			

			e1.r = (im_p1.r - c1.r) + (e0.r * 7.0/16);
			e1.g = (im_p1.g - c1.g) + (e0.g * 7.0/16);
			e1.b = (im_p1.b - c1.b) + (e0.b * 7.0/16);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
			}

			e2.r = (im_p2.r - c2.r) + (e1.r * 7.0/16);
			e2.g = (im_p2.g - c2.g) + (e1.g * 7.0/16);
			e2.b = (im_p2.b - c2.b) + (e1.b * 7.0/16);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
			}

			e3.r = (im_p3.r - c3.r) + (e2.r * 7.0/16);
			e3.g = (im_p3.g - c3.g) + (e2.g * 7.0/16);
			e3.b = (im_p3.b - c3.b) + (e2.b * 7.0/16);

			if (!singleline) {

				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e3.r*7.0/16, e3.g*7.0/16, e3.b*7.0/16));

		} else {

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
			}			

			e2.r = (im_p2.r - c2.r) + (e3.r * 7.0/16);
			e2.g = (im_p2.g - c2.g) + (e3.g * 7.0/16);
			e2.b = (im_p2.b - c2.b) + (e3.b * 7.0/16);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
			}

			e1.r = (im_p1.r - c1.r) + (e2.r * 7.0/16);
			e1.g = (im_p1.g - c1.g) + (e2.g * 7.0/16);
			e1.b = (im_p1.b - c1.b) + (e2.b * 7.0/16);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
			}

			e0.r = (im_p0.r - c0.r) + (e1.r * 7.0/16);
			e0.g = (im_p0.g - c0.g) + (e1.g * 7.0/16);
			e0.b = (im_p0.b - c0.b) + (e1.b * 7.0/16);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*1.0/16, e0.g*1.0/16, e0.b*1.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e0.r*7.0/16, e0.g*7.0/16, e0.b*7.0/16));
		}
		break;
	case 8:

		// Buckels - Coarse - Average

		// Determine the average block error for each colour component (R/G/B).
		e.r = ((im_p0.r - c0.r) + (im_p1.r - c1.r) + (im_p2.r - c2.r) + (im_p3.r - c3.r))/4;
		e.g = ((im_p0.g - c0.g) + (im_p1.g - c1.g) + (im_p2.g - c2.g) + (im_p3.g - c3.g))/4;
		e.b = ((im_p0.b - c0.b) + (im_p1.b - c1.b) + (im_p2.b - c2.b) + (im_p3.b - c3.b))/4;

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), e.r*2/8, e.g*2/8, e.b*2/8));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), e.r*2/8, e.g*2/8, e.b*2/8));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), e.r*2/8, e.g*2/8, e.b*2/8));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), e.r*2/8, e.g*2/8, e.b*2/8));

		imagePutPixel(im, x+(s*8)+0, y, alterPixel(imageGetPixel(im, x+(s*8)+0, y), e.r*1/8, e.g*1/8, e.b*1/8));
		imagePutPixel(im, x+(s*8)+1, y, alterPixel(imageGetPixel(im, x+(s*8)+1, y), e.r*1/8, e.g*1/8, e.b*1/8));
		imagePutPixel(im, x+(s*8)+2, y, alterPixel(imageGetPixel(im, x+(s*8)+2, y), e.r*1/8, e.g*1/8, e.b*1/8));
		imagePutPixel(im, x+(s*8)+3, y, alterPixel(imageGetPixel(im, x+(s*8)+3, y), e.r*1/8, e.g*1/8, e.b*1/8));

		if (!singleline) {

			imagePutPixel(im, x+(s*4)+0, y+1, alterPixel(imageGetPixel(im, x+(s*4)+0, y+1), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x+(s*4)+1, y+1, alterPixel(imageGetPixel(im, x+(s*4)+1, y+1), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x+(s*4)+2, y+1, alterPixel(imageGetPixel(im, x+(s*4)+2, y+1), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x+(s*4)+3, y+1, alterPixel(imageGetPixel(im, x+(s*4)+3, y+1), e.r*1/8, e.g*1/8, e.b*1/8));

			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e.r*2/8, e.g*2/8, e.b*2/8));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e.r*2/8, e.g*2/8, e.b*2/8));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e.r*2/8, e.g*2/8, e.b*2/8));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e.r*2/8, e.g*2/8, e.b*2/8));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), e.r*1/8, e.g*1/8, e.b*1/8));

			imagePutPixel(im, x+0+0, y+2, alterPixel(imageGetPixel(im, x+0+0, y+2), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x+0+1, y+2, alterPixel(imageGetPixel(im, x+0+1, y+2), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x+0+2, y+2, alterPixel(imageGetPixel(im, x+0+2, y+2), e.r*1/8, e.g*1/8, e.b*1/8));
			imagePutPixel(im, x+0+3, y+2, alterPixel(imageGetPixel(im, x+0+3, y+2), e.r*1/8, e.g*1/8, e.b*1/8));

		}
		break;
	case 9:

		// Buckels - Coarse - Swapped

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), (im_p0.r - c0.r)*2/8, (im_p3.g - c3.g)*2/8, (im_p2.b - c2.b)*2/8));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), (im_p1.r - c1.r)*2/8, (im_p0.g - c0.g)*2/8, (im_p3.b - c3.b)*2/8));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), (im_p2.r - c2.r)*2/8, (im_p1.g - c1.g)*2/8, (im_p0.b - c0.b)*2/8));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), (im_p3.r - c3.r)*2/8, (im_p2.g - c2.g)*2/8, (im_p1.b - c1.b)*2/8));

		imagePutPixel(im, x+(s*8)+0, y, alterPixel(imageGetPixel(im, x+(s*8)+0, y), (im_p2.r - c2.r)*1/8, (im_p0.g - c0.g)*1/8, (im_p3.b - c3.b)*1/8));
		imagePutPixel(im, x+(s*8)+1, y, alterPixel(imageGetPixel(im, x+(s*8)+1, y), (im_p3.r - c3.r)*1/8, (im_p1.g - c1.g)*1/8, (im_p0.b - c0.b)*1/8));
		imagePutPixel(im, x+(s*8)+2, y, alterPixel(imageGetPixel(im, x+(s*8)+2, y), (im_p0.r - c0.r)*1/8, (im_p2.g - c2.g)*1/8, (im_p1.b - c1.b)*1/8));
		imagePutPixel(im, x+(s*8)+3, y, alterPixel(imageGetPixel(im, x+(s*8)+3, y), (im_p1.r - c1.r)*1/8, (im_p3.g - c3.g)*1/8, (im_p2.b - c2.b)*1/8));


		if (!singleline) {
			imagePutPixel(im, x+(s*4)+0, y+1, alterPixel(imageGetPixel(im, x+(s*4)+0, y+1), (im_p1.r - c1.r)*1/8, (im_p2.g - c2.g)*1/8, (im_p3.b - c3.b)*1/8));
			imagePutPixel(im, x+(s*4)+1, y+1, alterPixel(imageGetPixel(im, x+(s*4)+1, y+1), (im_p0.r - c0.r)*1/8, (im_p3.g - c3.g)*1/8, (im_p2.b - c2.b)*1/8));
			imagePutPixel(im, x+(s*4)+2, y+1, alterPixel(imageGetPixel(im, x+(s*4)+2, y+1), (im_p3.r - c3.r)*1/8, (im_p1.g - c1.g)*1/8, (im_p0.b - c0.b)*1/8));
			imagePutPixel(im, x+(s*4)+3, y+1, alterPixel(imageGetPixel(im, x+(s*4)+3, y+1), (im_p2.r - c2.r)*1/8, (im_p0.g - c0.g)*1/8, (im_p1.b - c1.b)*1/8));

			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), (im_p3.r - c3.r)*2/8, (im_p1.g - c1.g)*2/8, (im_p2.b - c2.b)*2/8));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), (im_p2.r - c2.r)*2/8, (im_p0.g - c0.g)*2/8, (im_p3.b - c3.b)*2/8));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), (im_p0.r - c0.r)*2/8, (im_p3.g - c3.g)*2/8, (im_p1.b - c1.b)*2/8));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), (im_p1.r - c1.r)*2/8, (im_p2.g - c2.g)*2/8, (im_p0.b - c0.b)*2/8));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), (im_p2.r - c2.r)*1/8, (im_p3.g - c3.g)*1/8, (im_p1.b - c1.b)*1/8));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), (im_p3.r - c3.r)*1/8, (im_p2.g - c2.g)*1/8, (im_p0.b - c0.b)*1/8));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), (im_p1.r - c1.r)*1/8, (im_p0.g - c0.g)*1/8, (im_p3.b - c3.b)*1/8));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), (im_p0.r - c0.r)*1/8, (im_p1.g - c1.g)*1/8, (im_p2.b - c2.b)*1/8));

			imagePutPixel(im, x+0+0, y+2, alterPixel(imageGetPixel(im, x+0+0, y+2), (im_p1.r - c1.r)*1/8, (im_p2.g - c2.g)*1/8, (im_p3.b - c3.b)*1/8));
			imagePutPixel(im, x+0+1, y+2, alterPixel(imageGetPixel(im, x+0+1, y+2), (im_p0.r - c0.r)*1/8, (im_p3.g - c3.g)*1/8, (im_p2.b - c2.b)*1/8));
			imagePutPixel(im, x+0+2, y+2, alterPixel(imageGetPixel(im, x+0+2, y+2), (im_p3.r - c3.r)*1/8, (im_p1.g - c1.g)*1/8, (im_p0.b - c0.b)*1/8));
			imagePutPixel(im, x+0+3, y+2, alterPixel(imageGetPixel(im, x+0+3, y+2), (im_p2.r - c2.r)*1/8, (im_p0.g - c0.g)*1/8, (im_p1.b - c1.b)*1/8));
		}

		break;
	case 10:

		// Buckels - Fine
		e0.r = 0;
		e0.g = 0;
		e0.b = 0;
		e1.r = 0;
		e1.g = 0;
		e1.b = 0;
		e2.r = 0;
		e2.g = 0;
		e2.b = 0;
		e3.r = 0;
		e3.g = 0;
		e3.b = 0;

		if (!reverse) {
			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*1.0/8, e0.g*1.0/8, e0.b*1.0/8));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*2.0/8, e0.g*2.0/8, e0.b*2.0/8));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*1.0/8, e0.b*1.0/8, e0.b*1.0/8));
				imagePutPixel(im, x+0+0, y+2, alterPixel(imageGetPixel(im, x+0+0, y+2), e0.r*1.0/8, e0.g*1.0/8, e0.b*1.0/8));
			}			

			e1.r = e1.r + (e0.r * 2.0/8);
			e1.g = e1.g + (e0.g * 2.0/8);
			e1.b = e1.b + (e0.b * 2.0/8);

			e2.r = e2.r + (e0.r * 1.0/8);
			e2.g = e2.g + (e0.g * 1.0/8);
			e2.b = e2.b + (e0.b * 1.0/8);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*2.0/8, e1.g*2.0/8, e1.b*2.0/8));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));
				imagePutPixel(im, x+0+1, y+2, alterPixel(imageGetPixel(im, x+0+1, y+2), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));
			}

			e2.r = e2.r + (e1.r * 2.0/8);
			e2.g = e2.g + (e1.g * 2.0/8);
			e2.b = e2.b + (e1.b * 2.0/8);

			e3.r = e3.r + (e1.r * 1.0/8);
			e3.g = e3.g + (e1.g * 1.0/8);
			e3.b = e3.b + (e1.b * 1.0/8);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*2.0/8, e2.g*2.0/8, e2.b*2.0/8));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));
				imagePutPixel(im, x+0+2, y+2, alterPixel(imageGetPixel(im, x+0+2, y+2), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));
			}

			e3.r = e3.r + (e2.r * 2.0/8);
			e3.g = e3.g + (e2.g * 2.0/8);
			e3.b = e3.b + (e2.b * 2.0/8);

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));


			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {

				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*2.0/8, e3.g*2.0/8, e3.b*2.0/8));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));
				imagePutPixel(im, x+0+3, y+2, alterPixel(imageGetPixel(im, x+0+3, y+2), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e3.r*2.0/8, e3.g*2.0/8, e3.b*2.0/8));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));

		} else {

			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*2.0/8, e3.g*2.0/8, e3.b*2.0/8));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));
				imagePutPixel(im, x+0+3, y+2, alterPixel(imageGetPixel(im, x+0+3, y+2), e3.r*1.0/8, e3.g*1.0/8, e3.b*1.0/8));
			}			

			e2.r = e2.r + (e3.r * 2.0/8);
			e2.g = e2.g + (e3.g * 2.0/8);
			e2.b = e2.b + (e3.b * 2.0/8);

			e1.r = e1.r + (e3.r * 1.0/8);
			e1.g = e1.g + (e3.g * 1.0/8);
			e1.b = e1.b + (e3.b * 1.0/8);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*2.0/8, e2.g*2.0/8, e2.b*2.0/8));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));
				imagePutPixel(im, x+0+2, y+2, alterPixel(imageGetPixel(im, x+0+2, y+2), e2.r*1.0/8, e2.g*1.0/8, e2.b*1.0/8));
			}

			e1.r = e1.r + (e2.r * 2.0/8);
			e1.g = e1.g + (e2.g * 2.0/8);
			e1.b = e1.b + (e2.b * 2.0/8);

			e0.r = e0.r + (e2.r * 1.0/8);
			e0.g = e0.g + (e2.g * 1.0/8);
			e0.b = e0.b + (e2.b * 1.0/8);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*2.0/8, e1.g*2.0/8, e1.b*2.0/8));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));
				imagePutPixel(im, x+0+1, y+2, alterPixel(imageGetPixel(im, x+0+1, y+2), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));
			}

			e0.r = e0.r + (e1.r * 2.0/8);
			e0.g = e0.g + (e1.g * 2.0/8);
			e0.b = e0.b + (e1.b * 2.0/8);

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e1.r*1.0/8, e1.g*1.0/8, e1.b*1.0/8));

			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*1.0/8, e0.g*1.0/8, e0.b*1.0/8));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*2.0/8, e0.g*2.0/8, e0.b*2.0/8));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*1.0/8, e0.g*1.0/8, e0.b*1.0/8));
				imagePutPixel(im, x+0+0, y+2, alterPixel(imageGetPixel(im, x+0+0, y+2), e0.r*1.0/8, e0.g*1.0/8, e0.b*1.0/8));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e0.r*2.0/8, e0.g*2.0/8, e0.b*2.0/8));
			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), e0.r*1.0/8, e0.g*1.0/8, e0.b*1.0/8));
		}
		break;
	case 11:

		// Jarvis, Judice and Ninke - Coarse - Average

		// Determine the average block error for each colour component (R/G/B).
		e.r = ((im_p0.r - c0.r) + (im_p1.r - c1.r) + (im_p2.r - c2.r) + (im_p3.r - c3.r))/4;
		e.g = ((im_p0.g - c0.g) + (im_p1.g - c1.g) + (im_p2.g - c2.g) + (im_p3.g - c3.g))/4;
		e.b = ((im_p0.b - c0.b) + (im_p1.b - c1.b) + (im_p2.b - c2.b) + (im_p3.b - c3.b))/4;

		s = reverse ? -1 : 1;

		imagePutPixel(im, x+(s*4)+0, y, alterPixel(imageGetPixel(im, x+(s*4)+0, y), e.r*7/48, e.g*7/48, e.b*7/48));
		imagePutPixel(im, x+(s*4)+1, y, alterPixel(imageGetPixel(im, x+(s*4)+1, y), e.r*7/48, e.g*7/48, e.b*7/48));
		imagePutPixel(im, x+(s*4)+2, y, alterPixel(imageGetPixel(im, x+(s*4)+2, y), e.r*7/48, e.g*7/48, e.b*7/48));
		imagePutPixel(im, x+(s*4)+3, y, alterPixel(imageGetPixel(im, x+(s*4)+3, y), e.r*7/48, e.g*7/48, e.b*7/48));

		imagePutPixel(im, x+(s*8)+0, y, alterPixel(imageGetPixel(im, x+(s*8)+0, y), e.r*5/48, e.g*5/48, e.b*5/48));
		imagePutPixel(im, x+(s*8)+1, y, alterPixel(imageGetPixel(im, x+(s*8)+1, y), e.r*5/48, e.g*5/48, e.b*5/48));
		imagePutPixel(im, x+(s*8)+2, y, alterPixel(imageGetPixel(im, x+(s*8)+2, y), e.r*5/48, e.g*5/48, e.b*5/48));
		imagePutPixel(im, x+(s*8)+3, y, alterPixel(imageGetPixel(im, x+(s*8)+3, y), e.r*5/48, e.g*5/48, e.b*5/48));


		if (!singleline) {

			imagePutPixel(im, x+(s*8)+0, y+1, alterPixel(imageGetPixel(im, x+(s*8)+0, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*8)+1, y+1, alterPixel(imageGetPixel(im, x+(s*8)+1, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*8)+2, y+1, alterPixel(imageGetPixel(im, x+(s*8)+2, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*8)+3, y+1, alterPixel(imageGetPixel(im, x+(s*8)+3, y+1), e.r*1/16, e.g*1/16, e.b*1/16));

			imagePutPixel(im, x+(s*4)+0, y+1, alterPixel(imageGetPixel(im, x+(s*4)+0, y+1), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x+(s*4)+1, y+1, alterPixel(imageGetPixel(im, x+(s*4)+1, y+1), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x+(s*4)+2, y+1, alterPixel(imageGetPixel(im, x+(s*4)+2, y+1), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x+(s*4)+3, y+1, alterPixel(imageGetPixel(im, x+(s*4)+3, y+1), e.r*5/48, e.g*5/48, e.b*5/48));

			imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e.r*7/48, e.g*7/48, e.b*7/48));
			imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e.r*7/48, e.g*7/48, e.b*7/48));
			imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e.r*7/48, e.g*7/48, e.b*7/48));
			imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e.r*7/48, e.g*7/48, e.b*7/48));

			imagePutPixel(im, x-(s*4)+0, y+1, alterPixel(imageGetPixel(im, x-(s*4)+0, y+1), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x-(s*4)+1, y+1, alterPixel(imageGetPixel(im, x-(s*4)+1, y+1), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x-(s*4)+2, y+1, alterPixel(imageGetPixel(im, x-(s*4)+2, y+1), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x-(s*4)+3, y+1, alterPixel(imageGetPixel(im, x-(s*4)+3, y+1), e.r*5/48, e.g*5/48, e.b*5/48));

			imagePutPixel(im, x-(s*8)+0, y+1, alterPixel(imageGetPixel(im, x-(s*8)+0, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x-(s*8)+1, y+1, alterPixel(imageGetPixel(im, x-(s*8)+1, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x-(s*8)+2, y+1, alterPixel(imageGetPixel(im, x-(s*8)+2, y+1), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x-(s*8)+3, y+1, alterPixel(imageGetPixel(im, x-(s*8)+3, y+1), e.r*1/16, e.g*1/16, e.b*1/16));


			imagePutPixel(im, x+(s*8)+0, y+2, alterPixel(imageGetPixel(im, x+(s*8)+0, y+2), e.r*1/48, e.g*1/48, e.b*1/48));
			imagePutPixel(im, x+(s*8)+1, y+2, alterPixel(imageGetPixel(im, x+(s*8)+1, y+2), e.r*1/48, e.g*1/48, e.b*1/48));
			imagePutPixel(im, x+(s*8)+2, y+2, alterPixel(imageGetPixel(im, x+(s*8)+2, y+2), e.r*1/48, e.g*1/48, e.b*1/48));
			imagePutPixel(im, x+(s*8)+3, y+2, alterPixel(imageGetPixel(im, x+(s*8)+3, y+2), e.r*1/48, e.g*1/48, e.b*1/48));

			imagePutPixel(im, x+(s*4)+0, y+2, alterPixel(imageGetPixel(im, x+(s*4)+0, y+2), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*4)+1, y+2, alterPixel(imageGetPixel(im, x+(s*4)+1, y+2), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*4)+2, y+2, alterPixel(imageGetPixel(im, x+(s*4)+2, y+2), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x+(s*4)+3, y+2, alterPixel(imageGetPixel(im, x+(s*4)+3, y+2), e.r*1/16, e.g*1/16, e.b*1/16));

			imagePutPixel(im, x+0+0, y+2, alterPixel(imageGetPixel(im, x+0+0, y+2), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x+0+1, y+2, alterPixel(imageGetPixel(im, x+0+1, y+2), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x+0+2, y+2, alterPixel(imageGetPixel(im, x+0+2, y+2), e.r*5/48, e.g*5/48, e.b*5/48));
			imagePutPixel(im, x+0+3, y+2, alterPixel(imageGetPixel(im, x+0+3, y+2), e.r*5/48, e.g*5/48, e.b*5/48));

			imagePutPixel(im, x-(s*4)+0, y+2, alterPixel(imageGetPixel(im, x-(s*4)+0, y+2), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x-(s*4)+1, y+2, alterPixel(imageGetPixel(im, x-(s*4)+1, y+2), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x-(s*4)+2, y+2, alterPixel(imageGetPixel(im, x-(s*4)+2, y+2), e.r*1/16, e.g*1/16, e.b*1/16));
			imagePutPixel(im, x-(s*4)+3, y+2, alterPixel(imageGetPixel(im, x-(s*4)+3, y+2), e.r*1/16, e.g*1/16, e.b*1/16));

			imagePutPixel(im, x-(s*8)+0, y+2, alterPixel(imageGetPixel(im, x-(s*8)+0, y+2), e.r*1/48, e.g*1/48, e.b*1/48));
			imagePutPixel(im, x-(s*8)+1, y+2, alterPixel(imageGetPixel(im, x-(s*8)+1, y+2), e.r*1/48, e.g*1/48, e.b*1/48));
			imagePutPixel(im, x-(s*8)+2, y+2, alterPixel(imageGetPixel(im, x-(s*8)+2, y+2), e.r*1/48, e.g*1/48, e.b*1/48));
			imagePutPixel(im, x-(s*8)+3, y+2, alterPixel(imageGetPixel(im, x-(s*8)+3, y+2), e.r*1/48, e.g*1/48, e.b*1/48));

		}
		break;
	case 12:

		// Floyd-Steinberg - Spread2A

		e0.r = 0;
		e0.g = 0;
		e0.b = 0;
		e1.r = 0;
		e1.g = 0;
		e1.b = 0;
		e2.r = 0;
		e2.g = 0;
		e2.b = 0;
		e3.r = 0;
		e3.g = 0;
		e3.b = 0;

		if (!reverse) {
			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/32, e0.g*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/32, e0.b*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}			

			e1.r = e1.r + (e0.r * 7.0/32);
			e1.g = e1.g + (e0.g * 7.0/32);
			e1.b = e1.b + (e0.b * 7.0/32);

			e2.r = e2.r + (e0.r * 7.0/32);
			e2.g = e2.g + (e0.g * 7.0/32);
			e2.b = e2.b + (e0.b * 7.0/32);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/32, e0.g*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/32, e0.b*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}

			e2.r = e2.r + (e1.r * 7.0/32);
			e2.g = e2.g + (e1.g * 7.0/32);
			e2.b = e2.b + (e1.b * 7.0/32);

			e3.r = e3.r + (e1.r * 7.0/32);
			e3.g = e3.g + (e1.g * 7.0/32);
			e3.b = e3.b + (e1.b * 7.0/32);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/32, e0.g*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*5.0/32, e0.b*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}

			e3.r = e3.r + (e2.r * 7.0/32);
			e3.g = e3.g + (e2.g * 7.0/32);
			e3.b = e3.b + (e2.b * 7.0/32);

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e2.r*7.0/32, e2.g*7.0/32, e2.b*7.0/32));


			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*5.0/32, e0.g*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*5.0/32, e0.b*5.0/32, e0.b*5.0/32));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e3.r*7.0/32, e3.g*7.0/32, e3.b*7.0/32));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), e3.r*7.0/32, e3.g*7.0/32, e3.b*7.0/32));

		} else {

			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}			

			e2.r = e2.r + (e3.r * 7.0/32);
			e2.g = e2.g + (e3.g * 7.0/32);
			e2.b = e2.b + (e3.b * 7.0/32);

			e1.r = e1.r + (e3.r * 7.0/32);
			e1.g = e1.g + (e3.g * 7.0/32);
			e1.b = e1.b + (e3.b * 7.0/32);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}

			e1.r = e1.r + (e2.r * 7.0/32);
			e1.g = e1.g + (e2.g * 7.0/32);
			e1.b = e1.b + (e2.b * 7.0/32);

			e0.r = e0.r + (e2.r * 7.0/32);
			e0.g = e0.g + (e2.g * 7.0/32);
			e0.b = e0.b + (e2.b * 7.0/32);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}

			e0.r = e0.r + (e1.r * 7.0/32);
			e0.g = e0.g + (e1.g * 7.0/32);
			e0.b = e0.b + (e1.b * 7.0/32);

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e1.r*7.0/32, e1.g*7.0/32, e1.b*7.0/32));


			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*5.0/32, e3.g*5.0/32, e3.b*5.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e0.r*7.0/32, e0.g*7.0/32, e0.b*7.0/32));
			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), e0.r*7.0/32, e0.g*7.0/32, e0.b*7.0/32));
		}
		break;
	case 13:

		// Floyd-Steinberg - Spread2B

		e0.r = 0;
		e0.g = 0;
		e0.b = 0;
		e1.r = 0;
		e1.g = 0;
		e1.b = 0;
		e2.r = 0;
		e2.g = 0;
		e2.b = 0;
		e3.r = 0;
		e3.g = 0;
		e3.b = 0;

		if (!reverse) {
			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}			

			e1.r = e1.r + (e0.r * 7.0/32);
			e1.g = e1.g + (e0.g * 7.0/32);
			e1.b = e1.b + (e0.b * 7.0/32);

			e2.r = e2.r + (e0.r * 7.0/32);
			e2.g = e2.g + (e0.g * 7.0/32);
			e2.b = e2.b + (e0.b * 7.0/32);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}

			e2.r = e2.r + (e1.r * 7.0/32);
			e2.g = e2.g + (e1.g * 7.0/32);
			e2.b = e2.b + (e1.b * 7.0/32);

			e3.r = e3.r + (e1.r * 7.0/32);
			e3.g = e3.g + (e1.g * 7.0/32);
			e3.b = e3.b + (e1.b * 7.0/32);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}

			e3.r = e3.r + (e2.r * 7.0/32);
			e3.g = e3.g + (e2.g * 7.0/32);
			e3.b = e3.b + (e2.b * 7.0/32);

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e2.r*7.0/32, e2.g*7.0/32, e2.b*7.0/32));


			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/32, e0.g*3.0/32, e0.b*3.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e0.r*1.0/32, e0.g*1.0/32, e0.b*1.0/32));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e3.r*7.0/32, e3.g*7.0/32, e3.b*7.0/32));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), e3.r*7.0/32, e3.g*7.0/32, e3.b*7.0/32));

		} else {

			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}			

			e2.r = e2.r + (e3.r * 7.0/32);
			e2.g = e2.g + (e3.g * 7.0/32);
			e2.b = e2.b + (e3.b * 7.0/32);

			e1.r = e1.r + (e3.r * 7.0/32);
			e1.g = e1.g + (e3.g * 7.0/32);
			e1.b = e1.b + (e3.b * 7.0/32);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}

			e1.r = e1.r + (e2.r * 7.0/32);
			e1.g = e1.g + (e2.g * 7.0/32);
			e1.b = e1.b + (e2.b * 7.0/32);

			e0.r = e0.r + (e2.r * 7.0/32);
			e0.g = e0.g + (e2.g * 7.0/32);
			e0.b = e0.b + (e2.b * 7.0/32);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}

			e0.r = e0.r + (e1.r * 7.0/32);
			e0.g = e0.g + (e1.g * 7.0/32);
			e0.b = e0.b + (e1.b * 7.0/32);

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e1.r*7.0/32, e1.g*7.0/32, e1.b*7.0/32));


			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/32, e3.g*1.0/32, e3.b*1.0/32));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*3.0/32, e3.g*3.0/32, e3.b*3.0/32));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e0.r*7.0/32, e0.g*7.0/32, e0.b*7.0/32));
			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), e0.r*7.0/32, e0.g*7.0/32, e0.b*7.0/32));
		}
		break;
	case 14:

		// Floyd-Steinberg - Spread3

		e0.r = 0;
		e0.g = 0;
		e0.b = 0;
		e1.r = 0;
		e1.g = 0;
		e1.b = 0;
		e2.r = 0;
		e2.g = 0;
		e2.b = 0;
		e3.r = 0;
		e3.g = 0;
		e3.b = 0;

		if (!reverse) {
			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+0, y+1, alterPixel(imageGetPixel(im, x-4+0, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
			}			

			e1.r = e1.r + (e0.r * 7.0/48);
			e1.g = e1.g + (e0.g * 7.0/48);
			e1.b = e1.b + (e0.b * 7.0/48);

			e2.r = e2.r + (e0.r * 7.0/48);
			e2.g = e2.g + (e0.g * 7.0/48);
			e2.b = e2.b + (e0.b * 7.0/48);

			e3.r = e3.r + (e0.r * 7.0/48);
			e3.g = e3.g + (e0.g * 7.0/48);
			e3.b = e3.b + (e0.b * 7.0/48);


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
			}

			e2.r = e2.r + (e1.r * 7.0/48);
			e2.g = e2.g + (e1.g * 7.0/48);
			e2.b = e2.b + (e1.b * 7.0/48);

			e3.r = e3.r + (e1.r * 7.0/48);
			e3.g = e3.g + (e1.g * 7.0/48);
			e3.b = e3.b + (e1.b * 7.0/48);

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e2.r*7.0/48, e2.g*7.0/48, e2.b*7.0/48));


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
			}

			e3.r = e3.r + (e2.r * 7.0/48);
			e3.g = e3.g + (e2.g * 7.0/48);
			e3.b = e3.b + (e2.b * 7.0/48);

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e2.r*7.0/48, e2.g*7.0/48, e2.b*7.0/48));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), e2.r*7.0/48, e2.g*7.0/48, e2.b*7.0/48));


			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/48, e0.g*3.0/48, e0.b*3.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e0.r*5.0/48, e0.g*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e0.r*5.0/48, e0.b*5.0/48, e0.b*5.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
				imagePutPixel(im, x+4+3, y+1, alterPixel(imageGetPixel(im, x+4+3, y+1), e0.r*1.0/48, e0.g*1.0/48, e0.b*1.0/48));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), e3.r*7.0/48, e3.g*7.0/48, e3.b*7.0/48));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), e3.r*7.0/48, e3.g*7.0/48, e3.b*7.0/48));
			imagePutPixel(im, x+4+2, y, alterPixel(imageGetPixel(im, x+4+2, y), e3.r*7.0/48, e3.g*7.0/48, e3.b*7.0/48));

		} else {

			e3.r = e3.r + (im_p3.r - c3.r);
			e3.g = e3.r + (im_p3.g - c3.g);
			e3.b = e3.r + (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+3, y+1, alterPixel(imageGetPixel(im, x+4+3, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
			}			

			e2.r = e2.r + (e3.r * 7.0/48);
			e2.g = e2.g + (e3.g * 7.0/48);
			e2.b = e2.b + (e3.b * 7.0/48);

			e1.r = e1.r + (e3.r * 7.0/48);
			e1.g = e1.g + (e3.g * 7.0/48);
			e1.b = e1.b + (e3.b * 7.0/48);

			e0.r = e0.r + (e3.r * 7.0/48);
			e0.g = e0.g + (e3.g * 7.0/48);
			e0.b = e0.b + (e3.b * 7.0/48);


			e2.r = e2.r + (im_p2.r - c2.r);
			e2.g = e2.g + (im_p2.g - c2.g);
			e2.b = e2.b + (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+2, y+1, alterPixel(imageGetPixel(im, x+4+2, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
			}

			e1.r = e1.r + (e2.r * 7.0/48);
			e1.g = e1.g + (e2.g * 7.0/48);
			e1.b = e1.b + (e2.b * 7.0/48);

			e0.r = e0.r + (e2.r * 7.0/48);
			e0.g = e0.g + (e2.g * 7.0/48);
			e0.b = e0.b + (e2.b * 7.0/48);

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e1.r*7.0/48, e1.g*7.0/48, e1.b*7.0/48));


			e1.r = e1.r + (im_p1.r - c1.r);
			e1.g = e1.g + (im_p1.g - c1.g);
			e1.b = e1.b + (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+1, y+1, alterPixel(imageGetPixel(im, x+4+1, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
			}

			e0.r = e0.r + (e1.r * 7.0/48);
			e0.g = e0.g + (e1.g * 7.0/48);
			e0.b = e0.b + (e1.b * 7.0/48);

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e1.r*7.0/48, e1.g*7.0/48, e1.b*7.0/48));
			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), e1.r*7.0/48, e1.g*7.0/48, e1.b*7.0/48));


			e0.r = e0.r + (im_p0.r - c0.r);
			e0.g = e0.g + (im_p0.g - c0.g);
			e0.b = e0.b + (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+0, y+1, alterPixel(imageGetPixel(im, x-4+0, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x-4+1, y+1, alterPixel(imageGetPixel(im, x-4+1, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x-4+2, y+1, alterPixel(imageGetPixel(im, x-4+2, y+1), e3.r*1.0/48, e3.g*1.0/48, e3.b*1.0/48));
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e3.r*5.0/48, e3.g*5.0/48, e3.b*5.0/48));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/48, e3.g*3.0/48, e3.b*3.0/48));
			}

			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), e0.r*7.0/48, e0.g*7.0/48, e0.b*7.0/48));
			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), e0.r*7.0/48, e0.g*7.0/48, e0.b*7.0/48));
			imagePutPixel(im, x-4+1, y, alterPixel(imageGetPixel(im, x-4+1, y), e0.r*7.0/48, e0.g*7.0/48, e0.b*7.0/48));
		}
		break;
	case 15:

		// Floyd-Steinberg - Fine3

		if (!reverse) {
			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*1.0/16, e0.b*1.0/16, e0.b*1.0/16));
			}			

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
			}

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
			}

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {

				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
			imagePutPixel(im, x+4+2, y, alterPixel(imageGetPixel(im, x+4+2, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
			imagePutPixel(im, x+4+3, y, alterPixel(imageGetPixel(im, x+4+3, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));

		} else {

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
			}			

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
			}

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
			}

			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*1.0/16, e0.g*1.0/16, e0.b*1.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
			}

			imagePutPixel(im, x-4+0, y, alterPixel(imageGetPixel(im, x-4+0, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
			imagePutPixel(im, x-4+1, y, alterPixel(imageGetPixel(im, x-4+1, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/4, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/4, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/4));
		}
		break;
	case 16:

		// Floyd-Steinberg - Fine4

		if (!reverse) {
			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*1.0/16, e0.b*1.0/16, e0.b*1.0/16));
			}			

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
			}

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
			}

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {

				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
			}

			imagePutPixel(im, x+4+0, y, alterPixel(imageGetPixel(im, x+4+0, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/2, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/2, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/2));
			imagePutPixel(im, x+4+1, y, alterPixel(imageGetPixel(im, x+4+1, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/2, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/2, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/2));

		} else {

			e3.r = (im_p3.r - c3.r);
			e3.g = (im_p3.g - c3.g);
			e3.b = (im_p3.b - c3.b);

			if (!singleline) {
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e3.r*1.0/16, e3.g*1.0/16, e3.b*1.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e3.r*5.0/16, e3.g*5.0/16, e3.b*5.0/16));
				imagePutPixel(im, x+4+0, y+1, alterPixel(imageGetPixel(im, x+4+0, y+1), e3.r*3.0/16, e3.g*3.0/16, e3.b*3.0/16));
			}			

			e2.r = (im_p2.r - c2.r);
			e2.g = (im_p2.g - c2.g);
			e2.b = (im_p2.b - c2.b);

			if (!singleline) {
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e2.r*1.0/16, e2.g*1.0/16, e2.b*1.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e2.r*5.0/16, e2.g*5.0/16, e2.b*5.0/16));
				imagePutPixel(im, x+0+3, y+1, alterPixel(imageGetPixel(im, x+0+3, y+1), e2.r*3.0/16, e2.g*3.0/16, e2.b*3.0/16));
			}

			e1.r = (im_p1.r - c1.r);
			e1.g = (im_p1.g - c1.g);
			e1.b = (im_p1.b - c1.b);

			if (!singleline) {
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e1.r*1.0/16, e1.g*1.0/16, e1.b*1.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e1.r*5.0/16, e1.g*5.0/16, e1.b*5.0/16));
				imagePutPixel(im, x+0+2, y+1, alterPixel(imageGetPixel(im, x+0+2, y+1), e1.r*3.0/16, e1.g*3.0/16, e1.b*3.0/16));
			}

			e0.r = (im_p0.r - c0.r);
			e0.g = (im_p0.g - c0.g);
			e0.b = (im_p0.b - c0.b);

			if (!singleline) {
				imagePutPixel(im, x-4+3, y+1, alterPixel(imageGetPixel(im, x-4+3, y+1), e0.r*1.0/16, e0.g*1.0/16, e0.b*1.0/16));
				imagePutPixel(im, x+0+0, y+1, alterPixel(imageGetPixel(im, x+0+0, y+1), e0.r*5.0/16, e0.g*5.0/16, e0.b*5.0/16));
				imagePutPixel(im, x+0+1, y+1, alterPixel(imageGetPixel(im, x+0+1, y+1), e0.r*3.0/16, e0.g*3.0/16, e0.b*3.0/16));
			}

			imagePutPixel(im, x-4+2, y, alterPixel(imageGetPixel(im, x-4+2, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/2, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/2, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/2));
			imagePutPixel(im, x-4+3, y, alterPixel(imageGetPixel(im, x-4+3, y), (e3.r*7.0/16 + e2.r*7.0/16 + e1.r*7.0/16 + e0.r*7.0/16)/2, (e3.g*7.0/16 + e2.g*7.0/16 + e1.g*7.0/16 + e0.g*7.0/16)/2, (e3.b*7.0/16 + e2.b*7.0/16 + e1.b*7.0/16 + e0.b*7.0/16)/2));
		}
		break;
	default:
		// Do nothing
		break;
	}

	return 0;
}


static BlockImageRef rgbToBlock_Dither (ImageRef im, int ditherType)
{
	int x, y, i, x2;
	Pixel p, im_p0, im_p1, im_p2, im_p3;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	int prevBlock, currBlock, nextBlock, nextBlock_prev;
	unsigned int actBlockColourCur;
	int count;
	int reverse;
	int actBlockListOffset;
	int limitPrevBlock, limitNextBlock;

	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=4)
		{
			//Serpentine
			switch (optProcessDir) {
			case 0:
			case 1:
				reverse = optProcessDir;
				break;
			case 2:
				reverse = y%2;
				break;
			case 3:
				reverse = 1-(y%2);
				break;
			default:
				reverse = y%2;
				break;
			} /* switch */

			if (reverse) {
				x2 = 556 - x;
			} else {
				x2 = x;
			}

			if (optLimitToPrevOrNextBlock) {
				if (!reverse) {
					limitPrevBlock = prevBlock;
					limitNextBlock = -1;
				} else {
					limitPrevBlock = -1;
					limitNextBlock = prevBlock;
				}
			} else {
				limitPrevBlock = -1;
				limitNextBlock = -1;
			}

			currBlock = block_GetClosest(im, x2, y, &actBlockListOffset, limitPrevBlock, limitNextBlock);
			blockImagePutBlock(ib, x2/4, y, currBlock);

			if (optNextBlockCalculate) {
				// Determine (as close as possible) actual display block.
				if (((x2/4 == 139) && !reverse) || ((x2/4 == 0) && reverse)){
					nextBlock = 0; // End of line.
				} else {
					if (optNextBlockGuess) {
						nextBlock = currBlock;
					} else {
						nextBlock = adjBlockList[actBlockList[actBlockListOffset].nextBlockList][0];
					}
					nextBlock_prev = -1;
					count = 0;

					do {
						count += 1;
						nextBlock_prev = nextBlock;
						memcpy(&im_try->p[0], &im->p[im->w * y], im->w);

						if (!reverse) {
							actBlockColourCur = aDoubleHiResBlock[prevBlock][currBlock][nextBlock];
							image_Dither (im_try, x2, 0, actBlockColourCur, ditherType, reverse);
							nextBlock = block_GetClosest(im_try, x2+4, 0, &actBlockListOffset, limitPrevBlock, limitNextBlock);
						} else {
							actBlockColourCur = aDoubleHiResBlock[nextBlock][currBlock][prevBlock];
							image_Dither (im_try, x2, 0, actBlockColourCur, ditherType, reverse);
							nextBlock = block_GetClosest(im_try, x2-4, 0, &actBlockListOffset, limitPrevBlock, limitNextBlock);
						}
					} while ((nextBlock_prev != nextBlock) && (count < 5));
				}
			} else {
				if (optNextBlockGuess) {
					nextBlock = currBlock;
				} else {
					nextBlock = adjBlockList[actBlockList[actBlockListOffset].nextBlockList][0];
				}
			}

			if (!reverse) {
				actBlockColourCur = aDoubleHiResBlock[prevBlock][currBlock][nextBlock];
			} else {
				actBlockColourCur = aDoubleHiResBlock[nextBlock][currBlock][prevBlock];
			}
			image_Dither (im, x2, y, actBlockColourCur, ditherType, reverse);
			prevBlock = currBlock;

		}
	}

	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

static BlockImageRef rgbToBlock_DitherAlternate (ImageRef im, int ditherType)
{
	int x, y, i, x2;
	Pixel p, im_p0, im_p1, im_p2, im_p3;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	int prevBlock, currBlock, nextBlock, nextBlock_prev;
	unsigned int actBlockColourCur;
	int count;
	int reverse;
	int actBlockListOffset;
	int limitPrevBlock, limitNextBlock;

	for(y=0; y<im->h; y++)
	{
		// Step 1
		prevBlock = 0; // Used only for limit to border
		for(x=0; x<im->w; x+=8)
		{
			//Serpentine
			switch (optProcessDir) {
			case 0:
			case 1:
				reverse = optProcessDir;
				break;
			case 2:
				reverse = y%2;
				break;
			case 3:
				reverse = 1-(y%2);
				break;
			default:
				reverse = y%2;
				break;
			} /* switch */

			if (reverse) {
				x2 = 556 - x;
			} else {
				x2 = x;
			}

			// limit to border
			if (x2==0) {
				limitPrevBlock = prevBlock;
				limitNextBlock = -1;
			} else {
				limitPrevBlock = -1;
				limitNextBlock = -1;
			}

			currBlock = block_GetClosest(im, x2, y, &actBlockListOffset, limitPrevBlock, limitNextBlock);
			blockImagePutBlock(ib, x2/4, y, currBlock);

			actBlockColourCur = actBlockList[actBlockListOffset].pixelBlock;
			image_Dither (im, x2, y, actBlockColourCur, ditherType, reverse);

		}

		// Step 2
		for(x=4; x<im->w; x+=8)
		{
			//Serpentine
			switch (optProcessDir) {
			case 0:
			case 1:
				reverse = optProcessDir;
				break;
			case 2:
				reverse = y%2;
				break;
			case 3:
				reverse = 1-(y%2);
				break;
			default:
				reverse = y%2;
				break;
			} /* switch */

			if (reverse) {
				x2 = 556 - x;
			} else {
				x2 = x;
			}

			prevBlock = blockImageGetBlock(ib, (x2/4)-1, y);
			nextBlock = blockImageGetBlock(ib, (x2/4)+1, y);

			limitPrevBlock = prevBlock;
			limitNextBlock = nextBlock;

			currBlock = block_GetClosest(im, x2, y, &actBlockListOffset, limitPrevBlock, limitNextBlock);
			blockImagePutBlock(ib, x2/4, y, currBlock);

			if (!reverse) {
				actBlockColourCur = aDoubleHiResBlock[prevBlock][currBlock][nextBlock];
			} else {
				actBlockColourCur = aDoubleHiResBlock[nextBlock][currBlock][prevBlock];
			}
			image_Dither (im, x2, y, actBlockColourCur, ditherType, reverse);

		}

	}

	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

/***************************************************************************************************************************/
/* Riemersma */
enum {
  NONE,
  UP,
  LEFT,
  DOWN,
  RIGHT,
};

static int cur_x=0, cur_y=0, img_offset;
//ImageRef im;
ImageRef im_block;

#define SIZE 256                 /* queue size: number of pixels remembered */
#define MAX  256                /* relative weight of youngest pixel in the */

static int weights[SIZE];       /* weights for the errors of recent pixels */

static void init_weights(int a[],int size,int max)
{
	double m, v;
	m = exp((double) log((double) max)/(size-1));
	int i;

	for (i=0, v=1.0; i<size; i++) {
		a[i]=(int)(v+0.5);  /* store rounded value */
		v*=m;          /* next value */
	} /*for */
}

static void dither_pixel(ImageRef im, BlockImageRef ib, int img_offset)
{
	static double error_r[SIZE]; /* queue with error values of recent pixels */
	static double error_g[SIZE]; /* queue with error values of recent pixels */
	static double error_b[SIZE]; /* queue with error values of recent pixels */
	int i;
	double err_r,err_g,err_b;
	int currBlock, bc0, bc1, bc2, bc3;
	Pixel p;
	Pixel c0, c1, c2, c3;
	int actBlock;

	for (i=0,err_r=0L,err_g=0L,err_b=0L; i<SIZE; i++) {
		err_r+=error_r[i]*weights[i];
		err_g+=error_g[i]*weights[i];
		err_b+=error_b[i]*weights[i];
	}

	p.r=(im->p[img_offset+0].r + im->p[img_offset+1].r + im->p[img_offset+2].r + im->p[img_offset+3].r)/4 + err_r/MAX;
	p.g=(im->p[img_offset+0].g + im->p[img_offset+1].g + im->p[img_offset+2].g + im->p[img_offset+3].g)/4 + err_g/MAX;
	p.b=(im->p[img_offset+0].b + im->p[img_offset+1].b + im->p[img_offset+2].b + im->p[img_offset+3].b)/4 + err_b/MAX;
//	p.r = (p.r>=128) ? 255 : 0;
//	p.g = (p.g>=128) ? 255 : 0;
//	p.b = (p.b>=128) ? 255 : 0;
//	p.r=im->p[img_offset+0].r;
//	p.r = 0;
//	p.g = im->p[img_offset+0].g;
//	p.g = 0;
//	p.b = 0;
	imagePutPixel(im_block, 0, 0, p);
	imagePutPixel(im_block, 1, 0, p);
	imagePutPixel(im_block, 2, 0, p);
	imagePutPixel(im_block, 3, 0, p);

	/*
	p.r=im->p[img_offset+1].r + err_r/32;
	p.g=im->p[img_offset+1].g + err_g/32;
	p.b=im->p[img_offset+1].b + err_b/32;
	p.r = (p.r>=128) ? 255 : 0;
	p.g = (p.g>=128) ? 255 : 0;
	p.b = (p.b>=128) ? 255 : 0;
//	p.r=im->p[img_offset+1].r;
//	p.r = 0;
//	p.g = im->p[img_offset+1].g;
//	p.g = 0;
//	p.b = 0;
	imagePutPixel(im_block, 1, 0, p);

	p.r=im->p[img_offset+2].r + err_r/32;
	p.g=im->p[img_offset+2].g + err_g/32;
	p.b=im->p[img_offset+2].b + err_b/32;
	p.r = (p.r>=128) ? 255 : 0;
	p.g = (p.g>=128) ? 255 : 0;
	p.b = (p.b>=128) ? 255 : 0;
//	p.r=im->p[img_offset+2].r;
//	p.r = 0;
//	p.g = im->p[img_offset+2].g;
//	p.g = 0;
//	p.b = 0;
	imagePutPixel(im_block, 2, 0, p);

	p.r=im->p[img_offset+3].r + err_r/32;
	p.g=im->p[img_offset+3].g + err_g/32;
	p.b=im->p[img_offset+3].b + err_b/32;
	p.r = (p.r>=128) ? 255 : 0;
	p.g = (p.g>=128) ? 255 : 0;
	p.b = (p.b>=128) ? 255 : 0;
//	p.r=im->p[img_offset+3].r;
//	p.r = 0;
//	p.g = im->p[img_offset+3].g;
//	p.g = 0;
//	p.b = 0;
	imagePutPixel(im_block, 3, 0, p);
*/
	currBlock = block_GetClosest(im_block, 0, 0, &actBlock, -1, -1);

	memmove(error_r,error_r+1,(SIZE-1)*sizeof error_r[0]);    /* shift queue */
	memmove(error_g,error_g+1,(SIZE-1)*sizeof error_g[0]);    /* shift queue */
	memmove(error_b,error_b+1,(SIZE-1)*sizeof error_b[0]);    /* shift queue */

//	bc0 = (actBlock & 0xF000) >> 12;
//	bc1 = (actBlock & 0x0F00) >> 8;
//	bc2 = (actBlock & 0x00F0) >> 4;
//	bc3 = (actBlock & 0x000F);

//	c0 = pal[bc0];
//	c1 = pal[bc1];
//	c2 = pal[bc2];
//	c3 = pal[bc3];

//	error_r[15] = ((im->p[img_offset].r - pal[currBlock].r)+(im->p[img_offset+1].r - pal[currBlock].r)+(im->p[img_offset+2].r - pal[currBlock].r)+(im->p[img_offset+3].r - pal[currBlock].r))/4.0;
//	error_g[15] = ((im->p[img_offset].g - pal[currBlock].g)+(im->p[img_offset+1].g - pal[currBlock].g)+(im->p[img_offset+2].g - pal[currBlock].g)+(im->p[img_offset+3].g - pal[currBlock].g))/4.0;
//	error_b[15] = ((im->p[img_offset].b - pal[currBlock].b)+(im->p[img_offset+1].b - pal[currBlock].b)+(im->p[img_offset+2].b - pal[currBlock].b)+(im->p[img_offset+3].b - pal[currBlock].b))/4.0;

	error_r[SIZE-1] = (im->p[img_offset].r - pal[currBlock].r);
	error_g[SIZE-1] = (im->p[img_offset].g - pal[currBlock].g);
	error_b[SIZE-1] = (im->p[img_offset].b - pal[currBlock].b);

//	error_r[15] = ((im->p[img_offset].r - c0.r)+(im->p[img_offset+1].r - c1.r)+(im->p[img_offset+2].r - c2.r)+(im->p[img_offset+3].r - c3.r))/4.0;
//	error_g[15] = ((im->p[img_offset].g - c0.g)+(im->p[img_offset+1].g - c1.g)+(im->p[img_offset+2].g - c2.g)+(im->p[img_offset+3].g - c3.g))/4.0;
//	error_b[15] = ((im->p[img_offset].b - c0.b)+(im->p[img_offset+1].b - c1.b)+(im->p[img_offset+2].b - c2.b)+(im->p[img_offset+3].b - c3.b))/4.0;

	blockImagePutBlock(ib, cur_x/4, cur_y, currBlock);
}

static void move(ImageRef im, BlockImageRef ib, int direction)
{
  /* dither the current pixel */
  if (cur_x>=0 && cur_x<im->w && cur_y>=0 && cur_y<im->h)
    dither_pixel(im, ib, img_offset);

  /* move to the next pixel */
  switch (direction) {
  case LEFT:
    cur_x-=4;
    img_offset-=4;
    break;
  case RIGHT:
    cur_x+=4;
    img_offset+=4;
    break;
  case UP:
    cur_y--;
    img_offset-=im->w;
    break;
  case DOWN:
    cur_y++;
    img_offset+=im->w;
    break;
  } /* switch */
}

void hilbert_level(ImageRef im, BlockImageRef ib, int level, int direction)
{
  if (level==1) {
    switch (direction) {
    case LEFT:
      move(im, ib, RIGHT);
      move(im, ib, DOWN);
      move(im, ib, LEFT);
      break;
    case RIGHT:
      move(im, ib, LEFT);
      move(im, ib, UP);
      move(im, ib, RIGHT);
      break;
    case UP:
      move(im, ib, DOWN);
      move(im, ib, RIGHT);
      move(im, ib, UP);
      break;
    case DOWN:
      move(im, ib, UP);
      move(im, ib, LEFT);
      move(im, ib, DOWN);
      break;
    } /* switch */
  } else {
    switch (direction) {
    case LEFT:
      hilbert_level(im, ib, level-1,UP);
      move(im, ib, RIGHT);
      hilbert_level(im, ib, level-1,LEFT);
      move(im, ib, DOWN);
      hilbert_level(im, ib, level-1,LEFT);
      move(im, ib, LEFT);
      hilbert_level(im, ib, level-1,DOWN);
      break;
    case RIGHT:
      hilbert_level(im, ib, level-1,DOWN);
      move(im, ib, LEFT);
      hilbert_level(im, ib, level-1,RIGHT);
      move(im, ib, UP);
      hilbert_level(im, ib, level-1,RIGHT);
      move(im, ib, RIGHT);
      hilbert_level(im, ib, level-1,UP);
      break;
    case UP:
      hilbert_level(im, ib, level-1,LEFT);
      move(im, ib, DOWN);
      hilbert_level(im, ib, level-1,UP);
      move(im, ib, RIGHT);
      hilbert_level(im, ib, level-1,UP);
      move(im, ib, UP);
      hilbert_level(im, ib, level-1,RIGHT);
      break;
    case DOWN:
      hilbert_level(im, ib, level-1,RIGHT);
      move(im, ib, UP);
      hilbert_level(im, ib, level-1,DOWN);
      move(im, ib, LEFT);
      hilbert_level(im, ib, level-1,DOWN);
      move(im, ib, DOWN);
      hilbert_level(im, ib, level-1,LEFT);
      break;
    } /* switch */
  } /* if */
}

int log2(int value)
{
	int result=0;
	while (value>1) {
		value >>= 1;
		result++;
	} /*while */
	return result;
}

static BlockImageRef rgbToBlock_DitherRiemersma (ImageRef im)
{
	int level,size;

	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;

	im_block = imageNew(4, 1);
	if (!im_block) goto emalloc2;

	/* determine the required order of the Hilbert curve */
	size=560;
	level=log2(size);
	if ((1L << level) < size)
		level++;

	init_weights(weights,SIZE,MAX);
	img_offset=0;
	cur_x=0;
	cur_y=0;
	if (level>0)
		hilbert_level(im, ib, level, UP);
	move(im, ib, NONE);

	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;

emalloc2:
	if (im_block->p) free(im_block->p);
	if (im_block) free(im_block);
	return 0;
}

/***************************************************************************************************************************/
/* Yliluoma 3 */
#define COMPARE_RGB 1

/* 2x2 threshold map */
//static const unsigned char map[2*2] = {
//     0, 2,
//     3, 1 };

/* 4x4 threshold map */
//static const unsigned char map[4*4] = {
//     0, 8, 2,10,
//    12, 4,14, 6,
//     3,11, 1, 9,
//    15, 7,13, 5 };

/* 8x8 threshold map */
static const unsigned char map[8*8] = {
     0,48,12,60, 3,51,15,63,
    32,16,44,28,35,19,47,31,
     8,56, 4,52,11,59, 7,55,
    40,24,36,20,43,27,39,23,
     2,50,14,62, 1,49,13,61,
    34,18,46,30,33,17,45,29,
    10,58, 6,54, 9,57, 5,53,
    42,26,38,22,41,25,37,21 };

static const double Gamma = 2.2; // Gamma correction we use.

double GammaCorrect(double v)   { return pow(v, Gamma);       }
double GammaUncorrect(double v) { return pow(v, 1.0 / Gamma); }

/* Luminance for each palette entry, to be initialized as soon as the program begins */
static unsigned luma[16];
//static LabItem  meta[16];
static double   pal_g[16][3]; // Gamma-corrected palette entry

inline bool PaletteCompareLuma(unsigned index1, unsigned index2)
{
    return luma[index1] < luma[index2];
}

typedef std::vector<unsigned> MixingPlan;
MixingPlan DeviseBestMixingPlan(Pixel p, size_t limit)
{
	Pixel p0;
	
	// Input color in CIE L*a*b*
    LabItem input(p);

    std::map<unsigned, unsigned> Solution;

    // The penalty of our currently "best" solution.
    double current_penalty = -1;

    // First, find the closest color to the input color.
    // It is our seed.
    if(1)
    {
        unsigned chosen = 0;
        for(unsigned index=0; index<16; ++index)
        {
//            const unsigned color = pal[index];
    #if COMPARE_RGB
			double penalty = pixelDist(p, pal[index]);
    #else
            LabItem test_lab(pal[index]);
            double penalty = ColorCompare(input, test_lab);
    #endif
            if(penalty < current_penalty || current_penalty < 0)
                { current_penalty = penalty; chosen = index; }
        }

        Solution[chosen] = limit;
    }

    double dbllimit = 1.0 / limit;
    while(current_penalty != 0.0)
//    while(current_penalty > 0.0)
    {
        // Find out if there is a region in Solution that
        // can be split in two for benefit.
        double   best_penalty      = current_penalty;
        unsigned best_splitfrom    = ~0u;
        unsigned best_split_to[2]  = { 0,0};

        for(std::map<unsigned,unsigned>::iterator
            i = Solution.begin();
            i != Solution.end();
            ++i)
        {
            //if(i->second <= 1) continue;
            unsigned split_color = i->first;
            unsigned split_count = i->second;
            // Tally the other colors
            double sum[3] = {0,0,0};
            for(std::map<unsigned,unsigned>::iterator
                j = Solution.begin();
                j != Solution.end();
                ++j)
            {
                if(j->first == split_color) continue;
                sum[0] += pal_g[ j->first ][0] * j->second * dbllimit;
                sum[1] += pal_g[ j->first ][1] * j->second * dbllimit;
                sum[2] += pal_g[ j->first ][2] * j->second * dbllimit;
            }
            double portion1 = (split_count / 2            ) * dbllimit;
            double portion2 = (split_count - split_count/2) * dbllimit;
            for(unsigned a=0; a<16; ++a)
            {
                //if(a != split_color && Solution.find(a) != Solution.end()) continue;
                unsigned firstb = 0;
                if(portion1 == portion2) firstb = a+1;
                for(unsigned b=firstb; b<16; ++b)
                {
                    if(a == b) continue;
                    //if(b != split_color && Solution.find(b) != Solution.end()) continue;
                    int lumadiff = int(luma[a]) - int(luma[b]);
                    if(lumadiff < 0) lumadiff = -lumadiff;
                    if(lumadiff > 80000) continue;

                    double test[3] =
                        { GammaUncorrect(sum[0] + pal_g[a][0] * portion1 + pal_g[b][0] * portion2),
                          GammaUncorrect(sum[1] + pal_g[a][1] * portion1 + pal_g[b][1] * portion2),
                          GammaUncorrect(sum[2] + pal_g[a][2] * portion1 + pal_g[b][2] * portion2) };
                    // Figure out if this split is better than what we had
					p0.r = test[0]*255;
					p0.g = test[1]*255;
					p0.b = test[2]*255;
#if COMPARE_RGB
					double penalty = pixelDist(p, p0);
#else
                    LabItem test_lab(p0);
                    double penalty = ColorCompare(input, test_lab);
#endif
                    if(penalty < best_penalty)
                    {
                        best_penalty   = penalty;
                        best_splitfrom = split_color;
                        best_split_to[0] = a;
                        best_split_to[1] = b;
                    }
                    if(portion2 == 0) break;
        }   }   }
        if(best_penalty == current_penalty) break; // No better solution was found.

        std::map<unsigned,unsigned>::iterator i = Solution.find(best_splitfrom);
		unsigned split_count, split1, split2;
    #if COMPARE_RGB
		split_count = i->second, split1 = split_count/2, split2 = split_count-split1;
		Solution.erase(i);
    #else
		if (i != Solution.end())  // check temp is pointing to underneath element of a map
		{
			split_count = i->second, split1 = split_count/2, split2 = split_count-split1;
	        Solution.erase(i);
		} else {
			split_count = 0, split1 = split_count/2, split2 = split_count-split1;
		}
    #endif
        if(split1 > 0) Solution[best_split_to[0]] += split1;
        if(split2 > 0) Solution[best_split_to[1]] += split2;
        current_penalty = best_penalty;
    }

    // Sequence the solution.
    MixingPlan result;
    for(std::map<unsigned,unsigned>::iterator
        i = Solution.begin(); i != Solution.end(); ++i)
    {
        result.resize(result.size() + i->second, i->first);
    }
    // Sort the colors according to luminance
    std::sort(result.begin(), result.end(), PaletteCompareLuma);
    return result;
}

static BlockImageRef rgbToBlock_DitherYliluoma3 (ImageRef im)
{
	int i;
	Pixel p0, p1, p2, p3, p;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;

    for(unsigned i=0; i<16; ++i)
    {
        unsigned r = pal[i].r, g = pal[i].g, b = pal[i].b;
        luma[i] = r*299 + g*587 + b*114;
//        meta[i].Set(pal[i]);
        pal_g[i][0] = GammaCorrect(r/255.0);
        pal_g[i][1] = GammaCorrect(g/255.0);
        pal_g[i][2] = GammaCorrect(b/255.0);
    }
    for(unsigned y=0; y<im->h; ++y)
        for(unsigned x=0; x<im->w; x+=4)
        {
			p0 = imageGetPixel(im, x, y);
			p1 = imageGetPixel(im, x+1, y);
			p2 = imageGetPixel(im, x+2, y);
			p3 = imageGetPixel(im, x+3, y);
			p.r = (p0.r+p1.r+p2.r+p3.r)/4;
			p.g = (p0.g+p1.g+p2.g+p3.g)/4;
			p.b = (p0.b+p1.b+p2.b+p3.b)/4;
            unsigned map_value = map[(x & 7) + ((y & 7) << 3)]; //8x8
//            unsigned map_value = map[(x & 3) + ((y & 3) << 2)]; //4x4
//            unsigned map_value = map[(x & 1) + ((y & 1) << 1)]; //2x2
            MixingPlan plan = DeviseBestMixingPlan(p, 16);
            map_value = map_value * plan.size() / 64;
			blockImagePutBlock(ib, x/4, y, plan[ map_value ]);
        }


	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

/***************************************************************************************************************************/

static BlockImageRef rgbToBlock_Dither2at_a_time (ImageRef im)
{
	int x, y, i, j, k, x2;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	Pixel im_bp_p2, im_bp_p3, im_bi_p0, im_bi_p1, im_bi_p2, im_bi_p3, im_bj_p0, im_bj_p1, im_bj_p2, im_bj_p3, im_bk_p0, im_bk_p1;
	Pixel bp_p2, bp_p3, bi_p0, bi_p1, bi_p2, bi_p3, bj_p0, bj_p1, bj_p2, bj_p3, bk_p0, bk_p1;
	int bp_b2, bp_b3, bi_b0, bi_b1, bi_b2, bi_b3, bj_b0, bj_b1, bj_b2, bj_b3, bk_b0, bk_b1;
	int bi, bj;
	int prevBlock;
	double d, pdi, pdj, bd;
	unsigned int actBlockColour, actBlockColour0, actBlockColour1, actBlockColour2;
	int count;
	int reverse;

	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=8)
		{

			//Serpentine
//			reverse = y%2;
			reverse = 0;

			if (reverse) {
//??				x2 = 556 - x;
			} else {
				x2 = x;
			}

			im_bp_p2 = imageGetPixel(im, x2-2, y);
			im_bp_p3 = imageGetPixel(im, x2-1, y);

			im_bi_p0 = imageGetPixel(im, x2, y);
			im_bi_p1 = imageGetPixel(im, x2+1, y);
			im_bi_p2 = imageGetPixel(im, x2+2, y);
			im_bi_p3 = imageGetPixel(im, x2+3, y);

			im_bj_p0 = imageGetPixel(im, x2+4, y);
			im_bj_p1 = imageGetPixel(im, x2+5, y);
			im_bj_p2 = imageGetPixel(im, x2+6, y);
			im_bj_p3 = imageGetPixel(im, x2+7, y);

			im_bk_p0 = imageGetPixel(im, x2+9, y);
			im_bk_p1 = imageGetPixel(im, x2+10, y);

			bd = 0x7fffffff; // big number;
			bi = 0;

			for(i=0; i<16; i++)
			{
				for(j=0; j<16; j++)
				{
					actBlockColour = aDoubleHiResBlock[prevBlock][prevBlock][i];

					bp_b2 = (actBlockColour & 0x00F0) >> 4;
					bp_b3 = (actBlockColour & 0x000F);

					bp_p2 = pal[bp_b2];
					bp_p3 = pal[bp_b3];

					actBlockColour0 = aDoubleHiResBlock[prevBlock][i][j];

					bi_b0 = (actBlockColour0 & 0xF000) >> 12;
					bi_b1 = (actBlockColour0 & 0x0F00) >> 8;
					bi_b2 = (actBlockColour0 & 0x00F0) >> 4;
					bi_b3 = (actBlockColour0 & 0x000F);

					bi_p0 = pal[bi_b0];
					bi_p1 = pal[bi_b1];
					bi_p2 = pal[bi_b2];
					bi_p3 = pal[bi_b3];

					pdi = 0;
					d = pixelDist(im_bp_p2, bp_p2);
					pdi = pdi + d;
					d = pixelDist(im_bp_p3, bp_p3);
					pdi = pdi + d;

					d = pixelDist(im_bi_p0, bi_p0);
					pdi = pdi + d;
					d = pixelDist(im_bi_p1, bi_p1);
					pdi = pdi + d;
					d = pixelDist(im_bi_p2, bi_p2);
					pdi = pdi + d;
					d = pixelDist(im_bi_p3, bi_p3);
					pdi = pdi + d;

					for(k=0; k<16; k++)
					{
						actBlockColour1 = aDoubleHiResBlock[i][j][k];

						bj_b0 = (actBlockColour1 & 0xF000) >> 12;
						bj_b1 = (actBlockColour1 & 0x0F00) >> 8;
						bj_b2 = (actBlockColour1 & 0x00F0) >> 4;
						bj_b3 = (actBlockColour1 & 0x000F);

						bj_p0 = pal[bj_b0];
						bj_p1 = pal[bj_b1];
						bj_p2 = pal[bj_b2];
						bj_p3 = pal[bj_b3];

						actBlockColour2 = aDoubleHiResBlock[j][k][k];

						bk_b0 = (actBlockColour2 & 0xF000) >> 12;
						bk_b1 = (actBlockColour2 & 0x0F00) >> 8;

						bk_p0 = pal[bk_b0];
						bk_p1 = pal[bk_b1];

						d = pixelDist(im_bj_p0, bj_p0);
						pdj = pdi + d;
						d = pixelDist(im_bj_p1, bj_p1);
						pdj = pdj + d;
						d = pixelDist(im_bj_p2, bj_p2);
						pdj = pdj + d;
						d = pixelDist(im_bj_p3, bj_p3);
						pdj = pdj + d;

						d = pixelDist(im_bk_p0, bk_p0);
						pdj = pdj + d;
						d = pixelDist(im_bk_p1, bk_p1);
						pdj = pdj + d;

//						if (pdj < bd) {
//						if (pdj <= bd) {
//						if ((pdj <= bd) && (((prevBlock != i) && (i != j)) || (i == 0))) {
						if ((((pdj < bd)&&(optClosestLessThan == 0)) || ((pdj <= bd)&&(optClosestLessThan == 1))) && (((prevBlock != i) && (i != j)) || (i == 0))) {
							bd = pdj;
							bi = i;
							bj = j;
						}
					}
				}
			}
			blockImagePutBlock(ib, x/4, y, bi);
			blockImagePutBlock(ib, (x/4)+1, y, bj);
			prevBlock = bj;
		}
	}
	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

/*

	int x, y, i, x2;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	int prevBlock, currBlock, nextBlock, nextBlock_prev;
	unsigned int actBlockColourCur;
	int count;
	int reverse;


	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=8)
		{
			//Serpentine
			reverse = y%2;
//			reverse = 0;

			if (reverse) {
				x2 = 552 - x;
			} else {
				x2 = x;
			}

			currBlock = block_GetClosest(im, x2, y);
			blockImagePutBlock(ib, x2/4, y, currBlock);

			nextBlock = currBlock;


			// Determine (as close as possible) actual display block.
			if (((x2/4 == 139) && !reverse) || ((x2/4 == 0) && reverse)){
				nextBlock = 0; // End of line.
			} else {
				nextBlock = currBlock;
				nextBlock_prev = -1;
				count = 0;

				do {
					count += 1;
					nextBlock_prev = nextBlock;
					memcpy(&im_try->p[0], &im->p[im->w * y], im->w);

					if (!reverse) {
						actBlockColourCur = aDoubleHiResBlock[prevBlock][currBlock][nextBlock];
						image_Dither (im_try, x2, 0, actBlockColourCur, ditherType, reverse);
						nextBlock = block_GetClosest(im_try, x2+4, 0);
					} else {
						actBlockColourCur = aDoubleHiResBlock[nextBlock][currBlock][prevBlock];
						image_Dither (im_try, x2, 0, actBlockColourCur, ditherType, reverse);
						nextBlock = block_GetClosest(im_try, x2-4, 0);
					}
				} while ((nextBlock_prev != nextBlock) && (count < 5));
			}



			if (!reverse) {
				actBlockColourCur = aDoubleHiResBlock[prevBlock][currBlock][nextBlock];
			} else {
				actBlockColourCur = aDoubleHiResBlock[nextBlock][currBlock][prevBlock];
			}
			image_DitherOstromoukhov (im, x2, y, actBlockColourCur, ditherType, reverse);
			prevBlock = currBlock;

		}
	}

	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

*/


/***************************************************************************************************************************/

static BlockImageRef rgbToBlock_Convert0 (ImageRef im)
{
	int x, y, i;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	Pixel im_p0, im_p1, im_p2, im_p3, p;
	int bi, prevBlock;
	double d, pd, bd;

	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=4)
		{
			im_p0 = imageGetPixel(im, x, y);
			im_p1 = imageGetPixel(im, x+1, y);
			im_p2 = imageGetPixel(im, x+2, y);
			im_p3 = imageGetPixel(im, x+3, y);

			bd = 0x7fffffff; // big number;
			bi = 0;

			for(i=0; i<16; i++)
			{
				p = pal[i];

				pd = 0;
				d = pixelDist(im_p0, p);
				pd = pd + d;
				d = pixelDist(im_p1, p);
				pd = pd + d;
				d = pixelDist(im_p2, p);
				pd = pd + d;
				d = pixelDist(im_p3, p);
				pd = pd + d;

//				if (pd < bd) {
				if (((pd < bd)&&(optClosestLessThan == 0)) || ((pd <= bd)&&(optClosestLessThan == 1))) {
//				if ((pd <= bd) && ((prevBlock != i) || (i == 0))) {
					bd = pd;
					bi = i;
				}
			}
			blockImagePutBlock(ib, x/4, y, bi);
			prevBlock = bi;
		}
	}
	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}


static BlockImageRef rgbToBlock_Convert1 (ImageRef im)
{
	int x, y, i;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	Pixel im_p0, im_p1, im_p2, im_p3, p0, p1, p2, p3;
	int im_b0, im_b1, im_b2, im_b3, b0, b1, b2, b3;
	int bi;
	int prevBlock, currBlock, valid;
	double d, pd, bd;

	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=4)
		{
			im_p0 = imageGetPixel(im, x, y);
			im_p1 = imageGetPixel(im, x+1, y);
			im_p2 = imageGetPixel(im, x+2, y);
			im_p3 = imageGetPixel(im, x+3, y);

//			im_b0 = rgbToBlock_Pixel(im_p0);
//			im_b1 = rgbToBlock_Pixel(im_p1);
//			im_b2 = rgbToBlock_Pixel(im_p2);
//			im_b3 = rgbToBlock_Pixel(im_p3);

			bd = 0x7fffffff; // big number;
			bi = 0;

			for(i=0; i<159; i++)
			{
/*
				valid = 0;
				switch (actBlockList[i].prevBlockList) {
					case 1: // List 1
						switch (prevBlock) {
							case 0:
							case 2:
							case 4:
							case 6:
							case 8:
							case 10:
							case 12:
							case 14:
								valid = 1;
							break;
						}
						break;
					case 2: // List 2
						switch (prevBlock) {
							case 1:
							case 3:
							case 5:
							case 7:
							case 9:
							case 11:
							case 13:
							case 15:
								valid = 1;
							break;
						}
						break;
					case 3: // List 3
						switch (prevBlock) {
							case 0:
							case 4:
							case 8:
							case 12:
								valid = 1;
							break;
						}
						break;
					case 4: // List 4
						switch (prevBlock) {
							case 1:
							case 5:
							case 9:
							case 13:
								valid = 1;
							break;
						}
						break;
					case 5: // List 5
						switch (prevBlock) {
							case 2:
							case 6:
							case 10:
							case 14:
								valid = 1;
							break;
						}
						break;
					case 6: // List 6
						switch (prevBlock) {
							case 3:
							case 7:
							case 11:
							case 15:
								valid = 1;
							break;
						}
						break;
					case 7: // List 7
						switch (prevBlock) {
							case 0:
							case 8:
								valid = 1;
							break;
						}
						break;
					case 8: // List 8
						switch (prevBlock) {
							case 4:
							case 12:
								valid = 1;
							break;
						}
						break;
					default: // List 0
						valid = 0;
				}
*/
/*
				switch (actBlockList[i].nextBlockList) {
					case 0, 1, 2, 3, 4, 5, 6, 7: // List 9
						break;
					case 8, 9, 10, 11, 12, 13, 14, 15: // List 10
						break;
					case 0, 1, 2, 3: // List 11
						break;
					case 4, 5, 6, 7: // List 12
						break;
					case 8, 9, 10, 11: // List 13
						break;
					case 12, 13, 14, 15: // List 14
						break;
					case 0, 1: // List 15
						break;
					case 2, 3: // List 16
						break;
					default: // List 0
				}
*/



/*				valid = 1;
				if (valid) {
					b0 = (actBlockList[i].pixelBlock & 0xF000) >> 12;
					b1 = (actBlockList[i].pixelBlock & 0x0F00) >> 8;
					b2 = (actBlockList[i].pixelBlock & 0x00F0) >> 4;
					b3 = (actBlockList[i].pixelBlock & 0x000F);

//					printf("test1 %d %d %d %d\n", b0, b1, b2, b3);

					d = im_b0 - b0;
					pd = (d * d);//*(d * d);
					d = im_b1 - b1;
					pd = pd + (d * d);//*(d * d);
					d = im_b2 - b2;
					pd = pd + (d * d);//*(d * d);
					d = im_b3 - b3;
					pd = pd + (d * d);//*(d * d);

					actBlockList[i].matchAmount = pd;

					if (pd <= bd) {
						bd = pd;
						bi = i;
					}
				}
*/

				valid = 1;
				if (valid) {
					b0 = (actBlockList[i].pixelBlock & 0xF000) >> 12;
					b1 = (actBlockList[i].pixelBlock & 0x0F00) >> 8;
					b2 = (actBlockList[i].pixelBlock & 0x00F0) >> 4;
					b3 = (actBlockList[i].pixelBlock & 0x000F);

					p0 = pal[b0];
					p1 = pal[b1];
					p2 = pal[b2];
					p3 = pal[b3];

					pd = 0;
					d = pixelDist(im_p0, p0);
					pd = pd + d;
					d = pixelDist(im_p1, p1);
					pd = pd + d;
					d = pixelDist(im_p2, p2);
					pd = pd + d;
					d = pixelDist(im_p3, p3);
					pd = pd + d;

//					d = abs(pixelDist(im_p0, im_p1) - pixelDist(p0, p1));
//					pd = pd - d/4;
//					d = abs(pixelDist(im_p1, im_p2) - pixelDist(p1, p2));
//					pd = pd - d/4;
//					d = abs(pixelDist(im_p2, im_p3) - pixelDist(p2, p3));
//					pd = pd - d/4;
//					d = abs(pixelDist(im_p3, im_p0) - pixelDist(p3, p0));
//					pd = pd - d/4;

//					d = pixelDist(im_p0, p1);
//					pd = pd + d;
//					d = pixelDist(im_p1, p0);
//					pd = pd + d;
//					d = pixelDist(im_p2, p3);
//					pd = pd + d;
//					d = pixelDist(im_p3, p2);
//					pd = pd + d;

					actBlockList[i].matchAmount = (int) pd;
					
//					if (i%3) {  
						if (((pd < bd)&&(optClosestLessThan == 0)) || ((pd <= bd)&&(optClosestLessThan == 1))) {
//						if ((pd <= bd) && ((prevBlock != actBlockList[i].currBlock) || (i == 0))) {
//						if ((pd < bd) && (prevBlock != actBlockList[i].currBlock)) {
							bd = pd;
							bi = i;
						}
//					} else {
//						if (pd < bd) {
//							bd = pd;
//							bi = i;
//						}
//					}
				}

			}
			currBlock = actBlockList[bi].currBlock;

//			blockImagePutBlock(ib, x/4, y, rgbToBlock_Pixel(imageGetPixel(im, x, y)));
			blockImagePutBlock(ib, x/4, y, currBlock);
			prevBlock = currBlock;


		}
	}
	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

static BlockImageRef rgbToBlock_Convert2 (ImageRef im)
{
	int x, y, i, j, k;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	Pixel im_bp_p2, im_bp_p3, im_bi_p0, im_bi_p1, im_bi_p2, im_bi_p3, im_bj_p0, im_bj_p1, im_bj_p2, im_bj_p3, im_bk_p0, im_bk_p1;
	Pixel bp_p2, bp_p3, bi_p0, bi_p1, bi_p2, bi_p3, bj_p0, bj_p1, bj_p2, bj_p3, bk_p0, bk_p1;
	int bp_b2, bp_b3, bi_b0, bi_b1, bi_b2, bi_b3, bj_b0, bj_b1, bj_b2, bj_b3, bk_b0, bk_b1;
	int bi;
	int prevBlock;
	double d, pdi, pdj, bd;
	unsigned int actBlockColour, actBlockColour0, actBlockColour1, actBlockColour2;

	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=4)
		{
			im_bp_p2 = imageGetPixel(im, x-2, y);
			im_bp_p3 = imageGetPixel(im, x-1, y);

			im_bi_p0 = imageGetPixel(im, x, y);
			im_bi_p1 = imageGetPixel(im, x+1, y);
			im_bi_p2 = imageGetPixel(im, x+2, y);
			im_bi_p3 = imageGetPixel(im, x+3, y);

			im_bj_p0 = imageGetPixel(im, x+4, y);
			im_bj_p1 = imageGetPixel(im, x+5, y);
			im_bj_p2 = imageGetPixel(im, x+6, y);
			im_bj_p3 = imageGetPixel(im, x+7, y);

			im_bk_p0 = imageGetPixel(im, x+9, y);
			im_bk_p1 = imageGetPixel(im, x+10, y);

			bd = 0x7fffffff; // big number;
			bi = 0;

			for(i=0; i<16; i++)
			{
				for(j=0; j<16; j++)
				{
					actBlockColour = aDoubleHiResBlock[prevBlock][prevBlock][i];

					bp_b2 = (actBlockColour & 0x00F0) >> 4;
					bp_b3 = (actBlockColour & 0x000F);

					bp_p2 = pal[bp_b2];
					bp_p3 = pal[bp_b3];

					actBlockColour0 = aDoubleHiResBlock[prevBlock][i][j];

					bi_b0 = (actBlockColour0 & 0xF000) >> 12;
					bi_b1 = (actBlockColour0 & 0x0F00) >> 8;
					bi_b2 = (actBlockColour0 & 0x00F0) >> 4;
					bi_b3 = (actBlockColour0 & 0x000F);

					bi_p0 = pal[bi_b0];
					bi_p1 = pal[bi_b1];
					bi_p2 = pal[bi_b2];
					bi_p3 = pal[bi_b3];

					pdi = 0;
					d = pixelDist(im_bp_p2, bp_p2);
					pdi = pdi + d;
					d = pixelDist(im_bp_p3, bp_p3);
					pdi = pdi + d;

					d = pixelDist(im_bi_p0, bi_p0);
					pdi = pdi + d;
					d = pixelDist(im_bi_p1, bi_p1);
					pdi = pdi + d;
					d = pixelDist(im_bi_p2, bi_p2);
					pdi = pdi + d;
					d = pixelDist(im_bi_p3, bi_p3);
					pdi = pdi + d;

					for(k=0; k<16; k++)
					{
						actBlockColour1 = aDoubleHiResBlock[i][j][k];

						bj_b0 = (actBlockColour1 & 0xF000) >> 12;
						bj_b1 = (actBlockColour1 & 0x0F00) >> 8;
						bj_b2 = (actBlockColour1 & 0x00F0) >> 4;
						bj_b3 = (actBlockColour1 & 0x000F);

						bj_p0 = pal[bj_b0];
						bj_p1 = pal[bj_b1];
						bj_p2 = pal[bj_b2];
						bj_p3 = pal[bj_b3];

						actBlockColour2 = aDoubleHiResBlock[j][k][k];

						bk_b0 = (actBlockColour2 & 0xF000) >> 12;
						bk_b1 = (actBlockColour2 & 0x0F00) >> 8;

						bk_p0 = pal[bk_b0];
						bk_p1 = pal[bk_b1];

						d = pixelDist(im_bj_p0, bj_p0);
						pdj = pdi + d;
						d = pixelDist(im_bj_p1, bj_p1);
						pdj = pdj + d;
						d = pixelDist(im_bj_p2, bj_p2);
						pdj = pdj + d;
						d = pixelDist(im_bj_p3, bj_p3);
						pdj = pdj + d;

						d = pixelDist(im_bk_p0, bk_p0);
						pdj = pdj + d;
						d = pixelDist(im_bk_p1, bk_p1);
						pdj = pdj + d;

//						if (pdj < bd) {
//						if (pdj <= bd) {
						if ((((pdj < bd)&&(optClosestLessThan == 0)) || ((pdj <= bd)&&(optClosestLessThan == 1))) && ((prevBlock != i) || (i == 0))) {
//						if ((pdj <= bd) && (prevBlock != i)) {
							bd = pdj;
							bi = i;
						}
					}
				}
			}
			blockImagePutBlock(ib, x/4, y, bi);
			prevBlock = bi;
		}
	}
	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

static BlockImageRef rgbToBlock_2at_a_time (ImageRef im)
{
	int x, y, i, j, k;
	BlockImageRef ib;
	ib = blockImageNew(im->w/4, im->h);
	if (!ib) goto emalloc;
	Pixel im_bp_p2, im_bp_p3, im_bi_p0, im_bi_p1, im_bi_p2, im_bi_p3, im_bj_p0, im_bj_p1, im_bj_p2, im_bj_p3, im_bk_p0, im_bk_p1;
	Pixel bp_p2, bp_p3, bi_p0, bi_p1, bi_p2, bi_p3, bj_p0, bj_p1, bj_p2, bj_p3, bk_p0, bk_p1;
	int bp_b2, bp_b3, bi_b0, bi_b1, bi_b2, bi_b3, bj_b0, bj_b1, bj_b2, bj_b3, bk_b0, bk_b1;
	int bi, bj;
	int prevBlock;
	double d, pdi, pdj, bd;
	unsigned int actBlockColour, actBlockColour0, actBlockColour1, actBlockColour2;

	for(y=0; y<im->h; y++)
	{
		prevBlock = 0;
		for(x=0; x<im->w; x+=8)
		{
			im_bp_p2 = imageGetPixel(im, x-2, y);
			im_bp_p3 = imageGetPixel(im, x-1, y);

			im_bi_p0 = imageGetPixel(im, x, y);
			im_bi_p1 = imageGetPixel(im, x+1, y);
			im_bi_p2 = imageGetPixel(im, x+2, y);
			im_bi_p3 = imageGetPixel(im, x+3, y);

			im_bj_p0 = imageGetPixel(im, x+4, y);
			im_bj_p1 = imageGetPixel(im, x+5, y);
			im_bj_p2 = imageGetPixel(im, x+6, y);
			im_bj_p3 = imageGetPixel(im, x+7, y);

			im_bk_p0 = imageGetPixel(im, x+9, y);
			im_bk_p1 = imageGetPixel(im, x+10, y);

			bd = 0x7fffffff; // big number;
			bi = 0;

			for(i=0; i<16; i++)
			{
				for(j=0; j<16; j++)
				{
					actBlockColour = aDoubleHiResBlock[prevBlock][prevBlock][i];

					bp_b2 = (actBlockColour & 0x00F0) >> 4;
					bp_b3 = (actBlockColour & 0x000F);

					bp_p2 = pal[bp_b2];
					bp_p3 = pal[bp_b3];

					actBlockColour0 = aDoubleHiResBlock[prevBlock][i][j];

					bi_b0 = (actBlockColour0 & 0xF000) >> 12;
					bi_b1 = (actBlockColour0 & 0x0F00) >> 8;
					bi_b2 = (actBlockColour0 & 0x00F0) >> 4;
					bi_b3 = (actBlockColour0 & 0x000F);

					bi_p0 = pal[bi_b0];
					bi_p1 = pal[bi_b1];
					bi_p2 = pal[bi_b2];
					bi_p3 = pal[bi_b3];

					pdi = 0;
					d = pixelDist(im_bp_p2, bp_p2);
					pdi = pdi + d;
					d = pixelDist(im_bp_p3, bp_p3);
					pdi = pdi + d;

					d = pixelDist(im_bi_p0, bi_p0);
					pdi = pdi + d;
					d = pixelDist(im_bi_p1, bi_p1);
					pdi = pdi + d;
					d = pixelDist(im_bi_p2, bi_p2);
					pdi = pdi + d;
					d = pixelDist(im_bi_p3, bi_p3);
					pdi = pdi + d;

					for(k=0; k<16; k++)
					{
						actBlockColour1 = aDoubleHiResBlock[i][j][k];

						bj_b0 = (actBlockColour1 & 0xF000) >> 12;
						bj_b1 = (actBlockColour1 & 0x0F00) >> 8;
						bj_b2 = (actBlockColour1 & 0x00F0) >> 4;
						bj_b3 = (actBlockColour1 & 0x000F);

						bj_p0 = pal[bj_b0];
						bj_p1 = pal[bj_b1];
						bj_p2 = pal[bj_b2];
						bj_p3 = pal[bj_b3];

						actBlockColour2 = aDoubleHiResBlock[j][k][k];

						bk_b0 = (actBlockColour2 & 0xF000) >> 12;
						bk_b1 = (actBlockColour2 & 0x0F00) >> 8;

						bk_p0 = pal[bk_b0];
						bk_p1 = pal[bk_b1];

						d = pixelDist(im_bj_p0, bj_p0);
						pdj = pdi + d;
						d = pixelDist(im_bj_p1, bj_p1);
						pdj = pdj + d;
						d = pixelDist(im_bj_p2, bj_p2);
						pdj = pdj + d;
						d = pixelDist(im_bj_p3, bj_p3);
						pdj = pdj + d;

						d = pixelDist(im_bk_p0, bk_p0);
						pdj = pdj + d;
						d = pixelDist(im_bk_p1, bk_p1);
						pdj = pdj + d;

//						if (pdj < bd) {
//						if (pdj <= bd) {
						if ((((pdj < bd)&&(optClosestLessThan == 0)) || ((pdj <= bd)&&(optClosestLessThan == 1))) && (((prevBlock != i) && (i != j)) || (i == 0))) {
							bd = pdj;
							bi = i;
							bj = j;
						}
					}
				}
			}
			blockImagePutBlock(ib, x/4, y, bi);
			blockImagePutBlock(ib, (x/4)+1, y, bj);
			prevBlock = bj;
		}
	}
	return ib;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;
}

static void block_EnhanceSin_OLD (ImageRef im, BlockImageRef ib)
{
	int x, y, z1, z2, z3, b0, b1, b2, b3, i, bi;
	Pixel im_p0, im_p1, im_p2, im_p3, p0, p1, p2, p3;
	double d, pd, cur_pd, bd;
	unsigned int actBlockColour;

	for(y=0; y<ib->h; y++)
	{
		for(x=0; x<ib->w; x++)
		{
			im_p0 = imageGetPixel(im, x*4, y);
			im_p1 = imageGetPixel(im, (x*4)+1, y);
			im_p2 = imageGetPixel(im, (x*4)+2, y);
			im_p3 = imageGetPixel(im, (x*4)+3, y);

			z1 = blockImageGetBlock(ib, x-1, y);
			z2 = blockImageGetBlock(ib, x, y);
			z3 = blockImageGetBlock(ib, x+1, y);
			actBlockColour = aDoubleHiResBlock[z1][z2][z3];

			b0 = (actBlockColour & 0xF000) >> 12;
			b1 = (actBlockColour & 0x0F00) >> 8;
			b2 = (actBlockColour & 0x00F0) >> 4;
			b3 = (actBlockColour & 0x000F);

			p0 = pal[b0];
			p1 = pal[b1];
			p2 = pal[b2];
			p3 = pal[b3];

			pd = 0;
			d = pixelDist(im_p0, p0);
			pd = pd + d;
			d = pixelDist(im_p1, p1);
			pd = pd + d;
			d = pixelDist(im_p2, p2);
			pd = pd + d;
			d = pixelDist(im_p3, p3);
			cur_pd = pd + d;

			for(i=0; i<16; i++)
			{
				bd = 0x7fffffff; // big number;
				bi = 0;

				actBlockColour = aDoubleHiResBlock[z1][i][z3];

				b0 = (actBlockColour & 0xF000) >> 12;
				b1 = (actBlockColour & 0x0F00) >> 8;
				b2 = (actBlockColour & 0x00F0) >> 4;
				b3 = (actBlockColour & 0x000F);

				p0 = pal[b0];
				p1 = pal[b1];
				p2 = pal[b2];
				p3 = pal[b3];

				pd = 0;
				d = pixelDist(im_p0, p0);
				pd = pd + d;
				d = pixelDist(im_p1, p1);
				pd = pd + d;
				d = pixelDist(im_p2, p2);
				pd = pd + d;
				d = pixelDist(im_p3, p3);
				pd = pd + d;

				if (pd < bd) {
					bd = pd;
					bi = i;
				}
			}

			if (bd < cur_pd) {
//				printf("test1 %d, %d, %d, %d\n", z2, bi, (int)cur_pd, (int)bd);
//				printf("test1 %d, %d, %d\n", im_p0.r, im_p0.g, im_p0.b);
//				for (i = 0; i < 1000000000; i++) {
//				}
				blockImagePutBlock(ib, x, y, bi);
			}

		}
	}
}

static void block_EnhanceDbl_OLD (ImageRef im, BlockImageRef ib)
{
	int x, y, z1, z2, z3, z4, z5, z6, b0, b1, b2, b3, b4, b5, b6, b7, i, j, bi, bj;
	Pixel im_p0, im_p1, im_p2, im_p3, im_p4, im_p5, im_p6, im_p7, p0, p1, p2, p3, p4, p5, p6, p7;
	double d, pd, cur_pd, bd;
	unsigned int actBlockColour0, actBlockColour1;

	for(y=0; y<ib->h; y++)
	{
		for(x=0; x<ib->w; x++)
		{
			im_p0 = imageGetPixel(im, x*4, y);
			im_p1 = imageGetPixel(im, (x*4)+1, y);
			im_p2 = imageGetPixel(im, (x*4)+2, y);
			im_p3 = imageGetPixel(im, (x*4)+3, y);

			z1 = blockImageGetBlock(ib, x-1, y);
			z2 = blockImageGetBlock(ib, x, y);
			z3 = blockImageGetBlock(ib, x+1, y);
			actBlockColour0 = aDoubleHiResBlock[z1][z2][z3];

			b0 = (actBlockColour0 & 0xF000) >> 12;
			b1 = (actBlockColour0 & 0x0F00) >> 8;
			b2 = (actBlockColour0 & 0x00F0) >> 4;
			b3 = (actBlockColour0 & 0x000F);

			p0 = pal[b0];
			p1 = pal[b1];
			p2 = pal[b2];
			p3 = pal[b3];

			im_p4 = imageGetPixel(im, x*4, y);
			im_p5 = imageGetPixel(im, (x*4)+1, y);
			im_p6 = imageGetPixel(im, (x*4)+2, y);
			im_p7 = imageGetPixel(im, (x*4)+3, y);

			z4 = blockImageGetBlock(ib, x, y);
			z5 = blockImageGetBlock(ib, x+1, y);
			z6 = blockImageGetBlock(ib, x+2, y);
			actBlockColour1 = aDoubleHiResBlock[z4][z5][z6];

			b4 = (actBlockColour1 & 0xF000) >> 12;
			b5 = (actBlockColour1 & 0x0F00) >> 8;
			b6 = (actBlockColour1 & 0x00F0) >> 4;
			b7 = (actBlockColour1 & 0x000F);

			p4 = pal[b4];
			p5 = pal[b5];
			p6 = pal[b6];
			p7 = pal[b7];

			pd = 0;
			d = pixelDist(im_p0, p0);
			pd = pd + d;
			d = pixelDist(im_p1, p1);
			pd = pd + d;
			d = pixelDist(im_p2, p2);
			pd = pd + d;
			d = pixelDist(im_p3, p3);
			pd = pd + d;
			d = pixelDist(im_p4, p4);
			pd = pd + d;
			d = pixelDist(im_p5, p5);
			pd = pd + d;
			d = pixelDist(im_p6, p6);
			pd = pd + d;
			d = pixelDist(im_p7, p7);
			cur_pd = pd + d;

			for(i=0; i<16; i++)
			{
				actBlockColour0 = aDoubleHiResBlock[z1][i][z3];

				b0 = (actBlockColour0 & 0xF000) >> 12;
				b1 = (actBlockColour0 & 0x0F00) >> 8;
				b2 = (actBlockColour0 & 0x00F0) >> 4;
				b3 = (actBlockColour0 & 0x000F);

				p0 = pal[b0];
				p1 = pal[b1];
				p2 = pal[b2];
				p3 = pal[b3];

				for(j=0; j<16; j++)
				{
					bd = 0x7fffffff; // big number;
					bi = 0;
					bj = 0;

					actBlockColour1 = aDoubleHiResBlock[z4][j][z6];

					b4 = (actBlockColour1 & 0xF000) >> 12;
					b5 = (actBlockColour1 & 0x0F00) >> 8;
					b6 = (actBlockColour1 & 0x00F0) >> 4;
					b7 = (actBlockColour1 & 0x000F);

					p4 = pal[b4];
					p5 = pal[b5];
					p6 = pal[b6];
					p7 = pal[b7];

					pd = 0;
					d = pixelDist(im_p0, p0);
					pd = pd + d;
					d = pixelDist(im_p1, p1);
					pd = pd + d;
					d = pixelDist(im_p2, p2);
					pd = pd + d;
					d = pixelDist(im_p3, p3);
					pd = pd + d;
					d = pixelDist(im_p4, p4);
					pd = pd + d;
					d = pixelDist(im_p5, p5);
					pd = pd + d;
					d = pixelDist(im_p6, p6);
					pd = pd + d;
					d = pixelDist(im_p7, p7);
					pd = pd + d;

					if (pd < bd) {
						bd = pd;
						bi = i;
						bj = j;
					}
				}
			}

			if (bd < cur_pd) {
//				printf("test1 %d, %d, %d, %d\n", z2, bi, (int)cur_pd, (int)bd);
//				printf("test1 %d, %d, %d\n", im_p0.r, im_p0.g, im_p0.b);
//				for (i = 0; i < 1000000000; i++) {
//				}
				blockImagePutBlock(ib, x, y, bi);
				if (x+1<ib->w) {
					blockImagePutBlock(ib, x+1, y, bj);
				}
			}

		}
	}
}

static void block_EnhanceSin (ImageRef im, BlockImageRef ib)
{
	int x, y, zm2, zm1, z, zp1, zp2, bp0, bp1, bp2, bp3, bc0, bc1, bc2, bc3, bn0, bn1, bn2, bn3, i, bi;
	Pixel im_p0, im_p1, im_p2, im_p3, im_c0, im_c1, im_c2, im_c3, im_n0, im_n1, im_n2, im_n3, p0, p1, p2, p3, c0, c1, c2, c3, n0, n1, n2, n3;
	double d, pd, cur_pd, bd;
	unsigned int actBlockColourPre, actBlockColourCur, actBlockColourNxt;

	for(y=0; y<ib->h; y++)
	{
		for(x=0; x<ib->w; x++)
		{
			zm2 = blockImageGetBlock(ib, x-2, y);
			zm1 = blockImageGetBlock(ib, x-1, y);
			z = blockImageGetBlock(ib, x, y);
			zp1 = blockImageGetBlock(ib, x+1, y);
			zp2 = blockImageGetBlock(ib, x+2, y);

			if (im->w == 560) {
				im_p0 = imageGetPixel(im, ((x-1)*4), y);
				im_p1 = imageGetPixel(im, ((x-1)*4)+1, y);
				im_p2 = imageGetPixel(im, ((x-1)*4)+2, y);
				im_p3 = imageGetPixel(im, ((x-1)*4)+3, y);
			} else {
				im_p0 = imageGetPixel(im, (x-1), y);
				im_p1 = im_p0;
				im_p2 = im_p0;
				im_p3 = im_p0;
			}

			actBlockColourPre = aDoubleHiResBlock[zm2][zm1][z];

			bp0 = (actBlockColourPre & 0xF000) >> 12;
			bp1 = (actBlockColourPre & 0x0F00) >> 8;
			bp2 = (actBlockColourPre & 0x00F0) >> 4;
			bp3 = (actBlockColourPre & 0x000F);

			p0 = pal[bp0];
			p1 = pal[bp1];
			p2 = pal[bp2];
			p3 = pal[bp3];

			if (im->w == 560) {
				im_c0 = imageGetPixel(im, (x*4), y);
				im_c1 = imageGetPixel(im, (x*4)+1, y);
				im_c2 = imageGetPixel(im, (x*4)+2, y);
				im_c3 = imageGetPixel(im, (x*4)+3, y);
			} else {
				im_c0 = imageGetPixel(im, x, y);
				im_c1 = im_c0;
				im_c2 = im_c0;
				im_c3 = im_c0;
			}

			actBlockColourCur = aDoubleHiResBlock[zm1][z][zp1];

			bc0 = (actBlockColourCur & 0xF000) >> 12;
			bc1 = (actBlockColourCur & 0x0F00) >> 8;
			bc2 = (actBlockColourCur & 0x00F0) >> 4;
			bc3 = (actBlockColourCur & 0x000F);

			c0 = pal[bc0];
			c1 = pal[bc1];
			c2 = pal[bc2];
			c3 = pal[bc3];

			if (im->w == 560) {
				im_n0 = imageGetPixel(im, ((x+1)*4), y);
				im_n1 = imageGetPixel(im, ((x+1)*4)+1, y);
				im_n2 = imageGetPixel(im, ((x+1)*4)+2, y);
				im_n3 = imageGetPixel(im, ((x+1)*4)+3, y);
			} else {
				im_n0 = imageGetPixel(im, (x+1), y);
				im_n1 = im_n0;
				im_n2 = im_n0;
				im_n3 = im_n0;
			}

			actBlockColourNxt = aDoubleHiResBlock[z][zp1][zp2];

			bn0 = (actBlockColourNxt & 0xF000) >> 12;
			bn1 = (actBlockColourNxt & 0x0F00) >> 8;
			bn2 = (actBlockColourNxt & 0x00F0) >> 4;
			bn3 = (actBlockColourNxt & 0x000F);

			n0 = pal[bn0];
			n1 = pal[bn1];
			n2 = pal[bn2];
			n3 = pal[bn3];

			pd = 0;
//			d = pixelDist(im_p0, p0);
//			pd = pd + d;
//			d = pixelDist(im_p1, p1);
//			pd = pd + d;
			d = pixelDist(im_p2, p2);
			pd = pd + d;
			d = pixelDist(im_p3, p3);
			pd = pd + d;
			d = pixelDist(im_c0, c0);
			pd = pd + d;
			d = pixelDist(im_c1, c1);
			pd = pd + d;
			d = pixelDist(im_c2, c2);
			pd = pd + d;
			d = pixelDist(im_c3, c3);
			pd = pd + d;
			d = pixelDist(im_n0, n0);
			pd = pd + d;
			d = pixelDist(im_n1, n1);
//			pd = pd + d;
//			d = pixelDist(im_n2, n2);
//			pd = pd + d;
//			d = pixelDist(im_n3, n3);
			cur_pd = pd + d;

			bd = 0x7fffffff; // big number;
			bi = 0;

			for(i=0; i<16; i++)
			{
				actBlockColourPre = aDoubleHiResBlock[zm2][zm1][i];

				bp0 = (actBlockColourPre & 0xF000) >> 12;
				bp1 = (actBlockColourPre & 0x0F00) >> 8;
				bp2 = (actBlockColourPre & 0x00F0) >> 4;
				bp3 = (actBlockColourPre & 0x000F);

				p0 = pal[bp0];
				p1 = pal[bp1];
				p2 = pal[bp2];
				p3 = pal[bp3];

				actBlockColourCur = aDoubleHiResBlock[zm1][i][zp1];

				bc0 = (actBlockColourCur & 0xF000) >> 12;
				bc1 = (actBlockColourCur & 0x0F00) >> 8;
				bc2 = (actBlockColourCur & 0x00F0) >> 4;
				bc3 = (actBlockColourCur & 0x000F);

				c0 = pal[bc0];
				c1 = pal[bc1];
				c2 = pal[bc2];
				c3 = pal[bc3];

				actBlockColourNxt = aDoubleHiResBlock[i][zp1][zp2];

				bn0 = (actBlockColourNxt & 0xF000) >> 12;
				bn1 = (actBlockColourNxt & 0x0F00) >> 8;
				bn2 = (actBlockColourNxt & 0x00F0) >> 4;
				bn3 = (actBlockColourNxt & 0x000F);

				n0 = pal[bn0];
				n1 = pal[bn1];
				n2 = pal[bn2];
				n3 = pal[bn3];

				pd = 0;
//				d = pixelDist(im_p0, p0);
//				pd = pd + d;
//				d = pixelDist(im_p1, p1);
//				pd = pd + d;
				d = pixelDist(im_p2, p2);
				pd = pd + d;
				d = pixelDist(im_p3, p3);
				pd = pd + d;
				d = pixelDist(im_c0, c0);
				pd = pd + d;
				d = pixelDist(im_c1, c1);
				pd = pd + d;
				d = pixelDist(im_c2, c2);
				pd = pd + d;
				d = pixelDist(im_c3, c3);
				pd = pd + d;
				d = pixelDist(im_n0, n0);
				pd = pd + d;
				d = pixelDist(im_n1, n1);
				pd = pd + d;
//				d = pixelDist(im_n2, n2);
//				pd = pd + d;
//				d = pixelDist(im_n3, n3);
//				pd = pd + d;

				if (pd < bd) {
					bd = pd;
					bi = i;
				}
			}

			if (bd < cur_pd) {
//				printf("test1 %d, %d, %d, %d\n", zc2, bi, (int)cur_pd, (int)bd);
//				printf("test1 %d, %d, %d\n", im_p0.r, im_p0.g, im_p0.b);
//				for (i = 0; i < 1000000000; i++) {
//				}
				blockImagePutBlock(ib, x, y, bi);
			}

		}
	}
}

static void block_EnhanceDbl (ImageRef im, BlockImageRef ib)
{
	int x, y, zm2, zm1, z, zp1, zp2, zp3, bp0, bp1, bp2, bp3, bc0, bc1, bc2, bc3, bn0, bn1, bn2, bn3, bf0, bf1, bf2, bf3, i, j, bi, bj;
	Pixel im_p0, im_p1, im_p2, im_p3, im_c0, im_c1, im_c2, im_c3, im_n0, im_n1, im_n2, im_n3, im_f0, im_f1, im_f2, im_f3, p0, p1, p2, p3, c0, c1, c2, c3, n0, n1, n2, n3, f0, f1, f2, f3;
	double d, pd, cur_pd, bd;
	unsigned int actBlockColourPre, actBlockColourCur, actBlockColourNxt, actBlockColourFar;

	for(y=0; y<ib->h; y++)
	{
		for(x=0; x<ib->w; x++)
		{
			zm2 = blockImageGetBlock(ib, x-2, y);
			zm1 = blockImageGetBlock(ib, x-1, y);
			z = blockImageGetBlock(ib, x, y);
			zp1 = blockImageGetBlock(ib, x+1, y);
			zp2 = blockImageGetBlock(ib, x+2, y);
			zp3 = blockImageGetBlock(ib, x+3, y);

			if (im->w == 560 ) {
				im_p0 = imageGetPixel(im, ((x-1)*4), y);
				im_p1 = imageGetPixel(im, ((x-1)*4)+1, y);
				im_p2 = imageGetPixel(im, ((x-1)*4)+2, y);
				im_p3 = imageGetPixel(im, ((x-1)*4)+3, y);
			} else {
				im_p0 = imageGetPixel(im, (x-1), y);
				im_p1 = im_p0;
				im_p2 = im_p0;
				im_p3 = im_p0;
			}

			actBlockColourPre = aDoubleHiResBlock[zm2][zm1][z];

			bp0 = (actBlockColourPre & 0xF000) >> 12;
			bp1 = (actBlockColourPre & 0x0F00) >> 8;
			bp2 = (actBlockColourPre & 0x00F0) >> 4;
			bp3 = (actBlockColourPre & 0x000F);

			p0 = pal[bp0];
			p1 = pal[bp1];
			p2 = pal[bp2];
			p3 = pal[bp3];

			if (im->w == 560 ) {
				im_c0 = imageGetPixel(im, (x*4), y);
				im_c1 = imageGetPixel(im, (x*4)+1, y);
				im_c2 = imageGetPixel(im, (x*4)+2, y);
				im_c3 = imageGetPixel(im, (x*4)+3, y);
			} else {
				im_c0 = imageGetPixel(im, x, y);
				im_c1 = im_c0;
				im_c2 = im_c0;
				im_c3 = im_c0;
			}

			actBlockColourCur = aDoubleHiResBlock[zm1][z][zp1];

			bc0 = (actBlockColourCur & 0xF000) >> 12;
			bc1 = (actBlockColourCur & 0x0F00) >> 8;
			bc2 = (actBlockColourCur & 0x00F0) >> 4;
			bc3 = (actBlockColourCur & 0x000F);

			c0 = pal[bc0];
			c1 = pal[bc1];
			c2 = pal[bc2];
			c3 = pal[bc3];

			if (im->w == 560 ) {
				im_n0 = imageGetPixel(im, ((x+1)*4), y);
				im_n1 = imageGetPixel(im, ((x+1)*4)+1, y);
				im_n2 = imageGetPixel(im, ((x+1)*4)+2, y);
				im_n3 = imageGetPixel(im, ((x+1)*4)+3, y);
			} else {
				im_n0 = imageGetPixel(im, (x+1), y);
				im_n1 = im_n0;
				im_n2 = im_n0;
				im_n3 = im_n0;
			}

			actBlockColourNxt = aDoubleHiResBlock[z][zp1][zp2];

			bn0 = (actBlockColourNxt & 0xF000) >> 12;
			bn1 = (actBlockColourNxt & 0x0F00) >> 8;
			bn2 = (actBlockColourNxt & 0x00F0) >> 4;
			bn3 = (actBlockColourNxt & 0x000F);

			n0 = pal[bn0];
			n1 = pal[bn1];
			n2 = pal[bn2];
			n3 = pal[bn3];

			if (im->w == 560 ) {
				im_f0 = imageGetPixel(im, ((x+2)*4), y);
				im_f1 = imageGetPixel(im, ((x+2)*4)+1, y);
				im_f2 = imageGetPixel(im, ((x+2)*4)+2, y);
				im_f3 = imageGetPixel(im, ((x+2)*4)+3, y);
			} else {
				im_f0 = imageGetPixel(im, (x+2), y);
				im_f1 = im_f0;
				im_f2 = im_f0;
				im_f3 = im_f0;
			}

			actBlockColourFar = aDoubleHiResBlock[zp1][zp2][zp3];

			bf0 = (actBlockColourFar & 0xF000) >> 12;
			bf1 = (actBlockColourFar & 0x0F00) >> 8;
			bf2 = (actBlockColourFar & 0x00F0) >> 4;
			bf3 = (actBlockColourFar & 0x000F);

			f0 = pal[bf0];
			f1 = pal[bf1];
			f2 = pal[bf2];
			f3 = pal[bf3];

			pd = 0;
//			d = pixelDist(im_p0, p0);
//			pd = pd + d;
//			d = pixelDist(im_p1, p1);
//			pd = pd + d;
			d = pixelDist(im_p2, p2);
			pd = pd + d;
			d = pixelDist(im_p3, p3);
			pd = pd + d;
			d = pixelDist(im_c0, c0);
			pd = pd + d;
			d = pixelDist(im_c1, c1);
			pd = pd + d;
			d = pixelDist(im_c2, c2);
			pd = pd + d;
			d = pixelDist(im_c3, c3);
			pd = pd + d;
			d = pixelDist(im_n0, n0);
			pd = pd + d;
			d = pixelDist(im_n1, n1);
			pd = pd + d;
			d = pixelDist(im_n2, n2);
			pd = pd + d;
			d = pixelDist(im_n3, n3);
			pd = pd + d;
			d = pixelDist(im_f0, f0);
			pd = pd + d;
			d = pixelDist(im_f1, f1);
//			pd = pd + d;
//			d = pixelDist(im_f2, f2);
//			pd = pd + d;
//			d = pixelDist(im_f3, f3);
			cur_pd = pd + d;

			bd = 0x7fffffff; // big number;
			bi = 0;
			bj = 0;

			for(i=0; i<16; i++)
			{
				actBlockColourPre = aDoubleHiResBlock[zm2][zm1][i];

				bp0 = (actBlockColourPre & 0xF000) >> 12;
				bp1 = (actBlockColourPre & 0x0F00) >> 8;
				bp2 = (actBlockColourPre & 0x00F0) >> 4;
				bp3 = (actBlockColourPre & 0x000F);

				p0 = pal[bp0];
				p1 = pal[bp1];
				p2 = pal[bp2];
				p3 = pal[bp3];

				for(j=0; j<16; j++)
				{
					actBlockColourCur = aDoubleHiResBlock[zm1][i][j];

					bc0 = (actBlockColourCur & 0xF000) >> 12;
					bc1 = (actBlockColourCur & 0x0F00) >> 8;
					bc2 = (actBlockColourCur & 0x00F0) >> 4;
					bc3 = (actBlockColourCur & 0x000F);

					c0 = pal[bc0];
					c1 = pal[bc1];
					c2 = pal[bc2];
					c3 = pal[bc3];

					actBlockColourNxt = aDoubleHiResBlock[i][j][zp2];

					bn0 = (actBlockColourNxt & 0xF000) >> 12;
					bn1 = (actBlockColourNxt & 0x0F00) >> 8;
					bn2 = (actBlockColourNxt & 0x00F0) >> 4;
					bn3 = (actBlockColourNxt & 0x000F);

					n0 = pal[bn0];
					n1 = pal[bn1];
					n2 = pal[bn2];
					n3 = pal[bn3];

					actBlockColourFar = aDoubleHiResBlock[j][zp2][zp3];

					bf0 = (actBlockColourFar & 0xF000) >> 12;
					bf1 = (actBlockColourFar & 0x0F00) >> 8;
					bf2 = (actBlockColourFar & 0x00F0) >> 4;
					bf3 = (actBlockColourFar & 0x000F);

					f0 = pal[bf0];
					f1 = pal[bf1];
					f2 = pal[bf2];
					f3 = pal[bf3];

					pd = 0;
//					d = pixelDist(im_p0, p0);
//					pd = pd + d;
//					d = pixelDist(im_p1, p1);
//					pd = pd + d;
					d = pixelDist(im_p2, p2);
					pd = pd + d;
					d = pixelDist(im_p3, p3);
					pd = pd + d;
					d = pixelDist(im_c0, c0);
					pd = pd + d;
					d = pixelDist(im_c1, c1);
					pd = pd + d;
					d = pixelDist(im_c2, c2);
					pd = pd + d;
					d = pixelDist(im_c3, c3);
					pd = pd + d;
					d = pixelDist(im_n0, n0);
					pd = pd + d;
					d = pixelDist(im_n1, n1);
					pd = pd + d;
					d = pixelDist(im_n2, n2);
					pd = pd + d;
					d = pixelDist(im_n3, n3);
					pd = pd + d;
					d = pixelDist(im_f0, f0);
					pd = pd + d;
					d = pixelDist(im_f1, f1);
					pd = pd + d;
//					d = pixelDist(im_f2, f2);
//					pd = pd + d;
//					d = pixelDist(im_f3, f3);
//					pd = pd + d;

					if (pd < bd) {
						bd = pd;
						bi = i;
						bj = j;
					}
				}
			}

			if (bd < cur_pd) {
//				printf("test1 %d, %d, %d, %d\n", zc2, bi, (int)cur_pd, (int)bd);
//				printf("test1 %d, %d, %d\n", im_p0.r, im_p0.g, im_p0.b);
//				for (i = 0; i < 1000000000; i++) {
//				}
				blockImagePutBlock(ib, x, y, bi);
				if (x+1<ib->w) {
					blockImagePutBlock(ib, x+1, y, bj);
				}
			}

		}
	}
}

static void blockToDhgr (BlockImageRef ib, unsigned char *dhgr)
{
	int y,x,b,d,i;
	unsigned int ad;

	for (y = 0; y < ib->h; y += 1) {
		ad = baseaddr[y];
		for (x = 0; x < ib->w; x += 7) {
			d = 0;
			for (i = 0; i < 7; i += 1) {
				b = blockImageGetBlock(ib, x + i, y);

//				if (b == 0x0a || b == 0x05) {
//					if (i <= 1) {
//						b = 0x0a;
//					}
//					else
//					{
//						b = ((blockImageGetBlock(ib, x + (i - 2), y) & 8) ? 0x05 : 0x0a);
//					}
//				}

				b = ((b & 1) << 3) | ((b & 2) << 1) | ((b & 4) >> 1) | ((b & 8) >> 3);
				d = d | b << 4*i;
			}

			for (i = 0; i < 4; ++i) {
				unsigned char bits;
				unsigned int  addr;
				addr = ad + i/2;
				bits = (unsigned char)(d & 0x7f);
				if (i & 1) {
					dhgr[0x2000+addr] = bits;
				}
				else {
					dhgr[addr] = bits;
				}
				d = d >> 7;
			}
			
			ad += 2;
		}
	}
}

//static char nbuf[1024];

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char **argv) {

	int i, j, k, m, n, p;
//	ImageRef im, sm, gm;
	FILE *inf;
	FILE *outf;
	ImageRef sm, sm1, sm2, sm3;
	int fullResolution = 0;
	BlockImageRef ib, ib2;

	initBaseAddrs();

	if (optPreviewPalletEquProcessPallet) {
		optPreviewPallet = optProcessPallet;
	}
	memcpy(pal, palletlist[optProcessPallet], sizeof(pal));
	memcpy(previewpal, palletlist[optPreviewPallet], sizeof(previewpal));

	im_try = imageNew(560, 1);
	if (!im_try) goto emalloc2;

	strncpy(nbuf, "source.bmp", 1000);
	inf = fopen(nbuf, "rb");
	if (!inf) {
		printf("Failed to open file '%s'\n", nbuf);
	}
	else
	{
		sm = imageFromBMP(inf, &fullResolution);
		if (!sm) {
			fclose(inf);
			printf("Can't read BMP file '%s'\n", nbuf);
		}
/*
		sm = imageNew(140, 192);
		imageTest(sm);
*/
//		printf("Resolution %d\n", fullResolution);

		if (sm)
		{
			initDoubleHiResBlock();

			if (!fullResolution)
			{
				sm1 = rgbToRgb_Stretched(sm, 4, 2);
				strncpy(nbuf, "Test1_eff.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				ib = rgbToBlock (sm);
				if (!ib) goto emalloc;
				sm1 = blockToRgb_Actual(ib, 2);
				strncpy(nbuf, "Test1_act.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				block_EnhanceSin (sm, ib);
//				block_EnhanceDbl (sm, ib);
				sm1 = blockToRgb_Actual(ib, 2);
				strncpy(nbuf, "Test1_enh.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				blockToDhgr(ib, grbuf);
				strncpy(nbuf, "Test1.dhgr", 1000);
				outf = fopen(nbuf, "wb");
				fwrite(grbuf, 1, 0x4000, outf);
				fclose(outf);

				if (ib->b) free(ib->b);
				if (ib) free(ib);
			}
			else
			{
				sm1 = rgbToRgb_Stretched(sm, 1, 2);
				strncpy(nbuf, "O.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);
/*
				sm1 = rgbToRgb_16Colour(sm, 1, 2);
				strncpy(nbuf, "Test2_16c.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);
*/
//				if (1==2) {
				for (m = 0; m < 17; m++) {
				if (m == 4) { // Remove crappy process pallets
//				if (m == 16) { // 3D

					optProcessPallet = m;

					if (optPreviewPalletEquProcessPallet) {
						optPreviewPallet = optProcessPallet;
					}
					memcpy(pal, palletlist[optProcessPallet], sizeof(pal));
					memcpy(previewpal, palletlist[optPreviewPallet], sizeof(previewpal));


//					ib = rgbToBlock_Convert0(sm);
					ib = rgbToBlock_Convert1(sm);
//					ib = rgbToBlock_Convert2(sm);
//					ib = rgbToBlock_2at_a_time(sm);
					if (!ib) goto emalloc;
					sm1 = blockToRgb_Actual(ib, 2);
//					sprintf(nbuf, "Test2_p%d_act.bmp", optProcessPallet);
//					write_bmp(nbuf, sm1);
					if (sm1->p) free(sm1->p);
					if (sm1) free(sm1);
/*
					sm1 = blockToRgb_Storage(ib, 2);
					strncpy(nbuf, "Test2_sto.bmp", 1000);
					write_bmp(nbuf, sm1);
					if (sm1->p) free(sm1->p);
					if (sm1) free(sm1);

					blockToDhgr(ib, grbuf);
					strncpy(nbuf, "Test2.dhgr", 1000);
					outf = fopen(nbuf, "wb");
					fwrite(grbuf, 1, 0x4000, outf);
					fclose(outf);
*/
					block_EnhanceSin (sm, ib);
					block_EnhanceSin (sm, ib);
//					block_EnhanceDbl (sm, ib);
					sm1 = blockToRgb_Actual(ib, 2);
					sprintf(nbuf, "Q_p%d_act.bmp", optProcessPallet);
					write_bmp(nbuf, sm1);
					if (sm1->p) free(sm1->p);
					if (sm1) free(sm1);

/*
					sm1 = blockToRgb_Storage(ib, 2);
					strncpy(nbuf, "Test3_sto.bmp", 1000);
					write_bmp(nbuf, sm1);
					if (sm1->p) free(sm1->p);
					if (sm1) free(sm1);
*/
					blockToDhgr(ib, grbuf);
					sprintf(nbuf, "Q_p%d.dhgr", optProcessPallet);
					outf = fopen(nbuf, "wb");
					fwrite(grbuf, 1, 0x4000, outf);
					fclose(outf);
					if (ib->b) free(ib->b);
					if (ib) free(ib);
				}
				}
//				}
/*
//				sm2 = rgbToRgb_Stretched(sm, 1, 1); // Copy image

//				ib = rgbToBlock_DitherRiemersma(sm2);
//				ib = rgbToBlock_DitherYliluoma3(sm2);
//				ib = rgbToBlock_Dither2at_a_time(sm2);
				if (!ib) goto emalloc;
				sm1 = blockToRgb_Actual(ib, 2);
//				strncpy(nbuf, "Test4_act.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);
*/




				for (m = 0; m < 17; m++) {
//					if ((m == 0)||(m == 1)||(m == 3)||(m == 4)||(m == 8)||(m == 9)||(m == 10)||(m == 11)||(m == 15)) { // Remove crappy process pallets
//					if ((m == 0)||(m == 1)||(m == 3)||(m == 4)||(m == 7)||(m == 8)||(m == 10)||(m == 11)||(m == 12)||(m == 15)) { // Remove crappy process pallets
//					if ((m == 2)||(m == 5)||(m == 6)||(m == 9)||(m == 13)||(m == 14)) { // Remove crappy process pallets
//					if ((m == 0)||(m == 4)||(m == 8)||(m == 15)) { // Remove crappy process pallets
//					if ((m == 0)||(m == 4)||(m == 9)||(m == 15)) { // Remove crappy process pallets
//					if ((m == 3)||(m == 4)||(m == 9)) { // Remove crappy process pallets
//					if ((m == 0)||(m == 4)||(m == 8)||(m == 9)) { // Remove crappy process pallets
					if (m == 4) {
//					if ((m == 4)||(m == 16)) { // 3D
						optProcessPallet = m;
						if (optPreviewPalletEquProcessPallet) {
							optPreviewPallet = optProcessPallet;
						}
						memcpy(pal, palletlist[optProcessPallet], sizeof(pal));

						for (k = 0; k < 4; k++) {
							optProcessDir = k;
//							optProcessDir = 0;

							for (i = 0; i < 16; i++) {
//								if (i != 4) { // Remove the worst dithers
//								if (i == 15) { // Testing processing only one dither type
//								if ((i == 2)||(i == 5)||(i == 6)||(i == 7)||(i == 15)) { // Select the better dithers
								if ((i == 2)||(i == 6)||(i == 7)||(i == 15)) { // Select the better dithers

									for (p = 0; p < 2; p++) {
										optClosestLessThan = p;

										for (n = 0; n < 16; n++) {
//											if ((n == 3)||(n == 12)||(n == 15)) { // Remove crappy preview pallets
//											if ((n == 3)||(n == 4)||(n == 15)) { // Remove crappy preview pallets
											if (n == 4) {
												optPreviewPallet = n;
												memcpy(previewpal, palletlist[optPreviewPallet], sizeof(previewpal));

//												for (n = 1; n < 7; n++) {
//													optColourDifference = n;
													sm2 = rgbToRgb_Stretched(sm, 1, 1); // Copy image
									
													ib = rgbToBlock_Dither(sm2, i);
//													ib = rgbToBlock_DitherAlternate(sm2, i);
													if (!ib) goto emalloc;
													sm1 = blockToRgb_Actual(ib, 2);
													sprintf(nbuf, "D_pp%d_p%d_%s_d%d_c%d_act.bmp", optPreviewPallet, optProcessPallet, error_diffusion_list[i], optProcessDir, optClosestLessThan);
													write_bmp(nbuf, sm1);
													if (sm1->p) free(sm1->p);
													if (sm1) free(sm1);
//												}
											}
										}

										sm2 = rgbToRgb_Stretched(sm, 1, 1); // Copy image
										ib = rgbToBlock_Dither(sm2, i);
										blockToDhgr(ib, grbuf);
										sprintf(nbuf, "D_p%d_%s_d%d_c%d_act.dhgr", optProcessPallet, error_diffusion_list[i], optProcessDir, optClosestLessThan);
										outf = fopen(nbuf, "wb");
										fwrite(grbuf, 1, 0x4000, outf);
										fclose(outf);

									}
								}
							}
						}
					}
				}



/*
				sm1 = rgbToRgb_Stretched(sm2, 1, 2);
				strncpy(nbuf, "Test4_img.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				sm1 = blockToRgb_Storage(ib, 2);
				strncpy(nbuf, "Test4_sto.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				blockToDhgr(ib, grbuf);
				strncpy(nbuf, "Test4.dhgr", 1000);
				outf = fopen(nbuf, "wb");
				fwrite(grbuf, 1, 0x4000, outf);
				fclose(outf);

//				block_EnhanceSin (sm2, ib);
				block_EnhanceDbl (sm2, ib);
				sm1 = blockToRgb_Actual(ib, 2);
				strncpy(nbuf, "Test5_act.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				sm1 = blockToRgb_Storage(ib, 2);
				strncpy(nbuf, "Test5_sto.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				blockToDhgr(ib, grbuf);
				strncpy(nbuf, "Test5.dhgr", 1000);
				outf = fopen(nbuf, "wb");
				fwrite(grbuf, 1, 0x4000, outf);
				fclose(outf);
				if (ib->b) free(ib->b);
				if (ib) free(ib);

				if (sm2->p) free(sm2->p);
				if (sm2) free(sm2);

//				ib2 = blockImageNew(sm->w/4, sm->h);
//				if (!ib2) goto emalloc;
//				memcpy(ib2, ib, sizeof(ib));

				sm2 = rgbToRgb_Stretched(sm, 1, 1); // Copy image

				ib2 = rgbToBlock_Dither(sm2, 1);
				if (!ib2) goto emalloc;

//				block_EnhanceSin (sm, ib2);
//				block_EnhanceSin (sm, ib2);
//				block_EnhanceSin (sm, ib2);
				block_EnhanceDbl (sm, ib2);
				sm1 = blockToRgb_Actual(ib2, 2);
				strncpy(nbuf, "Test6_act.bmp", 1000);
				write_bmp(nbuf, sm1);
				if (sm1->p) free(sm1->p);
				if (sm1) free(sm1);

				if (ib2->b) free(ib2->b);
				if (ib2) free(ib2);
*/


				if (sm2->p) free(sm2->p);
				if (sm2) free(sm2);

			}

			if (sm->p) free(sm->p);
			if (sm) free(sm);
		}
	}

	printf("A2BestPix");

	for (j = 0; j < 1000000000; j++) {
	}

	return 0;

emalloc:
	if (ib->b) free(ib->b);
	if (ib) free(ib);
	return 0;

emalloc2:
	if (im_try->p) free(im_try->p);
	if (im_try) free(im_try);
	return 0;

}

