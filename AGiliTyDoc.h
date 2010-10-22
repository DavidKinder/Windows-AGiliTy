/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// AGiliTyDoc.h: Interface of document class
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CAGiliTyDoc : public CDocument
{
protected: // create from serialization only
  CAGiliTyDoc();
  DECLARE_DYNCREATE(CAGiliTyDoc)

// Attributes
public:

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CAGiliTyDoc)
  public:
  virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
  virtual void OnCloseDocument();
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CAGiliTyDoc();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
  //{{AFX_MSG(CAGiliTyDoc)
  afx_msg void OnFileRestoreGame();
  afx_msg void OnFileSaveGame();
  afx_msg void OnUpdateFileRestoreGame(CCmdUI* pCmdUI);
  afx_msg void OnUpdateFileSaveGame(CCmdUI* pCmdUI);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
