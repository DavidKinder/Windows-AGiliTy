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

#include "AGiliTyDlg.h"
#include "ColourButton.h"
#include "DarkMode.h"
#include "Dialogs.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog
/////////////////////////////////////////////////////////////////////////////

class COptionsDlg : public CAGiliTyDlg
{
// Construction
public:
  COptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
  //{{AFX_DATA(COptionsDlg)
  enum { IDD = IDD_OPTIONS };
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
  DarkModeGroupBox m_WindowGroup, m_ColourGroup;
  DarkModeCheckButton m_SizeCheck, m_ColoursCheck;
  DarkModeStatic m_TextLabel, m_BackLabel, m_EmphasisLabel, m_StatusLabel;
  ColourButton m_Text, m_Back, m_Emphasis, m_Status;
  DarkModeButton m_OK, m_Cancel;
};
