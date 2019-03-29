/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTy.cpp: Implementation of application class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AGiliTy.h"
#include "AGiliTyDoc.h"
#include "AGiliTyOptDlg.h"
#include "AGiliTyView.h"
#include "MainFrm.h"
#include "Dialogs.h"
#include "DSoundEngine.h"

#include <shlobj.h>

extern "C"
{
#include ".\generic\agility.h"
#include ".\generic\interp.h"
void set_default_options(void);
}

extern BOOL bResetCursor;
extern unsigned int *pScreenBuffer;
extern CMap<void*,void*,int,int> MemoryMap;
void __cdecl Win32_freeall(void);
void Win32_SetScreenSize(BOOL bSetWidth);
void Win32_Redraw(int iTop = 0, int iLeft = 0);

BOOL AFXAPI _AfxSetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);
void AFXAPI AfxGetModuleShortFileName(HINSTANCE hInst, CString& strShortName);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAGiliTyApp
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CAGiliTyApp, CWinApp)
  //{{AFX_MSG_MAP(CAGiliTyApp)
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
  ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
  ON_COMMAND(ID_VIEW_FONT, OnViewFont)
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  //}}AFX_MSG_MAP
  // Standard file based document commands
  ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
  ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

CAGiliTyApp::CAGiliTyApp()
{
  EnableHtmlHelp();
}

CAGiliTyApp theApp;

BOOL CAGiliTyApp::InitInstance()
{
  // Read in user settings
  SetRegistryKey(_T("David Kinder"));
  LoadStdProfileSettings();

  int fontSize = GetProfileInt("Display","Font Size",10);
  if (fontSize < 0)
    m_iFontPoints = -MulDiv(fontSize,72,DPI::getSystemDPI());
  else
    m_iFontPoints = fontSize;
  m_LogFont.lfHeight = -MulDiv(m_iFontPoints,DPI::getSystemDPI(),72);
  m_LogFont.lfCharSet = ANSI_CHARSET;
  m_LogFont.lfOutPrecision = OUT_TT_PRECIS;
  m_LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  m_LogFont.lfQuality = PROOF_QUALITY;
  m_LogFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
  strncpy(m_LogFont.lfFaceName,
    GetProfileString("Display","Font Name","Fixedsys"),LF_FACESIZE);

  m_bColours = GetProfileInt("Colours","Enabled",TRUE);
  m_TextColour = GetProfileInt("Colours","Text",RGB(0xFF,0xFF,0xFF));
  m_BackColour = GetProfileInt("Colours","Background",RGB(0x00,0x00,0x00));
  m_EmphasisColour = GetProfileInt("Colours","Emphasis",RGB(0xFF,0xFF,0x00));
  m_StatusColour = GetProfileInt("Colours","Status",RGB(0x00,0x00,0xFF));
  
  m_WindowRect.left = GetProfileInt("Window","Left",0);
  m_WindowRect.top = GetProfileInt("Window","Top",0);
  m_WindowRect.right = GetProfileInt("Window","Right",0);
  m_WindowRect.bottom = GetProfileInt("Window","Bottom",0);
  m_iWindowMax = GetProfileInt("Window","Maximized",0);
  if (m_iWindowMax)
    m_nCmdShow = SW_SHOWMAXIMIZED;
  m_toolBar = GetProfileInt("Window","Toolbar",1) ? TRUE : FALSE;
  m_statusBar = GetProfileInt("Window","Status Bar",1) ? TRUE : FALSE;
  m_bFixColumns = GetProfileInt("Window","Fix columns",TRUE);

  m_strFilePath =  GetProfileString("Files","File Path","");

  // Register document template
  CSingleDocTemplate* pDocTemplate;
  pDocTemplate = new CSingleDocTemplate(
    IDR_MAINFRAME,
    RUNTIME_CLASS(CAGiliTyDoc),
    RUNTIME_CLASS(CMainFrame),
    RUNTIME_CLASS(CAGiliTyView));
  AddDocTemplate(pDocTemplate);

  EnableShellOpen();
  RegisterShellFileTypes(FALSE);

  // Set up the icon for AGX game files
  CString path, type, key, value;
  AfxGetModuleShortFileName(AfxGetInstanceHandle(),path);
  pDocTemplate->GetDocString(type,CDocTemplate::regFileTypeId);
  key.Format("%s\\DefaultIcon",type);
  value.Format("%s,%d",path,0);
  _AfxSetRegKey(key,value);

  // Notify the shell that associations have changed
  ::SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,0,0);

  // Parse command line for standard shell commands, DDE, file open
  CCommandLineInfo cmdInfo;
  ParseCommandLine(cmdInfo);

  // Dispatch commands specified on the command line
  if (!ProcessShellCommand(cmdInfo))
    return FALSE;

  // Create font dialog
  m_pFontDialog = new DPI::FontDialog(&m_LogFont,
    CF_SCREENFONTS|CF_FIXEDPITCHONLY,m_pMainWnd);
  if (m_pFontDialog == NULL)
    return FALSE;

  m_pMainWnd->ShowWindow(SW_SHOW);
  m_pMainWnd->UpdateWindow();

  MemoryMap.InitHashTable(997);
  return TRUE;
}

int CAGiliTyApp::ExitInstance() 
{
  // Write out settings

  WriteProfileString("Display","Font Name",CString(m_LogFont.lfFaceName));
  WriteProfileInt("Display","Font Size",m_iFontPoints);

  WriteProfileInt("Colours","Enabled",m_bColours);
  WriteProfileInt("Colours","Text",m_TextColour);
  WriteProfileInt("Colours","Background",m_BackColour);
  WriteProfileInt("Colours","Emphasis",m_EmphasisColour);
  WriteProfileInt("Colours","Status",m_StatusColour);

  WriteProfileInt("Window","Left",m_WindowRect.left);
  WriteProfileInt("Window","Top",m_WindowRect.top);
  WriteProfileInt("Window","Right",m_WindowRect.right);
  WriteProfileInt("Window","Bottom",m_WindowRect.bottom);
  WriteProfileInt("Window","Maximized",m_iWindowMax);
  WriteProfileInt("Window","Toolbar",m_toolBar ? 1 : 0);
  WriteProfileInt("Window","Status Bar",m_statusBar ? 1 : 0);
  WriteProfileInt("Window","Fix Columns",m_bFixColumns);

  WriteProfileString("Files","File Path",m_strFilePath);

  CDSoundEngine::GetSoundEngine().Destroy();

  // Free memory
  delete m_pFontDialog;
  if (pScreenBuffer)
    delete pScreenBuffer;

  return CWinApp::ExitInstance();
}

BOOL CAGiliTyApp::OnIdle(LONG lCount) 
{
  static bool bRunning = false;

  if ((bRunning == false) && (m_strGameFile.IsEmpty() == FALSE))
  {
    bRunning = true;

    try
    {
      CString strGame;
      int iExtPos = m_strGameFile.ReverseFind('.');
      if (iExtPos > 0)
        strGame = m_strGameFile.Left(iExtPos);
      else
        strGame = m_strGameFile;

      set_default_options();
      init_interface(0,NULL);
      run_game(init_file_context(strGame.GetBuffer(256),fNONE));
      strGame.ReleaseBuffer();
    }
    catch (CUserException* pEx)
    {
      pEx->Delete();
      CAGiliTyView* pView = CAGiliTyView::GetView();
      if (pView)
        pView->Invalidate();
      AfxMessageBox("A fatal error has occured.",MB_ICONEXCLAMATION);
    }

    if (m_pMainWnd)
    {
      quitflag = 0;
      bRunning = false;
      m_strGameFile.Empty();
      OnFileNew();
      Win32_SetScreenSize(TRUE);
      Win32_Redraw();
    }
    else
    {
      agt_quit();
      quitflag = 1;
    }
  }

  return CWinApp::OnIdle(lCount);
}

void CAGiliTyApp::OnFileOpen() 
{
  SimpleFileDialog FileOpenDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_ENABLESIZING,
    "AGT Game Files (*.da1;*.agx)|*.da1;*.agx|All Files (*.*)|*.*||",AfxGetMainWnd());
  FileOpenDlg.m_ofn.lpstrTitle = "Open an AGT Game";
  FileOpenDlg.m_ofn.lpstrInitialDir = m_strFilePath;

  if (FileOpenDlg.DoModal() == IDOK)
  {
    OpenDocumentFile(FileOpenDlg.GetPathName());
    int iPos = FileOpenDlg.GetPathName().ReverseFind('\\');
    if (iPos > 0)
      m_strFilePath = FileOpenDlg.GetPathName().Left(iPos);
    else
      m_strFilePath.Empty();
  }
  bResetCursor = TRUE;
}

void CAGiliTyApp::OnViewOptions() 
{
  COptionsDlg Dialog;
  if (Dialog.DoOptionsModal(m_bColours,
    m_TextColour,m_BackColour,
    m_EmphasisColour,m_StatusColour,
    m_bFixColumns) == IDOK)
  {
    CAGiliTyView* pView = CAGiliTyView::GetView();
    if (pView)
      CAGiliTyView::GetView()->Invalidate();
  }
  bResetCursor = TRUE;
}

void CAGiliTyApp::OnViewFont() 
{
  // Change the font
  if (m_pFontDialog->DoModal() == IDOK)
  {
    m_iFontPoints = m_pFontDialog->m_cf.iPointSize / 10;

    CAGiliTyView* pView = CAGiliTyView::GetView();
    if (pView)
    {
      pView->ResizeWindow();
      Win32_SetScreenSize(TRUE);
    }
  }
  bResetCursor = TRUE;
}

COLORREF CAGiliTyApp::GetTextColour(int iColour, BOOL bEmphasis)
{
  COLORREF Colour;

  if (m_bColours)
  {
    if (bEmphasis)
    {
      switch (iColour)
      {
      case 0:
      default:
        Colour = m_EmphasisColour;
        break;
      case 1:
        Colour = RGB(0x60,0x60,0x60);
        break;
      case 2:
        Colour = RGB(0x50,0x50,0xFF);
        break;
      case 3:
        Colour = RGB(0x50,0xFF,0x50);
        break;
      case 4:
        Colour = RGB(0x50,0xFF,0xFF);
        break;
      case 5:
        Colour = RGB(0xFF,0x50,0x50);
        break;
      case 6:
        Colour = RGB(0xFF,0x50,0xFF);
        break;
      case 7:
        Colour = RGB(0xFF,0xFF,0x00);
        break;
      case 8:
        Colour = RGB(0xFF,0xFF,0xFF);
        break;
      }
    }
    else
    {
      switch (iColour)
      {
      case 0:
      default:
        Colour = m_TextColour;
        break;
      case 1:
        Colour = RGB(0x00,0x00,0x00);
        break;
      case 2:
        Colour = RGB(0x00,0x00,0xB0);
        break;
      case 3:
        Colour = RGB(0x00,0xB0,0x00);
        break;
      case 4:
        Colour = RGB(0x00,0xB0,0xB0);
        break;
      case 5:
        Colour = RGB(0xB0,0x00,0x00);
        break;
      case 6:
        Colour = RGB(0xB0,0x00,0xB0);
        break;
      case 7:
        Colour = RGB(0xC0,0x60,0x00);
        break;
      case 8:
        Colour = RGB(0xC0,0xC0,0xC0);
        break;
      }
    }
  }
  else
    Colour = ::GetSysColor(COLOR_WINDOWTEXT);

  return Colour;
}

COLORREF CAGiliTyApp::GetBackColour(void)
{
  COLORREF Colour;

  if (m_bColours)
    Colour = m_BackColour;
  else
    Colour = ::GetSysColor(COLOR_WINDOW);
  return Colour;
}

void CAGiliTyApp::GetStatusColours(COLORREF& StatusText, COLORREF& StatusBack)
{
  if (m_bColours)
  {
    StatusText = m_EmphasisColour;
    StatusBack = m_StatusColour;
  }
  else
  {
    StatusText = ::GetSysColor(COLOR_WINDOW);
    StatusBack = ::GetSysColor(COLOR_WINDOWTEXT);
  }
}

void CAGiliTyApp::OnAppAbout()
{
  BaseDialog aboutDlg(IDD_ABOUTBOX);
  aboutDlg.DoModal();
  bResetCursor = TRUE;
}
