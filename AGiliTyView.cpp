/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyView.cpp: Implementation of view class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AGiliTy.h"
#include "AGiliTyDoc.h"
#include "AGiliTyView.h"

#include "DarkMode.h"
#include "DpiFunctions.h"

extern "C" {
#include ".\generic\agility.h"
#include ".\generic\interp.h"
}

extern unsigned int *pScreenBuffer;
extern bool bInput;

void Win32_CaretOn(int& iFontWidth, int& iFontHeight);
void Win32_CaretOff(void);
void Win32_SetScreenSize(BOOL bSetWidth);
void Win32_Redraw(int iTop = 0, int iLeft = 0);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAGiliTyView
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CAGiliTyView, CView)

BEGIN_MESSAGE_MAP(CAGiliTyView, CView)
  //{{AFX_MSG_MAP(CAGiliTyView)
  ON_WM_CHAR()
  ON_WM_KEYDOWN()
  ON_WM_SIZE()
  ON_WM_ERASEBKGND()
  ON_WM_NCPAINT()
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

const unsigned int CAGiliTyView::CharEmphasis = 0x1000;
const unsigned int CAGiliTyView::CharStatus   = 0x2000;

CAGiliTyView::CAGiliTyView()
{
}

CAGiliTyView::~CAGiliTyView()
{
}

static WCHAR CP437_to_Unicode[] =
{
  0x00C7,0x00FC,0x00E9,0x00E2,0x00E4,0x00E0,0x00E5,0x00E7,0x00EA,0x00EB,0x00E8,0x00EF,0x00EE,0x00EC,0x00C4,0x00C5,
  0x00C9,0x00E6,0x00C6,0x00F4,0x00F6,0x00F2,0x00FB,0x00F9,0x00FF,0x00D6,0x00DC,0x00A2,0x00A3,0x00A5,0x20A7,0x0192,
  0x00E1,0x00ED,0x00F3,0x00FA,0x00F1,0x00D1,0x00AA,0x00BA,0x00BF,0x2310,0x00AC,0x00BD,0x00BC,0x00A1,0x00AB,0x00BB,
  0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,0x2555,0x2563,0x2551,0x2557,0x255D,0x255C,0x255B,0x2510,
  0x2514,0x2534,0x252C,0x251C,0x2500,0x253C,0x255E,0x255F,0x255A,0x2554,0x2569,0x2566,0x2560,0x2550,0x256C,0x2567,
  0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256B,0x256A,0x2518,0x250C,0x2588,0x2584,0x258C,0x2590,0x2580,
  0x03B1,0x00DF,0x0393,0x03C0,0x03A3,0x03C3,0x00B5,0x03C4,0x03A6,0x0398,0x03A9,0x03B4,0x221E,0x03C6,0x03B5,0x2229,
  0x2261,0x00B1,0x2265,0x2264,0x2320,0x2321,0x00F7,0x2248,0x00B0,0x2219,0x00B7,0x221A,0x207F,0x00B2,0x25A0,0x00A0
};

void CAGiliTyView::OnDraw(CDC* pDrawDC)
{
  CAGiliTyDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  if (pScreenBuffer == NULL)
    return;

  CDC* pDC = new CDC();
  pDC->CreateCompatibleDC(pDrawDC);

  CRect Client;
  GetClientRect(Client);

  CBitmap bmp;
  CSize size(Client.Width(),Client.Height());
  if (bmp.CreateCompatibleBitmap(pDrawDC,size.cx,size.cy) == FALSE)
  {
    delete pDC;
    return;
  }
  CBitmap* pbmpOld = pDC->SelectObject(&bmp);

  pDC->FillSolidRect(Client,pApp->GetBackColour());

  CFont* pOldFont = NULL;
  TEXTMETRIC FontInfo;

  // Set up the font
  CFont* pTextFont = new CFont();
  pTextFont->CreateFontIndirect(&(pApp->m_LogFont));
  pOldFont = pDC->SelectObject(pTextFont);
  pDC->GetTextMetrics(&FontInfo);
  int iFontWidth = (int)FontInfo.tmAveCharWidth;
  int iFontHeight = (int)FontInfo.tmHeight;

  pDC->SetTextAlign(TA_TOP|TA_LEFT);

  int iLinePos = 0;
  int i = 0;
  int j = 0;
  WCHAR* line = new WCHAR[screen_width];

  // Draw the current output
  for (i = 0; i < screen_height; i++)
  {
    unsigned int *pScreenLine = pScreenBuffer+(i*screen_width);

    for (j = 0; j < screen_width; j++)
    {
      line[j] = pScreenLine[j] & 0xFF;
      if (!fix_ascii_flag && (line[j] >= 0x80))
      {
        if (FontInfo.tmPitchAndFamily & TMPF_TRUETYPE)
          line[j] = CP437_to_Unicode[line[j]-0x80];
        else
          line[j] = trans_ibm[line[j]-0x80];
      }
    }

    j = 0;
    int iNextJ;
    COLORREF TextColour, BackColour;

    while (j < screen_width)
    {
      iNextJ = GetTextLine(pScreenLine,j);

      // Set text and background colours
      if (pScreenLine[j] & CharStatus)
        pApp->GetStatusColours(TextColour,BackColour);
      else
      {
        TextColour = pApp->GetTextColour(
          (pScreenLine[j] & 0x0F00) >> 8,pScreenLine[j] & CharEmphasis);
        BackColour = pApp->GetBackColour();
      }

      pDC->SetTextColor(TextColour);
      pDC->SetBkColor(BackColour);
      ::TextOutW(pDC->GetSafeHdc(),iFontWidth*j,iLinePos,line+j,iNextJ-j);

      j = iNextJ;
    }

    iLinePos += iFontHeight;
  }

  delete line;

  // Remove the font
  pDC->SelectObject(pOldFont);
  delete pTextFont;

  pDrawDC->BitBlt(0,0,size.cx,size.cy,pDC,0,0,SRCCOPY);
  pDC->SelectObject(pbmpOld);
  delete pDC;
}

void CAGiliTyView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if (pScreenBuffer)
  {
    // Add to the input buffer
    m_iInput.Add(nChar & 0xFF);
  }
  CView::OnChar(nChar, nRepCnt, nFlags);
}


void CAGiliTyView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  if (pScreenBuffer)
  {
    switch (nChar)
    {
    case VK_LEFT:    // Cursor left
    case VK_RIGHT:  // Cursor right
    case VK_UP:      // Cursor up
    case VK_DOWN:    // Cursor down
    case VK_HOME:    // Home
    case VK_END:    // End
    case VK_DELETE:  // Delete
      m_iInput.Add(nChar + 0x100);
      break;
    }
  }
  CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAGiliTyView::OnSize(UINT nType, int cx, int cy) 
{
  CView::OnSize(nType, cx, cy);
  Win32_SetScreenSize(TRUE);
  Win32_Redraw();
}

CAGiliTyView* CAGiliTyView::GetView(void)
{
  CFrameWnd* pFrame = (CFrameWnd*)(AfxGetApp()->m_pMainWnd);
  if (pFrame == NULL)
    return NULL;
  CView* pView = pFrame->GetActiveView();
  if (pView == NULL)
    return NULL;

  // Fail if view is of wrong kind
  if (!pView->IsKindOf(RUNTIME_CLASS(CAGiliTyView)))
    return NULL;

  return (CAGiliTyView*)pView;
}

int CAGiliTyView::GetTextLine(unsigned int* pScreenLine, int iPosition)
{
  unsigned int iCharInfo = pScreenLine[iPosition] & 0xFF00;
  BOOL bSearch = TRUE;

  while (bSearch)
  {
    if (iPosition < screen_width)
    {
      if ((pScreenLine[iPosition] & 0xFF00) == iCharInfo)
        iPosition++;
      else
        bSearch = FALSE;
    }
    else
      bSearch = FALSE;
  }

  return iPosition;
}

void CAGiliTyView::ResizeWindow(void)
{
  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  CFrameWnd* pFrame = (CFrameWnd*)GetParent();
  if (pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)) == FALSE)
    pFrame = NULL;

  if (pApp->m_bFixColumns && pFrame && (screen_width > 0))
  {
    // Only resize if frame not maximized
    WINDOWPLACEMENT Place;
    pFrame->GetWindowPlacement(&Place);
    if (Place.showCmd != SW_SHOWMAXIMIZED)
    {
      CRect ViewRect, FrameRect;

      // Get the size of the client view area
      GetClientRect(ViewRect);

      // Get the window size of the parent frame
      pFrame->GetWindowRect(FrameRect);

      // Create an instance of the current font
      CFont Font;
      Font.CreateFontIndirect(&(pApp->m_LogFont));

      CPaintDC DC(this);
      TEXTMETRIC FontInfo;
      CFont* pOldFont = DC.SelectObject(&Font);
      DC.GetTextMetrics(&FontInfo);
      int iFontWidth = (int)FontInfo.tmAveCharWidth;
      DC.SelectObject(pOldFont);

      // Work out the new frame width
      FrameRect.right += (screen_width*iFontWidth)-ViewRect.Width();

      // Check that the window is not off the screen
      CRect MonRect = DPI::getMonitorRect(pFrame);
      if (FrameRect.right > MonRect.right)
      {
        int offsetLeft = FrameRect.left-MonRect.left;
        int offsetRight = FrameRect.right-MonRect.right;
        if (offsetRight > offsetLeft)
          offsetRight = offsetLeft;
        if (offsetRight  > 0)
          FrameRect.OffsetRect(-1*offsetRight,0);
      }

      // Resize the frame
      pFrame->MoveWindow(FrameRect,TRUE);
    }
  }
}

BOOL CAGiliTyView::OnEraseBkgnd(CDC* pDC) 
{
  return 1;
}

void CAGiliTyView::OnNcPaint()
{
  DarkMode* dark = DarkMode::GetActive(this);
  if (dark)
  {
    CWindowDC dc(this);
    CRect r = dark->PrepareNonClientBorder(this,dc);
    dc.FillSolidRect(r,dark->GetColour(DarkMode::Dark3));
    dc.SelectClipRgn(NULL);
  }
  else
    Default();
}

void CAGiliTyView::OnKillFocus(CWnd* pNewWnd)
{
  Win32_CaretOff();
  CView::OnKillFocus(pNewWnd);
}

void CAGiliTyView::OnSetFocus(CWnd* pOldWnd)
{
  CView::OnSetFocus(pOldWnd);
  if (bInput)
  {
    int iFontWidth, iFontHeight;
    Win32_CaretOn(iFontWidth,iFontHeight);
  }
}

#ifdef _DEBUG
void CAGiliTyView::AssertValid() const
{
  CView::AssertValid();
}

void CAGiliTyView::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}

CAGiliTyDoc* CAGiliTyView::GetDocument() // non-debug version is inline
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAGiliTyDoc)));
  return (CAGiliTyDoc*)m_pDocument;
}
#endif //_DEBUG
