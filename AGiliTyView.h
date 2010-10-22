/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyView.h: Interface of view class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CAGiliTyView : public CView
{
protected: // create from serialization only
  CAGiliTyView();
  DECLARE_DYNCREATE(CAGiliTyView)

// Attributes
public:
  CAGiliTyDoc* GetDocument();

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAGiliTyView)
  public:
  virtual void OnDraw(CDC* pDC);  // overridden to draw this view
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CAGiliTyView();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
  //{{AFX_MSG(CAGiliTyView)
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  static CAGiliTyView* GetView(void);
  static int GetTextLine(unsigned int* pScreenLine, int iPosition);
  static void TextOutColour(CDC* pDC, int x, int y, LPCTSTR lpszString, int nCount, COLORREF TextColour, COLORREF BackColour);
  void ResizeWindow(void);

  static const int CharEmphasis;
  static const int CharStatus;

  CArray<int,int> m_iInput;
};

#ifndef _DEBUG  // debug version in AGiliTyView.cpp
inline CAGiliTyDoc* CAGiliTyView::GetDocument()
   { return (CAGiliTyDoc*)m_pDocument; }
#endif
