#pragma once

#include <string>

//additional structures
struct pixel
{
	unsigned char type;
	unsigned char data;
	bool operator==(pixel& p)
	{
		return ((p.type == type) && (p.data == data));
	}
};

enum types
{
	T_PIXEL,
	T_PLAYER_COLOR,
	T_TRANSPARENT,
	T_SHADOW,
	T_OUTLINE_SHIELD,
	T_OUTLINE_COLOR
};

//SLP structures

struct file_header
{
	char  version[4];
	long  num_frames;
	char  comment[24];
};

struct frame_info
{
	unsigned long	cmd_table_offset;
	unsigned long	outline_table_offset;
	unsigned long	palette_offset;
	unsigned long	properties;
	long	width;
	long	height;
	long	hotspot_x;
	long	hotspot_y;
	//
	pixel** data;
};

struct rowedge
{
	short left, right;
};

class SLP
{
private:
	std::string file;

	file_header hdr;
	frame_info* frame_i;
	rowedge** edge;
	long** rowoffset;

	bool loaded;
	long size;

	long pixels_total;

public:
	SLP();
	~SLP();

	bool load(const char* filename);
	void optimize(const char* filename, bool);
	long count_pixels(long*);
	void color_replace(unsigned char* c1, unsigned char* c2, int count);
	long get_pixel_count();
	long getSize();

	pixel** data;

	bool load_terrainmap(const char* filename, const char* palette, int, int);

};
