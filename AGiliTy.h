/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTy.h: Interface of application class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
  #error include 'stdafx.h' before including this file for PCH
#endif

#include "DpiFunctions.h"
#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAGiliTyApp:
// See AGiliTy.cpp for the implementation of this class
/////////////////////////////////////////////////////////////////////////////

class CAGiliTyApp : public CWinApp
{
public:
  CAGiliTyApp();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAGiliTyApp)
  public:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
  virtual BOOL OnIdle(LONG lCount);
  //}}AFX_VIRTUAL

// Implementation

  //{{AFX_MSG(CAGiliTyApp)
  afx_msg void OnAppAbout();
  afx_msg void OnViewOptions();
  afx_msg void OnViewFont();
  afx_msg void OnFileOpen();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  CString GetDefaultFixedFont(void);

  COLORREF GetTextColour(int iColour, BOOL bEmphasis);
  COLORREF GetBackColour(void);
  void GetStatusColours(COLORREF& StatusText, COLORREF& StatusBack);

  CRect m_WindowRect;
  int m_iWindowMax;

  BOOL m_toolBar;
  BOOL m_statusBar;

  LOGFONT m_LogFont;
  int m_iFontPoints;
  DPI::FontDialog* m_pFontDialog;

  CString m_strGameFile;
  CString m_strFilePath;

  BOOL m_bFixColumns;

protected:
  BOOL m_bColours;
  COLORREF m_TextColour;
  COLORREF m_BackColour;
  COLORREF m_EmphasisColour;
  COLORREF m_StatusColour;
};
