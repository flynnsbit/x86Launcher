/* ui.c, Draws the user interface for the x68Launcher.
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
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "ui.h"

#ifndef __HAS_GFX
#include "gfx.h"
#define __HAS_GFX
#endif

#ifndef __HAS_BMP
#include "bmp.h"
#define __HAS_BMP
#endif

#ifndef __HAS_PAL
#include "palette.h"
#define __HAS_PAL
#endif

// bmpdata_t structures are needed permanently for all ui 
// bitmap elements, as we may need to repaint the screen at
// periodic intervals after having dialogue boxes or menus open.
bmpdata_t 	*ui_checkbox_bmp;
bmpdata_t 	*ui_checkbox_empty_bmp;
bmpdata_t 	*ui_main_bmp;
bmpdata_t 	*ui_list_bmp;
bmpdata_t 	*ui_art_bmp;
bmpdata_t 	*ui_title_bmp;
bmpdata_t 	*ui_year_bmp;
bmpdata_t 	*ui_genre_bmp;
bmpdata_t 	*ui_company_bmp;
bmpdata_t 	*ui_series_bmp;
bmpdata_t 	*ui_select_bmp;
bmpdata_t 	*ui_path_bmp;
bmpdata_t	*ui_font_bmp;		// Generic, just used during loading each font and then freed

// Fonts
fontdata_t      *ui_font;

// We should only need one file handle, as we'll load all of the ui
// bitmap assets sequentially.... just remember to close it at the 
// end of each function!
FILE 		*ui_asset_reader;

// Status of UI asset loading
static int      ui_fonts_status;
static int      ui_assets_status;


void ui_Init(){
	// Set the basic palette entries for all the user interface elements
	// NOT including any bitmaps we load - just the basic colours
	
	// Reset all palette entries
	pal_ResetAll();
	
	// Set basic UI palette entries
	pal_SetUI();	
}

void ui_Close(){
	if (ui_assets_status == UI_ASSETS_LOADED){
		bmp_Destroy(ui_checkbox_bmp);
		bmp_Destroy(ui_checkbox_empty_bmp);
		//bmp_Destroy(ui_main_bmp);
		bmp_Destroy(ui_list_bmp);
		bmp_Destroy(ui_art_bmp);
		bmp_Destroy(ui_title_bmp);
		bmp_Destroy(ui_year_bmp);
		bmp_Destroy(ui_genre_bmp);
		bmp_Destroy(ui_company_bmp);
		bmp_Destroy(ui_series_bmp);
		bmp_Destroy(ui_path_bmp);
		bmp_Destroy(ui_select_bmp);
	}
}

int ui_DisplayArtwork(FILE *screenshot_file, bmpdata_t *screenshot_bmp, state_t *state, imagefile_t *imagefile){

	int status;
	int has_screenshot;
	char msg[65];
	
	// Restart artwork display
	// =======================
	// Close previous screenshot file handle
	// =======================
	if (screenshot_file != NULL){
		fclose(screenshot_file);
		screenshot_file = NULL;
	}
	
	// Clear artwork window
	gfx_BoxFill(ui_artwork_xpos, ui_artwork_ypos, ui_artwork_xpos + 320, ui_artwork_ypos + 200, PALETTE_UI_BLACK);
	memset(state->selected_image, '\0', sizeof(state->selected_image)); 
	
	// Construct full path of image
	sprintf(msg, "%s\\%s", state->selected_game->path, imagefile->next->filename);
	strcpy(state->selected_image, msg);
	if (UI_VERBOSE){
		printf("%s.%d\t Selected artwork filename [%s]\n", __FILE__, __LINE__, imagefile->next->filename);
	}
	
	// =======================
	// Open new screenshot file, ready parse
	// =======================
	if (UI_VERBOSE){
		printf("%s.%d\t Opening artwork file\n", __FILE__, __LINE__);	
	}
	screenshot_file = fopen(state->selected_image, "rb");
	if (screenshot_file == NULL){
		if (UI_VERBOSE){
			printf("%s.%d\t Error, unable to open artwork file\n", __FILE__, __LINE__);	
		}
		has_screenshot = 0;
	} 
	else {
		// =======================
		// Load header of screenshot bmp
		// =======================
		if (UI_VERBOSE){
			printf("%s.%d\t Reading BMP data\n", __FILE__, __LINE__);	
		}
		status = bmp_ReadImage(screenshot_file, screenshot_bmp, 1, 1, 1);
		if (status != 0){
			if (UI_VERBOSE){
				printf("%s.%d\t Error, BMP read call returned error\n", __FILE__, __LINE__);	
			}
			has_screenshot = 0;
		} else {
			has_screenshot = 1;	
		}
		if (has_screenshot){
			// Reset free palette region
			if (UI_VERBOSE){
				printf("%s.%d\t Resetting free palette region entries\n", __FILE__, __LINE__);	
			}
			pal_ResetFree();
			// Set free palette region
			if (UI_VERBOSE){
				printf("%s.%d\t Setting new palette entries\n", __FILE__, __LINE__);	
			}
			pal_BMP2Palette(screenshot_bmp, 0);
			// Display bitmap
			if (UI_VERBOSE){
				printf("%s.%d\t Rendering BMP to buffer\n", __FILE__, __LINE__);	
			}
			gfx_Bitmap(ui_artwork_xpos + ((320 - screenshot_bmp->width) / 2) , ui_artwork_ypos + ((200 - screenshot_bmp->height) / 2), screenshot_bmp);
		}
	}
	if (UI_VERBOSE){
		printf("%s.%d\t Call to display %s complete\n", __FILE__, __LINE__, imagefile->next->filename);	
	}
	fclose(screenshot_file);
	return UI_OK;
}

int	ui_DrawConfirmPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat){
	// Draw a confirmation box to start the game
	
	// Draw drop-shadow
	gfx_BoxFillTranslucent(ui_launch_popup_xpos + 60, ui_launch_popup_ypos - 30, ui_launch_popup_xpos + 260, ui_launch_popup_ypos + 50, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(ui_launch_popup_xpos + 50, ui_launch_popup_ypos - 40, ui_launch_popup_xpos + 250, ui_launch_popup_ypos + 40, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(ui_launch_popup_xpos + 50, ui_launch_popup_ypos - 40, ui_launch_popup_xpos + 250, ui_launch_popup_ypos + 40, PALETTE_UI_LGREY);
	
	gfx_Puts(ui_launch_popup_xpos + 110, ui_launch_popup_ypos - 30, ui_font, "Start Game?");
	
	gfx_Puts(ui_launch_popup_xpos + 60, ui_launch_popup_ypos - 5, ui_font, "Confirm (Enter)");
	gfx_Puts(ui_launch_popup_xpos + 60, ui_launch_popup_ypos + 15, ui_font, "Cancel (Esc)");
	
	
	return UI_OK;
}

int ui_DrawFilterPrePopup(state_t *state, int toggle){
	// Draw a popup that allows the user to toggle filter mode between genre, series and off

	// Draw drop-shadow
	gfx_BoxFillTranslucent(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 10, ui_launch_popup_xpos + 10 + ui_launch_popup_width, ui_launch_popup_ypos + 10 + ui_launch_popup_height + 30, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height + 30, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height + 30, PALETTE_UI_LGREY);
	
	// Box title
	gfx_Puts(ui_launch_popup_xpos + 90, ui_launch_popup_ypos + 10, ui_font, "Enable Filter?");
	// No filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 35, ui_font, "No filter - Show all games");
	// Genre filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 65, ui_font, "By Genre");
	// Series filter text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 95, ui_font, "By Series");
	
	// Toggle which entry is selected
	if (toggle == 1){
		state->selected_filter++;	
	} 
	if (toggle == -1){
		state->selected_filter--;	
	}
	
	// Detect wraparound
	if (state->selected_filter >= FILTER_SERIES){
		state->selected_filter = FILTER_SERIES;
	}
	if (state->selected_filter < 1){
		state->selected_filter = FILTER_NONE;
	}
	
	if (state->selected_filter == FILTER_NONE){
		// none
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_GENRE){
		// genre
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_empty_bmp);
	}
	if (state->selected_filter == FILTER_SERIES){
		// series
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 95, ui_checkbox_bmp);
	}
	
	return UI_OK;
}

int ui_DrawFilterPopup(state_t *state, int toggle){
	// Draw a page of filter choices for the user to select
	
	int i;
	int status;
	
	// Draw drop-shadow
	gfx_BoxFillTranslucent(40, 50, GFX_COLS - 30, GFX_ROWS - 20, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(30, 40, GFX_COLS - 40, GFX_ROWS - 40, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(30, 40, GFX_COLS - 40, GFX_ROWS - 40, PALETTE_UI_LGREY);
	
	// Box title
	if (state->selected_filter == FILTER_GENRE){
		gfx_Puts(240, 45, ui_font, "Select Genre");
	}
	if (state->selected_filter == FILTER_SERIES){
		gfx_Puts(240, 45, ui_font, "Select Series");
	}
	
	if (toggle == -1){
		state->selected_filter_string--;
	}
	if (toggle == 1){
		state->selected_filter_string++;
	}
	
	if (state->selected_filter_string < 1){
		state->selected_filter_string = FILTER_NONE;
	}
	if (state->selected_filter_string > state->available_filter_strings){
		state->selected_filter_string = state->available_filter_strings;
	}
	
	for(i=0; i<MAXIMUM_FILTER_STRINGS; i++){
		
		if ((state->filter_strings[i] != NULL) && (strcmp(state->filter_strings[i], "") != 0)){
		
			// Column 1
			if (i < 11){
				if (i == state->selected_filter_string){
					gfx_Bitmap(45, 70 + (i * 25), ui_checkbox_bmp);
				} else {
					gfx_Bitmap(45, 70 + (i * 25), ui_checkbox_empty_bmp);
				}
				gfx_Puts(70, 70 + (i * 25), ui_font, state->filter_strings[i]);
			}
			
			// Column 2
			if ((i >= 11) && (i < 22)){
				if (i == state->selected_filter_string){
					gfx_Bitmap(230, 70 + ((i - 11) * 25), ui_checkbox_bmp);
				} else {
					gfx_Bitmap(230, 70 + ((i - 11) * 25), ui_checkbox_empty_bmp);
				}
				gfx_Puts(255, 70 + ((i - 11) * 25), ui_font, state->filter_strings[i]);
			}
			
			// Column 3
			if (i >= 22){
				if (i == state->selected_filter_string){
					gfx_Bitmap(420, 70 + ((i - 22) * 25), ui_checkbox_bmp);
				} else {
					gfx_Bitmap(420, 70 + ((i - 22) * 25), ui_checkbox_empty_bmp);
				}
				gfx_Puts(445, 70 + ((i - 22) * 25), ui_font, state->filter_strings[i]);
			}	
		}
	}
	
	return UI_OK;
	
}

int ui_DrawInfoBox(){
	// Draw a fresh info panel
	
	// Text boxes.....
	// Title
	// Year
	// Genre
	// Company
	// Series
	
	// Buttons.....
	// Metadata
	// Artwork
	// Start file
	// MIDI
	// MIDI RS
	// Filter
	
	return UI_OK;
}

int	ui_DrawLaunchPopup(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat, int toggle){
	// Draw the popup window that lets us select from the main or alternate start file
	// in order to launch a game
	
	int status;	
	
	// Draw drop-shadow
	gfx_BoxFillTranslucent(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 10, ui_launch_popup_xpos + 10 + ui_launch_popup_width, ui_launch_popup_ypos + 10 + ui_launch_popup_height, PALETTE_UI_DGREY);
	
	// Draw main box
	gfx_BoxFill(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height, PALETTE_UI_BLACK);
	
	// Draw main box outline
	gfx_Box(ui_launch_popup_xpos, ui_launch_popup_ypos, ui_launch_popup_xpos + ui_launch_popup_width, ui_launch_popup_ypos + ui_launch_popup_height, PALETTE_UI_LGREY);
	
	gfx_Puts(ui_launch_popup_xpos + 50, ui_launch_popup_ypos + 10, ui_font, "Select which file to run:");
	
	// Start file text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 35, ui_font, launchdat->start);
	
	// Alt start file text
	gfx_Puts(ui_launch_popup_xpos + 35, ui_launch_popup_ypos + 65, ui_font, launchdat->alt_start);
	
	if (toggle == 1){
		state->selected_start = !state->selected_start;	
	}
	
	if (state->selected_start == 0){
		// Checkbox for start
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_empty_bmp);		
	} else {
		// Checkbox for alt_start
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 35, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_launch_popup_xpos + 10, ui_launch_popup_ypos + 65, ui_checkbox_bmp);
	}
	
	return UI_OK;
}


int	ui_DrawMainWindow(){
	// Draw the background of the main user interface window
	
	int status;
	
	// Initially, we can use a single solid bmp, with all of the ui elements on, but later as we overlay other things on top of it,
	// we'll need to refresh various individual elements
	
	status = gfx_Bitmap(0, 0, ui_main_bmp);
	if (status == 0){
		return UI_OK;
	} else {
		return UI_ERR_FUNCTION_CALL;
	}
}

int ui_DrawSplash(){
	/*
		Show the initial splash screen which is shown during the loading
		process whilst bmp assets are loaded into ram, the disk is scanned
		for games and other stuff before the user can access the main menu.
	*/
	int			status;
	bmpdata_t 	*logo_bmp;
	
	// Load splash logo
	ui_asset_reader = fopen(splash_logo, "rb");
	if (ui_asset_reader == NULL){
		printf("Unable to open file\n");
		return UI_ERR_FILE;	
	}
	logo_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (logo_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for splash bitmap.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	logo_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, logo_bmp, 1, 1, 1);
	if (status != 0){
		printf("Unable to read BMP\n");
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Set the palette entries for the splash logo
	pal_ResetFree();
	pal_BMP2Palette(logo_bmp, 0);
	
	// Copy the bitmap to the vram buffer
	gfx_Bitmap((GFX_COLS / 2) - (logo_bmp->width / 2), (GFX_ROWS / 2) - (logo_bmp->height / 2), logo_bmp);
	
	// Destroy in-memory bitmap
	bmp_Destroy(logo_bmp);
	
	// Splash loaded okay
	return UI_OK;
	
}

int ui_DrawSplashProgress(int redraw, int progress_width){
	// Draw a progress bar
	int status;
	
	if (redraw){
		status = gfx_Box(
			splash_progress_x_pos, 
			splash_progress_y_pos, 
			splash_progress_x_pos + splash_progress_width, 
			splash_progress_y_pos + splash_progress_height, 
			PALETTE_UI_LGREY
			);      
		if (status != 0){
			return UI_ERR_FUNCTION_CALL;
		}
	}
	
	// Update progress bar to 'percent'
	status = gfx_BoxFill(
		splash_progress_x_pos + 2, 
		(splash_progress_y_pos + 2),
		(splash_progress_x_pos + 2 + abs(progress_width)),
		(splash_progress_y_pos + (splash_progress_height - 2)),
		PALETTE_UI_RED
	);
	if (status != 0){
		return UI_ERR_FUNCTION_CALL;
	}
	
	// Progress bar drawn okay
	return UI_OK;
}

int ui_LoadAssets(){
	// Load all UI bitmap assets from disk
	
	int status;
	
	// Default to assets not loaded
	ui_assets_status = UI_ASSETS_MISSING;
	
	// list select bmp
	ui_ProgressMessage("Loading browser select icon...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_select);
	}
	ui_asset_reader = fopen(ui_select, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_select_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_select_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for browser select icon.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	
	ui_select_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_select_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_select_bmp, 1);
	//pal_BMPRemap(ui_checkbox_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Checkbox
	ui_ProgressMessage("Loading checkbox...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_check_box);
	}
	ui_asset_reader = fopen(ui_check_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_checkbox_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_checkbox_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for checkbox icon.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_checkbox_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_checkbox_bmp, 1);
	//pal_BMPRemap(ui_checkbox_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Checkbox - empty
	ui_ProgressMessage("Loading checkbox (empty) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_check_box_unchecked);
	}
	ui_asset_reader = fopen(ui_check_box_unchecked, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_checkbox_empty_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_checkbox_empty_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for checkbox(empty) icon.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_checkbox_empty_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_checkbox_empty_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_checkbox_empty_bmp, 1);
	//pal_BMPRemap(ui_checkbox_empty_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Main background
	ui_ProgressMessage("Loading main UI background ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_main);
	}
	ui_asset_reader = fopen(ui_main, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_main_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_main_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for main UI background.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_main_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_main_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_main_bmp, 1);
	//pal_BMPRemap(ui_main_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// List window
	ui_ProgressMessage("Loading browser background ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_list_box);
	}
	ui_asset_reader = fopen(ui_list_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_list_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_list_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for browser list background.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_list_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_list_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_list_bmp, 1);
	//pal_BMPRemap(ui_list_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Art window
	ui_ProgressMessage("Loading artwork background ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_art_box);
	}
	ui_asset_reader = fopen(ui_art_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_art_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_art_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for artwork background.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_art_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_art_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_art_bmp, 1);
	//pal_BMPRemap(ui_art_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Game title bar
	ui_ProgressMessage("Loading text entry (title) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_title_box);
	}
	ui_asset_reader = fopen(ui_title_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_title_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_title_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for title text.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_title_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_title_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_title_bmp, 1);
	//pal_BMPRemap(ui_title_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Game year bar
	ui_ProgressMessage("Loading text entry (year) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_year_box);
	}
	ui_asset_reader = fopen(ui_year_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_year_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_year_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for year text.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_year_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_year_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_year_bmp, 1);
	//pal_BMPRemap(ui_year_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Game genre bar
	ui_ProgressMessage("Loading text entry (genre) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_genre_box);
	}
	ui_asset_reader = fopen(ui_genre_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_genre_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_genre_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for genre text.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_genre_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_genre_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_genre_bmp, 1);
	//pal_BMPRemap(ui_genre_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Game company bar
	ui_ProgressMessage("Loading text entry (company) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_company_box);
	}
	ui_asset_reader = fopen(ui_company_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_company_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_company_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for company text.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_company_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_company_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_company_bmp, 1);
	//pal_BMPRemap(ui_company_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Game series bar
	ui_ProgressMessage("Loading text entry (series) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_series_box);
	}
	ui_asset_reader = fopen(ui_series_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_series_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_series_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for series text.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_series_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_series_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_series_bmp, 1);
	//pal_BMPRemap(ui_series_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	
	// Path bar
	ui_ProgressMessage("Loading text entry (path) ...");
	gfx_Flip();
	if (BMP_VERBOSE){
		printf("%s.%d\t Loading %s\n", __FILE__, __LINE__, ui_path_box);
	}
	ui_asset_reader = fopen(ui_path_box, "rb");
	if (ui_asset_reader == NULL){
			return UI_ERR_FILE;     
	}
	ui_path_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	if (ui_path_bmp == NULL){
		printf("%s.%d\t Unable to allocate memory for path text.\n", __FILE__, __LINE__);
		return UI_ERR_BMP;
	}
	ui_path_bmp->pixels = NULL;
	status = bmp_ReadImage(ui_asset_reader, ui_path_bmp, 1, 1, 1);
	pal_BMP2Palette(ui_path_bmp, 1);
	//pal_BMPRemap(ui_path_bmp);
	if (status != 0){
			fclose(ui_asset_reader);
			return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	
	// Set assets loaded status
	ui_assets_status = UI_ASSETS_LOADED;
	return UI_OK;
}

int ui_LoadFonts(){
	int status;
        
	// Default to assets not loaded
	ui_fonts_status = UI_ASSETS_MISSING;
	
	// =========================
	// main font
	// =========================
	ui_asset_reader = fopen(ui_font_name, "rb");
	if (ui_asset_reader == NULL){
		if (UI_VERBOSE){
				printf("%s.%d\t Error loading UI font data\n", __FILE__, __LINE__);
		}
		return UI_ERR_FILE;     
	}
	
	ui_font = (fontdata_t *) malloc(sizeof(fontdata_t));
	ui_font_bmp = (bmpdata_t *) malloc(sizeof(bmpdata_t));
	ui_font_bmp->pixels = NULL;
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_font, 1, 0, 0, ui_font_width, ui_font_height);
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_font, 0, 1, 0, ui_font_width, ui_font_height);
	pal_BMP2Palette(ui_font_bmp, 1);
	status = bmp_ReadFont(ui_asset_reader, ui_font_bmp, ui_font, 0, 0, 1, ui_font_width, ui_font_height);
	
	
	if (status != 0){
		if (UI_VERBOSE){
				printf("%s.%d\t Error processing UI font data\n", __FILE__, __LINE__);
		}
		fclose(ui_asset_reader);
		return UI_ERR_BMP;
	}
	fclose(ui_asset_reader);
	ui_font->ascii_start = ui_font_ascii_start;           
	ui_font->n_symbols = ui_font_total_syms;
	ui_font->unknown_symbol = ui_font_unknown;
	
	// Destroy the temporary bmp structure, as the font is now in a font structure
	bmp_Destroy(ui_font_bmp);
	
	return UI_OK;
}

int ui_ProgressMessage(char *c){
	int x;
	
	x = (GFX_COLS / 2) - ((ui_font->width * strlen(c)) / 2);

	// Mask out anything that was on this line before
	gfx_BoxFill(0, ui_progress_font_y_pos - 1, GFX_COLS, ui_progress_font_y_pos + ui_font->height, PALETTE_UI_BLACK);
	
	// Print the text
	return gfx_Puts(x, ui_progress_font_y_pos, ui_font, c);
}

int	ui_ReselectCurrentGame(state_t *state){
	// Simply updates the selected_gameid with whatever line / page is currently selected
	// Should be called every time up/down/pageup/pagedown is detected whilst in browser pane
	
	int	startpos;	// Index of first displayable element of state->selected_items
	int	endpos;		// Index to last displayable element of state->selected_items
	int 	i;			// Loop counter
	int	selected;	// Counter to match the selected_line number
	int	gameid;		// ID of the current game we are iterating through in the selected_list
	
	// Don't allow startpos to go negative
	startpos = (state->selected_page - 1) * ui_browser_max_lines;
	if (startpos < 0){
		startpos = 0;	
	}
	
	// If we're on the last page, then make sure we only loop over the number of entries
	// that are on this page... not just all 22
	if ((startpos + ui_browser_max_lines) > state->selected_max){
		endpos = state->selected_max;
	} else {
		endpos = startpos + ui_browser_max_lines;
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t Reselecting game: %d, endpos: %d, selected line: %d\n", __FILE__, __LINE__, startpos, endpos, state->selected_line);
	}
		
	selected = 0;
	for(i = startpos; i <= endpos ; i++){
		gameid = state->selected_list[i];
		
		// This is the current selected game
		if (selected == state->selected_line){
			//if (UI_VERBOSE){
			//	printf("%s.%d\t Page [%d/%d], Line [%d/%d], Game ID [%d]\n", __FILE__, __LINE__, state->selected_page, state->total_pages, state->selected_line, endpos, gameid);	
			//}
			state->selected_gameid = gameid;
			return UI_OK;
		}	
		selected++;
	}
	
	return UI_OK;
}

int ui_StatusMessage(char *c){
	// Output a status message in the status bar at the bottom of the screen in the main UI

	// Clear the progress message row
	//gfx_BoxFill(0, ui_progress_font_y_pos - 1, GFX_COLS, ui_progress_font_y_pos + ui_font->height, PALETTE_UI_BLACK);
	
	return gfx_Puts(0, 380, ui_font, c);
}

int ui_UpdateBrowserPane(state_t *state, gamedata_t *gamedata){
	// UPdate the contents of the game browser pane

	// selected_list : contains the gameids that are in the currently filtered selection (e.g. ALL, shooter genre only, only by Konami, etc)
	// selected_max : is the count of how many games are in the current selection
	// selected_page : is the page (browser list can show 0 - x items per page) into the selected_list
	// selected_line : is the line of the selected_page that is highlighted
	
	gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	int			y;				// Vertical position offset for each row
	int 			i;				// Loop counter
	int 			gameid;			// ID of each game in selected_list
	char		msg[64];		// Message buffer for each row
	int			startpos;			// Index of first displayable element of state->selected_items
	int			endpos;			// Index to last displayable element of state->selected_items
	
	// Don't allow startpos to go negative
	startpos = (state->selected_page - 1) * ui_browser_max_lines;
	if (startpos < 0){
		startpos = 0;	
	}
	
	// If we're on the last page, then make sure we only draw the number of lines
	// that are on this page... not just all 22
	if ((startpos + ui_browser_max_lines) > state->selected_max){
		endpos = state->selected_max;
	} else {
		endpos = startpos + ui_browser_max_lines;
	}
	
	// Clear all lines
	gfx_Bitmap(ui_browser_panel_x_pos, ui_browser_panel_y_pos, ui_list_bmp);
	
	// Display the entries for this page
	gamedata_head = gamedata;
	y = ui_browser_font_y_pos;
	if (UI_VERBOSE){
		printf("%s.%d\t Building browser menu [%d-%d]\n", __FILE__, __LINE__, startpos, endpos);
	}
	for(i = startpos; i < endpos ; i++){
		gamedata = gamedata_head;
		gameid = state->selected_list[i];
		selected_game = getGameid(gameid, gamedata);
		if (UI_VERBOSE){
			printf("%s.%d\t - Line %d: Game ID %d, %s\n", __FILE__, __LINE__, i, gameid, selected_game->name);
		}
		sprintf(msg, "%s", selected_game->name);
		gfx_Puts(ui_browser_font_x_pos, y, ui_font, msg);
		y += ui_font->height + 2;
	}
	gamedata = gamedata_head;
	
	return UI_OK;
}

int ui_UpdateBrowserPaneStatus(state_t *state){
	// Draw browser pane status message in status panel
	char	msg[64];		// Message buffer for the status bar
	int y_pos;
	// Blank out any previous selection cursor
	gfx_BoxFill(ui_browser_cursor_xpos, ui_browser_font_y_pos, ui_browser_cursor_xpos + ui_select_bmp->width, ui_browser_footer_font_ypos, PALETTE_UI_BLACK);

	// Insert selection cursor
	if (state->selected_line == 0){
		y_pos = 0;
	} else {
		y_pos = (ui_font->height + 2 ) * (state->selected_line);
	}
	if (UI_VERBOSE){
		printf("%s.%d\t Drawing selection icon at line %d, x:%d y:%d\n", __FILE__, __LINE__, state->selected_line, ui_browser_cursor_xpos, (ui_browser_font_y_pos + y_pos));
	}
	gfx_Bitmap(ui_browser_cursor_xpos, ui_browser_font_y_pos + y_pos, ui_select_bmp);
	
	// Text at bottom of browser pane
	sprintf(msg, "Line %02d/%02d             Page %02d/%02d", state->selected_line, ui_browser_max_lines, state->selected_page, state->total_pages);
	gfx_Puts(ui_browser_footer_font_xpos, ui_browser_footer_font_ypos, ui_font, msg);
	
	return UI_OK;
}

int ui_UpdateInfoPane(state_t *state, gamedata_t *gamedata, launchdat_t *launchdat){
	// Draw the contents of the info panel with current selected game, current filter mode, etc
	
	// TO DO
	// Clear text on load
	// snprintf instead of sprintf to limit string sizes
	
	int			status;
	char		status_msg[64];		// Message buffer for anything needing to be printed onscreen
	char		info_name[64];
	char		info_year[8];
	char		info_company[32];
	char		info_path[64];
	char		info_genre[16];
	char		info_series[32];
	
	gamedata_t	*gamedata_head;	// Pointer to the start of the gamedata list, so we can restore it
	gamedata_t	*selected_game;	// Gamedata object for the currently selected line
	//launchdat_t	*launchdat;		// Metadata object representing the launch.dat file for this game
	
	// Store gamedata head
	gamedata_head = gamedata;
	
	// Clear all existing text
	// title
	gfx_Bitmap(ui_info_name_xpos, ui_info_name_ypos, ui_title_bmp);
	// company
	gfx_Bitmap(ui_info_company_xpos, ui_info_company_ypos, ui_company_bmp);
	// path
	gfx_Bitmap(ui_info_path_xpos, ui_info_path_ypos, ui_path_bmp);
	// year
	gfx_Bitmap(ui_info_year_xpos, ui_info_year_ypos, ui_year_bmp);
	// genre
	gfx_Bitmap(ui_info_genre_xpos, ui_info_genre_ypos, ui_genre_bmp);
	// series
	gfx_Bitmap(ui_info_series_xpos, ui_info_series_ypos, ui_series_bmp);
	
	if (state->selected_filter != 0){
		gfx_Bitmap(ui_checkbox_filter_active_xpos, ui_checkbox_filter_active_ypos, ui_checkbox_bmp);
	} else {
		gfx_Bitmap(ui_checkbox_filter_active_xpos, ui_checkbox_filter_active_ypos, ui_checkbox_empty_bmp);	
	}
	
	if (UI_VERBOSE){
		printf("%s.%d\t Selected State\n", __FILE__, __LINE__);
		printf("%s.%d\t - Page: [%d]\n", __FILE__, __LINE__, state->selected_page);
		printf("%s.%d\t - Line: [%d]\n", __FILE__, __LINE__, state->selected_line);
		printf("%s.%d\t - selected game id: [%d]\n", __FILE__, __LINE__,  state->selected_gameid);
		printf("%s.%d\t - retrieved game id: [%d]\n", __FILE__, __LINE__, state->selected_game->gameid);
		printf("%s.%d\t - has_dat: [%d]\n", __FILE__, __LINE__, state->selected_game->has_dat);
		printf("%s.%d\t - has_images: [%d]\n", __FILE__, __LINE__, state->has_images);
	}
	
	// See if it has a launch.dat metadata file
	if (state->selected_game != NULL){
		if (state->selected_game->has_dat != 0){
			if (launchdat == NULL){
				// ======================
				// Unable to load launch.dat	 from disk
				// ======================
				sprintf(status_msg, "ERROR: Unable to load metadata file: %s\%s", state->selected_game->path, GAMEDAT);
				gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_bmp);
				gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
				gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
				gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
				gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
				sprintf(info_name, " %s", state->selected_game->name);
				sprintf(info_year, "N/A");
				sprintf(info_company, " N/A");
				sprintf(info_genre, "N/A");
				sprintf(info_series, "");
				sprintf(info_path, " %s", state->selected_game->path);
			} else {
				// ======================
				// Loaded launch.dat from disk
				// ======================
				
				if (UI_VERBOSE){
					printf("%s.%d\t Info - metadata: yes\n", __FILE__, __LINE__);
					printf("%s.%d\t Info - artwork: [%d]\n", __FILE__, __LINE__, state->has_images);
					printf("%s.%d\t Info - start file: [%s]\n", __FILE__, __LINE__, launchdat->start);
					printf("%s.%d\t Info - alt_start file: [%s]\n", __FILE__, __LINE__, launchdat->alt_start);
					printf("%s.%d\t Info - midi: [%d]\n", __FILE__, __LINE__, launchdat->midi);
					printf("%s.%d\t Info - midi serial: [%s]\n", __FILE__, __LINE__, launchdat->midi_serial);
				}
				
				gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_bmp);
				
				if (state->has_images == 1){
					gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);	
				}
				
				if (launchdat->midi == 1){
					gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
				}
				
				if (launchdat->midi_serial == 1){
					gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
				}
				
				if ((launchdat->start != NULL) && (strcmp(launchdat->start, "") != 0)){
					gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_bmp);
				} else {
					gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
				}
				
				if (launchdat->realname != NULL){
					sprintf(info_name, " %s", launchdat->realname);
				} else {
					sprintf(info_name, " %s", state->selected_game->name);
				}
				if (UI_VERBOSE){
					printf("%s.%d\t Info - name: %s\n", __FILE__, __LINE__, info_name);
				}
				
				if (launchdat->genre != NULL){
					sprintf(info_genre, "%s", launchdat->genre);
				} else {
					sprintf(info_genre, "N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t Info - genre: %s\n", __FILE__, __LINE__, info_genre);
				}
				
				if (launchdat->series != NULL){
					sprintf(info_series, "%s", launchdat->series);
				} else {
					sprintf(info_series, "N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t Info - series: %s\n", __FILE__, __LINE__, info_series);
				}
				
				if (launchdat->year != 0){
					sprintf(info_year, "%d", launchdat->year);
				} else {
					sprintf(info_year, "N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t Info - year: %s\n", __FILE__, __LINE__, info_year);
				}
				
				if ((strlen(launchdat->developer) > 0) && (strlen(launchdat->publisher) > 0)){
					sprintf(info_company, " %s / %s", launchdat->developer, launchdat->publisher);
				} else if (strlen(launchdat->developer) > 0){
					sprintf(info_company, " %s", launchdat->developer);
				} else if (strlen(launchdat->publisher) > 0){
					sprintf(info_company, " %s", launchdat->publisher);
				} else {
					sprintf(info_company, " N/A");
				}
				if (UI_VERBOSE){
					printf("%s.%d\t Info - company: %s\n", __FILE__, __LINE__, info_company);
				}
				
				// Start file
				
				// Number of images/screenshots
	
				sprintf(info_path, " %s", state->selected_game->path);
			}
		} else {
			// ======================
			// We can only use the basic directory information
			// ======================
			gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
			gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
				
			sprintf(info_name, " %s", state->selected_game->name);
			sprintf(info_year, "N/A");
			sprintf(info_company, " N/A");
			sprintf(info_genre, "N/A");
			sprintf(info_series, "N/A");
			sprintf(info_path, " %s", state->selected_game->path);		
		}	
	} else {
		// ======================
		// Error in logic, gameid is not set
		// ======================
		gfx_Bitmap(ui_checkbox_has_metadata_xpos, ui_checkbox_has_metadata_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_startbat_xpos, ui_checkbox_has_startbat_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_images_xpos, ui_checkbox_has_images_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_midi_xpos, ui_checkbox_has_midi_ypos, ui_checkbox_empty_bmp);
		gfx_Bitmap(ui_checkbox_has_midi_serial_xpos, ui_checkbox_has_midi_serial_ypos, ui_checkbox_empty_bmp);
		sprintf(status_msg, "ERROR, unable to find gamedata object for ID %d", state->selected_gameid);
		gfx_Puts(ui_info_name_text_xpos, ui_info_name_text_ypos, ui_font, status_msg);
		gamedata = gamedata_head;
		return UI_OK;
	}
	
	// ===========================
	// Now print out all data, regardless of source
	// ===========================
	gfx_Puts(ui_info_name_text_xpos, ui_info_name_text_ypos, ui_font, info_name);
	gfx_Puts(ui_info_year_text_xpos, ui_info_year_text_ypos, ui_font, info_year);
	gfx_Puts(ui_info_company_text_xpos, ui_info_company_text_ypos, ui_font, info_company);
	gfx_Puts(ui_info_genre_text_xpos, ui_info_genre_text_ypos, ui_font, info_genre);
	gfx_Puts(ui_info_series_text_xpos, ui_info_series_text_ypos, ui_font, info_series);
	gfx_Puts(ui_info_path_text_xpos, ui_info_path_text_ypos, ui_font, info_path);
	gamedata = gamedata_head;
	return UI_OK;
}
