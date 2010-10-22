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
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMainFrame, MenuBarFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, MenuBarFrameWnd)
  //{{AFX_MSG_MAP(CMainFrame)
  ON_WM_CREATE()
  ON_COMMAND(ID_HELP_HELPTOPICS, OnHelp)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
  ID_SEPARATOR,           // status line indicator
  ID_INDICATOR_CAPS,
  ID_INDICATOR_NUM,
  ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  if (MenuBarFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  
  if (!CreateBar(IDR_MAINFRAME,IDB_TOOLBAR32))
    return -1;

  if (!m_statusBar.Create(this) ||
      !m_statusBar.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT)))
  {
    TRACE0("Failed to create status bar\n");
    return -1;
  }

  ShowControlBar(&m_toolBar,pApp->m_toolBar,TRUE);
  ShowControlBar(&m_statusBar,pApp->m_statusBar,TRUE);
  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  cs.style &= ~FWS_ADDTOTITLE;

  CRect& rPlace = pApp->m_WindowRect;
  if (rPlace.Width() > 0)
  {
    cs.x = rPlace.left;
    cs.y = rPlace.top;
    cs.cx = rPlace.Width();
    cs.cy = rPlace.Height();
  }

  return MenuBarFrameWnd::PreCreateWindow(cs);
}

BOOL CMainFrame::DestroyWindow() 
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  // Save the window state
  WINDOWPLACEMENT Place;
  GetWindowPlacement(&Place);
  pApp->m_iWindowMax = (Place.showCmd == SW_SHOWMAXIMIZED);

  // Save the window position and size
  GetWindowRect(pApp->m_WindowRect);

  // Save the toolbar and status bar
  pApp->m_toolBar = m_toolBar.GetStyle() & WS_VISIBLE;
  pApp->m_statusBar = m_statusBar.GetStyle() & WS_VISIBLE;

  return MenuBarFrameWnd::DestroyWindow();
}

void CMainFrame::OnHelp() 
{
  HtmlHelp(0,HH_HELP_FINDER);
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
