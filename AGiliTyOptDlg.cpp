/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyOptDlg.cpp: Implementation of options dialog class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AGiliTy.h"
#include "AGiliTyOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog
/////////////////////////////////////////////////////////////////////////////

COptionsDlg::COptionsDlg(CWnd* pParent) : BaseDialog(COptionsDlg::IDD, pParent)
{
  //{{AFX_DATA_INIT(COptionsDlg)
  m_bFixColumns = FALSE;
  //}}AFX_DATA_INIT
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  BaseDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(COptionsDlg)
  DDX_Check(pDX, IDC_SIZE_WINDOW, m_bFixColumns);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsDlg, BaseDialog)
  //{{AFX_MSG_MAP(COptionsDlg)
  ON_BN_CLICKED(IDC_COLOURS, OnColoursOnOff)
  ON_WM_DESTROY()
  ON_WM_HELPINFO()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog() 
{
  if (!BaseDialog::OnInitDialog())
    return FALSE;
  
  // Subclass the colour buttons
  m_Text.SubclassDlgItem(IDC_TEXT,this);
  m_Back.SubclassDlgItem(IDC_BACK,this);
  m_Emphasis.SubclassDlgItem(IDC_EMPHASIS,this);
  m_Status.SubclassDlgItem(IDC_STATUS,this);

  // Subclass the controls for dark mode
  m_WindowGroup.SubclassDlgItem(IDC_STATIC_WINDOW,this);
  m_ColourGroup.SubclassDlgItem(IDC_STATIC_COLOUR,this);
  m_SizeCheck.SubclassDlgItem(IDC_SIZE_WINDOW,this,IDR_DARK_CHECK);
  m_ColoursCheck.SubclassDlgItem(IDC_COLOURS,this,IDR_DARK_CHECK);
  m_TextLabel.SubclassDlgItem(IDCS_TEXT,this);
  m_BackLabel.SubclassDlgItem(IDCS_BACK,this);
  m_EmphasisLabel.SubclassDlgItem(IDCS_EMPHASIS,this);
  m_StatusLabel.SubclassDlgItem(IDCS_STATUS,this);
  m_OK.SubclassDlgItem(IDOK,this);
  m_Cancel.SubclassDlgItem(IDCANCEL,this);

  m_ColoursCheck.SetCheck(m_bColours);
  SetColBtnState();

  // Set up context sensitive help
  GetDlgItem(IDC_STATIC_WINDOW)->SetWindowContextHelpId(0x10001);
  GetDlgItem(IDC_SIZE_WINDOW)->SetWindowContextHelpId(0x10000);
  GetDlgItem(IDC_STATIC_COLOUR)->SetWindowContextHelpId(0x10002);
  GetDlgItem(IDC_COLOURS)->SetWindowContextHelpId(0x10003);
  GetDlgItem(IDC_TEXT)->SetWindowContextHelpId(0x10004);
  GetDlgItem(IDC_BACK)->SetWindowContextHelpId(0x10005);
  GetDlgItem(IDC_EMPHASIS)->SetWindowContextHelpId(0x10006);
  GetDlgItem(IDC_STATUS)->SetWindowContextHelpId(0x10007);
  
  return TRUE;
}

void COptionsDlg::OnDestroy() 
{
  m_bColours = m_ColoursCheck.GetCheck();
  BaseDialog::OnDestroy();
}

int COptionsDlg::DoOptionsModal(BOOL& bEnabled,
    COLORREF& TextColour, COLORREF& BackColour,
    COLORREF& EmphasisColour, COLORREF& StatusColour,
    BOOL& bFixColumns)
{
  m_bColours = bEnabled;
  m_Text.SetCurrentColour(TextColour);
  m_Back.SetCurrentColour(BackColour);
  m_Emphasis.SetCurrentColour(EmphasisColour);
  m_Status.SetCurrentColour(StatusColour);
  m_bFixColumns = bFixColumns;

  int iStatus = BaseDialog::DoModal();
  if (iStatus == IDOK)
  {
    bEnabled = m_bColours;
    TextColour = m_Text.GetCurrentColour();
    BackColour = m_Back.GetCurrentColour();
    EmphasisColour = m_Emphasis.GetCurrentColour();
    StatusColour = m_Status.GetCurrentColour();
    bFixColumns = m_bFixColumns;
  }
  return iStatus;
}

HWND WINAPI AfxHtmlHelp(HWND hWnd, LPCTSTR szHelpFilePath, UINT nCmd, DWORD_PTR dwData);

BOOL COptionsDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
  static DWORD helpIds[] =
  {
    IDC_SIZE_WINDOW,1,
    IDC_COLOURS,2,
    IDC_TEXT,3,
    IDCS_TEXT,3,
    IDC_EMPHASIS,4,
    IDCS_EMPHASIS,4,
    IDC_BACK,5,
    IDCS_BACK,5,
    IDC_STATUS,6,
    IDCS_STATUS,6,
    0,0
  };

  if (pHelpInfo->iContextType == HELPINFO_WINDOW)
  {
    // Is there a help topic for this control?
    DWORD* id = helpIds;
    while (*id != 0)
    {
      if (pHelpInfo->iCtrlId == *id)
      {
        CString helpFile(AfxGetApp()->m_pszHelpFilePath);
        helpFile.Append("::/options.txt");

        // Show the help popup
        AfxHtmlHelp((HWND)pHelpInfo->hItemHandle,helpFile,
          HH_TP_HELP_WM_HELP,(DWORD_PTR)helpIds);
        return TRUE;
      }
      id += 2;
    }
    AfxGetApp()->HtmlHelp(0,HH_HELP_FINDER);
  }
  return TRUE;
}

void COptionsDlg::OnColoursOnOff() 
{
  SetColBtnState();
}

void COptionsDlg::SetColBtnState(void)
{
  BOOL bEnabled = m_ColoursCheck.GetCheck();

  m_Text.EnableWindow(bEnabled);
  m_Back.EnableWindow(bEnabled);
  m_Emphasis.EnableWindow(bEnabled);
  m_Status.EnableWindow(bEnabled);

  m_TextLabel.EnableWindow(bEnabled);
  m_BackLabel.EnableWindow(bEnabled);
  m_EmphasisLabel.EnableWindow(bEnabled);
  m_StatusLabel.EnableWindow(bEnabled);
}
