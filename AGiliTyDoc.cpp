/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyDoc.cpp: Implementation of document class
//
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AGiliTy.h"
#include "AGiliTyDoc.h"
#include "AGiliTyView.h"

extern "C"
{
#include ".\generic\agility.h"
#include ".\generic\interp.h"
}

extern BOOL bResetCursor;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAGiliTyDoc
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CAGiliTyDoc, CDocument)

BEGIN_MESSAGE_MAP(CAGiliTyDoc, CDocument)
  //{{AFX_MSG_MAP(CAGiliTyDoc)
  ON_COMMAND(ID_FILE_RESTORE_GAME, OnFileRestoreGame)
  ON_COMMAND(ID_FILE_SAVE_GAME, OnFileSaveGame)
  ON_UPDATE_COMMAND_UI(ID_FILE_RESTORE_GAME, OnUpdateFileRestoreGame)
  ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_GAME, OnUpdateFileSaveGame)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CAGiliTyDoc::CAGiliTyDoc()
{
}

CAGiliTyDoc::~CAGiliTyDoc()
{
}

BOOL CAGiliTyDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
  if (!CDocument::OnOpenDocument(lpszPathName))
    return FALSE;

  CString& strGame = ((CAGiliTyApp*)AfxGetApp())->m_strGameFile;
  if (strGame.IsEmpty())
    strGame = lpszPathName;
  else
  {
    CString strGameArg;
    strGame = lpszPathName;
    int iExtPos = strGame.ReverseFind('.');
    if (iExtPos > 0)
      strGameArg = strGame.Left(iExtPos);
    else
      strGameArg = strGame;

    agt_newgame(init_file_context(strGameArg.GetBuffer(256),fNONE));
    strGameArg.ReleaseBuffer();

    CAGiliTyView* pView = CAGiliTyView::GetView();
    if (pView)
      pView->SendMessage(WM_CHAR,(TCHAR)0x0D);
  }

  CAGiliTyApp* pApp = (CAGiliTyApp*)AfxGetApp();
  ASSERT_VALID(pApp);

  CString strPathName(lpszPathName);
  int iPos = strPathName.ReverseFind('\\');
  if (iPos > 0)
    pApp->m_strFilePath = strPathName.Left(iPos);
  else
    pApp->m_strFilePath.Empty();

  bResetCursor = TRUE;
  return TRUE;
}

void CAGiliTyDoc::OnCloseDocument() 
{
  CString& strGame = ((CAGiliTyApp*)AfxGetApp())->m_strGameFile;
  if (strGame.IsEmpty() == FALSE)
  {
    agt_quit();
    quitflag = 1;
  }

  CDocument::OnCloseDocument();
}

void CAGiliTyDoc::OnFileRestoreGame() 
{
  CString& strGame = ((CAGiliTyApp*)AfxGetApp())->m_strGameFile;
  if (strGame.IsEmpty() == FALSE)
  {
    agt_restore();
    CAGiliTyView* pView = CAGiliTyView::GetView();
    if (pView)
      pView->SendMessage(WM_CHAR,(TCHAR)0x0D);
  }
  bResetCursor = TRUE;
}


void CAGiliTyDoc::OnUpdateFileRestoreGame(CCmdUI* pCmdUI) 
{
  CString& strGame = ((CAGiliTyApp*)AfxGetApp())->m_strGameFile;
  pCmdUI->Enable(strGame.IsEmpty() == FALSE);
}

void CAGiliTyDoc::OnFileSaveGame() 
{
  CString& strGame = ((CAGiliTyApp*)AfxGetApp())->m_strGameFile;
  if (strGame.IsEmpty() == FALSE)
  {
    agt_save();
    CAGiliTyView* pView = CAGiliTyView::GetView();
    if (pView)
      pView->SendMessage(WM_CHAR,(TCHAR)0x0D);
  }
  bResetCursor = TRUE;
}

void CAGiliTyDoc::OnUpdateFileSaveGame(CCmdUI* pCmdUI) 
{
  CString& strGame = ((CAGiliTyApp*)AfxGetApp())->m_strGameFile;
  pCmdUI->Enable(strGame.IsEmpty() == FALSE);
}

#ifdef _DEBUG
void CAGiliTyDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void CAGiliTyDoc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG
