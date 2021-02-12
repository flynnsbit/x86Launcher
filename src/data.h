/* data.h, Prototypes and  game data structures for x68Launcher.
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

#define SAVEFILE				"launcher.txt"		// A text file holding the list of all found directories
#define INIFILE				"launcher.ini"		// the ini file holding settings for the main application
#define GAMEDAT				"launch.dat"			// the name of the data file in the game dir to load
#define RUNBAT				"run.bat"			// the name of the batch file which will contain the path to the chosen game exe
#define DEFAULT_GENRE		"Unknown Genre"		// Default genre
#define DEFAULT_YEAR 		0					// Default year
#define DEFAULT_START		"!start.bat"			// Default replacement for !start.bat is... erm... !start.bat
#define DEFAULT_PUBLISHER	""					// Default publisher
#define DEFAULT_DEVELOPER	""					// Default developer
#define MAX_IMAGES			16					// max number of images we try to load
#define IMAGE_BUFFER_SIZE	256					// Maximum size of bytes in game screenshot string (8 + 22 + overhead) 
#define MAX_DIRS				16					// Maximum number of game search paths - 16 sounds... okay?
#define MAX_FILENAME_SIZE    13					// 8 + 3 + 1 for the ., plus 1 for \0
#define MAX_STRING_SIZE		32
#define MAX_SEARCHDIRS_SIZE	1024
#define DATA_VERBOSE			0
#define MAX_PATH_SIZE		65

// A linked list of the games which we have scraped
typedef struct gamedata {
	int gameid;					// Unique ID for this game - assigned at scan time
	char drive;					// Drive letter
	char path[MAX_PATH_SIZE];	// Full drive and path name; e.g. A:\Games\FinalFight
	char name[MAX_STRING_SIZE];	// Just the directory name; e.g. FinalFight
	int has_dat;					// Flag to indicate __launch.dat was found in the game directory
	struct gamedata *next;		// Pointer to next gamedata entry
} gamedata_t;

// A launchdat object is loaded for the parsed launch.dat file included with a game.
// Only the currently selected game has this object loaded.
typedef struct launchdat {
	char realname[MAX_STRING_SIZE];		// A 'friendly' name to display the game as, instead of just the directory name
	char genre[MAX_STRING_SIZE];			// A string to represent the genre, in case we want to filter by genre
	int year;							// Year the game was released
	int midi;							// Supports MIDI out
	int midi_serial;						// Supports MIDI serial out
	char series[MAX_STRING_SIZE];		// Series name; e.g. Gradius, Streetfighter, etc.
	char publisher[MAX_STRING_SIZE];		// The name of the publisher
	char developer[MAX_STRING_SIZE];		// The name of the developer
	char start[MAX_FILENAME_SIZE];		// Name of the main start file
	char alt_start[MAX_FILENAME_SIZE];	// Name of an alternative start file (e.g a config utility)
	char images[IMAGE_BUFFER_SIZE];		// String containing all the image filenames
} launchdat_t;

// A sounddat object is loaded for the parsed launch.dat file included with a game.
// Only the currently selected game has this object loaded.
typedef struct sounddat {
	unsigned char beeper;				// Game supports PC speaker
	unsigned char tandy;					// Tandy 3-voice
	unsigned char adlib;					// Adlib FM or compatibles
	unsigned char soundblaster;			// Soundblaster, or equivalent FM + Digital effects
	unsigned char mt32;					// MT-32, LAPC or other LA-synthesis modules
	unsigned char gm;					// General MIDI
	unsigned char covox;					// Covox Speech Thing
	unsigned char disney;				// Disney Sound Source
} sounddat_t;

// A videodat object is loaded for the parsed launch.dat file included with a game.
// Only the currently selected game has this object loaded.
typedef struct videodat {
	unsigned char text;					// Game supports text mode
	unsigned char hercules;				// Hi-resolution Hercules text mode
	unsigned char cga;					// CGA 4 colour graphics
	unsigned char ega;					// EGA 16 colour graphics
	unsigned char vga;					// VGA 256 colour graphics at up to 320x240, or 16 colour at 640x480
	unsigned char svga;					// VGA 256 colour graphics at 640x400 and upwards
} videodat_t;

// A list of artwork, parsed from the launchdat file included with a game.
//typedef struct imagefile {
//	char filename[MAX_FILENAME_SIZE];	// Filename of an image
//	struct imagefile *prev;				// Pointer to the previous image file for this game
//	struct imagefile *next;				// Pointer to the next image file for this game
//} imagefile_t;

typedef struct imagefile {
	char filename[MAX_IMAGES][MAX_FILENAME_SIZE];	// Filename of an image
	short selected;
	short first;
	short last;
} imagefile_t;


// A list of game directories to search and scrape at run time.
typedef struct gamedir {
	char path[MAX_PATH_SIZE];			// Path to search for games
	struct gamedir *next;				// Link to the next search path
} gamedir_t;

// Holds the configuration of our application from launcher.ini
typedef struct config {
	short timers;						// Whether to time function calls
	short verbose;						// Verbose/debug flag
	short save;						// Save the list of all games to a text file
	short preload_names;				// Flag to indicate wheter a launch.dat is loaded at scrape-time to pick up real names
	short keyboard_test;
	char dirs[MAX_SEARCHDIRS_SIZE];	// String containing all game dirs to search - it will then be parsed into a list below:
	struct gamedir *dir;				// List of all the game search dirs
} config_t;

// Function prototypes
gamedata_t *		getLastGamedata(gamedata_t *gamedata);
imagefile_t *	getLastImage(imagefile_t *imagefile);
int				removeGamedata(gamedata_t *gamedata);
//int 				removeImagefile(imagefile_t *imagefile);
int 				sortGamedata(gamedata_t *gamedata, int verbose);
int 				swapGamedata(gamedata_t *gamedata1, gamedata_t *gamedata2);
int 				getLaunchdata(gamedata_t *gamedata, launchdat_t *launchdat);
int 				getImageList(launchdat_t *launchdat, imagefile_t *imagefile);
int 				getIni(config_t *config, int verbose);
int 				getDirList(config_t *config, gamedir_t *gamedir, int verbose);
gamedata_t * 	getGameid(int gameid, gamedata_t *gamedata);
