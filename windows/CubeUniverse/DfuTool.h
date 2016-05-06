
// DfuTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define MAX_UPDATE_NUM		3

#define  MVSPRINT(fmt)	TCHAR buffer[2048];\
	va_list argptr;\
	int cnt;\
	va_start(argptr, fmt);\
	cnt = vsprintf(buffer, fmt, argptr);\
	va_end(argptr)

// CDfuToolApp:
// See DfuTool.cpp for the implementation of this class
//

class CDfuToolApp : public CWinAppEx
{
public:
	CDfuToolApp();


	TCHAR	m_szRunExeName[MAX_PATH];			//应用程序
	TCHAR	m_szRunDir[MAX_PATH];			//应用程序运行的目录
	CString m_strCubeBinDir;
	CString m_strINIFile;

	CString m_strUpdateFile[MAX_UPDATE_NUM];

	// GDI+
// 	GdiplusStartupInput		m_gdiplusStartupInput;
// 	ULONG_PTR				m_gdiplusToken;

	void LoadParam();
	void SaveParam();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CDfuToolApp theApp;

extern TCHAR *g_strWebAddr; //=_T("http://cubetube.org/static/recovery");
extern TCHAR *g_strUpdateFile[MAX_UPDATE_NUM];
extern TCHAR *g_strLibusbk;
