/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyDlg.h: Base dialog class
//
/////////////////////////////////////////////////////////////////////////////

#include "Dialogs.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CAGiliTyDlg : public BaseDialog
{
  DECLARE_DYNAMIC(CAGiliTyDlg)

public:
  CAGiliTyDlg(UINT templateId, CWnd* parent = NULL);

  virtual INT_PTR DoModal();

  DECLARE_MESSAGE_MAP()
};
