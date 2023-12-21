/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyDlg.cpp: Base dialog class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "MainFrm.h"
#include "AGiliTyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Implementation of the CAGiliTyDlg dialog
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CAGiliTyDlg, BaseDialog)

BEGIN_MESSAGE_MAP(CAGiliTyDlg, BaseDialog)
END_MESSAGE_MAP()

CAGiliTyDlg::CAGiliTyDlg(UINT templateId, CWnd* parent) : BaseDialog(templateId,parent)
{
}

INT_PTR CAGiliTyDlg::DoModal()
{
  m_pParentWnd = AfxGetMainWnd();

  CMainFrame* frame = NULL;
  if (m_pParentWnd && m_pParentWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)))
    frame = (CMainFrame*)m_pParentWnd;

  if (frame)
    frame->SetModalDialog(this);
  INT_PTR result = BaseDialog::DoModal();
  if (frame)
    frame->SetModalDialog(NULL);
  return result;
}
