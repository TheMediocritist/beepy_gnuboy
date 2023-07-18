
/*
 * Support for the Linux framebuffer device
 * Copyright 2001 Laguna
 * MGA BES code derived from fbtv
 * Copyright Gerd Knorr
 * This file may be distributed under the terms of the GNU GPL.
 */

#undef _GNU_SOURCE
 #define _GNU_SOURCE
 #include <string.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <sys/mman.h>
 #include <linux/fb.h>
 #include <sys/ioctl.h>
 #include <fcntl.h>
 
 #include "defs.h"
 #include "fb.h"
 #include "rc.h"
 #include "sys.h"
 
 struct fb fb;
 
 #define FBSET_CMD "fbset"
 static char *fb_mode;
 static int fb_depth;
 static int vmode[3];
 
 #define FB_DEVICE "/dev/fb0"
 static char *fb_device;
 
 static int fbfd = -1;
 static byte *fbmap;
 static int maplen;
 static byte *mmio;
 static int bes;
 static int base;
 static int use_yuv = -1;
 static int use_interp = 1;
 
 static struct fb_fix_screeninfo fi;
 static struct fb_var_screeninfo vi, initial_vi;
 
 rcvar_t vid_exports[] =
 {
	 RCV_VECTOR("vmode", &vmode, 3, "video mode: w h bpp"),
	 RCV_STRING("fb_device", &fb_device, "frame buffer device"),
	 RCV_STRING("fb_mode", &fb_mode, "run fbset with the specified mode string"),
	 RCV_INT("fb_depth", &fb_depth, "override fbcon depth"),
	 RCV_BOOL("yuv", &use_yuv, "force hardware YUV scaling"),
	 RCV_BOOL("yuvinterp", &use_interp, "disable YUV interpolation filter"),
	 RCV_END
 };
 
 static byte *new_fbmap = NULL; // New pointer variable for the array
 
 static void wrio4(int a, int v)
 {
 #ifndef IS_LITTLE_ENDIAN
	 v = (v<<24) | ((v&0xff00)<<8) | ((v&0xff0000)>>8) | (v>>24);
 #endif
	 *(int*)(mmio+a) = v;
 }
 
 static void overlay_switch()
 {
	 if (!fb.yuv) return;
	 if (!fb.enabled)
	 {
		 bes = 0;
		 return;
	 }
	 if (bes) return;
	 bes = 1;
	 memset(new_fbmap, 0, maplen);
}

static void overlay_init()
{
	if (!mmio | !use_yuv) return;
	if (use_yuv < 0) if ((vmode[0] < 320) || (vmode[1] < 288)) return;
	switch (fi.accel)
	{
#ifdef FB_ACCEL_MATROX_MGAG200
	case FB_ACCEL_MATROX_MGAG200:
#endif
#ifdef FB_ACCEL_MATROX_MGAG400
	case FB_ACCEL_MATROX_MGAG400:
#endif
		break;
	default:
		return;
	}
	fb.w = 160;
	fb.h = 144;
	fb.pitch = 640;
	fb.pelsize = 4;
	fb.yuv = 1;
	fb.cc[0].r = fb.cc[1].r = fb.cc[2].r = fb.cc[3].r = 0;
	fb.cc[0].l = 0;
	fb.cc[1].l = 24;
	fb.cc[2].l = 8;
	fb.cc[3].l = 16;
	base = vi.yres * vi.xres_virtual * ((vi.bits_per_pixel+7)>>3);
	
	maplen = base + fb.pitch * fb.h;
}

static void plain_init()
{
	fb.w = vi.xres;
	fb.h = vi.yres;
	fb.pelsize = (vi.bits_per_pixel+7)>>3;
	fb.pitch = vi.xres_virtual * fb.pelsize;
	fb.indexed = fi.visual == FB_VISUAL_PSEUDOCOLOR;

	fb.cc[0].r = 8 - vi.red.length;
	fb.cc[1].r = 8 - vi.green.length;
	fb.cc[2].r = 8 - vi.blue.length;
	fb.cc[0].l = vi.red.offset;
	fb.cc[1].l = vi.green.offset;
	fb.cc[2].l = vi.blue.offset;
	
	maplen = fb.pitch * fb.h;
}

void vid_init()
{
	char cmd[256];

	kb_init();

	if (!fb_device)
		if (!(fb_device = getenv("FRAMEBUFFER")))
			fb_device = strdup(FB_DEVICE);
	fbfd = open(fb_device, O_RDWR);
	if (fbfd < 0) die("cannot open %s\n", fb_device);
	
	ioctl(fbfd, FBIOGET_VSCREENINFO, &initial_vi);
	initial_vi.xoffset = initial_vi.yoffset = 0;

	if (fb_mode)
	{
		sprintf(cmd, FBSET_CMD " %.80s", fb_mode);
		system(cmd);
	}
	
	ioctl(fbfd, FBIOGET_VSCREENINFO, &vi);
	if (fb_depth) vi.bits_per_pixel = fb_depth;
	vi.xoffset = vi.yoffset = 0;
	vi.accel_flags = 0;
	vi.activate = FB_ACTIVATE_NOW;
	ioctl(fbfd, FBIOPUT_VSCREENINFO, &vi);
	ioctl(fbfd, FBIOGET_VSCREENINFO, &vi);
	ioctl(fbfd, FBIOGET_FSCREENINFO, &fi);

	if (!vmode[0] || !vmode[1])
	{
		int scale = rc_getint("scale");
		if (scale < 1) scale = 1;
		vmode[0] = 160 * scale;
		vmode[1] = 144 * scale;
	}
	if (vmode[0] > vi.xres) vmode[0] = vi.xres;
	if (vmode[1] > vi.yres) vmode[1] = vi.yres;
	
	mmio = mmap(0, fi.mmio_len, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, fi.smem_len);
	if ((long)mmio == -1) mmio = 0;

	overlay_init();
	
		if (!fb.yuv) plain_init();
	
		// Allocate memory for the new array
		maplen = base + fb.pitch * fb.h;
		new_fbmap = (byte *)malloc(maplen);
		if (!new_fbmap) die("cannot allocate memory for new_fbmap\n");
	
		fbmap = mmap(0, maplen, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
		if (!fbmap) die("cannot mmap %s (%d bytes)\n", fb_device, maplen);
	
		fb.ptr = new_fbmap; // Point fb.ptr to the new array
	
		memset(new_fbmap, 0, maplen);
		fb.dirty = 0;
		fb.enabled = 1;
	
		overlay_switch();
	}

static void framebuffer_copy()
	{
		// // Source area
		// int src_width = 160; // Width of the source area
		// int src_height = 120; // Height of the source area
		// int src_x = 80; // X-coordinate of the top-left corner of the source area
		// int src_y = 40; // Y-coordinate of the top-left corner of the source area
	
		// // Destination area
		// int dest_x = 0; // X-coordinate of the top-left corner of the destination area
		// int dest_y = 40; // Y-coordinate of the top-left corner of the destination area
		// int dest_width = src_width * 2; // Width of the destination area (doubled scale)
		// int dest_height = src_height * 2; // Height of the destination area (doubled scale)
		// int pixel_bytes = 4;
		
		// for (int y = 0; y < dest_height; y++)
		// {
		// 	for (int x = 0; x < dest_width*pixel_bytes; x+pixel_bytes)
		// 	{
		// 		fbmap[(dest_y + y) * dest_width + x] = new_fbmap[(src_y + y) * src_width + x/2];
		// 	}
		// }
		
	for (int row = 60; row < 180; row++)
		{
			for (int b = 320; b < 1120; b++)
			{
				fbmap[(row-60)*3200+b*2] = new_fbmap[row*(400*4)+b];
				fbmap[(row-60)*3200+b*2+1600] = new_fbmap[row*(400*4)+b];
			}
		}
		
	// 	// Source area
	// 	int src_width = 400; // Width of the source area
	// 	int src_height = 240; // Height of the source area
	// 	int src_x = 0; // X-coordinate of the top-left corner of the source area
	// 	int src_y = 0; // Y-coordinate of the top-left corner of the source area
	// 
	// 	// Destination area
	// 	int dest_x = 0; // X-coordinate of the top-left corner of the destination area
	// 	int dest_y = 0; // Y-coordinate of the top-left corner of the destination area
	// 	int dest_width = src_width;// * 2; // Width of the destination area (doubled scale)
	// 	int dest_height = src_height;// * 2; // Height of the destination area (doubled scale)
	// 
	// 	// Copy the area from new_fbmap to fbmap with scaling
	// 	for (int y = 0; y < dest_height; y++)
	// 	{
	// 		for (int x = 0; x < dest_width*(vi.bits_per_pixel / 8); x++)
	// 		{
	// 			// Calculate the corresponding position in the source area
	// 			int src_pos_x = (x * src_width) / dest_width + src_x;
	// 			int src_pos_y = (y * src_height) / dest_height + src_y;
	// 
	// 			// Calculate the corresponding position in the destination area
	// 			int dest_pos_x = x + dest_x;
	// 			int dest_pos_y = y + dest_y;
	// 
	// 			// Copy the pixel from the source area to the destination area
	// 			fbmap[(dest_pos_y * vi.xres_virtual + dest_pos_x) * (vi.bits_per_pixel / 8)] =
	// 				new_fbmap[(src_pos_y * vi.xres_virtual + src_pos_x) * (vi.bits_per_pixel / 8)];
	// 		}
	// 	}
	}

void vid_close()
	{
		fb.enabled = 0;
		overlay_switch();
		kb_close();
		ioctl(fbfd, FBIOPUT_VSCREENINFO, &initial_vi);
		memset(new_fbmap, 0, maplen);
		if (new_fbmap)
		{
			free(new_fbmap); // Free the new array memory if it was allocated
			new_fbmap = NULL;
		}
	}

void vid_preinit()
{
}

void vid_settitle(char *title)
{
}

void vid_setpal(int i, int r, int g, int b)
{
	unsigned short rr = r<<8, gg = g<<8, bb = b<<8;
	struct fb_cmap cmap;
	memset(&cmap, 0, sizeof cmap);
	cmap.start = i;
	cmap.len = 1;
	cmap.red = &rr;
	cmap.green = &gg;
	cmap.blue = &bb;
	ioctl(fbfd, FBIOPUTCMAP, &cmap);
}

void vid_begin()
{
	overlay_switch();
	//framebuffer_copy();
}

void vid_end()
{
	framebuffer_copy();
	overlay_switch();
}

void ev_poll(int wait)
{
	(void) wait;
	kb_poll();
	joy_poll();
}
