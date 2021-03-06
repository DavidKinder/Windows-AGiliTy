/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyOptDlg.h: Interface of options dialog class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ColourButton.h"
#include "Dialogs.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog
/////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public BaseDialog
{
// Construction
public:
  COptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(COptionsDlg)
  enum { IDD = IDD_OPTIONS };
  CButton  m_Colours;
  BOOL  m_bFixColumns;
  //}}AFX_DATA

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(COptionsDlg)
  protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

// Implementation
protected:

  // Generated message map functions
  //{{AFX_MSG(COptionsDlg)
  virtual BOOL OnInitDialog();
  afx_msg void OnColoursOnOff();
  afx_msg void OnDestroy();
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

public:
  int DoOptionsModal(BOOL& bEnabled,
    COLORREF& TextColour, COLORREF& BackColour,
    COLORREF& EmphasisColour, COLORREF& StatusColour,
    BOOL& bFixColumns);

protected:
  void SetColBtnState(void);

  BOOL m_bColours;
  ColourButton m_Text, m_Back, m_Emphasis, m_Status;
};
