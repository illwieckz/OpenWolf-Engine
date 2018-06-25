////////////////////////////////////////////////////////////////////////////////////////
// Copyright(C) 1999 - 2005 Id Software, Inc.
// Copyright(C) 2000 - 2006 Tim Angus
// Copyright(C) 2011 - 2018 Dusan Jocic <dusanjocic@msn.com>
//
// This file is part of OpenWolf.
//
// OpenWolf is free software; you can redistribute it
// and / or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// OpenWolf is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenWolf; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 - 1301  USA
//
// -------------------------------------------------------------------------------------
// File name:   gui_shared.h
// Version:     v1.00
// Created:
// Compilers:   Visual Studio 2015
// Description:
// -------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GUI_SHARED_H__
#define __GUI_SHARED_H__

#ifndef __Q_SHARED_H__
#include <qcommon/q_shared.h>
#endif
#ifndef __R_TYPES_H__
#include <GPURenderer/r_types.h>
#endif
#ifndef __KEYCODES_H__
#include <GUI/keycodes.h>
#endif
#ifndef __MENUDEF_H__
#include <GUI/menudef.h>
#endif

#define MAX_MENUNAME 32
#define MAX_ITEMTEXT 64
#define MAX_ITEMACTION 64
#define MAX_MENUDEFFILE 4096
#define MAX_MENUFILE 32768
#define MAX_MENUS 256
#define MAX_MENUITEMS 512
#define MAX_COLOR_RANGES 10
#define MAX_OPEN_MENUS 16

#define WINDOW_MOUSEOVER       0x00000001  // mouse is over it, non exclusive
#define WINDOW_HASFOCUS        0x00000002  // has cursor focus, exclusive
#define WINDOW_VISIBLE         0x00000004  // is visible
#define WINDOW_GREY            0x00000008  // is visible but grey ( non-active )
#define WINDOW_DECORATION      0x00000010  // for decoration only, no mouse, keyboard, etc..
#define WINDOW_FADINGOUT       0x00000020  // fading out, non-active
#define WINDOW_FADINGIN        0x00000040  // fading in
#define WINDOW_MOUSEOVERTEXT   0x00000080  // mouse is over it, non exclusive
#define WINDOW_INTRANSITION    0x00000100  // window is in transition
#define WINDOW_FORECOLORSET    0x00000200  // forecolor was explicitly set ( used to color alpha images or not )
#define WINDOW_HORIZONTAL      0x00000400  // for list boxes and sliders, vertical is default this is set of horizontal
#define WINDOW_LB_LEFTARROW    0x00000800  // mouse is over left/up arrow
#define WINDOW_LB_RIGHTARROW   0x00001000  // mouse is over right/down arrow
#define WINDOW_LB_THUMB        0x00002000  // mouse is over thumb
#define WINDOW_LB_PGUP         0x00004000  // mouse is over page up
#define WINDOW_LB_PGDN         0x00008000  // mouse is over page down
#define WINDOW_ORBITING        0x00010000  // item is in orbit
#define WINDOW_OOB_CLICK       0x00020000  // close on out of bounds click
#define WINDOW_WRAPPED         0x00080000  // wrap text
#define WINDOW_FORCED          0x00100000  // forced open
#define WINDOW_POPUP           0x00200000  // popup
#define WINDOW_BACKCOLORSET    0x00400000  // backcolor was explicitly set
#define WINDOW_TIMEDVISIBLE    0x00800000  // visibility timing ( NOT implemented )
#define WINDOW_DONTCLOSEALL    0x01000000  // don't close during Menus_CloseAll()
#define WINDOW_DRAG            0x08000000  // KTW: Added for draggable windows


// CGAME cursor type bits
#define CURSOR_NONE          0x00000001
#define CURSOR_ARROW        0x00000002
#define CURSOR_SIZER        0x00000004

#ifdef CGAMEDLL
#define STRING_POOL_SIZE 128*1024
#else
#define STRING_POOL_SIZE 384*1024
#endif
#define MAX_STRING_HANDLES 4096

#define MAX_SCRIPT_ARGS 12
#define MAX_EDITFIELD 256
#define ITEM_VALUE_OFFSET 8

#define ART_FX_BASE      "menu/art/fx_base"
#define ART_FX_BLUE      "menu/art/fx_blue"
#define ART_FX_CYAN      "menu/art/fx_cyan"
#define ART_FX_GREEN     "menu/art/fx_grn"
#define ART_FX_RED       "menu/art/fx_red"
#define ART_FX_TEAL      "menu/art/fx_teal"
#define ART_FX_WHITE     "menu/art/fx_white"
#define ART_FX_YELLOW    "menu/art/fx_yel"

#define ASSET_GRADIENTBAR           "ui/assets/gradientbar2.tga"
#define ASSET_SCROLLBAR             "ui/assets/scrollbar.tga"
#define ASSET_SCROLLBAR_ARROWDOWN   "ui/assets/scrollbar_arrow_dwn_a.tga"
#define ASSET_SCROLLBAR_ARROWUP     "ui/assets/scrollbar_arrow_up_a.tga"
#define ASSET_SCROLLBAR_ARROWLEFT   "ui/assets/scrollbar_arrow_left.tga"
#define ASSET_SCROLLBAR_ARROWRIGHT  "ui/assets/scrollbar_arrow_right.tga"
#define ASSET_SCROLL_THUMB          "ui/assets/scrollbar_thumb.tga"
#define ASSET_SLIDER_BAR            "ui/assets/slider2.tga"
#define ASSET_SLIDER_THUMB          "ui/assets/sliderbutt_1.tga"
#define ASSET_CORNERIN              "ui/assets/cornerIn"
#define ASSET_CORNEROUT             "ui/assets/cornerOut"
#define SCROLLBAR_SIZE 16.0f
#define SCROLLBAR_WIDTH (SCROLLBAR_SIZE*DC->aspectScale)
#define SCROLLBAR_HEIGHT SCROLLBAR_SIZE
#define SLIDER_WIDTH (96.0f*DC->aspectScale)
#define SLIDER_HEIGHT 16.0f
#define SLIDER_THUMB_WIDTH (12.0f*DC->aspectScale)
#define SLIDER_THUMB_HEIGHT 20.0f
#define NUM_CROSSHAIRS      10

typedef struct
{
    StringEntry command;
    StringEntry args[MAX_SCRIPT_ARGS];
}
scriptDef_t;


typedef struct
{
    F32 x;    // horiz position
    F32 y;    // vert position
    F32 w;    // width
    F32 h;    // height;
}
rectDef_t;

typedef rectDef_t rectangle;

// FIXME: do something to separate text vs window stuff

typedef struct
{
    rectangle rect;                 // client coord rectangle
    S32 aspectBias;                 // direction in which to aspect compensate
    rectangle rectClient;           // screen coord rectangle
    StringEntry name;               //
    StringEntry group;              // if it belongs to a group
    StringEntry cinematicName;      // cinematic name
    S32 cinematic;                  // cinematic handle
    S32 style;                      //
    S32 border;                     //
    S32 ownerDraw;                  // ownerDraw style
    S32 ownerDrawFlags;             // show flags for ownerdraw items
    F32 borderSize;               //
    S32 flags;                      // visible, focus, mouseover, cursor
    rectangle rectEffects;          // for various effects
    rectangle rectEffects2;         // for various effects
    S32 offsetTime;                 // time based value for various effects
    S32 nextTime;                   // time next effect should cycle
    vec4_t foreColor;               // text color
    vec4_t backColor;               // border color
    vec4_t borderColor;             // border color
    vec4_t outlineColor;            // border color
    qhandle_t background;           // background asset
}
windowDef_t;

typedef windowDef_t Window;

typedef struct
{
    vec4_t  color;
    F32    low;
    F32    high;
}
colorRangeDef_t;

// FIXME: combine flags into bitfields to save space
// FIXME: consolidate all of the common stuff in one structure for menus and items
// THINKABOUTME: is there any compelling reason not to have items contain items
// and do away with a menu per say.. major issue is not being able to dynamically allocate
// and destroy stuff.. Another point to consider is adding an alloc free call for vm's and have
// the engine just allocate the pool for it based on a cvar
// many of the vars are re-used for different item types, as such they are not always named appropriately
// the benefits of c++ in DOOM will greatly help crap like this
// FIXME: need to put a type ptr that points to specific type info per type
//
#define MAX_LB_COLUMNS 16

typedef struct columnInfo_s
{
    S32 pos;
    S32 width;
    S32 align;
}
columnInfo_t;

typedef struct listBoxDef_s
{
    S32 startPos;
    S32 endPos;
    S32 drawPadding;
    S32 cursorPos;
    F32 elementWidth;
    F32 elementHeight;
    S32 elementStyle;
    S32 numColumns;
    columnInfo_t columnInfo[MAX_LB_COLUMNS];
    StringEntry doubleClick;
    bool notselectable;
    bool noscrollbar;
}
listBoxDef_t;

typedef struct comboBoxDef_s
{
    S32 cursorPos;
}
comboBoxDef_t;

typedef struct editFieldDef_s
{
    F32 minVal;                  //  edit field limits
    F32 maxVal;                  //
    F32 defVal;                  //
    F32 range;                   //
    S32 maxChars;                  // for edit fields
    S32 maxPaintChars;             // for edit fields
    S32 maxFieldWidth;             // for edit fields
    S32 paintOffset;               //
}
editFieldDef_t;

#define MAX_MULTI_CVARS 32

typedef struct multiDef_s
{
    StringEntry cvarList[MAX_MULTI_CVARS];
    StringEntry cvarStr[MAX_MULTI_CVARS];
    F32 cvarValue[MAX_MULTI_CVARS];
    S32 count;
    bool strDef;
}
multiDef_t;

typedef struct modelDef_s
{
    S32 angle;
    vec3_t origin;
    F32 fov_x;
    F32 fov_y;
    S32 rotationSpeed;
    
    vec3_t axis;
    
    S32   animated;
    S32   startframe;
    S32   numframes;
    S32   fps;
    
    S32   frame;
    S32   oldframe;
    F32 backlerp;
    S32   frameTime;
}
modelDef_t;

#define CVAR_ENABLE    0x00000001
#define CVAR_DISABLE  0x00000002
#define CVAR_SHOW      0x00000004
#define CVAR_HIDE      0x00000008

typedef struct itemDef_s
{
    Window window;                 // common positional, border, style, layout info
    rectangle textRect;            // rectangle the text ( if any ) consumes
    S32 type;                      // text, button, radiobutton, checkbox, textfield, listbox, combo
    S32 alignment;                 // left center right
    S32 textalignment;             // ( optional ) alignment for text within rect based on text width
    S32 textvalignment;            // ( optional ) alignment for text within rect based on text width
    F32 textalignx;              // ( optional ) text alignment x coord
    F32 textaligny;              // ( optional ) text alignment x coord
    F32 textscale;               // scale percentage from 72pts
    S32 textStyle;                 // ( optional ) style, normal and shadowed are it for now
    StringEntry text;              // display text
    void* parent;                  // menu owner
    qhandle_t asset;               // handle to asset
    StringEntry mouseEnterText;    // mouse enter script
    StringEntry mouseExitText;     // mouse exit script
    StringEntry mouseEnter;        // mouse enter script
    StringEntry mouseExit;         // mouse exit script
    StringEntry action;            // select script
    StringEntry onFocus;           // select script
    StringEntry leaveFocus;        // select script
    StringEntry onTextEntry;       // called when text entered
    StringEntry cvar;              // associated cvar
    StringEntry cvarTest;          // associated cvar for enable actions
    StringEntry enableCvar;         // enable, disable, show, or hide based on value, this can contain a list
    S32 cvarFlags;                 //  what type of action to take on cvarenables
    sfxHandle_t focusSound;
    S32 numColors;                 // number of color ranges
    colorRangeDef_t colorRanges[MAX_COLOR_RANGES];
    F32 special;                 // F32 used for feeder id's etc.. diff per type
    S32 modifier;                 // S32 used for feeder id's etc.. diff per type
    S32 cursorPos;                 // cursor position in characters
    void* typeData;                // type specific data ptr's
}
itemDef_t;

typedef struct
{
    Window window;
    StringEntry  font;                // font
    bool fullScreen;              // covers entire screen
    S32 itemCount;                    // number of items;
    S32 fontIndex;                    //
    S32 cursorItem;                    // which item as the cursor
    S32 fadeCycle;                    //
    F32 fadeClamp;                  //
    F32 fadeAmount;                  //
    StringEntry onOpen;                // run when the menu is first opened
    StringEntry onClose;              // run when the menu is closed
    StringEntry onESC;                // run when the menu is closed
    StringEntry soundName;            // background loop sound for menu
    StringEntry listenCvar;           // Cvar to be executed as an uiscript
    
    vec4_t focusColor;                // focus color for items
    vec4_t disableColor;              // focus color for items
    itemDef_t* items[MAX_MENUITEMS];  // items this menu contains
}
menuDef_t;

typedef struct
{
    StringEntry fontStr;
    StringEntry cursorStr;
    StringEntry gradientStr;
    fontInfo_t textFont;
    fontInfo_t smallFont;
    fontInfo_t bigFont;
    qhandle_t cursor;
    qhandle_t gradientBar;
    qhandle_t scrollBarArrowUp;
    qhandle_t scrollBarArrowDown;
    qhandle_t scrollBarArrowLeft;
    qhandle_t scrollBarArrowRight;
    qhandle_t scrollBar;
    qhandle_t scrollBarThumb;
    qhandle_t buttonMiddle;
    qhandle_t buttonInside;
    qhandle_t solidBox;
    qhandle_t sliderBar;
    qhandle_t sliderThumb;
    qhandle_t cornerOut;
    qhandle_t cornerIn;
    sfxHandle_t menuEnterSound;
    sfxHandle_t menuExitSound;
    sfxHandle_t menuBuzzSound;
    sfxHandle_t itemFocusSound;
    F32 fadeClamp;
    S32 fadeCycle;
    F32 fadeAmount;
    F32 shadowX;
    F32 shadowY;
    vec4_t shadowColor;
    F32 shadowFadeClamp;
    bool fontRegistered;
    UTF8 emoticons[ MAX_EMOTICONS ][ MAX_EMOTICON_NAME_LEN ];
    qhandle_t emoticonShaders[ MAX_EMOTICONS ];
    S32 emoticonWidths[ MAX_EMOTICONS ];
    S32 emoticonCount;
}
cachedAssets_t;

typedef struct
{
    StringEntry name;
    void ( *handler )( itemDef_t* item, UTF8** args );
}
commandDef_t;

typedef struct
{
    qhandle_t ( *registerShaderNoMip )( StringEntry p );
    void ( *setColor )( const vec4_t v );
    void ( *drawHandlePic )( F32 x, F32 y, F32 w, F32 h, qhandle_t asset );
    void ( *drawStretchPic )( F32 x, F32 y, F32 w, F32 h, F32 s1, F32 t1, F32 s2, F32 t2, qhandle_t hShader );
    qhandle_t ( *registerModel )( StringEntry p );
    void ( *modelBounds )( qhandle_t model, vec3_t min, vec3_t max );
    void ( *fillRect )( F32 x, F32 y, F32 w, F32 h, const vec4_t color );
    void ( *drawRect )( F32 x, F32 y, F32 w, F32 h, F32 size, const vec4_t color );
    void ( *drawRoundedRect )( F32 x, F32 y, F32 w, F32 h, F32 size, const vec4_t color );
    void ( *fillRoundedRect )( F32 x, F32 y, F32 w, F32 h, F32 size, const vec4_t color );
    void ( *drawSides )( F32 x, F32 y, F32 w, F32 h, F32 size );
    void ( *drawTopBottom )( F32 x, F32 y, F32 w, F32 h, F32 size );
    void ( *clearScene )( void );
    void ( *addRefEntityToScene )( const refEntity_t* re );
    void ( *renderScene )( const refdef_t* fd );
    void ( *registerFont )( StringEntry pFontname, S32 pointSize, fontInfo_t* font );
    void ( *ownerDrawItem )( F32 x, F32 y, F32 w, F32 h, F32 text_x,
                             F32 text_y, S32 ownerDraw, S32 ownerDrawFlags,
                             S32 align, S32 textalign, S32 textvalign,
                             F32 special, F32 scale, vec4_t foreColor,
                             vec4_t backColor, qhandle_t shader, S32 textStyle );
    F32( *getValue )( S32 ownerDraw );
    bool ( *ownerDrawVisible )( S32 flags );
    void ( *runScript )( UTF8** p );
    void ( *getCVarString )( StringEntry cvar, UTF8* buffer, S32 bufsize );
    F32( *getCVarValue )( StringEntry cvar );
    void ( *setCVar )( StringEntry cvar, StringEntry value );
    void ( *drawTextWithCursor )( F32 x, F32 y, F32 scale, vec4_t color, StringEntry text, S32 cursorPos, UTF8 cursor, S32 limit, S32 style );
    void ( *setOverstrikeMode )( bool b );
    bool ( *getOverstrikeMode )( void );
    void ( *startLocalSound )( sfxHandle_t sfx, S32 channelNum );
    bool ( *ownerDrawHandleKey )( S32 ownerDraw, S32 flags, F32* special, S32 key );
    S32( *feederCount )( F32 feederID );
    StringEntry( *feederItemText )( F32 feederID, S32 index, S32 column, qhandle_t* handle );
    qhandle_t ( *feederItemImage )( F32 feederID, S32 index );
    void ( *feederSelection )( F32 feederID, S32 index );
    S32( *feederInitialise )( F32 feederID );
    void ( *keynumToStringBuf )( S32 keynum, UTF8* buf, S32 buflen );
    void ( *getBindingBuf )( S32 keynum, UTF8* buf, S32 buflen );
    void ( *setBinding )( S32 keynum, StringEntry binding );
    void ( *executeText )( S32 exec_when, StringEntry text );
    void ( *Error )( S32 level, StringEntry error, ... );
    void ( *Print )( StringEntry msg, ... );
    void ( *Pause )( bool b );
    S32( *ownerDrawWidth )( S32 ownerDraw, F32 scale );
    StringEntry( *ownerDrawText )( S32 ownerDraw );
    sfxHandle_t ( *registerSound )( StringEntry name, bool compressed );
    void ( *startBackgroundTrack )( StringEntry intro, StringEntry loop, S32 fadeupTime );
    void ( *stopBackgroundTrack )( void );
    S32( *playCinematic )( StringEntry name, F32 x, F32 y, F32 w, F32 h );
    void ( *stopCinematic )( S32 handle );
    void ( *drawCinematic )( S32 handle, F32 x, F32 y, F32 w, F32 h );
    void ( *runCinematicFrame )( S32 handle );
    S32( *getFileList )( StringEntry path, StringEntry extension, UTF8* listbuf, S32 bufsize );
    
    F32      yscale;
    F32      xscale;
    F32      aspectScale;
    S32        realTime;
    S32        frameTime;
    F32      cursorx;
    F32      cursory;
    F32      cursordx;
    F32      cursordy;
    bool  debug;
    
    cachedAssets_t Assets;
    
    glconfig_t glconfig;
    qhandle_t  whiteShader;
    qhandle_t gradientImage;
    qhandle_t cursor;
    F32 FPS;
    
    bool  hudloading;
}
displayContextDef_t;

StringEntry String_Alloc( StringEntry p );
void String_Init( void );
void String_Report( void );
void Init_Display( displayContextDef_t* dc );
void Display_ExpandMacros( UTF8* buff );
void Menu_Init( menuDef_t* menu );
void Item_Init( itemDef_t* item );
void Menu_PostParse( menuDef_t* menu );
menuDef_t* Menu_GetFocused( void );
void Menu_HandleKey( menuDef_t* menu, S32 key, bool down );
void Menu_HandleMouseMove( menuDef_t* menu, F32 x, F32 y );
void Menu_ScrollFeeder( menuDef_t* menu, S32 feeder, bool down );
bool Float_Parse( UTF8** p, F32* f );
bool Color_Parse( UTF8** p, vec4_t* c );
bool Int_Parse( UTF8** p, S32* i );
bool Rect_Parse( UTF8** p, rectDef_t* r );
bool String_Parse( UTF8** p, StringEntry* out );
bool Script_Parse( UTF8** p, StringEntry* out );
bool PC_Float_Parse( S32 handle, F32* f );
bool PC_Color_Parse( S32 handle, vec4_t* c );
bool PC_Int_Parse( S32 handle, S32* i );
bool PC_Rect_Parse( S32 handle, rectDef_t* r );
bool PC_String_Parse( S32 handle, StringEntry* out );
bool PC_Script_Parse( S32 handle, StringEntry* out );
S32 Menu_Count( void );
void Menu_New( S32 handle );
void Menu_PaintAll( void );
menuDef_t* Menus_ActivateByName( StringEntry p );
void Menu_Reset( void );
bool Menus_AnyFullScreenVisible( void );
void  Menus_Activate( menuDef_t* menu );

displayContextDef_t* Display_GetContext( void );
void* Display_CaptureItem( S32 x, S32 y );
bool Display_MouseMove( void* p, F32 x, F32 y );
S32 Display_CursorType( S32 x, S32 y );
bool Display_KeyBindPending( void );
menuDef_t* Menus_FindByName( StringEntry p );
itemDef_t* Menu_FindItemByName( menuDef_t* menu, StringEntry p );
void Menus_CloseByName( StringEntry p );
void Display_HandleKey( S32 key, bool down, S32 x, S32 y );
void LerpColor( vec4_t a, vec4_t b, vec4_t c, F32 t );
void Menus_CloseAll( bool force );
void Menu_Paint( menuDef_t* menu, bool forcePaint );
void Menu_SetFeederSelection( menuDef_t* menu, S32 feeder, S32 index, StringEntry name );
void Display_CacheAll( void );

typedef void ( CaptureFunc )( void* p );

void UI_InstallCaptureFunc( CaptureFunc* f, void* data, S32 timeout );
void UI_RemoveCaptureFunc( void );

void* UI_Alloc( S32 size );
void UI_InitMemory( void );
bool UI_OutOfMemory( void );

void Controls_GetConfig( void );
void Controls_SetConfig( bool restart );
void Controls_SetDefaults( void );

//for cg_draw.c
void Item_Text_Wrapped_Paint( itemDef_t* item );
void UI_DrawTextBlock( rectDef_t* rect, F32 text_x, F32 text_y, vec4_t color,
                       F32 scale, S32 textalign, S32 textvalign,
                       S32 textStyle, StringEntry text );
void UI_Text_Paint( F32 x, F32 y, F32 scale, vec4_t color, StringEntry text, F32 adjust, S32 limit, S32 style );
void UI_Text_Paint_Limit( F32* maxX, F32 x, F32 y, F32 scale,
                          vec4_t color, StringEntry text, F32 adjust, S32 limit );
F32 UI_Text_Width( StringEntry text, F32 scale, S32 limit );
F32 UI_Text_Height( StringEntry text, F32 scale, S32 limit );
F32 UI_Text_EmWidth( F32 scale );
F32 UI_Text_EmHeight( F32 scale );

S32      trap_PC_AddGlobalDefine( UTF8* define );
S32      trap_PC_LoadSource( StringEntry filename );
S32      trap_PC_FreeSource( S32 handle );
S32      trap_PC_ReadToken( S32 handle, pc_token_t* pc_token );
S32      trap_PC_SourceFileAndLine( S32 handle, UTF8* filename, S32* line );

void    BindingFromName( StringEntry cvar );
extern UTF8 g_nameBind1[ 32 ];
extern UTF8 g_nameBind2[ 32 ];

#endif //!__GUI_SHARED_H__
