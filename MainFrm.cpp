/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// MainFrm.cpp: Implementation of frame class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AGiliTy.h"
#include "AGiliTyDlg.h"
#include "MainFrm.h"
#include "DpiFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern BOOL bResetCursor;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMainFrame, MenuBarFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, MenuBarFrameWnd)
  //{{AFX_MSG_MAP(CMainFrame)
  ON_WM_CREATE()
  ON_WM_SETTINGCHANGE()
  ON_COMMAND(ID_HELP_HELPTOPICS, OnHelp)
  //}}AFX_MSG_MAP
  ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

static UINT indicators[] =
{
  ID_SEPARATOR,           // status line indicator
  ID_INDICATOR_CAPS,
  ID_INDICATOR_NUM,
  ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame() : m_dpi(96), m_modalDialog(NULL)
{
}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::SetModalDialog(CWnd* dialog)
{
  m_modalDialog = dialog;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  m_dpi = DPI::getWindowDPI(this);

  // Now we have a window, set the font height
  pApp->m_LogFont.lfHeight = -MulDiv(pApp->m_iFontPoints,m_dpi,72);

  // Restore the window size and position from DPI neutral values
  CRect rPlace = pApp->m_WindowRect;
  if (rPlace.Width() > 0)
  {
    DPI::ContextUnaware dpiUnaware;
    MoveWindow(rPlace);
  }

  if (MenuBarFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  
  if (!CreateNewBar(IDR_MAINFRAME,IDR_TOOLBAR))
    return -1;

  if (!m_statusBar.Create(this) ||
      !m_statusBar.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT)))
  {
    TRACE0("Failed to create status bar\n");
    return -1;
  }

  ShowControlBar(&m_toolBar,pApp->m_toolBar,TRUE);
  ShowControlBar(&m_statusBar,pApp->m_statusBar,TRUE);

  // Turn on dark mode, if needed
  SetDarkMode(DarkMode::GetEnabled(DARKMODE_REGISTRY));
  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style &= ~FWS_ADDTOTITLE;
  return MenuBarFrameWnd::PreCreateWindow(cs);
}

BOOL CMainFrame::DestroyWindow() 
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  // Save the window state
  WINDOWPLACEMENT Place;
  {
    DPI::ContextUnaware dpiUnaware;
    GetWindowPlacement(&Place);
  }
  pApp->m_iWindowMax = (Place.showCmd == SW_SHOWMAXIMIZED);
  pApp->m_WindowRect = Place.rcNormalPosition;

  // Save the toolbar and status bar
  pApp->m_toolBar = m_toolBar.GetStyle() & WS_VISIBLE;
  pApp->m_statusBar = m_statusBar.GetStyle() & WS_VISIBLE;

  return MenuBarFrameWnd::DestroyWindow();
}

void CMainFrame::OnHelp() 
{
  HtmlHelp(0,HH_HELP_FINDER);
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
  MenuBarFrameWnd::OnSettingChange(uFlags,lpszSection);

  if ((m_dark != NULL) != DarkMode::IsEnabled(DARKMODE_REGISTRY))
  {
    SetDarkMode(DarkMode::GetEnabled(DARKMODE_REGISTRY));
    if (m_modalDialog != NULL)
    {
      if (m_modalDialog->IsKindOf(RUNTIME_CLASS(CAGiliTyDlg)))
        ((CAGiliTyDlg*)m_modalDialog)->SetDarkMode(DarkMode::GetActive(m_modalDialog));
    }

    if (m_dark != NULL)
      DarkMode::SetAppDarkMode();
  }
}

LRESULT CMainFrame::OnDpiChanged(WPARAM wparam, LPARAM lparam)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  int newDpi = (int)HIWORD(wparam);
  if (m_dpi != newDpi)
  {
    CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
    pApp->m_LogFont.lfHeight = -MulDiv(pApp->m_iFontPoints,newDpi,72);
    m_dpi = newDpi;
  }

  bResetCursor = TRUE;
  MoveWindow((LPRECT)lparam,TRUE);

  // Force the menu and status bars to update
  UpdateDPI(newDpi);
  m_statusBar.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT));
  return 0;
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
  MenuBarFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
  MenuBarFrameWnd::Dump(dc);
}

#endif //_DEBUG
