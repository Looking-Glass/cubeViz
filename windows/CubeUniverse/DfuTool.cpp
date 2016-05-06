
// DfuTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DfuTool.h"
#include "DfuToolDlg.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#pragma comment(lib,"Advapi32.lib")

/*
http://cubetube.org/static/recovery/firmware-part1.bin
http://cubetube.org/static/recovery/firmware-part2.bin
http://cubetube.org/static/recovery/demo.bin

dfu-util -d 2b04:d006 -a 0 -s 0x8020000 -D firmware-part1.bin
dfu-util -d 2b04:d006 -a 0 -s 0x8060000 -D firmware-part2.bin
dfu-util -d 2b04:d006 -a 0 -s 0x80A0000:leave -D demo.bin

D:\工作事务\威客\DFU下载工具\DfuTool\Release>dfu-util -d 2b04:d006 -a 0 -s 0x80A0000:leave -D demo.bin
dfu-util 0.8-msvc

Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
Copyright 2010-2014 Tormod Volden and Stefan Schmidt
This program is Free Software and has ABSOLUTELY NO WARRANTY
Please report bugs to dfu-util@lists.gnumonks.org

Invalid DFU suffix signature
A valid DFU suffix will be required in a future dfu-util release!!!
Opening DFU capable USB device...
ID 2b04:d006
Run-time device DFU version 011a
Claiming USB DFU Interface...
Setting Alternate Setting #0 ...
Determining device status: state = dfuIDLE, status = 0
dfuIDLE, continuing
DFU mode device DFU version 011a
Device returned transfer size 4096
DfuSe interface name: "Internal Flash   "
Downloading to address = 0x080a0000, size = 24376
Download        [=========================] 100%        24376 bytes
Download done.
File downloaded successfully
*/

TCHAR *g_strWebAddr=_T("http://cubetube.org/static/recovery");
TCHAR *g_strUpdateFile[MAX_UPDATE_NUM]={
	_T("firmware-part1.bin"),
	_T("firmware-part2.bin"),
	_T("demo.bin")
};

// CDfuToolApp

BEGIN_MESSAGE_MAP(CDfuToolApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDfuToolApp construction

CDfuToolApp::CDfuToolApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDfuToolApp object

CDfuToolApp theApp;


// CDfuToolApp initialization

BOOL CDfuToolApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
//	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	SetRegistryKey(_T("WbjSoft\\CubeViz1.0"));
//	SetDebugPrivileges();

	::GetModuleFileName(m_hInstance, m_szRunExeName, 210);
	_tcscpy(m_szRunDir,m_szRunExeName);
	//	::GetModuleFileName(m_hInstance, m_szRunDir, 210);
	TCHAR *pLastPos = _tcsrchr(m_szRunDir, _T('\\'));
	if(*(pLastPos-1) == _T(':'))	
		*(pLastPos+1)	= 0;
	else
		*pLastPos		= 0;

	m_strCubeBinDir.Format("%s\\CubeBin",m_szRunDir); //bin文件目录	
	if(_access(m_strCubeBinDir,0)!=0)
		CreateDirectory(m_strCubeBinDir,NULL);

	m_strINIFile.Format("%s\\CubeViz.ini", m_szRunDir);
	LoadParam();

	// START GDI+ SUB SYSTEM
//	GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

// 	m_image = new ImageEx( _T("GIF"), _T("DFUMODE") );
// 	//m_image = new ImageEx( L"Dfu_Mode.gif" );

	CDfuToolDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CDfuToolApp::ExitInstance()
{
	SaveParam();

	// Shutdown GDI+ subystem
//	GdiplusShutdown(m_gdiplusToken);

	return CWinAppEx::ExitInstance();
}
void CDfuToolApp::LoadParam()
{
	int i;
	CString strFile,strPath;
	TCHAR ss[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	TCHAR szExtName[MAX_PATH];
	for(i=0;i<MAX_UPDATE_NUM;i++)
	{
#if 0;
		_stprintf(ss,_T("UpdateFile%d"),i+1);
		m_strUpdateFile[i]=GetProfileString(_T("SysSet"),ss);

		if(m_strUpdateFile[i].GetLength()>0 && _access((const char *)m_strUpdateFile[i],0)!=0)	//文件不存在
		{
			m_strUpdateFile[i].Empty();
		}
		if(m_strUpdateFile[i].GetLength()==0)	//空,在运行目录下找
		{
			strFile.Format(_T("%s\\%s"),m_szRunDir,g_strUpdateFile[i]);
			if(_access((const char *)strFile,0)==0)	//文件存在
			{
				m_strUpdateFile[i]=g_strUpdateFile[i];	
			}
		}
		else
		{
			strPath=GetFolderFromFullpath(m_strUpdateFile[i],szFileName,szExtName);
			if(strPath.CompareNoCase(m_szRunDir)==0)
			{
				m_strUpdateFile[i].Format("%s%s",szFileName,szExtName); //=g_strUpdateFile[i];	
			}
		}
#else
		m_strUpdateFile[i].Empty();
#endif
	}
//	m_strImportDir=ss;
// 	::GetPrivateProfileString("SysSet","ExportDir","",ss,MAX_PATH-1,m_strINIFile);
}
void CDfuToolApp::SaveParam()
{
	int i;
	TCHAR ss[MAX_PATH];
	for(i=0;i<MAX_UPDATE_NUM;i++)
	{
		_stprintf(ss,_T("UpdateFile%d"),i+1);
		WriteProfileString(_T("SysSet"),ss,m_strUpdateFile[i]);
	}
//	::WritePrivateProfileString("SysSet","ImportDir",m_strImportDir,m_strINIFile);
}
