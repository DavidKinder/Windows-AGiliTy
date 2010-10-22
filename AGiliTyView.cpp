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

extern "C" {
#include ".\generic\agility.h"
#include ".\generic\interp.h"
}

extern unsigned int *pScreenBuffer;
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
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

const int CAGiliTyView::CharEmphasis = 0x1000;
const int CAGiliTyView::CharStatus   = 0x2000;

CAGiliTyView::CAGiliTyView()
{
}

CAGiliTyView::~CAGiliTyView()
{
}

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
  char* pszLine = new char[screen_width];

  // Draw the current output
  for (i = 0; i < screen_height; i++)
  {
    unsigned int *pScreenLine = pScreenBuffer+(i*screen_width);

    for (j = 0; j < screen_width; j++)
      pszLine[j] = pScreenLine[j] & 0xFF;

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

      TextOutColour(pDC,iFontWidth*j,iLinePos,pszLine+j,iNextJ-j,TextColour,BackColour);
      j = iNextJ;
    }

    iLinePos += iFontHeight;
  }

  delete pszLine;

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

void CAGiliTyView::TextOutColour(CDC* pDC, int x, int y, LPCTSTR lpszString, int nCount, COLORREF TextColour, COLORREF BackColour)
{
  ASSERT(pDC);
  ASSERT(lpszString);

  pDC->SetTextColor(TextColour);
  pDC->SetBkColor(BackColour);
  pDC->TextOut(x,y,lpszString,nCount);
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
      CFont* pFont = new CFont();
      pFont->CreateFontIndirect(&(pApp->m_LogFont));

      CPaintDC DC(this);
      TEXTMETRIC FontInfo;
      CFont* pOldFont = DC.SelectObject(pFont);
      DC.GetTextMetrics(&FontInfo);
      int iFontWidth = (int)FontInfo.tmAveCharWidth;
      DC.SelectObject(pOldFont);
      delete pFont;

      // Work out the new frame width
      FrameRect.right += (screen_width*iFontWidth)-ViewRect.Width();

      // Check that the window is not off the screen
      int iScreenWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
      if (FrameRect.right > iScreenWidth)
        FrameRect.OffsetRect(-1*FrameRect.left,0);

      // Resize the frame
      pFrame->MoveWindow(FrameRect,TRUE);
    }
  }
}

BOOL CAGiliTyView::OnEraseBkgnd(CDC* pDC) 
{
  return 1;
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
