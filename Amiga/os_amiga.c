/* os_amiga.c -- Interface routine for the Amiga. */
/*		 Written by David Kinder.	  */

#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <devices/audio.h>
#include <exec/exec.h>
#include <graphics/gfxbase.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include "agility.h"
#include "interp.h"

#define SHIFT (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)
#define HISTORY_SIZE 10
enum chars
  { CHAR_LEFT = 0x100,
    CHAR_SHIFTLEFT,
    CHAR_RIGHT,
    CHAR_SHIFTRIGHT,
    CHAR_UP,
    CHAR_DOWN };

extern struct Library *IntuitionBase;
extern struct GfxBase *GfxBase;
struct Screen *Screen;
struct Window *Window;
struct Window *OldWindowPtr;
struct RastPort *RPort;
struct Menu *Menus;
struct Process *ThisProcess;
struct FileRequester *FileReq;
APTR Visual;
char *History[HISTORY_SIZE];
char Version[] = "$VER:AGiliTy 1.1.2 (18.3.22)";
char GameName[256];

struct MsgPort *AudioPort;
struct IOAudio *AudioIO;
LONG AudioDev = -1;
BYTE *AudioWave;

int scroll_count;
int box,boxx;
int last_text_out = 0;
int init_done = 0;

void print_statline(void);
void busy_pointer(int busy);
void draw_cursor(void);
void get_string(char *buffer, int len);
void add_history(char *buffer);
int use_history(char *buffer, int hpos, int left);
void file_req(char *s,char *title,char *pattern,char *dir,ULONG save);
void get_new_game(char *name, char *dir);

void agt_delay(int n)
{
  if (!BATCH_MODE && !fast_replay)
  {
    print_statline();
    Delay(50*n);
  }
}

void agt_tone(int hz,int ms)
{
static UBYTE channels[] = { 1,2,4,8 };
ULONG clock;

  if (!sound_on)
    return;

  clock = (GfxBase->DisplayFlags & PAL) ? 3546895 : 3579545;

  if (AudioPort == NULL)
    AudioPort = CreateMsgPort();
  if (AudioPort == NULL)
    return;

  if (AudioIO == NULL)
    AudioIO = CreateIORequest(AudioPort,sizeof(struct IOAudio));
  if (AudioIO == NULL)
    return;

  if (AudioDev != 0)
  {
    AudioIO->ioa_Request.io_Command = ADCMD_ALLOCATE;
    AudioIO->ioa_Request.io_Flags = ADIOF_NOWAIT;
    AudioIO->ioa_AllocKey = 0;
    AudioIO->ioa_Data = channels;
    AudioIO->ioa_Length = 4;
    AudioDev = OpenDevice(AUDIONAME,0,(struct IORequest *)AudioIO,0);
  }
  if (AudioDev != 0)
    return;

  if (AudioWave == NULL)
    AudioWave = AllocVec(2,MEMF_CHIP|MEMF_PUBLIC);
  if (AudioWave == NULL)
    return;
  AudioWave[0] = 127;
  AudioWave[1] = -127;

  AudioIO->ioa_Request.io_Command = CMD_WRITE;
  AudioIO->ioa_Request.io_Flags = ADIOF_PERVOL;
  AudioIO->ioa_Data = AudioWave;
  AudioIO->ioa_Length = 2;
  AudioIO->ioa_Period = clock/(2*hz);
  AudioIO->ioa_Volume = 64;
  AudioIO->ioa_Cycles = (hz*ms)/1000;

  BeginIO(AudioIO);
  WaitPort(AudioPort);
  GetMsg(AudioPort);
}

int agt_rand(int a,int b)
{
  return a+(rand()>>2)%(b-a+1);
}

char *agt_input(int in_type)
{
char *s;

  scroll_count = 0;
  print_statline();
  s = rmalloc(256);
  get_string(s,256);
  if (script_on) fprintf(scriptfile,"%s",s);
  agt_newline();
  last_text_out = 0;
  return s;
}

char agt_getkey(rbool echo_char)
{
char s[2];
int c = 128;

  scroll_count = 0;
  print_statline();
  s[1] = '\0';
  while (c >= 128) c = get_character();
  if (echo_char && c > 32)
  {
    SetAPen(RPort,2);
    s[0] = c;
    agt_puts(s);
    agt_newline();
    SetAPen(RPort,1);
  }
  last_text_out = 0;
  return c;
}

void agt_textcolor(int c)
{
  if (Window == NULL) return;

  switch (c)
  {
    case -1:
      while ((RPort->AlgoStyle & FSF_BOLD) == 0)
	SetSoftStyle(RPort,RPort->AlgoStyle|FSF_BOLD,AskSoftStyle(RPort));
      break;
    case -2:	/* Fall-through intended */
    case 7:
      while (RPort->AlgoStyle != 0)
	SetSoftStyle(RPort,FS_NORMAL,AskSoftStyle(RPort));
      break;
  }
}

void agt_statline(const char *s)
{
int x,y;

  SetAPen(RPort,2);
  SetBPen(RPort,3);
  x = RPort->cp_x;
  y = RPort->cp_y;
  Move(RPort,0,RPort->TxBaseline);
  Text(RPort,s,strlen(s));
  Move(RPort,x,y);
  SetAPen(RPort,1);
  SetBPen(RPort,0);
}

void agt_clrscr(void)
{
  SetAPen(RPort,0);
  RectFill(RPort,
	   0,RPort->TxHeight,
	   Window->Width-1,(screen_height*RPort->TxHeight)-1);
  SetAPen(RPort,1);
  Move(RPort,0,RPort->TxHeight);

  curr_x = 0;
  if (script_on) fprintf(scriptfile,"\n\n\n\n");
  scroll_count = 0;
}

void agt_puts(const char *s)
{
  if (Window == NULL)
  {
    printf("%s",s);
    return;
  }

  last_text_out = 1;

  Move(RPort,RPort->cp_x,RPort->cp_y+RPort->TxBaseline);
  Text(RPort,s,strlen(s));
  Move(RPort,RPort->cp_x,RPort->cp_y-RPort->TxBaseline);
  if (RPort->cp_x/RPort->TxWidth >= screen_width) RPort->cp_x = 0;

  curr_x = RPort->cp_x/RPort->TxWidth;
  if (script_on) fprintf(scriptfile,"%s",s);
}

void agt_newline(void)
{
  if (Window == NULL)
  {
    printf("\n");
    curr_x = 0;
    return;
  }

  if (RPort->cp_y/RPort->TxHeight >= screen_height-1)
  {
    ClipBlit(RPort,0,RPort->TxHeight*2,
	     RPort,0,RPort->TxHeight,
	     Window->Width,(screen_height-2)*RPort->TxHeight,0xC0);
    SetAPen(RPort,0);
    RectFill(RPort,
	     0,(screen_height-1)*RPort->TxHeight,
	     Window->Width-1,(screen_height*RPort->TxHeight)-1);
    SetAPen(RPort,1);
    Move(RPort,boxx,(screen_height-1)*RPort->TxHeight);
  }
  else Move(RPort,boxx,RPort->cp_y+RPort->TxHeight);

  if ((box == 0) && !BATCH_MODE && !fast_replay)
  {
    scroll_count++;
    if (scroll_count >= screen_height-2)
    {
      Move(RPort,0,RPort->cp_y+RPort->TxBaseline);
      Text(RPort,"[MORE]",6);
      Move(RPort,RPort->cp_x,RPort->cp_y-RPort->TxBaseline);
      agt_getkey(0);
      Move(RPort,0,RPort->cp_y+RPort->TxBaseline);
      Text(RPort,"      ",6);
      Move(RPort,0,RPort->cp_y-RPort->TxBaseline);
      scroll_count = 0;
    }
  }

  curr_x = 0;
  if (script_on) fprintf(scriptfile,"\n");
}

void start_interface(fc_type fc)
{
  if (!stable_random)
    srand(time(0));
}

void init_interface(int argc,char *argv[])
{
struct Screen *default_pub_screen;
static WORD screen_pens[] = { -1 };
static struct NewMenu menus[] =
 {{ NM_TITLE,"Project",0,0,0,0 },
  { NM_ITEM,"New Game...","N",0,0,0 },
  { NM_ITEM, "Save...","S",0,0,0},
  { NM_ITEM, "Restore...","R",0,0,0},
  { NM_ITEM, NM_BARLABEL,0,0,0,0},
  { NM_ITEM,"About...","?",0,0,0 },
  { NM_ITEM,"Quit","Q",0,0,0 },
  { NM_END,0,0,0,0 }};

  if (IntuitionBase->lib_Version < 37) exit(0);

  script_on = 0;
  scriptfile = 0;
  center_on = par_fill_on = 0;
  scroll_count = 0;
  debugfile = stderr;
  DEBUG_OUT = 0;
  PURE_INPUT = 0;
  screen_height = 25;
  screen_width = 78;

  if ((default_pub_screen = LockPubScreen(0)) == 0) exit(0);
  if ((Screen = OpenScreenTags(0,
    SA_Pens,screen_pens,
    SA_SysFont,1,
    SA_DisplayID,GetVPModeID(&default_pub_screen->ViewPort),
    SA_Overscan,OSCAN_TEXT,
    SA_Depth,2,
    SA_Type,CUSTOMSCREEN|AUTOSCROLL,
    SA_Title,"AGiliTy",TAG_DONE)) == 0) exit(0);
  UnlockPubScreen(0,default_pub_screen);

  if ((Window = OpenWindowTags(0,
    WA_Left,0,
    WA_Top,Screen->BarHeight+1,
    WA_Width,Screen->Width,
    WA_Height,Screen->Height-Screen->BarHeight-1,
    WA_Flags,
      WFLG_ACTIVATE|WFLG_SMART_REFRESH|WFLG_NEWLOOKMENUS|WFLG_BORDERLESS|
      WFLG_BACKDROP,
    WA_IDCMP,IDCMP_VANILLAKEY|IDCMP_RAWKEY|IDCMP_MENUPICK,
    WA_CustomScreen,Screen,TAG_DONE)) == 0) exit(0);

  ThisProcess = (struct Process *)FindTask(0);
  OldWindowPtr = ThisProcess->pr_WindowPtr;
  ThisProcess->pr_WindowPtr = Window;

  if ((Visual = GetVisualInfo(Screen,TAG_DONE)) == 0) exit(0);
  if ((Menus = CreateMenus(menus,GTMN_NewLookMenus,1,TAG_DONE)) == 0)
    exit(0);
  LayoutMenus(Menus,Visual,GTMN_NewLookMenus,1,TAG_DONE);
  SetMenuStrip(Window,Menus);

  RPort = Window->RPort;
  SetDrMd(RPort,JAM2);
  SetAPen(RPort,1);
  SetBPen(RPort,0);

  screen_height = Window->Height/RPort->TxHeight;
  screen_width = Window->Width/RPort->TxWidth;
  status_width = screen_width;
  agt_clrscr();
  init_done = 1;
}

void close_interface(void)
{
  if (scriptfile != 0) fclose(scriptfile);
}

__autoexit void clear_up(void)
{
int i;

  if ((init_done != 0) && (last_text_out != 0))
  {
    agt_newline();
    agt_puts("[Hit any key to exit.]");
    agt_getkey(0);
  }

  for (i = 0; i < HISTORY_SIZE; i++)
    if (History[i] != NULL) FreeVec(History[i]);
  if (FileReq) FreeAslRequest(FileReq);
  if (Menus) FreeMenus(Menus);
  if (Visual) FreeVisualInfo(Visual);
  if (ThisProcess) ThisProcess->pr_WindowPtr = OldWindowPtr;
  if (Window) CloseWindow(Window);
  if (Screen) CloseScreen(Screen);
  if (AudioWave) FreeVec(AudioWave);
  if (AudioDev == 0) CloseDevice((struct IORequest *)AudioIO);
  if (AudioIO) DeleteIORequest(AudioIO);
  if (AudioPort) DeleteMsgPort(AudioPort);
}

int get_character(void)
{
struct IntuiMessage *imsg;
ULONG class;
UWORD code, qualifier;

static struct EasyStruct requester =
  { sizeof(struct EasyStruct),0,"AGiliTy",
    "AGiliTy AGT Interpreter 1.1.2\n"
    "© 1996-1999,2001 by Robert Masenten\n\n"
    "Amiga Version by David Kinder","Continue" };

  draw_cursor();
  for (;;)
  {
    while (imsg = (struct IntuiMessage *)GetMsg(Window->UserPort))
    {
      class = imsg->Class;
      code = imsg->Code;
      qualifier = imsg->Qualifier;
      ReplyMsg((struct Message *)imsg);
      switch (class)
      {
	case IDCMP_VANILLAKEY:
	  draw_cursor();
	  return code;
	  break;
	case IDCMP_RAWKEY:
	  switch (code)
	  {
	    case 0x4C:
	      draw_cursor();
	      return CHAR_UP;
	      break;
	    case 0x4D:
	      draw_cursor();
	      return CHAR_DOWN;
	      break;
	    case 0x4E:
	      draw_cursor();
	      return (qualifier & SHIFT) ? CHAR_SHIFTRIGHT : CHAR_RIGHT;
	      break;
	    case 0x4F:
	      draw_cursor();
	      return (qualifier & SHIFT) ? CHAR_SHIFTLEFT : CHAR_LEFT;
	      break;
	  }
	  break;
	case IDCMP_MENUPICK:
	  if (MENUNUM(code) == 0)
	  {
	    switch (ITEMNUM(code))
 	    {
	      case 0:
		get_new_game(GameName,"");
		if (strcmp(GameName,"") != 0)
		{
		  draw_cursor();
		  SetAPen(RPort,1);
		  agt_newgame(init_file_context(GameName,fNONE));
		  return 0x0D;
		}
		break;
	      case 1:
		draw_cursor();
		SetAPen(RPort,1);
		agt_save();
		return 0x0D;
		break;
	      case 2:
		agt_restore();
		draw_cursor();
		SetAPen(RPort,1);
		return 0x0D;
		break;
	      case 4:
		busy_pointer(1);
		EasyRequest(Window,&requester,0);
		busy_pointer(0);
		break;
	      case 5:
		last_text_out = 0;
		exit(0);
		break;
	    }
	  }
	  break;
      }
    }
    WaitPort(Window->UserPort);
  }
}

void busy_pointer(int busy)
{
  if (IntuitionBase->lib_Version >= 39)
    SetWindowPointer(Window,WA_BusyPointer,busy,TAG_DONE);
}

void draw_cursor(void)
{
  SetDrMd(RPort,COMPLEMENT);
  RectFill(RPort,
    RPort->cp_x,RPort->cp_y,
    RPort->cp_x+RPort->TxWidth-1,RPort->cp_y+RPort->TxHeight-1);
  SetDrMd(RPort,JAM2);
}

void get_string(char *buffer, int len)
{
int c, end = 0, cursor = 0, hpos = -1;
char co;
UWORD left;

  *buffer = '\0';
  left = RPort->cp_x;
  for (;;)
  {
    SetAPen(RPort,2);
    c = get_character();
    switch (c)
    {
      case 0x0D:
	*(buffer+end) = '\0';
	add_history(buffer);
	*(buffer+end) = '\n';
	*(buffer+end+1) = '\0';
	SetAPen(RPort,1);
	return;
	break;
      case 0x7F:
	if ((end > 0) && (cursor < end))
	{
	  memmove(buffer+cursor,buffer+cursor+1,end-cursor+1);
	  ClipBlit(RPort,RPort->cp_x+RPort->TxWidth,RPort->cp_y,
	    RPort,RPort->cp_x,RPort->cp_y,
	    Window->Width-RPort->cp_x-RPort->TxWidth,
	    RPort->TxHeight,0xC0);
	  end--;
	}
	break;
      case 0x08:
	if ((end > 0) && (cursor > 0))
	{
	  memmove(buffer+cursor-1,buffer+cursor,end-cursor+1);
	  ClipBlit(RPort,RPort->cp_x,RPort->cp_y,
	    RPort,RPort->cp_x-RPort->TxWidth,RPort->cp_y,
	    Window->Width-RPort->cp_x-RPort->TxWidth,
	    RPort->TxHeight,0xC0);
	  Move(RPort,RPort->cp_x-RPort->TxWidth,RPort->cp_y);
	  end--;
	  cursor--;
	}
	break;
      case CHAR_LEFT:
	if (cursor > 0)
	{
	  cursor--;
	  Move(RPort,RPort->cp_x-RPort->TxWidth,RPort->cp_y);
	}
	break;
      case CHAR_SHIFTLEFT:
	if (cursor > 0)
	{
	  Move(RPort,left,RPort->cp_y);
	  cursor = 0;
	}
	break;
      case CHAR_RIGHT:
	if (cursor < end)
	{
	  cursor++;
	  Move(RPort,RPort->cp_x+RPort->TxWidth,RPort->cp_y);
	}
	break;
      case CHAR_SHIFTRIGHT:
	if (cursor < end)
	{
	  Move(RPort,RPort->cp_x+(RPort->TxWidth*(end-cursor)),RPort->cp_y);
	  cursor = end;
	}
	break;
      case CHAR_UP:
	if ((hpos < HISTORY_SIZE-1) && (History[hpos+1]))
	  cursor = end = use_history(buffer,++hpos,left);
	break;
      case CHAR_DOWN:
	if ((hpos > 0) && (History[hpos-1]))
	  cursor = end = use_history(buffer,--hpos,left);
	else
	  cursor = end = use_history(buffer,-1,left);
	break;
      default:
	if ((end < len-3) && (isprint((unsigned char)c) != 0) && ( c < 128)
	  && (RPort->cp_x+(RPort->TxWidth*(end-cursor+2)) < Window->Width))
	{
	  memmove(buffer+cursor+1,buffer+cursor,end-cursor+1);
	  *(buffer+cursor++) = c;
	  ClipBlit(RPort,RPort->cp_x,RPort->cp_y,
	    RPort,RPort->cp_x+RPort->TxWidth,RPort->cp_y,
	    Window->Width-RPort->cp_x-RPort->TxWidth,
	    RPort->TxHeight,0xC0);
	  end++;
	  Move(RPort,RPort->cp_x,RPort->cp_y+RPort->TxBaseline);
	  co = c;
	  Text(RPort,&co,1);
	  Move(RPort,RPort->cp_x,RPort->cp_y-RPort->TxBaseline);
	}
	break;
    }
  }
}

void add_history(char *buffer)
{
char *hist;
int i;

  if (strcmp(buffer,"") == 0) return;
  if ((hist = AllocVec(strlen(buffer)+1,MEMF_CLEAR)) == NULL) return;
  strncpy(hist,buffer,strlen(buffer));
  if (History[HISTORY_SIZE-1] != NULL) FreeVec(History[HISTORY_SIZE-1]);
  for (i = HISTORY_SIZE-1; i > 0; i--) History[i] = History[i-1];
  History[0] = hist;
}

int use_history(char *buffer, int hpos, int left)
{
  strcpy(buffer,(hpos > -1) ? History[hpos] : "");
  SetAPen(RPort,0);
  RectFill(RPort,left,RPort->cp_y,
    Window->Width-1,RPort->cp_y+RPort->TxHeight-1);
  SetAPen(RPort,2);
  Move(RPort,left,RPort->cp_y);
  agt_puts(buffer);
  return strlen(buffer);
}

void file_req(char *s,char *title,char *pattern,char *dir,ULONG save)
{
  if (FileReq == NULL)
  {
    if ((FileReq = AllocAslRequestTags(ASL_FileRequest,
      ASLFR_SleepWindow,1,
      ASLFR_RejectIcons,1,
      ASLFR_InitialDrawer,dir,TAG_DONE)) == NULL) exit(0);
  }
  if (AslRequestTags(FileReq,
    ASLFR_Window,Window,
    ASLFR_DoPatterns,pattern == NULL ? 0 : 1,
    ASLFR_InitialPattern,pattern == NULL ? "#?" : pattern,
    ASLFR_InitialFile,"",
    ASLFR_TitleText,title,
    ASLFR_DoSaveMode,save,TAG_DONE))
  {
    strcpy(s,FileReq->fr_Drawer);
    AddPart(s,FileReq->fr_File,256);
  }
  else strcpy(s,"");
}

wbmain(struct WBStartup *wb_msg)
{
struct DiskObject *icon;
char start_dir[256];
char *args[2], *dir;

  args[0] = wb_msg->sm_ArgList[0].wa_Name;
  args[1] = GameName;
  strcpy(start_dir,"");

  if (icon = GetDiskObject(args[0]))
  {
    if (dir = FindToolType(icon->do_ToolTypes,"DIR")) strcpy(start_dir,dir);
    FreeDiskObject(icon);
  }

  get_new_game(GameName,start_dir);
  if (strcmp(GameName,"") != 0)
    main(2,args);
  exit(0);
}

void get_new_game(char *name, char *dir)
{
char *file, *ext;

  file_req(name,"Select an AGT Game","#?.(da1|agx)",dir,0);
  file = FilePart(name);
  if (ext = strrchr(file,'.')) *ext = '\0';
}

FILE *get_user_file(int ft)
{
char *fname, *otype, *title;
char *p, *q;
int save;
FILE *fd;

  switch (ft)
  {
    case 0:
      title = "Script File";
      save = 1;
      otype = "a";
      break;
    case 1:
      title = "Save Game";
      save = 1;
      otype = "wb";
      break;
    case 2:
      title = "Restore Game";
      save = 0;
      otype = "rb";
      break;
    case 3:
      title = "Read Log";
      save = 0;
      otype = "r";
      break;
    case 4:
      title = "Write Log";
      save = 1;
      otype = "w";
      break;
    default:
      writeln("<INT ERR: invalid file type>");
      return NULL;
  }

  fname = rmalloc(256);
  file_req(fname,title,NULL,"",save);

  for(p = fname; isspace(*p); p++);
  for(q = fname; *p!=0; p++, q++) *q=*p;
  *q = 0;

  if (q==fname)
  {
    writeln("Never mind.");
    rfree(fname);
    return NULL;
  }

  if (otype[0] == 'w')
  {
    fd = fopen(fname,"r");
    if (fd)
    {
      fclose(fd);
      if (!yesno("This file already exists; overwrite?"))
      {
	rfree(fname);
	return NULL;
      }
    }
  }

  fd = fopen(fname,otype);
  if (fd == NULL) writeln("Cannot open file.");
  rfree(fname);
  return fd;
}

void set_default_filenames(fc_type fc)
{
}

rbool agt_option(int optnum, char *optstr[], rbool setflag)
{
  return 0; 
}

genfile agt_globalfile(int fid)
{
  genfile cfg = 0;
  if (fid == 0)
  {
    cfg = (genfile)fopen("agil.cfg","r");
    if (cfg == 0)
      cfg = (genfile)fopen("PROGDIR:agil.cfg","r");
  }
  return cfg;
}

void agt_makebox(int width, int height, unsigned long flags)
{
  box = 1;
  boxx = flags & TB_NOCENT ? 0 : ((screen_width-width)/2)*RPort->TxWidth;
  if (flags & TB_TTL)
    Move(RPort,boxx,(((screen_height-height)/4)*RPort->TxHeight));
  else
    Move(RPort,boxx,RPort->cp_y-(screen_height-height)*RPort->TxHeight);
}

void agt_qnewline(void)
{
  agt_newline();
}

void agt_endbox(void)
{
  box = 0;
  boxx = 0;
  agt_newline();
}
