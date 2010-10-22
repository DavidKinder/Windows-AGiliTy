/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyWin32.cpp: Interface to Win32 MFC port
//
/////////////////////////////////////////////////////////////////////////////

// Undefine memory allocation, deallocation and exit functions
#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef free
#undef free
#endif

#ifdef exit
#undef exit
#endif

#pragma warning(disable : 4273)

#include "StdAfx.h"
#include "AGiliTy.h"
#include "AGiliTyDoc.h"
#include "AGiliTyView.h"
#include "Dialogs.h"

extern "C" {
#include ".\generic\agility.h"
#include ".\generic\interp.h"
}

#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL bResetCursor = FALSE;

// Input line history
CArray<CString, CString&> History;

// Map of memory allocations
CMap<void*,void*,int,int> MemoryMap;

extern "C" void* Win32_malloc(size_t size)
{
  void* p = malloc(size);
  if (p)
    MemoryMap.SetAt(p,0);
  return p;
}

extern "C" void* Win32_realloc(void *memblock,size_t size)
{
  int iDummy;
  if (MemoryMap.Lookup(memblock,iDummy))
    MemoryMap.RemoveKey(memblock);

  void* p = realloc(memblock,size);
  if (p)
    MemoryMap.SetAt(p,0);
  return p;
}

extern "C" void Win32_free(void* memoryblock)
{
  int iDummy;
  if (MemoryMap.Lookup(memoryblock,iDummy))
    MemoryMap.RemoveKey(memoryblock);
  free(memoryblock);
}

extern "C" void __cdecl Win32_exit(int)
{
  AfxThrowUserException();
}

void __cdecl Win32_freeall(void)
{
  POSITION Pos;
  void* MemPtr;
  int iDummy;

  int i = MemoryMap.GetCount();
  int j = 0;

  Pos = MemoryMap.GetStartPosition();
  while (Pos != NULL)
  {
    j++;
    MemoryMap.GetNextAssoc(Pos,MemPtr,iDummy);
    if (MemPtr)
      free(MemPtr);
  }
  MemoryMap.RemoveAll();
}

unsigned int *pScreenBuffer = NULL;
unsigned int iTextFlags = 0;
int iCursorY = 1;
int iBoxX = 0;
int iScrollCount = 0;
bool bInBox = false;

void Win32_SetCharacter(int x, int y, char c);
void Win32_CheckMsgLoop(void);
void Win32_Redraw(int iTop = 0, int iLeft = 0);
void Win32_CaretOn(int& iFontWidth, int& iFontHeight);
void Win32_CaretOff(void);
void Win32_SetScreenSize(BOOL bSetWidth);
void Win32_ResizeScreen(int iOldWidth, int iOldHeight);

void Win32_SetCharacter(int x, int y, char c)
{
  if (pScreenBuffer)
  {
    if ((x >= 0) && (x < screen_width))
    {
      if ((y >= 0) && (y < screen_height))
        pScreenBuffer[(y*screen_width)+x] = (unsigned int)c | iTextFlags;
    }
  }
}

void Win32_CheckMsgLoop(void)
{
  if (quitflag)
    return;

  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  MSG msg;
  while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
  { 
    if (!pApp->PumpMessage())
    {
      agt_quit();
      quitflag = 1;
      if (AfxGetMainWnd())
        AfxGetMainWnd()->PostMessage(WM_CLOSE);
      ::PostQuitMessage(0);
      return;
    }
  } 

  LONG lIdle = 0;
  while (AfxGetApp()->CWinApp::OnIdle(lIdle++));
}

void Win32_Redraw(int iTop, int iLeft)
{
  CAGiliTyView* pView = CAGiliTyView::GetView();
  if (pView)
  {
    if ((iTop > 0) || (iLeft > 0))
    {
      CRect RedrawRect;
      pView->GetClientRect(RedrawRect);
      RedrawRect.top += iTop;
      RedrawRect.left += iLeft;
      pView->RedrawWindow(RedrawRect);
    }
    else
      pView->RedrawWindow();
  }
}

void Win32_CaretOn(int& iFontWidth, int& iFontHeight)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  CAGiliTyView* pView = CAGiliTyView::GetView();
  if (pView == NULL)
    return;

  CFont* pFont = new CFont();
  pFont->CreateFontIndirect(&(pApp->m_LogFont));

  CPaintDC DC(pView);
  TEXTMETRIC FontInfo;
  CFont* pOldFont = DC.SelectObject(pFont);
  DC.GetTextMetrics(&FontInfo);
  iFontWidth = (int)FontInfo.tmAveCharWidth;
  iFontHeight = (int)FontInfo.tmHeight;
  DC.SelectObject(pOldFont);
  delete pFont;

  pView->CreateSolidCaret(iFontWidth,iFontHeight);
  pView->ShowCaret();
}

void Win32_CaretOff(void)
{
  CAGiliTyView* pView = CAGiliTyView::GetView();
  if (pView == NULL)
    return;

  pView->HideCaret();
  ::DestroyCaret();
}

void Win32_SetScreenSize(BOOL bSetWidth)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  CAGiliTyView* pView = CAGiliTyView::GetView();
  if (pView == NULL)
    return;

  // Get the size of the client view area
  CRect ViewRect;
  pView->GetClientRect(ViewRect);

  CFont* pFont = new CFont();
  pFont->CreateFontIndirect(&(pApp->m_LogFont));

  CPaintDC DC(pView);
  TEXTMETRIC FontInfo;
  CFont* pOldFont = DC.SelectObject(pFont);
  DC.GetTextMetrics(&FontInfo);
  int iFontWidth = (int)FontInfo.tmAveCharWidth;
  int iFontHeight = (int)FontInfo.tmHeight;
  DC.SelectObject(pOldFont);
  delete pFont;

  int iOldWidth = screen_width;
  int iOldHeight = screen_height;
  if (bSetWidth)
    screen_width = ViewRect.Width() / iFontWidth;
  screen_height = ViewRect.Height() / iFontHeight;
  status_width = screen_width;
  Win32_ResizeScreen(iOldWidth,iOldHeight);
}

void Win32_ResizeScreen(int iOldWidth, int iOldHeight)
{
  unsigned int* pOldBuffer = pScreenBuffer;
  pScreenBuffer = new unsigned int[screen_width*screen_height];
  for (int i = 0; i < screen_width*screen_height; i++)
    pScreenBuffer[i] = ' ';

  if (pOldBuffer)
  {
    for (int y = 0; y < screen_height; y++)
    {
      for (int x = 0; x < screen_width; x++)
      {
        if ((x < iOldWidth) && (y < iOldHeight))
          pScreenBuffer[(y*screen_width)+x] = pOldBuffer[(y*iOldWidth)+x];
      }
    }
    delete pOldBuffer;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Wait for n seconds
/////////////////////////////////////////////////////////////////////////////
extern "C" void agt_delay(int n)
{
  if (!BATCH_MODE)
  {
    print_statline();
    Win32_Redraw();

    DWORD StartTick = GetTickCount();
    while ((GetTickCount() < StartTick+(n*1000)) && (AfxGetMainWnd() != NULL))
      Win32_CheckMsgLoop();
  }
}

/////////////////////////////////////////////////////////////////////////////
// Produce a hz-Hertz sound for ms milliseconds
/////////////////////////////////////////////////////////////////////////////
extern "C" void agt_tone(int hz,int ms)
{
  if (hz > 0)
  {
    // Under Windows NT, the Win32 Beep() call can be used. Under
    // Windows 95, Beep() just plays the default sound event, whatever
    // that might be. In this case the only recourse is to access the
    // PC speaker directly.
    
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
      _outp(0x43,0xb6);
      unsigned int iFreqPerTick = (unsigned int)(1193180L / hz);
      _outp(0x42,(char)iFreqPerTick);
      _outp(0x42,(char)(iFreqPerTick >> 8));
      int iControl = _inp(0x61);
      _outp(0x61,iControl|0x3);

      DWORD StartTick = GetTickCount();
      while ((GetTickCount() < StartTick+ms) && (AfxGetMainWnd() != NULL))
        Win32_CheckMsgLoop();

      _outp(0x61,iControl);
    }
    else
      Beep(hz,ms);
  }
}

/////////////////////////////////////////////////////////////////////////////
// Return a random number from a to b inclusive
/////////////////////////////////////////////////////////////////////////////
extern "C" int agt_rand(int a,int b)
{
  return a+(rand()>>2)%(b-a+1);
}

/////////////////////////////////////////////////////////////////////////////
// Read a line from the keyboard, allocating space for it using malloc
// in_type: 0=command, 1=number, 2=question, 3=userstr, 4=filename,
//          5=RESTART,RESTORE,UNDO,QUIT
/////////////////////////////////////////////////////////////////////////////
extern "C" char *agt_input(int in_type)
{
  char *s = NULL;
  const int iMaxInput = 256;
  int iInputChar = '\0';
  int iInputX = curr_x;
  int iInputY = iCursorY;
  int iEditX = iInputX;
  int iFontWidth, iFontHeight;
  int iHistory = -1;    // Current history position

  print_statline();
  s = (char *)rmalloc(iMaxInput);
  *s = '\0';

  iScrollCount = 0;
  bResetCursor = FALSE;
  Win32_Redraw();
  Win32_CaretOn(iFontWidth,iFontHeight);

  while (iInputChar != 13)
  {
    CAGiliTyView* pView = CAGiliTyView::GetView();
    if (pView == NULL)
      return s;

    if (bResetCursor)
    {
      Win32_CaretOff();
      Win32_CaretOn(iFontWidth,iFontHeight);
      Win32_Redraw();
      bResetCursor = FALSE;
    }
    pView->SetCaretPos(CPoint(iFontWidth*iEditX,iFontHeight*iCursorY));
    Win32_CheckMsgLoop();

    // Get the view again in case exiting has caused it to be deleted
    pView = CAGiliTyView::GetView();
    if (pView == NULL)
      return s;

    CArray<int,int>& iInput = pView->m_iInput;
    if (iInput.GetSize() > 0)
    {
      BOOL bRedrawLine = FALSE;
      iInputChar = iInput[0];
      iInput.RemoveAt(0);

      switch (iInputChar)
      {
      case 0x100 + VK_UP:      // Cursor up
        if (iHistory < History.GetSize()-1)
          iHistory++;
        if ((iHistory >= 0) && (History.GetSize() > 0))
        {
          strcpy(s,History[iHistory]);
          iEditX = (int)strlen(s)+iInputX;
          bRedrawLine = TRUE;
        }
        break;
      case 0x100 + VK_DOWN:    // Cursor down
        if (iHistory > 0)
          iHistory--;
        if ((iHistory >= 0) && (History.GetSize() > 0))
        {
          strcpy(s,History[iHistory]);
          iEditX = (int)strlen(s)+iInputX;
          bRedrawLine = TRUE;
        }
        break;
      case 0x100 + VK_LEFT:    // Cursor left
        if (iEditX > iInputX)
          iEditX--;
        break;
      case 0x100 + VK_RIGHT:  // Cursor right
        if (iEditX-iInputX < (int)strlen(s))
          iEditX++;
        break;
      case 0x100 + VK_HOME:    // Home
        iEditX = iInputX;
        break;
      case 0x100 + VK_END:    // End
        iEditX = (int)strlen(s)+iInputX;
        break;
      case 0x100 + VK_DELETE:  // Delete
        if (iEditX-iInputX < (int)strlen(s))
        {
          for (int i = iEditX - iInputX; i < (int)strlen(s); i++)
            s[i] = s[i+1];
        }
        break;
      case 8:                  // Backspace
        if (iEditX > iInputX)
        {
          for (int i = iEditX - iInputX-1; i < (int)strlen(s); i++)
            s[i] = s[i+1];
          iEditX--;
        }
        break;
      default:
        if (isprint(iInputChar))
        {
          if (iEditX < screen_width-1)
          {
            int iPos = iEditX - iInputX;
            if (iPos < iMaxInput-1)
            {
              for (int i = strlen(s); i >= iPos; i--)
                s[i+1] = s[i];
              s[iPos] = (char)iInputChar;
            }
            iEditX++;
          }
        }
        break;
      }

      int i = 0;
      for (; i < (int)strlen(s); i++)
        Win32_SetCharacter(iInputX+i, iInputY, s[i]);
      while (i < screen_width)
        Win32_SetCharacter(iInputX+(i++), iInputY, ' ');
      if (bRedrawLine)
        Win32_Redraw(iInputY*iFontHeight);
      else
        Win32_Redraw(iInputY*iFontHeight,(iEditX-1)*iFontWidth);
    }
    else
      ::WaitMessage();
  }

  // Store in input history
  if (strlen(s) > 0)
  {
    CString strInput(s);
    History.InsertAt(0,strInput);
    if (History.GetSize() > 20)
      History.RemoveAt(History.GetSize()-1);
  }

  if (DEBUG_OUT) 
    fprintf(debugfile,"%s\n",s);
  if (script_on)
    fprintf(scriptfile,"%s\n",s);

  Win32_CaretOff();
  agt_newline();

  return s;
}

/////////////////////////////////////////////////////////////////////////////
// Reads a character and returns it, possibly reading in a full line
// depending on the platform
// If echo_char=1, echo character. If 0, then the character is not echoed
/////////////////////////////////////////////////////////////////////////////
extern "C" char agt_getkey(rbool echo_char)
{
  int iInputChar = '\0';
  int iFontWidth, iFontHeight;

  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  CAGiliTyView* pView = CAGiliTyView::GetView();
  if (pView == NULL)
    return 0;

  print_statline();
  bResetCursor = FALSE;
  Win32_Redraw();
  Win32_CaretOn(iFontWidth,iFontHeight);

  while (iInputChar == '\0')
  {
    pView = CAGiliTyView::GetView();
    if (pView == NULL)
      return 0;

    if (bResetCursor)
    {
      Win32_CaretOff();
      Win32_CaretOn(iFontWidth,iFontHeight);
      Win32_Redraw();
      bResetCursor = FALSE;
    }
    pView->SetCaretPos(CPoint(iFontWidth*curr_x,iFontHeight*iCursorY));
    Win32_CheckMsgLoop();

    // Get the view again in case exiting has caused it to be deleted
    pView = CAGiliTyView::GetView();
    if (pView == NULL)
      return 0;

    CArray<int,int>& iInput = pView->m_iInput;
    if (iInput.GetSize() > 0)
    {
      iInputChar = iInput[0];
      iInput.RemoveAt(0);
    }
    else
      ::WaitMessage();
  }

  if (echo_char && (iInputChar > 32))
  {
    char pszEcho[2];
    pszEcho[0] = (char)iInputChar;
    pszEcho[1] = '\0';
    agt_puts(pszEcho);
    agt_newline();
  }

  Win32_CaretOff();
  return (char)iInputChar;
}

/////////////////////////////////////////////////////////////////////////////
// Set text color to color #c, where the colors are as follows:
//  0=Black, 1=Blue, 2=Green, 3=Cyan, 4=Red, 5=Magenta, 6=Brown,
//  7=White("normal"), 8=Blinking.
//  9=*Just* White (not neccessarily "normal" and no need to turn off
//    blinking)
// Also used to set other text attributes:
// -1=emphasized text, used (e.g.) for room titles
// -2=end emphasized text
/////////////////////////////////////////////////////////////////////////////
extern "C" void agt_textcolor(int c)
{
  switch (c)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
    iTextFlags = (iTextFlags & 0xF000) | ((c+1) << 8);
    break;
  case 7:
    iTextFlags = (iTextFlags & 0xF000);
    iTextFlags &= ~CAGiliTyView::CharEmphasis;
    break;
  case 9:
    iTextFlags = (iTextFlags & 0xF000) | (8 << 8);
    break;
  case -1:
    iTextFlags |= CAGiliTyView::CharEmphasis;
    break;
  case -2:
    iTextFlags &= ~CAGiliTyView::CharEmphasis;
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Output a string on the status line
/////////////////////////////////////////////////////////////////////////////
extern "C" void agt_statline(const char *s)
{
  iTextFlags |= CAGiliTyView::CharStatus;
  for (size_t i = 0; i < strlen(s); i++)
    Win32_SetCharacter(i,0,s[i]);
  iTextFlags &= ~CAGiliTyView::CharStatus;
}

/////////////////////////////////////////////////////////////////////////////
// Clear the screen and put the cursor at the upper left corner, below
// the status line
/////////////////////////////////////////////////////////////////////////////
extern "C" void agt_clrscr(void)
{
  for (int i = 0; i < screen_width*screen_height; i++)
    pScreenBuffer[i] = ' ';

  iCursorY = 1;
  iScrollCount = 0;
  curr_x = 0;

  if (DEBUG_OUT)
    fprintf(debugfile,"\n\n<CLRSCR>\n\n");
  if (script_on)
    fprintf(scriptfile,"\n\n\n\n");
}

extern "C" void agt_puts(const char *s)
{
  for (size_t i = 0; i < strlen(s); i++)
  {
    Win32_SetCharacter(curr_x,iCursorY,s[i]);
    if (curr_x < screen_width)
      curr_x++;
    if (curr_x >= screen_width)
      curr_x = 0;
  }

  if (DEBUG_OUT)
    fprintf(debugfile,"%s",s);
  if (script_on)
    fprintf(scriptfile,"%s",s);
}

extern "C" void agt_newline(void)
{
  if (pScreenBuffer == NULL)
  {
    curr_x = 0;
    return;
  }

  curr_x = iBoxX;

  if (iCursorY >= screen_height-1)
  {
    int i = 0;
    for (; i < screen_width*(screen_height-1); i++)
      pScreenBuffer[i] = pScreenBuffer[i+screen_width];
    for (i = 0; i < screen_width; i++)
      pScreenBuffer[(screen_width*(screen_height-1))+i] = ' ';
    iCursorY = screen_height-1;
  }
  else
    iCursorY++;

  if (bInBox == 0)
  {
    iScrollCount++;
    if (iScrollCount >= screen_height-1)
    {
      curr_x = 0;
      agt_puts("  --MORE--");
      agt_getkey(0);
      curr_x = 0;
      agt_puts("          ");
      curr_x = 0;
      iScrollCount = 0;
    }
  }

  if (DEBUG_OUT)
    fprintf(debugfile,"\n");
  if (script_on)
    fprintf(scriptfile,"\n");
}

/////////////////////////////////////////////////////////////////////////////
// Flags: TB_TTL, TB_BORDER
/////////////////////////////////////////////////////////////////////////////
extern "C" void agt_makebox(int width,int height,unsigned long flags)
{
  bInBox = true;
  if (flags & TB_BORDER)
    iTextFlags |= CAGiliTyView::CharStatus;
  iBoxX = flags & TB_NOCENT ? 0 : (screen_width-width)/2;
  curr_x = iBoxX;
  if (flags & TB_TTL)
    iCursorY = (screen_height-height)/4;
  else
    iCursorY -= screen_height-height;
  if (iCursorY < 0)
    iCursorY = 0;
}

extern "C" void agt_qnewline(void)
{
  agt_newline();
}

extern "C" void agt_endbox(void)
{
  bInBox = false;
  iTextFlags &= ~CAGiliTyView::CharStatus;
  iBoxX = 0;
  agt_newline();
}

/////////////////////////////////////////////////////////////////////////////
// If setflag is 0, then the option was prefixed with NO_
/////////////////////////////////////////////////////////////////////////////
extern "C" rbool agt_option(int optnum,char *optstr[],rbool setflag)
{
  return 0; 
}

extern "C" genfile agt_globalfile(int fid)
{
  genfile cfgFile = 0;

  if (fid == 0)
  {
    cfgFile = (genfile)fopen("agil.cfg","rt");
    if (cfgFile == 0)
    {
      // Get the full path to this executable
      CString cfgFileName;
      ::GetModuleFileName(NULL,cfgFileName.GetBuffer(_MAX_PATH),_MAX_PATH);
      cfgFileName.ReleaseBuffer();

      // Remove the name of the executable
      int last = cfgFileName.ReverseFind('\\');
      if (last == -1)
        last = cfgFileName.ReverseFind(':');
      if (last >= 0)
        cfgFileName = cfgFileName.Left(last+1);
      else
        cfgFileName.Empty();

      cfgFileName += "agil.cfg";
      cfgFile = (genfile)fopen(cfgFileName,"rt");
    }
  }
  return cfgFile;
}

extern "C" void init_interface(int argc,char *argv[])
{
  atexit(Win32_freeall);

  script_on = 0;
  scriptfile = 0;
  center_on = 0;
  par_fill_on = 0;
  debugfile = NULL;
  DEBUG_OUT = 0;
  PURE_INPUT = 0;

  screen_width = 80;
  Win32_SetScreenSize(FALSE);
  agt_clrscr();
  iTextFlags = (7 << 8);
}

extern "C" void start_interface(fc_type fc)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  if (stable_random)
    srand(6);
  else 
    srand(time(0));

  if (pApp->m_bFixColumns)
    screen_width = 80;
  Win32_SetScreenSize(FALSE);
  agt_clrscr();
  CAGiliTyView::GetView()->ResizeWindow();
}

extern "C" void close_interface(void)
{
  if (pScreenBuffer)
    delete pScreenBuffer;
  pScreenBuffer = NULL;

  if (scriptfile != NULL)
    close_pfile(scriptfile,0);
}

FILE *get_user_file(int ft)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);
  
  char *fname, *otype, *title, *ext;
  char *p, *q;
  int save;
  FILE *fd;
  LPCTSTR pszFilter = NULL;

  switch (ft)
  {
  case 0:
    title = "Script File";
    save = 1;
    otype = "a";
    pszFilter = "Script Files (*.scr)|*.scr|All Files (*.*)|*.*||";
    ext = "scr";
    break;
  case 1:
    title = "Save Game";
    save = 1;
    otype = "wb";
    pszFilter = "Saved Game Files (*.sav)|*.sav|All Files (*.*)|*.*||";
    ext = "sav";
    break;
  case 2:
    title = "Restore Game";
    save = 0;
    otype = "rb";
    pszFilter = "Saved Game Files (*.sav)|*.sav|All Files (*.*)|*.*||";
    ext = "sav";
    break;
  case 3:
    title = "Read Log";
    save = 0;
    otype = "r";
    pszFilter = "Log Files (*.log)|*.log|All Files (*.*)|*.*||";
    ext = "log";
    break;
  case 4:
    title = "Write Log";
    save = 1;
    otype = "w";
    pszFilter = "Log Files (*.log)|*.log|All Files (*.*)|*.*||";
    ext = "log";
    break;
  default:
    writeln("<INT ERR: invalid file type>");
    return NULL;
  }

  fname = (char *)rmalloc(256);

  SimpleFileDialog* pFileDlg;
  if (pszFilter == NULL)
    pszFilter = "All Files (*.*)|*.*||";
  if (save)
  {
    pFileDlg = new SimpleFileDialog(FALSE,ext,NULL,
      OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,pszFilter,NULL);
  }
  else
  {
    pFileDlg = new SimpleFileDialog(TRUE,ext,NULL,
      OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_ENABLESIZING,pszFilter,NULL);
  }
  pFileDlg->m_ofn.lpstrTitle = title;
  pFileDlg->m_ofn.lpstrInitialDir = pApp->m_strFilePath;

  if (pFileDlg->DoModal() != IDOK)
  {
    delete pFileDlg;
    rfree(fname);
    return NULL;
  }
  strcpy(fname,pFileDlg->GetPathName());
  int iPos = pFileDlg->GetPathName().ReverseFind('\\');
  if (iPos > 0)
    pApp->m_strFilePath = pFileDlg->GetPathName().Left(iPos);
  else
    pApp->m_strFilePath.Empty();
  delete pFileDlg;

  for(p = fname; isspace(*p); p++);
  for(q = fname; *p != 0; p++, q++)
    *q = *p;
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
  if (fd == NULL)
    writeln("Cannot open file.");
  rfree(fname);
  return fd;
}

void set_default_filenames(fc_type fc)
{
}
