/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// MainFrm.h: Interface of frame class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBar.h"

#define DARKMODE_REGISTRY "Software\\David Kinder\\AGiliTy"

class CMainFrame : public MenuBarFrameWnd
{
protected: // create from serialization only
  CMainFrame();
  DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CMainFrame)
  public:
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL DestroyWindow();
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CMainFrame();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
  DarkModeStatusBar m_statusBar;

// Generated message map functions
protected:
  //{{AFX_MSG(CMainFrame)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnHelp();
  //}}AFX_MSG
  afx_msg LRESULT OnDpiChanged(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()

  int m_dpi;
};
