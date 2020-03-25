#include "stdafx.h"

#include "slp.h"
#include "palette.h"

using namespace cimg_library;

SLP::SLP()
{
	pixels_total = 0;
	loaded = false;
}

SLP::~SLP()
{
}

bool SLP::load(const char* filename)
{
	FILE* f;
	fopen_s(&f, filename, "rb");
	if (!f)
		return false;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	unsigned char* fbase = (unsigned char*)malloc(size);
	fseek(f, 0, SEEK_SET);
	fread_s(fbase, size, size, 1, f);
	fclose(f);
	unsigned char* fptr = fbase;

	//file_header hdr;
	memcpy(&hdr, fptr, sizeof(file_header));
	fptr += sizeof(file_header);

	frame_i = new frame_info[hdr.num_frames];
	for (int i = 0; i < hdr.num_frames; i++)
	{
		frame_i[i].cmd_table_offset = *(unsigned long*)fptr;		fptr += 4;
		frame_i[i].outline_table_offset = *(unsigned long*)fptr;	fptr += 4;
		frame_i[i].palette_offset = *(unsigned long*)fptr;			fptr += 4;
		frame_i[i].properties = *(unsigned long*)fptr;				fptr += 4;
		frame_i[i].width = *(long*)fptr;							fptr += 4;
		frame_i[i].height = *(long*)fptr;							fptr += 4;
		frame_i[i].hotspot_x = *(long*)fptr;						fptr += 4;
		frame_i[i].hotspot_y = *(long*)fptr;						fptr += 4;
		frame_i[i].data = NULL;
	}

	edge = new rowedge*[hdr.num_frames];
	rowoffset = new long*[hdr.num_frames];
	for (int i = 0; i < hdr.num_frames; i++)
	{
		fptr = fbase + frame_i[i].outline_table_offset;

		edge[i] = new rowedge[frame_i[i].height];
		memcpy(edge[i], fptr, sizeof(rowedge) * frame_i[i].height);
		fptr += sizeof(rowedge) * frame_i[i].height;

		rowoffset[i] = new long[frame_i[i].height];
		memcpy(rowoffset[i], fptr, sizeof(long) * frame_i[i].height);

		frame_i[i].data = new pixel*[frame_i[i].height];
		for (int j = 0; j < frame_i[i].height; j++)
		{
			pixel px;
			px.type = T_TRANSPARENT;
			px.data = 0;
			frame_i[i].data[j] = new pixel[frame_i[i].width];
			//parse line
			if ((edge[i][j].left == -32768) && (edge[i][j].right == -32768))	//entire line is blank
			{
				for (int k = 0; k < frame_i[i].width; k++)
					frame_i[i].data[j][k] = px;
				((int*)(fbase + frame_i[i].cmd_table_offset))[j] = 0;
				continue;
			}
			//left edge
			int k = 0;	//current position in line
			for (; k < edge[i][j].left; k++)
				frame_i[i].data[j][k] = px;
			//parse actual data
			unsigned char cmd;
			int count;
			unsigned char* ptr = fbase + ((int*)(fbase + frame_i[i].cmd_table_offset))[j];
			while (1)
			{
				cmd = *ptr;
				ptr++;

				if ((cmd & 0x03) == 0x00)   //check 2 right bits (color list)
				{
					px.type = T_PIXEL;
					count = cmd >> 2;
					for (int t = 0; t < count; t++)
					{
						px.data = *ptr;
						frame_i[i].data[j][k] = px;
						k++;
						ptr++;
					}
				}
				else if ((cmd & 0x03) == 0x01)		//skip
				{
					px.type = T_TRANSPARENT;
					count = cmd >> 2;
					if (!count)
					{
						count = *ptr;
						ptr++;
					}
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
				}
				else if ((cmd & 0x0F) == 0x02)		//big color list
				{
					px.type = T_PIXEL;
					count = cmd >> 4;
					count = count * 0x100 + *ptr;
					ptr++;
					for (int t = 0; t < count; t++)
					{
						px.data = *ptr;
						frame_i[i].data[j][k] = px;
						k++;
						ptr++;
					}
				}
				else if ((cmd & 0x0F) == 0x03)		//big skip
				{
					px.type = T_TRANSPARENT;
					count = cmd >> 4;
					count = count * 0x100 + *ptr;
					ptr++;
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
				}
				else if ((cmd & 0x0F) == 0x06)		//player color list
				{
					px.type = T_PLAYER_COLOR;
					count = cmd >> 4;
					if (!count)
					{
						count = *ptr;
						ptr++;
					}
					for (int t = 0; t < count; t++)
					{
						px.data = *ptr;
						frame_i[i].data[j][k] = px;
						k++;
						ptr++;
					}
				}
				else if ((cmd & 0x0F) == 0x07)		//color fill
				{
					px.type = T_PIXEL;
					count = cmd >> 4;
					if (!count)
					{
						count = *ptr;
						ptr++;
					}
					px.data = *ptr;
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
					ptr++;
				}
				else if ((cmd & 0x0F) == 0x0A)		//player color fill
				{
					px.type = T_PLAYER_COLOR;
					count = cmd >> 4;
					if (!count)
					{
						count = *ptr;
						ptr++;
					}
					px.data = *ptr;
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
					ptr++;
				}
				else if ((cmd & 0x0F) == 0x0B)		//shadow
				{
					px.type = T_SHADOW;
					count = cmd >> 4;
					if (!count)
					{
						count = *ptr;
						ptr++;
					}
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
				}
				else if (cmd == 0x7E)		//outline shield, run
				{
					px.type = T_OUTLINE_SHIELD;
					count = *ptr;
					ptr++;
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
				}
				else if (cmd == 0x5E)		//outline color, run
				{
					px.type = T_OUTLINE_COLOR;
					count = *ptr;
					ptr++;
					for (int t = 0; t < count; t++)
					{
						frame_i[i].data[j][k] = px;
						k++;
					}
				}
				else if (cmd == 0x6E)		//outline shield, single
				{
					px.type = T_OUTLINE_SHIELD;
					frame_i[i].data[j][k] = px;
					k++;
				}
				else if (cmd == 0x4E)		//outline color, single
				{
					px.type = T_OUTLINE_COLOR;
					frame_i[i].data[j][k] = px;
					k++;
				}

				else if (cmd == 0x0F)		//end of line
				{
					break;
				}
				else
				{
					__debugbreak();
				}
			}
			//fill the rest with blank pixels;
			px.type = T_TRANSPARENT;
			for (; k < frame_i[i].width; k++)
				frame_i[i].data[j][k] = px;

			/*for (int k = 0; k < frame_i[i].width; k++)
				if (frame_i[i].data[j][k].type != TRANSPARENT)
					continue;
			((int*)(fbase + frame_i[i].cmd_table_offset))[j] = 0;*/
		}
	}

	free(fbase);
	loaded = true;
}

long SLP::getSize() { return size; }

bool SLP::load_terrainmap(const char* filename, const char* palette, int dx, int dy)
{
	CImg<unsigned char>* img = NULL;
	try
	{
		img = new CImg<unsigned char>(filename);
	}
	catch (CImgIOException e)
	{
		printf(" error: cannot open file\n");
		exit(0);
		return false;
	}
	catch (CImgInstanceException e)
	{
		printf(" error: out of memory\n");
		exit(0);
		return false;
	}
	printf(" OK\nParsing image...");
	//check dimensions
	int width = img->width();
	int height = img->height();

	if ((width % 48 != 1) || (height % 24 != 1))
		return false;
	if ((width / 48) != (height / 24))
		return false;
	if ((dx == -1) || (dy == -1))
	{
		dx = width / 96;
		dy = height / 48;
	}
	if ((dx + dy) * 24 >= height)
	{
		printf(" error: specified map size is out of range\n");
		exit(0);
		return false;
	}

	//load palette (temp)
	T_PALETTE pal(palette);
	//create frames
	memcpy(hdr.version, "2.0N", 4);
	memset(hdr.comment, 0, sizeof(hdr.comment));
	strncpy_s(hdr.comment, "terrainmap v1.0", 15);
	
	hdr.num_frames = dx*dy;
	edge = new rowedge*[hdr.num_frames];
	rowoffset = new long*[hdr.num_frames];
	frame_i = new frame_info[hdr.num_frames];
	for (int i = 0; i < hdr.num_frames; i++)
	{
		memset(&frame_i[i], 0, sizeof(frame_i[i]));
		frame_i[i].width = 97;
		frame_i[i].height = 49;
		edge[i] = new rowedge[frame_i[i].height];
		rowoffset[i] = new long[frame_i[i].height];
		frame_i[i].data = new pixel*[frame_i[i].height];
		//frame offsets
		int off_x = (i % dx) * 48 + (i / dx) * 48;
		int off_y = dx * 24 - (i % dx) * 24 + (i / dx) * 24 - 24;
		for (int j = 0; j < frame_i[i].height; j++)
		{
			frame_i[i].data[j] = new pixel[frame_i[i].width];
			edge[i][j].left = j < 24 ? 48 - j * 2 : j * 2 - 48;
			edge[i][j].right = edge[i][j].left;
			int k = 0;
			for (; k < edge[i][j].left; k++)
				frame_i[i].data[j][k].type = T_TRANSPARENT;
			for (; k < frame_i[i].width - edge[i][j].right; k++)
			{
				frame_i[i].data[j][k].type = T_PIXEL;
				frame_i[i].data[j][k].data = pal.get_index(
					RGB(
						(*img)(off_x + k, off_y + j, 0),
						(*img)(off_x + k, off_y + j, 1),
						(*img)(off_x + k, off_y + j, 2))
				);
			}
			for (; k < frame_i[i].width; k++)
				frame_i[i].data[j][k].type = T_TRANSPARENT;
		}
	}
	size = width*height;
	delete img;

	return true;
}

//parser state machine states
enum states
{
	COPY,
	FILL,
	SET
};

////////////------------------------
int print_TEST(unsigned char** ptr, int type, int len, pixel* row, int start)
{
	unsigned char* p = *ptr;
	unsigned char cmd = 0;
	cmd = 0x0E;
	*p++ = cmd;
	*ptr = p;
	return start + len;
}
////////////------------------------

int print_FILL(unsigned char** ptr, int type, int len, pixel* row, int start)
{
	//////
	//start = print_TEST(ptr, type, len, row, start);
	//////
	unsigned char* p = *ptr;
	unsigned char cmd = 0;
	switch (type)
	{
	case T_PIXEL:
		if (len < 16)
		{
			cmd = len << 4;
			cmd |= 0x07;
			*p++ = cmd;
		}
		else if (len <= 0xFF)
		{
			cmd = 0x07;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
		{
			print_FILL(&p, type, len - 0xFF, row, start);
			cmd = 0x07;
			*p++ = cmd;
			*p++ = 0xFF;
		}
		break;
	case T_PLAYER_COLOR:
		if (len < 16)
		{
			cmd = len << 4;
			cmd |= 0x0A;
			*p++ = cmd;
		}
		else if (len <= 0xFF)
		{
			cmd = 0x0A;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
			__debugbreak();
		break;
	default:
		__debugbreak();
		break;
	}
	*p++ = row[start].data;
	*ptr = p;
	return start + len;
}

int print_COPY(unsigned char** ptr, int type, int len, pixel* row, int start)
{
	//////
	//start = print_TEST(ptr, type, len, row, start);
	//////
	unsigned char* p = *ptr;
	/*bool fill_flag = false;
	if (len >= 3)	//check run end
	{
		if ((row[start + len - 1] == row[start + len - 2]) && (row[start + len - 1] == row[start + len - 3]))
		{
			len -= 3;
			fill_flag = true;
		}
	}*/
	unsigned char cmd = 0;
	switch (type)
	{
	case T_PIXEL:
		if (len < 64)
		{
			cmd = len << 2;
			*p++ = cmd;
		}
		else if (len <= 0xFFF)
		{
			cmd = len >> 4;
			cmd &= 0xF0;
			cmd |= 0x02;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
			__debugbreak();
		break;
	case T_PLAYER_COLOR:
		if (len < 16)
		{
			cmd = len << 4;
			cmd |= 0x06;
			*p++ = cmd;
		}
		else if (len <= 0xFF)
		{
			cmd = 0x06;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
			__debugbreak();
		break;
	default:
		__debugbreak();
		break;
	}
	for (int i = 0; i < len; i++)
	{
		*p = row[start++].data;
		p++;
	}

	//if (fill_flag)
	//	start = print_FILL(&p, type, 3, row, start);

	*ptr = p;
	return start;
}

int print_SET(unsigned char** ptr, int type, int len, pixel* row, int start)
{
	//////
	//start = print_TEST(ptr, type, len, row, start);
	//////
	unsigned char* p = *ptr;
	unsigned char cmd = 0;
	switch (type)
	{
	case T_SHADOW:
		if (len < 16)
		{
			cmd = len << 4;
			cmd |= 0x0B;
			*p++ = cmd;
		}
		else if (len <= 0xFF)
		{
			cmd = 0x0B;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
		{
			cmd = 0x0B;
			*p++ = cmd;
			*p++ = 0xFF;
			//start = print_SET(&p, type, len - 0xFF, row, start += 0xFF);
			print_SET(&p, type, len - 0xFF, row, start);
		}
		break;
	case T_TRANSPARENT:
		if (len < 64)
		{
			cmd = len << 2;
			cmd |= 0x01;
			*p++ = cmd;
		}
		else if (len <= 0xFFF)
		{
			cmd = len >> 4;
			cmd &= 0xF0;
			cmd |= 0x03;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
			__debugbreak();
		break;
	case T_OUTLINE_COLOR:
		if (len > 1)
		{
			cmd = 0x5E;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
		{
			cmd = 0x4E;
			*p++ = cmd;
		}
		break;
	case T_OUTLINE_SHIELD:
		if (len > 1)
		{
			cmd = 0x7E;
			*p++ = cmd;
			*p++ = len & 0xFF;
		}
		else
		{
			cmd = 0x6E;
			*p++ = cmd;
		}
		break;
	}
	*ptr = p;
	return start + len;
}

bool compare_edges(rowedge** edge, int height, int frame)
{
	for (int i = 0; i < height; i++)
		if ((edge[frame][i].left != edge[frame - 1][i].left) || (edge[frame][i].right != edge[frame - 1][i].right))
			return false;
	return true;
}

void SLP::optimize(const char* filename, bool allow_fill)
{
	long new_size = 0;
	void* new_slp;
	if (allow_fill)
		new_slp = malloc(size * 10);
	else
		new_slp = malloc(size * 1.5);
	if (!new_slp)
	{
		printf(" error: out of memory\n");
		exit(0);
	}
	unsigned char* ptr = (unsigned char*)new_slp;

	memcpy(ptr, &hdr, sizeof(file_header));
	ptr += sizeof(file_header);

	//print header later, now skip
	ptr += hdr.num_frames * 8 * sizeof(long);

	for (int i = 0; i < hdr.num_frames; i++)
	{
		//dirty hack: store array here temporarly
		if (frame_i[i].height > 0)
			frame_i[i].cmd_table_offset = (unsigned long)malloc(sizeof(unsigned long)*frame_i[i].height);
		else
			frame_i[i].cmd_table_offset = 0;

		for (int j = 0; j < frame_i[i].height; j++)
		{
			//
			unsigned char* temp_ptr = ptr;
			//
			((unsigned long*)frame_i[i].cmd_table_offset)[j] = ptr - (unsigned char*)new_slp;
			int k = 0;
			while ((frame_i[i].data[j][k].type == T_TRANSPARENT) && (k < (frame_i[i].width - edge[i][j].right)))
				k++;
			if (k >= frame_i[i].width)
			{
				edge[i][j].left = -32768; edge[i][j].right = -32768;
				continue;
			}
			else
			{
				edge[i][j].left = k;
				if (edge[i][j].right == -32768)
					edge[i][j].right = 0;
			}

			int start = k;
			//main scan loop
			int state;
			int repeat = 1;
			int len = 1;
			int type = frame_i[i].data[j][k].type;
			int data = frame_i[i].data[j][k].data;
			switch (frame_i[i].data[j][k].type)
			{
			case T_PIXEL:
			case T_PLAYER_COLOR:
				state = COPY;
				break;
			case T_SHADOW:
			case T_TRANSPARENT:
			case T_OUTLINE_COLOR:
			case T_OUTLINE_SHIELD:
				state = SET;
				break;
			default:
				__debugbreak();
				break;
			}
			k++;

			//below is an implementation of a state machine, refer to the graph
			while (k < (frame_i[i].width - edge[i][j].right))
			{
				switch (state)
				{
				case COPY:
					if (frame_i[i].data[j][k].type == type)
					{
						if ((repeat < 3) || !allow_fill)
						{
							len++;
							if (data == frame_i[i].data[j][k].data)
								repeat++;
							else
								repeat = 1;
						}
						else
						{
							len -= 3;
							if (len != 0)
								start = print_COPY(&ptr, type, len, frame_i[i].data[j], start);
							state = FILL;
							len = 3;
							k--;
						}
					}
					else
					{
						start = print_COPY(&ptr, type, len, frame_i[i].data[j], start);
						len = 1;
						repeat = 1;
						switch (frame_i[i].data[j][k].type)
						{
						case T_PIXEL:
						case T_PLAYER_COLOR:
							break;
						case T_SHADOW:
						case T_TRANSPARENT:
						case T_OUTLINE_COLOR:
						case T_OUTLINE_SHIELD:
							state = SET;
							break;
						default:
							__debugbreak();
							break;
						}
					}
					break;
				case FILL:
					if ((frame_i[i].data[j][k].type == type) && (frame_i[i].data[j][k].data == data))
						len++;
					else
					{
						start = print_FILL(&ptr, type, len, frame_i[i].data[j], start);
						len = 1;
						repeat = 1;
						switch (frame_i[i].data[j][k].type)
						{
						case T_PIXEL:
						case T_PLAYER_COLOR:
							state = COPY;
							break;
						case T_SHADOW:
						case T_TRANSPARENT:
						case T_OUTLINE_COLOR:
						case T_OUTLINE_SHIELD:
							state = SET;
							break;
						default:
							__debugbreak();
							break;
						}
					}
					break;
				case SET:
					if (frame_i[i].data[j][k].type == type)
						len++;
					else
					{
						start = print_SET(&ptr, type, len, frame_i[i].data[j], start);
						len = 1;
						repeat = 1;
						switch (frame_i[i].data[j][k].type)
						{
						case T_PIXEL:
						case T_PLAYER_COLOR:
							state = COPY;
							break;
						case T_SHADOW:
						case T_TRANSPARENT:
						case T_OUTLINE_COLOR:
						case T_OUTLINE_SHIELD:
							break;
						default:
							__debugbreak();
							break;
						}
					}
					break;
				default:
					__debugbreak();
					break;
				}
				type = frame_i[i].data[j][k].type;
				data = frame_i[i].data[j][k].data;
				k++;
			}
			//print what's left
			switch (state)
			{
			case COPY:
				start = print_COPY(&ptr, type, len, frame_i[i].data[j], start);
				break;
			case FILL:
				start = print_FILL(&ptr, type, len, frame_i[i].data[j], start);
				break;
			case SET:
				if (type != T_TRANSPARENT)
					start = print_SET(&ptr, type, len, frame_i[i].data[j], start);
				else
					edge[i][j].right = len;		//might be off by one, check!!!!!!
				break;
			default:
				__debugbreak();
				break;
			}
			*ptr = 0x0F;	ptr++;
			//for (unsigned char* p = temp_ptr; p < (ptr - 3); p++)
			//	if ((*p == 0xF9) && (*(p + 1) == 0xF9) && (*(p + 2) == 0xF9))
			//		__debugbreak();
		}
	}
	unsigned char* prev_frame_start_ptr = 0;
	unsigned char* frame_start_ptr = 0;
	//print outline offsets
	for (int i = 0; i < hdr.num_frames; i++)
	{
		prev_frame_start_ptr = frame_start_ptr;
		if (frame_i[i].cmd_table_offset)
			frame_start_ptr = (unsigned char*)new_slp + ((unsigned long*)frame_i[i].cmd_table_offset)[0];
		else
		{
			frame_i[i].width = 0;
			frame_start_ptr = 0;
			continue;
		}

		frame_i[i].outline_table_offset = ptr - (unsigned char*)new_slp;

		//trim outline rows
		short min = 0x7FFF;
		for (int j = 0; j < frame_i[i].height; j++)
			if ((edge[i][j].left < min) && (edge[i][j].left != -32768))
				min = edge[i][j].left;
		if (min == 0x7FFF)	//frame is blank
		{
			frame_i[i].height = 0;
			frame_i[i].width = 0;
			frame_i[i].cmd_table_offset = 0;
			frame_i[i].outline_table_offset = 0;
			continue;
		}
		for (int j = 0; j < frame_i[i].height; j++)
		{
			if (edge[i][j].left != -32768)
				edge[i][j].left -= min;
			if (edge[i][j].right != -32768)
				edge[i][j].right += min;
		}
		if (min != 0x7FFF)
			frame_i[i].hotspot_x -= min;

		//now, remove redudant rows
		int first = 0;
		int last = frame_i[i].height;
		while ((edge[i][first].left == -32768) && (edge[i][first].right == -32768))
		{
			first++;
			frame_i[i].hotspot_y--;
			frame_i[i].height--;
		}
		do
		{
			last--;
			frame_i[i].height--;
		}
		while ((edge[i][last].left == -32768) && (edge[i][last].right == -32768));

		frame_i[i].height++;

		//compare with previous frame
		if (!prev_frame_start_ptr || (frame_start_ptr == prev_frame_start_ptr) ||
			memcmp(prev_frame_start_ptr, frame_start_ptr, frame_start_ptr - prev_frame_start_ptr) ||
			!((frame_i[i].height == frame_i[i - 1].height) && compare_edges(edge, frame_i[i].height, i)))
		{
			short* p = (short*)ptr;
			for (int j = first; j <= last; j++)
			{
				*p = edge[i][j].left; p++;
				*p = edge[i][j].right; p++;
			}
			ptr = (unsigned char*)p;

			//print cmd table offsets
			unsigned long cmd_table_offset_tmp = (unsigned long)(ptr - (unsigned char*)new_slp);
			unsigned long* q = (unsigned long*)ptr;
			for (int j = first; j <= last; j++)
			{
				*q = ((unsigned long*)frame_i[i].cmd_table_offset)[j];	q++;
			}
			ptr = (unsigned char*)q;
			free((void*)frame_i[i].cmd_table_offset);
			frame_i[i].cmd_table_offset = cmd_table_offset_tmp;
		}
		else	//frames are equal
		{
			memmove(frame_start_ptr, frame_start_ptr + (frame_start_ptr - prev_frame_start_ptr),
				ptr - (frame_start_ptr + (frame_start_ptr - prev_frame_start_ptr)));
			ptr -= frame_start_ptr - prev_frame_start_ptr;
			free((void*)frame_i[i].cmd_table_offset);
			
			for (int k = 0; k < i; k++)
				frame_i[k].cmd_table_offset -= frame_start_ptr - prev_frame_start_ptr;
			frame_i[i].cmd_table_offset = frame_i[i - 1].cmd_table_offset;
			frame_i[i].outline_table_offset = frame_i[i - 1].outline_table_offset;

			for (int k = 0; k < hdr.num_frames; k++)
			{
				if (k > i)
					for (int s = 0; s < frame_i[k].height; s++)
						((unsigned long*)frame_i[k].cmd_table_offset)[s] -= frame_start_ptr - prev_frame_start_ptr;
				frame_i[k].outline_table_offset -= frame_start_ptr - prev_frame_start_ptr;
			}

			frame_start_ptr = prev_frame_start_ptr;
		}
	}
	size = ptr - (unsigned char*)new_slp;
	ptr = (unsigned char*)new_slp + sizeof(file_header);
	for (int i = 0; i < hdr.num_frames; i++)
	{
		*(unsigned long*)ptr = frame_i[i].cmd_table_offset;		ptr += 4;
		*(unsigned long*)ptr = frame_i[i].outline_table_offset;	ptr += 4;
		*(unsigned long*)ptr = frame_i[i].palette_offset;		ptr += 4;
		*(unsigned long*)ptr = frame_i[i].properties;			ptr += 4;
		*(long*)ptr = frame_i[i].width;							ptr += 4;
		*(long*)ptr = frame_i[i].height;						ptr += 4;
		*(long*)ptr = frame_i[i].hotspot_x;						ptr += 4;
		*(long*)ptr = frame_i[i].hotspot_y;						ptr += 4;
	}

	FILE* f;
	fopen_s(&f, filename, "wb");
	if (!f)
	{
		printf(" error: cannot write file\n");
		exit(0);
	}
	fwrite(new_slp, size, 1, f);
	fclose(f);
	free(new_slp);
}

long SLP::count_pixels(long* p)
{
	long count = 0;
	pixels_total = 0;
	for (int i = 0; i < hdr.num_frames; i++)
		for (int j = 0; j < frame_i[i].height; j++)
			for (int k = 0; k < frame_i[i].width; k++)
			{
				//if ((frame_i[i].data[j][k].type == PIXEL) && (frame_i[i].data[j][k].data == p))
				//	count++;
				if (frame_i[i].data[j][k].type == T_PIXEL)
				{
					pixels_total++;
					p[frame_i[i].data[j][k].data]++;
				}
			}
	return pixels_total;
}

void SLP::color_replace(unsigned char* c1, unsigned char* c2, int count)
{
	for (int i = 0; i < hdr.num_frames; i++)
		for (int j = 0; j < frame_i[i].height; j++)
			for (int k = 0; k < frame_i[i].width; k++)
				if (frame_i[i].data[j][k].type == T_PIXEL)
					for (int t = 0; t < count; t++)
						if (frame_i[i].data[j][k].data == c1[t])
							frame_i[i].data[j][k].data = c2[t];
}

long SLP::get_pixel_count()
{
	return pixels_total;
}

//TODO:
//INVESTIGATE SLPS IN RELEASE FOLDER
//FIX CLIFFS
//INTERFACE SLPS
//WORK ON REPEAT COMMAND