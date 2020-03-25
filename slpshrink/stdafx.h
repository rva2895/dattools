// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <map>

#define cimg_display 0
#define cimg_use_png
//#define cimg_use_z
#include "C:\\Users\\Vasya\\Documents\\Visual Studio 2015\\Projects\\CImg-2.4.0_pre090618\\CImg.h"

typedef struct {
	double r;       // a fraction between 0 and 1
	double g;       // a fraction between 0 and 1
	double b;       // a fraction between 0 and 1
} rgb;

typedef struct {
	double h;       // angle in degrees
	double s;       // a fraction between 0 and 1
	double v;       // a fraction between 0 and 1
} hsv;

hsv   rgb2hsv(rgb in);
rgb   hsv2rgb(hsv in);

rgb mix(rgb fg, rgb bg, double fa, double ba);

// TODO: reference additional headers your program requires here
