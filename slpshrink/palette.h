#pragma once

class T_PALETTE
{
private:
	COLORREF entries[256];
	HPALETTE pal;
	std::map<COLORREF, int> cache;
public:
	T_PALETTE(const char*);
	int get_index(COLORREF);
	COLORREF get_entry(int index);
};
