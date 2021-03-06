#ifndef DEFINE_H
#define DEFINE_H

#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
 
//VersionInfoStart
#define SAVE_VERSION 72
#define MINOR_VERSION 0
#define BETA
#define BUILD_NUM 138
//VersionInfoEnd

#define MOD_SAVE_VERSION 8 //This is not the version number of my mod anymore, it's only changed when I change the stamp/save saving code
#define BETA_VERSION 71
#define BETA_MINOR_VER 1
#define RELEASE_VERSION 71
#define RELEASE_MINOR_VER 3

#define IDENT_VERSION "G" //Change this if you're not Simon! It should be a single letter

#define MTOS_EXPAND(str) #str
#define MTOS(str) MTOS_EXPAND(str)

#define SERVER "powdertoy.co.uk"
#define SCRIPTSERVER "powdertoy.co.uk"
#define STATICSERVER "static.powdertoy.co.uk"

#define LOCAL_SAVE_DIR "Saves"

#define APPDATA_SUBDIR "\\HardWIRED"

#define THUMB_CACHE_SIZE 256

#ifndef M_PI
#define M_PI 3.14159265f
#endif
#ifndef M_GRAV
#define M_GRAV 6.67300e-1
#endif

#define IMGCONNS 3
#define TIMEOUT 100
#define HTTP_TIMEOUT 10

#ifdef RENDERER
#define MENUSIZE 0
#define BARSIZE 0
#else
#define MENUSIZE 40
#define BARSIZE 17
#endif
#define XRES	612
#define YRES	384
#define NPART XRES*YRES

#define XCNTR   306
#define YCNTR   192

#define MAX_DISTANCE sqrt(pow(XRES, 2.0f)+pow(YRES, 2.0f))

#define GRAV_DIFF

#define MAXSIGNS 16
#define TAG_MAX 256

#define ZSIZE_D	16
#define ZFACTOR_D	8
extern unsigned char ZFACTOR;
extern unsigned char ZSIZE;

#define CELL    4
#define ISTP    (CELL/2)
#define CFDS	(4.0f/CELL)

#define AIR_TSTEPP 0.3f
#define AIR_TSTEPV 0.4f
#define AIR_VADV 0.3f
#define AIR_VLOSS 0.999f
#define AIR_PLOSS 0.9999f

#define GRID_X 5
#define GRID_Y 4
#define GRID_P 3
#define GRID_S 6
#define GRID_Z 3

#define CATALOGUE_X 4
#define CATALOGUE_Y 3
#define CATALOGUE_S 6
#define CATALOGUE_Z 3

#define STAMP_MAX 240

//#define SAVE_OPS

#define NGOL 25
#define NGOLALT 24 //NGOL should be 24, but use this var until I find out why

#define CIRCLE_BRUSH 0
#define SQUARE_BRUSH 1
#define TRI_BRUSH 2
#define BRUSH_NUM 3


#define GRAVFFT
#define LUACONSOLE
//#define PYCONSOLE
//#define PYEXT
//no longer needed

#ifdef PIX16
typedef unsigned short pixel;
#else
typedef unsigned int pixel;
#endif

#define SURF_RANGE     10
#define NORMAL_MIN_EST 3
#define NORMAL_INTERP  20
#define NORMAL_FRAC    16

#define REFRACT        0x80000000

/* heavy flint glass, for awesome refraction/dispersion
   this way you can make roof prisms easily */
#define GLASS_IOR      1.9
#define GLASS_DISP     0.07

#ifdef WIN32
#define strcasecmp stricmp
#endif
#if defined(WIN32) && !defined(__GNUC__)
#define fmin min
#define fminf min
#define fmax max
#define fmaxf max
#endif

#if defined(WIN32) && !defined(__GNUC__)
#define TPT_INLINE _inline
#else
#define TPT_INLINE inline
#endif

#define SDEUT
//#define REALHEAT

#define DEBUG_PARTS		0x0001
#define DEBUG_PARTCOUNT	0x0002
#define DEBUG_DRAWTOOL	0x0004
#define DEBUG_PERFORMANCE_CALC 0x0008
#define DEBUG_PERFORMANCE_FRAME 0x0010

typedef unsigned char uint8;

extern int amd;

extern int FPSB;

int NUM_PARTS;
int GRAV;
int GRAV_R;
int GRAV_G;
int GRAV_B;
int GRAV_R2;
int GRAV_G2;
int GRAV_B2;

extern int legacy_enable;
extern int sound_enable;
extern int kiosk_enable;
extern int aheat_enable;
extern int decorations_enable;
extern int active_menu;
extern int hud_enable;
extern int pretty_powder;
extern int drawgrav_enable;
extern int ngrav_enable;
int limitFPS;
int water_equal_test;
extern int quickoptions_tooltip_fade;
extern int favMenu[19];
extern int alt_hud;
extern int finding;
extern int locked;
extern int highesttemp;
extern int lowesttemp;
extern int heatmode;
extern int maxframes;
extern int secret_els;
extern int save_as;
#define HUD_OPTIONS 43
extern int hud_current[HUD_OPTIONS];
extern int hud_modnormal[HUD_OPTIONS];
extern int hud_moddebug[HUD_OPTIONS];
extern int hud_normal[HUD_OPTIONS];
extern int hud_debug[HUD_OPTIONS];
extern void *prop_value;
extern int prop_format;
extern unsigned int prop_offset;

extern int drawinfo;
extern int currentTime;
extern int totaltime;
extern int totalafktime;
extern int afktime;
extern int frames;
extern double totalfps;
extern double maxfps;
extern int prevafktime;
extern int timesplayed;

extern int debug_flags;
#define DEBUG_PERF_FRAMECOUNT 256
extern int debug_perf_istart;
extern int debug_perf_iend;
extern long debug_perf_frametime[DEBUG_PERF_FRAMECOUNT];
extern long debug_perf_partitime[DEBUG_PERF_FRAMECOUNT];
extern long debug_perf_time;

extern int debug_flags;
#define DEBUG_PERF_FRAMECOUNT 256
extern int debug_perf_istart;
extern int debug_perf_iend;
extern long debug_perf_frametime[DEBUG_PERF_FRAMECOUNT];
extern long debug_perf_partitime[DEBUG_PERF_FRAMECOUNT];
extern long debug_perf_time;

extern int active_menu;

extern int sys_pause;
extern int framerender;

extern int mousex, mousey;

struct sign
{
	int x,y,ju;
	char text[256];
};
typedef struct sign sign;

struct stamp
{
	char name[11];
	pixel *thumb;
	int thumb_w, thumb_h, dodelete;
};
typedef struct stamp stamp;

int frameidx;
int MSIGN;
int CGOL;
int ISGOL;
int ISLOVE;
int ISLOLZ;
int ISGRAV;
int ISWIRE;
int GSPEED;
int love[XRES/9][YRES/9];
int lolz[XRES/9][YRES/9];
unsigned char gol[XRES][YRES];
unsigned char gol2[XRES][YRES][NGOL+1];
int SEC;
int SEC2;
int console_mode;
int REPLACE_MODE;
int CURRENT_BRUSH;
int GRID_MODE;
int VINE_MODE;
int DEBUG_MODE;
int GENERATION;
int ISSPAWN1;
int ISSPAWN2;
extern sign signs[MAXSIGNS];
extern stamp stamps[STAMP_MAX];
extern int stamp_count;
extern int itc;
extern char itc_msg[64];

extern int do_open;
extern int sys_pause;
extern int sys_shortcuts;
extern int legacy_enable; //Used to disable new features such as heat, will be set by commandline or save.
extern int framerender;
extern pixel *vid_buf;

extern unsigned char last_major, last_minor, update_flag, last_build;

extern char http_proxy_string[256];

//Functions in main.c
void thumb_cache_inval(char *id);
void thumb_cache_add(char *id, void *thumb, int size);
int thumb_cache_find(char *id, void **thumb, int *size);
void clear_sim(void);
void del_stamp(int d);
void sdl_seticon(void);
void play_sound(char *file);
int set_scale(int scale, int kiosk);
#endif
