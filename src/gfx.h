/* gfx.h, GVRAM Graphical function prototypes for the pc98launcher.
 Copyright (C) 2020  John Snowdon
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

#define GFX_VERBOSE		0			// Turn on/off gfx-specific debug output
#define GFX_VESA_TEXT	0x03			// Standard DOS text mode
#define GFX_VESA_DESIRED	0x100		// The default VESA mode we want
#define GFX_BPP			8			// Colour depth that we want
#define GFX_BITPLANES	1			// The number of bitplanes we want
#define GFX_ROWS			400			// NUmbe of pixels in a row
#define GFX_COLS			640			// Number of pixels in a column
#define GFX_ROW_SIZE		640		 	// NUmber of bytes in a row (pixels per row * bytes per pixel)
#define GFX_COL_SIZE 	400			// NUmber of bytes in a column
#define GFX_PIXEL_SIZE	1			// 1 byte per pixel

#define RGB_BLACK		0x0000		// Simple RGB definition for a black 16bit pixel (5551 representation?)
#define RGB_WHITE		0xFFFF		// Simple RGB definition for a white 16bit pixel (5551 representation?)

#define GFX_ERR_UNSUPPORTED_BPP		-254
#define GFX_ERR_MISSING_BMPHEADER	-253
#define GFX_TEXT_OK           		-252 // Output of text data ok
#define GFX_TEXT_INVALID      		-251 // Attempted output of an unsupported font glyph (too wide, too heigh, etc)

#define VRAM_START					0		// Relative start offset into the local memory buffer
#define VRAM_END						256000	// End of the local memory buffer, should be GFX_ROWS * GFX_COLS * GFX_PIXEL_SIZE

/* **************************** */
/* Function prototypes */
/* **************************** */

int			gfx_Bitmap(int x, int y, bmpdata_t *bmpdata);
int 			gfx_BitmapAsync(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate, int remap_palette, int reserved_palette);
int 			gfx_BitmapAsyncFull(int x, int y, bmpdata_t *bmpdata, FILE *bmpfile, bmpstate_t *bmpstate, int remap_palette, int reserved_palette);
int 			gfx_Box(int x1, int y1, int x2, int y2, unsigned char palette);
int 			gfx_BoxFill(int x1, int y1, int x2, int y2, unsigned char palette);
int			gfx_BoxFillTranslucent(int x1, int y1, int x2, int y2, unsigned char palette);
void			gfx_Clear();
void			gfx_Close();
void			gfx_Flip();
long int		gfx_GetXYaddr(unsigned short int x, unsigned short int y);
int			gfx_Init();
int 			gfx_Puts(int x, int y, fontdata_t *fontdata, char *c);
void			gfx_TextOff();
void			gfx_TextOn();
