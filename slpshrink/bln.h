#pragma once

class BLN
{
private:
	//unsigned char entries[256][256][20];
	unsigned char* entries;
public:
	BLN(const char*);
	~BLN();
	void make_bln(const char*);
};
