#pragma once
#include "stdafx.h"

class ICM
{
private:
	cimg_library::CImg<unsigned char>* img;
	unsigned char icms[9][32][32][32];
public:
	ICM(const char* palette);
	~ICM();
	void make_image(const char* filename);
	void make_icm(const char* filename);
};
