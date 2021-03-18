/////////////////////////////////////////////////////////////////////////////
//
// AGiliTy AGT Interpreter
// Visual C++ MFC Windows interface by David Kinder
//
// StdAfx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#define VC_EXTRALEAN
#define _AFX_ALL_WARNINGS
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS

#define _WIN32_WINNT 0x0600

// Undefine memory allocation, deallocation and exit functions
#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef free
#undef free
#endif

#ifdef exit
#undef exit
#endif

#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxadv.h>
#include <afxtempl.h>
#include <afxmt.h>
