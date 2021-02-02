/* palette.c, Palette get/set functions for mapping 8bit bitmap colours
 to those available in the pc98Launcher via the PC-98 PEGC hardware.
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

#include <stdio.h>
#include <conio.h>

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

#ifndef __HAS_PAL
#include "palette.h"
#define __HAS_PAL
#endif

unsigned int free_palettes_used;			// Current number of palette entries used
unsigned int reserved_palettes_used;		// Current number of palette entries used

int pal_BMPState2Palette(bmpdata_t *bmpdata, bmpstate_t *bmpstate, int reserved){
	// Set palette entries based on the current data in a bmpstate->pixels structure - 
	
	return pal_BMP2Palette_(bmpdata, bmpstate, reserved, 0);
	
}

int pal_BMP2Palette(bmpdata_t *bmpdata, int reserved){
	// Set palette entries based on the current data in a bmp->pixels structure - 
	// this is just a single line, so we can't go remapping all of the pixels
	// in the main bmp->pixels structure (which is empty if loading a bmp asyncrhonously).
	
	return pal_BMP2Palette_(bmpdata, NULL, reserved, 1);
}

int pal_BMP2Palette_(bmpdata_t *bmpdata, bmpstate_t *bmpstate, int reserved, int is_full_bmp){
	// Set palette entries based on a specific bmpdata structure
	// restricted == 1 if we are setting the 32 reserved colours of the user interface
	
	int i;
	int f;
	
	if (reserved == 1){
		// Set palette entries for the restricted region (UI elements)
		
		if (PALETTE_VERBOSE){
			printf("%s.%d\t pal_BMP2Palette() Setting reserved UI palette entries\n", __FILE__, __LINE__);
		}
		
		if (reserved_palettes_used < PALETTES_RESERVED){
			f = PALETTES_FREE;
			for(i = 0; i < bmpdata->colours; i++){
				
				if (i >= PALETTES_RESERVED){
					// Somehow this image has more colours than we have reserved palette entries
					if (PALETTE_VERBOSE){
						printf("%s.%d\t pal_BMP2Palette() Warning, more colours than available in reserved palette entry space!\n", __FILE__, __LINE__);
					}
					return -1;
				}
				
				if (reserved_palettes_used < PALETTES_RESERVED){
					pal_Set((i + PALETTES_FREE), bmpdata->palette[i].r, bmpdata->palette[i].g, bmpdata->palette[i].b);
					bmpdata->palette[i].new_palette_entry = i + PALETTES_FREE;
					reserved_palettes_used++;
				} else {
					// No free palette entries remaining - this shouldnt happen with the UI!!!
					if (PALETTE_VERBOSE){
						printf("%s.%d\t pal_BMP2Palette() Warning, we ran out of reserved palette entries before processing all colours!\n", __FILE__, __LINE__);
					}
					return -1;
				}
			}
			// Remap the bitmap pixels to the new palette indices
			pal_BMPRemap(bmpdata);
			return bmpdata->colours;
		} else {
			// Maximum number of palette entries reached
			if (PALETTE_VERBOSE){
				printf("%s.%d\t pal_BMP2Palette() Reserved UI palette entries already set\n", __FILE__, __LINE__);
			}
			// Remap the bitmap pixels to the new palette indices
			for (i = 0; i < bmpdata->colours; i++){
				bmpdata->palette[i].new_palette_entry = i + PALETTES_FREE;	
			}
			if (is_full_bmp){
				pal_BMPRemap(bmpdata);
			} else {
				pal_BMPStateRemap(bmpdata, bmpstate);
			}
			return -1;
		}
		
	} else {
		// Set palette entries for the free region (artwork etc)
		if (free_palettes_used < PALETTES_FREE){
			for(i = 0; i < bmpdata->colours; i++){
				if (free_palettes_used < PALETTES_FREE){
					pal_Set(i, bmpdata->palette[i].r, bmpdata->palette[i].g, bmpdata->palette[i].b);
					bmpdata->palette[i].new_palette_entry = i;
					free_palettes_used++;
				}
			}			
			return bmpdata->colours;
		} else {
			// Maximum number of palette entries reached	
			return -1;
		}
	}
}

int pal_BMPRemap(bmpdata_t *bmpdata){

	unsigned char *px;
	unsigned char c;
	int i;
	long int pos;
	long int px_remapped, colours_remapped, remapped;
	
	px_remapped = 0;
	colours_remapped = 0;
	
	if (bmpdata->pixels == NULL){
		if (PALETTE_VERBOSE){
			printf("%s.%d\t pal_BMPRemap() Unable to remap palette; no pixel data in bmp structure\n", __FILE__, __LINE__);
		}
		return PALETTE_NO_PIXELS;
	} else {
		
		px = bmpdata->pixels;
		// A single loop over all the pixels
		for (pos = 0; pos < bmpdata->size; pos++){
			// Read the value of the current pixel, this is the palette entry number
			c = *px;
			// Set it to the new palette entry number
			*px = (unsigned char) bmpdata->palette[c].new_palette_entry;
			px_remapped++;
			// Step to next pixel
			px++;
		}
		
		if (PALETTE_VERBOSE){
			printf("%s.%d\t pal_BMPRemap() Total of %ld pixels remapped\n", __FILE__, __LINE__, px_remapped);
		}
		return PALETTE_OK;
	}
}

int pal_BMPStateRemap(bmpdata_t *bmpdata, bmpstate_t *bmpstate){

	unsigned char *px;
	unsigned char c;
	int i;
	long int pos;
	int px_remapped, colours_remapped, remapped;
	
	px_remapped = 0;
	colours_remapped = 0;
	
	if (bmpdata->pixels == NULL){
		if (PALETTE_VERBOSE){
			printf("%s.%d\t pal_BMPStateRemap() Unable to remap palette; no pixel data in bmp structure\n", __FILE__, __LINE__);
		}
		return PALETTE_NO_PIXELS;
	} else {
		
		px = bmpstate->pixels;
		// A single loop over all the pixels
		for (pos = 0; pos < bmpstate->width_bytes; pos++){
			// Read the value of the current pixel, this is the palette entry number
			c = *px;
			// Set it to the new palette entry number
			*px = (unsigned char) bmpdata->palette[c].new_palette_entry;
			px_remapped++;
			// Step to next pixel
			px++;
		}
		
		if (PALETTE_VERBOSE){
			printf("%s.%d\t pal_BMPStateRemap() Total of %d pixels remapped\n", __FILE__, __LINE__, px_remapped);
		}
		return PALETTE_OK;
	}
}

void pal_ResetAll(){
	// Reset all palette entries
	
	unsigned int i;
	
	if (PALETTE_VERBOSE){
		printf("%s.%d\t pal_ResetAll() Resetting all palette entries\n", __FILE__, __LINE__);		
	}
	
	for (i = 0; i < 256; i++){
		outp(VGA_PALETTE_MASK_ADDR, 0xFF);
		outp(VGA_PALETTE_SEL_ADDR, i);
		outp(VGA_PALETTE_SET_ADDR, 0x0);
		outp(VGA_PALETTE_SET_ADDR, 0x0);
		outp(VGA_PALETTE_SET_ADDR, 0x0);	
	}
	
	reserved_palettes_used = 0;
	free_palettes_used = 0;
	
	return;
}

void pal_ResetFree(){
	// Reset non-reserved palette entries
	
	unsigned int i;
	
	if (PALETTE_VERBOSE){
		printf("%s.%d\t pal_Reset() FreeResetting free palette entries range (0-%d)\n", __FILE__, __LINE__, PALETTES_FREE);		
	}
	
	for (i = 0; i < PALETTES_FREE; i++){
		outp(VGA_PALETTE_MASK_ADDR, 0xFF);
		outp(VGA_PALETTE_SEL_ADDR, i);
		outp(VGA_PALETTE_SET_ADDR, 0x0);
		outp(VGA_PALETTE_SET_ADDR, 0x0);
		outp(VGA_PALETTE_SET_ADDR, 0x0);	
	}
	
	free_palettes_used = 0;
	
	return;
}

void pal_Set(unsigned char idx, unsigned char r, unsigned char g, unsigned char b){
	
	if (PALETTE_VERBOSE){
		printf("%s.%d\t pal_Set() Set palette #%3d r:%3d g:%3d b:%3d (DAC mode %dbpp)\n", __FILE__, __LINE__, idx, r, g, b, vga_dac_type);
	}
	
	outp(VGA_PALETTE_MASK_ADDR, 0xFF);
	outp(VGA_PALETTE_SEL_ADDR, idx);
	if (vga_dac_type == VGA_PALETTE_8BPP){
		outp(VGA_PALETTE_SET_ADDR, r);
		outp(VGA_PALETTE_SET_ADDR, g);
		outp(VGA_PALETTE_SET_ADDR, b);
	} else {
		outp(VGA_PALETTE_SET_ADDR, r >> 2);
		outp(VGA_PALETTE_SET_ADDR, g >> 2);
		outp(VGA_PALETTE_SET_ADDR, b >> 2);
	}
	return;
}

void pal_SetUI(){
	// Set the 16 UI palette colour entries
	
	if (PALETTE_VERBOSE){
		printf("%s.%d\t pal_SetUI() Setting additional UI palette entries\n", __FILE__, __LINE__);
	}
	
	pal_Set(PALETTE_UI_BLACK, 0,   0,   0);
	pal_Set(PALETTE_UI_WHITE, 255, 255, 255);
	pal_Set(PALETTE_UI_LGREY, 180, 180, 180);
	pal_Set(PALETTE_UI_MGREY, 90,  90,  90);
	pal_Set(PALETTE_UI_DGREY, 30,  30,  30);
	pal_Set(PALETTE_UI_RED,   220, 0,   0);
	pal_Set(PALETTE_UI_GREEN, 0,   220, 0);
	pal_Set(PALETTE_UI_BLUE,  0,   0,   220);
	pal_Set(PALETTE_UI_YELLOW,180, 220, 20);
}

void pal_Get(){
	// Read and display current palette entries
	
	unsigned short idx;
	unsigned char r, g, b;
	
	for (idx = 0; idx < 256; idx++){
		outp(VGA_PALETTE_MASK_ADDR, 0xFF);
		outp(VGA_PALETTE_SEL_ADDR, idx);
		r = inp(VGA_PALETTE_SET_ADDR);
		g = inp(VGA_PALETTE_SET_ADDR);
		b = inp(VGA_PALETTE_SET_ADDR);
		printf("%s.%d\t pal_Get() Get palette #%3d : r:%3d g:%3d b:%3d\n", __FILE__, __LINE__, idx, r, g, b);
	}
}
