#include "StdAfx.h"

#include <io.h>
#include <ShlObj.h>
//#include <odbcinst.h>
#include "NewFunc.h"

#include <devguid.h>
// The next 3 includes are needed for serial port enumeration
#include <objbase.h>
#include <initguid.h>

#include <Setupapi.h>
#include <afxinet.h>

int EnumDevice(IN LPGUID lpGuid,OUT CStringArray & strArrayDevName,BOOL bPresent=TRUE,OUT  CStringArray *pstrArrayHID=NULL,OUT  CStringArray *pstrArrayHandle=NULL);//bOnlyListHiden=FALSE);
bool ChangeDevStatus(DWORD newstatus, DWORD selecteditem, HDEVINFO devInfo);   

static char *fmt0[3]={"%.3f","%.2f","%d"};
static char *fmt1[3]={"%.3f h","%.2f m","%d s"};
static char *fmt2[3]={"%.3f h","%.2f min","%d sec"};
static float fxs[3]={1/3600.0f,1/60.0f,1.0f};
/*
enum {TIMEMODE_HOUR=0,TIMEMODE_MIN=1,TIMEMODE_SEC=2,
	TIMEMODE_MARK_UNIT1=0x8000,	//只用最简缩写(一个字符)表示单位
	TIMEMODE_MARK_UNIT2=0x4000	//只用正常缩写(一个或多字符)表示单位
};
*/
LPTSTR ToTimeStr(time_t tv,int nTimeMode,LPTSTR szOutStr)
{
	if(szOutStr==NULL)
		return NULL;
	char * *pp;
	pp=fmt0;
	if(nTimeMode&TIMEMODE_MARK_UNIT1)	
		pp=fmt1;
	if(nTimeMode&TIMEMODE_MARK_UNIT1)	
		pp=fmt2;
	int idx=2;
	if((nTimeMode&0xFF)==TIMEMODE_HOUR)
		idx=0;
	else if((nTimeMode&0xFF)==TIMEMODE_MIN)
		idx=1;
	if(idx==2)
		sprintf(szOutStr,pp[idx],tv);
	else
		sprintf(szOutStr,pp[idx],(float)tv*fxs[idx]);
	return szOutStr;
}
LPTSTR ToTimeStr(time_t tv,int nTimeMode,CString & strOut)
{
	char * *pp;
	pp=fmt0;
	if(nTimeMode&TIMEMODE_MARK_UNIT1)	
		pp=fmt1;
	if(nTimeMode&TIMEMODE_MARK_UNIT2)	
		pp=fmt2;
	int idx=2;
	if((nTimeMode&0xFF)==TIMEMODE_HOUR)
		idx=0;
	else if((nTimeMode&0xFF)==TIMEMODE_MIN)
		idx=1;
	if(idx==2)
		strOut.Format(pp[idx],tv);
	else
		strOut.Format(pp[idx],(float)tv*fxs[idx]);
	return (LPTSTR)&strOut;
}

//将nValue赋给串str,当nValue=0:清空,否则转换int值
void SetStringInt(CString & str,int nValue)
{
	if(nValue!=0)
		str.Format("%d",nValue);
	else
		str.Empty();
}
//转换串str为UINT值,为负数时,用绝对值
UINT GetStringUInt(CString & str)
{
	if(str.IsEmpty())
		return 0;
	int n=atoi(str);
	return abs(n);
}

char *GetDateStr(time_t tTime,char sdate[])
{
	return GetTimeDigitStr(tTime,sdate,0);
}
//取时间的数字串,存于szTime[],m=0:日期,如:"070203"(2007-02-03),m=1:日期时间,"070203123000"
//m=2:没有秒的日期时间 "0702031230"
char *GetTimeDigitStr(time_t tTime,char szTime[],int m)
{
	CTime ct(tTime);
//	ct=tTime;
	if(m==0)	//仅日期
		sprintf(szTime,"%02d%02d%02d",ct.GetYear()%100,ct.GetMonth(),ct.GetDay());
	else if(m==1)	//全部
		sprintf(szTime,"%02d%02d%02d%02d%02d%02d",ct.GetYear()%100,ct.GetMonth(),ct.GetDay(),ct.GetHour(),ct.GetMinute(),ct.GetSecond());
	else	//没有秒
		sprintf(szTime,"%02d%02d%02d%02d%02d",ct.GetYear()%100,ct.GetMonth(),ct.GetDay(),ct.GetHour(),ct.GetMinute());
	return szTime;
}

CString GetCurrentTimeStr()
{
	CTime Time=CTime::GetCurrentTime();
	return Time.Format("%Y-%m-%d %H:%M:%S");
}

CString GetCurrentTimeStr(time_t tTime)
{
	CTime Time(tTime);
	return Time.Format("%Y-%m-%d %H:%M:%S");
}


int PrintMsgWarning(LPCTSTR fmt, ...)
{	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	return AfxMessageBox(buffer,MB_OK|MB_ICONWARNING);
}

int PrintMsgError(LPCTSTR fmt, ...)
{	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	return AfxMessageBox(buffer,MB_OK|MB_ICONERROR);
}
int PrintMsgInfo(LPCTSTR fmt, ...)
{	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	return AfxMessageBox(buffer,MB_OK|MB_ICONINFORMATION);
}
/*void PrintPrompt(LPCTSTR fmt, ...)
{
	CMainFrame *pFrame;
	char buffer[2048];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	pFrame=(CMainFrame *)theApp.m_pMainWnd; // AfxGetMainWnd();
	if(pFrame==NULL)
		return ;
	pFrame->SetMessageText(buffer);
}
*/
void PrintMsgToFile(LPCTSTR fname,LPCTSTR fmt, ...)
{
	char buffer[4096];
	va_list argptr;
	int cnt;
	va_start(argptr, fmt);
	cnt = vsprintf(buffer, fmt, argptr);
	va_end(argptr);
	AddStrToFile(fname,buffer,TRUE);
}

void AddStrToFile(LPCTSTR lpfname,LPCTSTR szStr,BOOL bAddCurTime)
{
	CFile file;
	BOOL bb=FALSE;
	bb=file.Open(lpfname,CFile::modeReadWrite);
	if(!bb)
	{
		bb=file.Open(lpfname,CFile::modeWrite|CFile::modeCreate);
	}
	if(!bb)
		return;

	CString str,ss;
	if(bAddCurTime)
	{
		CTime ct=CTime::GetCurrentTime();
		ss=ct.Format("[%Y-%m-%d %H:%M:%S]");
		str.Format("%s %s\xd\xa",ss,szStr);
	}
	else
		str.Format("%s\xd\xa",szStr);
	file.SeekToEnd();
	file.Write(str,str.GetLength());
	file.Close();
}

//取位数,nMaxBitNum:最大位数
int GetBitNum(BYTE cBitByte[],int nMaxBitNum)
{
	int i,m=0;
	for (i=0;i<nMaxBitNum;i++)
	{
		if(GET_CHANBIT(cBitByte,i))
			m++;
	}
	return m;
}
//取位表中有效字节数(到最后一个有1的位占的字节数),nMaxBitNum:最大位数
int GetValidBitByteNum(BYTE cBitByte[],int nMaxBitNum)
{
	int i,m=-1;
	for (i=0;i<nMaxBitNum;i++)
	{
		if(GET_CHANBIT(cBitByte,i))
			m=i;
	}
	if(m==-1)
		return 0;
	return m/8+1;
}

void PrintHexMessage(LPCTSTR lpBuf,int nlen)
{
	CString strTemp="";
//	BYTE *pBuf=(BYTE *)&stQBI;
	int len=16,n=(nlen+15)/16;
	for(int i=0;i<n;i++)
	{
		if(i==n-1 && (nlen%16)!=0)
			len=nlen%16;
		strTemp+=ToHexStr((BYTE *)lpBuf,len);
		strTemp+="\xd\xa";
		lpBuf+=len;
	}
	AfxMessageBox(strTemp);
}
CString ToHexStr(BYTE buf[],int len)
{
	CString str;
	char ss[20];
	int i;
	str="";
	for(i=0;i<len;i++)
	{
		sprintf(ss,"%02X ",buf[i]);
		str+=ss;
	}
	return str;
}

void EnableControl(CWnd *pWnd,UINT id[],int num,BOOL bEnable)
{
	int i;
	CWnd *pCWnd;	
	if(pWnd==NULL)
		return;
	for(i=0;i<num;i++)
	{
		pCWnd=pWnd->GetDlgItem(id[i]);
		if(pCWnd)
		{
			if(pCWnd->IsWindowEnabled()!=bEnable)
				pCWnd->EnableWindow(bEnable);
		}
	}
}
void ShowControl(CWnd *pWnd,UINT id[],int num,BOOL bShow)
{
	int i;
	CWnd *pCWnd;	
	if(pWnd==NULL)
		return;
	int nShow=SW_SHOW;
	if(!bShow)
		nShow=SW_HIDE;
	for(i=0;i<num;i++)
	{
		pCWnd=pWnd->GetDlgItem(id[i]);
		if(pCWnd) pCWnd->ShowWindow(nShow);
	}
}


WORD GetWordChkSum(BYTE buf[],int n)
{
	int i;
	WORD sw=0;
	for(i=0;i<n;i++)
	{
		sw+=buf[i];
	}
	return sw;
}

void LocateWindow(CWnd *pWnd,CWnd *pLocationWnd,CWnd *pParent)
{
	CRect rectCaptionBar;
	if(pParent==NULL || pWnd==NULL || pLocationWnd==NULL)
		return;
	pLocationWnd->GetWindowRect (&rectCaptionBar);
	pParent->ScreenToClient (&rectCaptionBar);
	pWnd->SetWindowPos (&pParent->wndTop, rectCaptionBar.left, rectCaptionBar.top, 
		rectCaptionBar.Width(), 
		rectCaptionBar.Height(),
		SWP_NOACTIVATE);
}
void LocateWindow(UINT nID,UINT nLocationID,CWnd *pParent)
{
	CWnd *pWnd,*pLocationWnd;
	if(pParent==NULL)
		return;
	pWnd=pParent->GetDlgItem(nID);
	pLocationWnd=pParent->GetDlgItem(nLocationID);
	LocateWindow(pWnd,pLocationWnd,pParent);
}

char *GetDateTimeStr(time_t ttime,char *str)
{
	struct tm* ptmTemp = localtime(&ttime);
	if (ptmTemp == NULL ||
		!_tcsftime(str, 30, "%Y-%m-%d %H:%M:%S", ptmTemp))
		str[0] = '\0';
	return str;
}

CString  OpenDir(LPCTSTR  cDlgName)  
{  

	char   Mycom[MAX_PATH]="E:\\";  
	BROWSEINFO   Myfold;  
	Myfold.hwndOwner=NULL;  
	Myfold.pidlRoot=NULL;  
	Myfold.pszDisplayName=Mycom;  
	Myfold.lpszTitle=cDlgName;  
	Myfold.ulFlags=BIF_RETURNONLYFSDIRS;  
	Myfold.lpfn=NULL;  
	Myfold.lParam=NULL;  
	Myfold.iImage=NULL;  
	Mycom[0]='\0';  
	LPITEMIDLIST lpIIL=SHBrowseForFolder(&Myfold);
	//	if(lpIIL==NULL)

	SHGetPathFromIDList(lpIIL,Mycom);  
	return(Mycom);  
}
int Touch(LPCTSTR lpPath, BOOL bValidate)
{
	if(lpPath==NULL)
	{
		return 1;
	}

	TCHAR szPath[MAX_PATH];
	_tcscpy(szPath, lpPath);
	int nLen = _tcslen(szPath);

	//path must be "x:\..."
	if( ( nLen<3 ) || 
		( ( szPath[0]<_T('A') || _T('Z')<szPath[0] ) && 
		  ( szPath[0]<_T('a') || _T('z')<szPath[0] ) ||
		( szPath[1]!=_T(':') )|| 
		( szPath[2]!=_T('\\') )
		)
	  )
	{
		return 1;
	}

	int i;
	if(nLen==3)
	{
		if(!bValidate)
		{
			if(_access(szPath, 0)!=0)
			{
				return 2;
			}
		}
		return 0;
	}

	i = 3;
	BOOL bLastOne=TRUE;
	LPTSTR lpCurrentName;
	while(szPath[i]!=0)
	{
		lpCurrentName = &szPath[i];
		while( (szPath[i]!=0) && (szPath[i]!=_T('\\')) )
		{
			i++;
		}

		bLastOne =(szPath[i]==0);
		szPath[i] = 0;

		if( !IsFileNameValid(lpCurrentName) )
		{
			return 1;
		}

		if(!bValidate)
		{
			CreateDirectory(szPath, NULL);
			if(_taccess(szPath, 0)!=0)
			{
				return 2;
			}
		}

		if(bLastOne)
		{
			break; //it's done
		}
		else
		{
			szPath[i] = _T('\\');
		}

		i++;
	}

	return (bLastOne?0:1);
}

BOOL IsFileNameValid(LPCTSTR lpFileName)
{
	if(lpFileName==NULL)
	{
		return FALSE;
	}

	int nLen = _tcslen(lpFileName);
	if(nLen<=0)
	{
		return FALSE;
	}

	//check first char
	switch(lpFileName[0])
	{
	case _T('.'):
	case _T(' '):
	case _T('\t'):
		return FALSE;
	}

	//check last char
	switch(lpFileName[nLen-1])
	{
	case _T('.'):
	case _T(' '):
	case _T('\t'):
		return FALSE;
	}

	//check all
	int i=0;
	while(lpFileName[i]!=0)
	{
		switch(lpFileName[i])
		{
		case _T('\\'):
		case _T('/'):
		case _T(':'):
		case _T('*'):
		case _T('?'):
		case _T('\"'):
		case _T('<'):
		case _T('>'):
		case _T('|'):
			return FALSE;
		}
		i++;
	}
	return TRUE;
}
/*
void BackupOldChlDateFile(const char *pszChlDateFile)
{
	char szFullPathName[200], szDirTemp[200], szFileName[50], *pLastPos;

	if(_access(pszChlDateFile, 00) != 0)	return;
	strcpy(szFullPathName, pszChlDateFile);
	pLastPos = strrchr(szFullPathName, '\\');
	ASSERT(pLastPos != NULL);
	strcpy(szFileName, pLastPos+1);
	*pLastPos = NULL;	
	pLastPos = strrchr(szFullPathName, '\\');
	ASSERT(pLastPos != NULL);
	*(pLastPos+1) = NULL;
	strcat(szFullPathName, m_szBakDataSubDir);
	strcat(szFullPathName, szFileName);
	if(_access(szFullPathName, 00) == 0)	_unlink(szFullPathName);
	else{
		strcpy(szDirTemp, szFullPathName);
		pLastPos = strrchr(szDirTemp, '\\');
		*pLastPos = '\0';
		strcat(szDirTemp, "\\NUL");
		if(_access(szDirTemp, 00) != 0){
			pLastPos = strrchr(szDirTemp, '\\');
			*pLastPos = '\0';
			_mkdir(szDirTemp);
		}
	}
	rename(pszChlDateFile, szFullPathName);	
}*/

int round(double fv)
{
	int fi=0;
	if(fv==0.0f)
		return 0;
	fi=int(fv);
	if(double(fi)==fv)
		return fi;
	if(fv>0.0f)
	{
		return fi+1;
	}
	return fi-1;
}
//取合适的绘图坐标y轴范围,m=0:电流,=1:电压,=2:容量
void GetFixYRange(int m,float & fmax,float & fmin)
{
	float fv1=fmin,fv2=fmax;
	float ff=max(fabs(fv1),fabs(fv2));
	float fa=200.0f;
 	if(m==0)
 		fa=ff*10/100;
 	else
		fa=ff*5/100;
	fa=floor(fa/100.0f+0.5)*100.0f;
	if(fa<200)
		fa=200;
	if(m==1)
		fa=100.0f;

	fv2=floor(fv2/100.0f+0.5)*100.0f+fa;
	fv1=floor(fv1/100.0f-0.5)*100.0f-fa;
	if(m==0)	//I
	{
		fv2=max(fabs(fv1),fabs(fv2));
		fv1=-fv2;
	}
	else if(m==1)	//V
	{
		if(fv2<4500) 
			fv2=4500;
		if(fv1<0)
			fv1=0;
		if(fv2-fv1<1000)
		{
			fv1=fv2-1000;
			if(fv1<0)
				fv1=0;
			fv2=fv1+1000;
		}
	}
	else	//C
	{
		if(fv1<0)
			fv1=0;
		if(fv2-fv1<1000)
			fv2=fv1+1000;
	}
	fmax=fv2; fmin=fv1;
}
//文件目录转成LPITEMIDLIST
LPITEMIDLIST ParsePidlFromPath(LPCSTR path)
{    
	OLECHAR szOleChar[MAX_PATH];    
	LPSHELLFOLDER IpsfDeskTop;    
	LPITEMIDLIST lpifq;    
	ULONG ulEaten, ulAttribs;    
	HRESULT hres;    
	SHGetDesktopFolder(&IpsfDeskTop);    
	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,path,-1,szOleChar,sizeof(szOleChar));    
	hres = IpsfDeskTop ->ParseDisplayName(NULL, NULL, szOleChar, &ulEaten, &lpifq, &ulAttribs);    
	hres=IpsfDeskTop->Release( );        
	if(FAILED(hres))
		return NULL;
	return lpifq;
}

//将szStr串中有号隔开的数取出，存于fResult[]中,nMaxN:最多取的个数,返回实际个数
//如 1.23,45.6,12.45 等
int GetFloatFromStr(IN LPCTSTR szStr,OUT float fResult[],IN int nMaxN)
{
	int ln;
	TCHAR ss[MAX_PATH];
	TCHAR *p=(TCHAR *)szStr;
	if(nMaxN<=0)
		return 0;

	while(*p==_T(' ')) p++;
	ln=_tcslen(p);
	if(ln==0||ln>MAX_PATH-1)
		return 0;
	int m=0;
	for (;;)
	{
		TCHAR *pp=_tcschr(p,_T(','));
		if(pp)
		{
			ln=pp-p;
			_tcsncpy(ss,p,ln); ss[ln]=0;
		}
		else
		{
			_tcscpy(ss,p); ln=_tcslen(p);
		}
		if(ln==0)
			fResult[m]=0.0f;
		else
			fResult[m]=_tstof(ss); 
		m++;
		if(m>=nMaxN)
			return m;
		if(pp==NULL)
			break;
		p=pp+1;
	}
	return m;
}

//将fResult[]中的浮点数转换在串存于szStr中(用','号隔开),返回实际个数,超长度则返回0
//如 1.23,45.6,12.45 等
int SetFloatToStr(IN float fResult[],IN int nNum,OUT LPTSTR szStr)
{
	CString strTemp;
	int ln;
	TCHAR ss[MAX_PATH];
	szStr[0]=0;
	strTemp.Empty();
	for (int i=0;i<nNum;i++)
	{
		sprintf(ss,"%g,",fResult[i]);
		strTemp+=ss;
	}
	ln=strTemp.GetLength();
	if(ln>0) ln--;		//去掉最后一个','
	if(ln==0 || ln>MAX_PATH-1)
		return 0;
	_tcsncpy(szStr,(LPCTSTR)strTemp,ln); szStr[ln]=0;
	return nNum;
}

//取文件名串lpszStr路径后面的文件名,返回该地址,名长度(不包括扩展名)在pLen中
TCHAR *GetFileName(LPCTSTR lpszStr,int *pLen)
{
	int ln;
	TCHAR *p=(TCHAR *)lpszStr;
	TCHAR *pp;
	pp=_tcsrchr(p,_T('\\'));
	if(pp) p=pp+1;
	pp=_tcsrchr(p,_T('.'));
	if(pp==NULL)
	{
		ln=_tcslen(p);
	}
	else
	{
		ln=pp-p;
	}
	if(pLen) *pLen=ln;
	return p;
}
//从文件全名从取文件夹名称(目录名)
CString GetFolderFromFullpath(LPCTSTR lpFullpath,OUT TCHAR *pFileName,OUT TCHAR *pExtName)
{
	TCHAR drive[_MAX_DRIVE];
	TCHAR dir[_MAX_DIR];
	TCHAR fname[_MAX_FNAME];
	TCHAR ext[_MAX_EXT];
	_tsplitpath( lpFullpath, drive, dir, fname, ext);

	CString strPath;
	int ln=_tcslen(dir);
	if(ln>0)
	{
		if(dir[ln-1]==_T('\\'))
		{	ln--; dir[ln]=0; }
	}
	strPath.Format(_T("%s%s"),drive,dir);
	if(pFileName)
		_tcscpy(pFileName,fname);
	if(pExtName)
		_tcscpy(pExtName,ext);
	return strPath;
}

BOOL is_x64(void)
{
	BOOL ret = FALSE;
	BOOL (__stdcall *pIsWow64Process)(HANDLE, PBOOL) = NULL;
	// Detect if we're running a 32 or 64 bit system
	if (sizeof(uintptr_t) < 8) {
		pIsWow64Process = (BOOL (__stdcall *)(HANDLE, PBOOL))
			GetProcAddress(GetDLLHandle("kernel32"), "IsWow64Process");
		if (pIsWow64Process != NULL) {
			(*pIsWow64Process)(GetCurrentProcess(), &ret);
		}
	} else {
		ret = TRUE;
	}
	return ret;
}

// From smartmontools os_win32.cpp
const char* PrintWinVersion(OUT int & nWindowsVersion)
{
	OSVERSIONINFOEXA vi, vi2;
	const char* w = 0;
	const char* w64 = "32 bit";
	char* vptr;
	size_t vlen;
	unsigned major, minor;
	ULONGLONG major_equal, minor_equal;
	BOOL ws;
//	int nWindowsVersion;
	static char WindowsVersionStr[128] = "Windows ";

	nWindowsVersion = WINDOWS_UNDEFINED;
	safe_strcpy(WindowsVersionStr, sizeof(WindowsVersionStr), "Windows Undefined");

	memset(&vi, 0, sizeof(vi));
	vi.dwOSVersionInfoSize = sizeof(vi);
	if (!GetVersionExA((OSVERSIONINFOA *)&vi)) {
		memset(&vi, 0, sizeof(vi));
		vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		if (!GetVersionExA((OSVERSIONINFOA *)&vi))
			return WindowsVersionStr;
	}

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) {

		if (vi.dwMajorVersion > 6 || (vi.dwMajorVersion == 6 && vi.dwMinorVersion >= 2)) {
			// Starting with Windows 8.1 Preview, GetVersionEx() does no longer report the actual OS version
			// See: http://msdn.microsoft.com/en-us/library/windows/desktop/dn302074.aspx

			major_equal = VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL);
			for (major = vi.dwMajorVersion; major <= 9; major++) {
				memset(&vi2, 0, sizeof(vi2));
				vi2.dwOSVersionInfoSize = sizeof(vi2); vi2.dwMajorVersion = major;
				if (!VerifyVersionInfoA(&vi2, VER_MAJORVERSION, major_equal))
					continue;
				if (vi.dwMajorVersion < major) {
					vi.dwMajorVersion = major; vi.dwMinorVersion = 0;
				}

				minor_equal = VerSetConditionMask(0, VER_MINORVERSION, VER_EQUAL);
				for (minor = vi.dwMinorVersion; minor <= 9; minor++) {
					memset(&vi2, 0, sizeof(vi2)); vi2.dwOSVersionInfoSize = sizeof(vi2);
					vi2.dwMinorVersion = minor;
					if (!VerifyVersionInfoA(&vi2, VER_MINORVERSION, minor_equal))
						continue;
					vi.dwMinorVersion = minor;
					break;
				}

				break;
			}
		}

		if (vi.dwMajorVersion <= 0xf && vi.dwMinorVersion <= 0xf) {
			ws = (vi.wProductType <= VER_NT_WORKSTATION);
			nWindowsVersion = vi.dwMajorVersion << 4 | vi.dwMinorVersion;
			switch (nWindowsVersion) {
			case 0x50: w = "2000";
				break;
			case 0x51: w = "XP";
				break;
			case 0x52: w = (!GetSystemMetrics(89)?"2003":"2003_R2");
				break;
			case 0x60: w = (ws?"Vista":"2008");
				break;
			case 0x61: w = (ws?"7":"2008_R2");
				break;
			case 0x62: w = (ws?"8":"2012");
				break;
			case 0x63: w = (ws?"8.1":"2012_R2");
				break;
			case 0x64: w = (ws?"10":"2015");
				break;
			default:
				if (nWindowsVersion < 0x50)
					nWindowsVersion = WINDOWS_UNSUPPORTED;
				else
					w = "11 or later";
				break;
			}
		}
	}

	if (is_x64())
		w64 = "64-bit";

	vptr = &WindowsVersionStr[sizeof("Windows ") - 1];
	vlen = sizeof(WindowsVersionStr) - sizeof("Windows ") - 1;
	if (!w)
		safe_sprintf(vptr, vlen, "%s %u.%u %s", (vi.dwPlatformId==VER_PLATFORM_WIN32_NT?"NT":"??"),
		(unsigned)vi.dwMajorVersion, (unsigned)vi.dwMinorVersion, w64);
	else if (vi.wServicePackMinor)
		safe_sprintf(vptr, vlen, "%s SP%u.%u %s", w, vi.wServicePackMajor, vi.wServicePackMinor, w64);
	else if (vi.wServicePackMajor)
		safe_sprintf(vptr, vlen, "%s SP%u %s", w, vi.wServicePackMajor, w64);
	else
		safe_sprintf(vptr, vlen, "%s %s", w, w64);
	return WindowsVersionStr;
}

//////////////////////////////////////////////////////////////////////////
/// 从资源文件中加载图片
/// @param [in] pImage 图片指针
/// @param [in] nResID 资源号
/// @param [in] lpTyp 资源类型
//////////////////////////////////////////////////////////////////////////
bool LoadImageFromResource(IN CImage* pImage,
	IN UINT nResID, 
	IN LPCTSTR lpTyp)
{
	if ( pImage == NULL) return false;

	pImage->Destroy();

	// 查找资源
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL) return false;

	// 加载资源
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}

	// 锁定内存中的指定资源
	LPVOID lpVoid    = ::LockResource(hImgData);

	LPSTREAM pStream = NULL;
	DWORD dwSize    = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew    = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte    = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);

	// 解除内存中的指定资源
	::GlobalUnlock(hNew);

	// 从指定内存创建流对象
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if ( ht != S_OK )
	{
		GlobalFree(hNew);
	}
	else
	{
		// 加载图片
		pImage->Load(pStream);

		GlobalFree(hNew);
	}

	// 释放资源
	::FreeResource(hImgData);

	return true;
}

//---------------------------------------------------------------
// Routine for enumerating the available serial ports.
// Throws a CString on failure, describing the error that
// occurred. If bIgnoreBusyPorts is TRUE, ports that can't
// be opened for read/write access are not included.

BOOL EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts)
{
	// Clear the output array
	asi.RemoveAll();
#if 1
	CString strName;
	CStringArray strArrayDev;
	CStringArray strArrayHID;
	strArrayHID.RemoveAll();
	int n=EnumDevice((LPGUID)&GUID_DEVCLASS_PORTS,strArrayDev,1,&strArrayHID);
	if(n==0)
		return FALSE;
	for (int i=0;i<n;i++)
	{
		SSerInfo rsi;
		strName=strArrayDev.GetAt(i);
		rsi.strFriendlyName=strName;
		rsi.strPortName = rsi.strFriendlyName;
		int startdex = rsi.strFriendlyName.Find(_T(" ("));
		int enddex = rsi.strFriendlyName.Find(_T(")"));
		if (startdex > 0 && enddex == (rsi.strFriendlyName.GetLength()-1))
		{
			rsi.strPortName=rsi.strFriendlyName.Mid(startdex+2,enddex-startdex-2);
		}
		rsi.strPortName.MakeUpper();
		int idx=rsi.strPortName.Find(_T("COM"));
		if(idx>=0)
			rsi.nComPort=atoi(rsi.strPortName.Mid(idx+3)); //_tstoi
		else 
			rsi.nComPort=-1;
		rsi.strHID=strArrayHID.GetAt(i);
		asi.Add(rsi);
	}
#else
	// Use different techniques to enumerate the available serial
	// ports, depending on the OS we're using
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	if (!::GetVersionEx(&vi)) {
		return FALSE;
	}
	// Handle windows 9x and NT4 specially
	if (vi.dwMajorVersion < 5) {
		return FALSE;
	}
	else {
		// Win2k and later support a standard API for
		// enumerating hardware devices.
		EnumPortsWdm(asi);
	}

	for (int ii=0; ii<asi.GetSize(); ii++)
	{
		SSerInfo& rsi = asi[ii];    
		if (bIgnoreBusyPorts) {
			// Only display ports that can be opened for read/write
			HANDLE hCom = CreateFile(rsi.strDevPath,
				GENERIC_READ | GENERIC_WRITE,
				0,    /* comm devices must be opened w/exclusive-access */
				NULL, /* no security attrs */
				OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
				0,    /* not overlapped I/O */
				NULL  /* hTemplate must be NULL for comm devices */
				);
			if (hCom == INVALID_HANDLE_VALUE) {
				// It can't be opened; remove it.
				asi.RemoveAt(ii);
				ii--;               
				continue;
			}
			else {
				// It can be opened! Close it and add it to the list
				::CloseHandle(hCom);
			}
		}

		// Come up with a name for the device.
		// If there is no friendly name, use the port name.
		if (rsi.strFriendlyName.IsEmpty())
			rsi.strFriendlyName = rsi.strPortName;

		// If there is no description, try to make one up from
		// the friendly name.
		if (rsi.strPortDesc.IsEmpty()) 
		{
			// If the port name is of the form "ACME Port (COM3)"
			// then strip off the " (COM3)"
			rsi.strPortDesc = rsi.strFriendlyName;
			int startdex = rsi.strPortDesc.Find(_T(" ("));
			int enddex = rsi.strPortDesc.Find(_T(")"));
			if (startdex > 0 && enddex == (rsi.strPortDesc.GetLength()-1))
			{
				rsi.strPortDesc = rsi.strPortDesc.Left(startdex); // 
				// 				AfxMessageBox(rsi.strPortDesc);
			}
		}
		if (rsi.strPortName.IsEmpty()) 
		{
			// If the port name is of the form "ACME Port (COM3)"
			// then strip off the "COM3"
			rsi.strPortName = rsi.strFriendlyName;
			int startdex = rsi.strFriendlyName.Find(_T(" ("));
			int enddex = rsi.strFriendlyName.Find(_T(")"));
			if (startdex > 0 && enddex == (rsi.strFriendlyName.GetLength()-1))
			{
				rsi.strPortName=rsi.strFriendlyName.Mid(startdex+2,enddex-startdex-2);
			}
		}
		rsi.strPortName.MakeUpper();
		int idx=rsi.strPortName.Find(_T("COM"));
		if(idx>=0)
			rsi.nComPort=_tstoi(rsi.strPortName.Mid(idx+3));
		else 
			rsi.nComPort=-1;
	}
#endif
	return TRUE;
}

int EnumDevice(IN LPGUID lpGuid,OUT CStringArray & strArrayDevName,BOOL bPresent,OUT  CStringArray *pstrArrayHID,OUT  CStringArray *pstrArrayHandle)
{
    HDEVINFO   hDevInfo;      
    SP_DEVINFO_DATA   DeviceInfoData;      
    DWORD   i;      
    CString temp;   
    CString str;   
//	strArray.RemoveAll();
	DWORD dflag=0;//DIGCF_PRESENT;
	if(bPresent)
		dflag|=DIGCF_PRESENT;
	if(lpGuid==NULL)
		dflag|=DIGCF_ALLCLASSES;
	
    hDevInfo   =   SetupDiGetClassDevs(lpGuid,   0,   0,dflag);       //|  
    if   (hDevInfo   ==   INVALID_HANDLE_VALUE)      
    {      
        //   Insert   error   handling   here.      
        return  0 ;      
    }      
   
    //   Enumerate   through   all   devices   in   Set.      
   
    DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);      
    for   (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData);i++)      
    {      
        DWORD   DataT;      
        //LPTSTR   buffer   =   NULL;      
        TCHAR   buffer[2048];      
        DWORD   buffersize   =sizeof(buffer);     
// 		if(bOnlyListHiden)
// 		{
// 			if(!IsClassHidden(&DeviceInfoData.ClassGuid))
// 				continue;
// 		}
		buffer[0]=0;
        while   (!SetupDiGetDeviceRegistryProperty(      
            hDevInfo,&DeviceInfoData,SPDRP_FRIENDLYNAME,//SPDRP_DEVICEDESC,    
            &DataT,(PBYTE)buffer,buffersize,&buffersize))      
        {      
            if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
                break;      
        }      
		if(buffer[0]==0)
		{
			while   (!SetupDiGetDeviceRegistryProperty(      
				hDevInfo,&DeviceInfoData,SPDRP_DEVICEDESC,    
				&DataT,(PBYTE)buffer,buffersize,&buffersize))      
			{      
				if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
					break;      
			}      
		}
//        temp.Format(_T("<VALUE>%s</VALUE>"),buffer);      
//        str   +=   temp;      
		str.Format(_T("%s"),buffer);
//		str.Format(_T("%d"),DeviceInfoData.DevInst);
		strArrayDevName.Add(str);
		if(pstrArrayHID)
		{
			buffer[0]=0;
			while   (!SetupDiGetDeviceRegistryProperty(
				hDevInfo,&DeviceInfoData,SPDRP_HARDWAREID,//SPDRP_FRIENDLYNAME,//SPDRP_DEVICEDESC,    
				&DataT,(PBYTE)buffer,buffersize,&buffersize))      
			{      
				if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
					break;      
			}
			TCHAR *p1,*p2=NULL;
			p1=buffer;
			for (int k=0;k<buffersize;k++)
			{
				if(k!=0 && buffer[k]==0)
				{
					p2=buffer+k+1; break;
				}
			}
			if(*p2==_T('*'))
				p2=p1;
			if(p2==NULL)
				p2=p1;
			str.Format(_T("%s"),p2); //buffer);
			pstrArrayHID->Add(str);
		}
		if(pstrArrayHandle)
		{
			str.Format(_T("%d"),DeviceInfoData.DevInst);
			pstrArrayHandle->Add(str);
		}
//        if   (buffer)   LocalFree(buffer);      
    }      
    if   (   GetLastError()!=NO_ERROR   &&      
        GetLastError()!=ERROR_NO_MORE_ITEMS   )      
    {      
        return 0  ;      
    }      
   
    //     Cleanup      
    SetupDiDestroyDeviceInfoList(hDevInfo);     
  return strArrayDevName.GetSize();
}
void ListCtrlInit(CListCtrl *pList)
{
	pList->ModifyStyleEx(0, WS_EX_CLIENTEDGE);
//	AddExStyle (LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES); //|LVS_EX_CHECKBOXES);
	DWORD dwStyle = ::SendMessage (pList->m_hWnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	// Add the full row select and grid line style to the existing extended styles.
	dwStyle |= (LVS_EX_SUBITEMIMAGES ); //LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|  //|LVS_EX_CHECKBOXES); //dwNewStyle;
	// Sets the current extended style ( a DWORD ).
	::SendMessage (pList->m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);
}

void ListCtrlBuildColumns(CListCtrl *pList,int nCols, int * nWidth, LPCTSTR pFieldName[])
{
	//insert columns
	int i;
	pList->DeleteAllItems();
	int nColumnCount = pList->GetHeaderCtrl()->GetItemCount();
	// Delete all of the columns.
	for (i=0;i < nColumnCount;i++)
	{
		pList->DeleteColumn(0);
	}
//	int ww;
//	LOGFONT lf;
//	pList->GetFont()->GetLogFont(&lf);
//	if(lf.lfWidth==0) lf.lfWidth=lf.lfHeight;

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH |  LVCF_SUBITEM; 
	if(pFieldName) lvc.mask|=LVCF_TEXT;
//	CString	strTemp;
	for(i = 0; i < nCols; i++)
	{
		lvc.iSubItem = i;
//		strTemp.LoadString(iCol[i]);
		if(pFieldName) lvc.pszText = (char *)pFieldName[i]; //(char*)(LPCTSTR)strTemp;
//		ww=nWidth[i]*lf.lfWidth/4000;
//		if(ww==0) ww=nWidth[i];
		lvc.cx = nWidth[i];
		lvc.fmt = LVCFMT_LEFT;
		pList->InsertColumn(i,&lvc);
	}
}
BOOL LoadFileToString(LPCSTR strFile,CString & strOut)
{
	CString ss;
	CStdioFile file;
	strOut.Empty();
	BOOL bb=file.Open(strFile,CFile::modeRead);
	if(!bb)
		return FALSE;
	while(1)
	{
		bb=file.ReadString(ss);
		if(!bb)
			break;
		strOut+="\xd\xa";
		strOut+=ss;
	}
	file.Close();
	return bb;
}

BOOL SaveToFile(LPCTSTR strFile,void *buff,int len)
{
	CFile file;
	BOOL bb=file.Open(strFile,CFile::modeCreate|CFile::modeWrite);
	if(!bb)
		return FALSE;
//	PrintMsgInfo((LPCTSTR)buff);
	file.Write(buff,len);
	file.Close();
	return TRUE;
}

BOOL GetURLReturnStr(LPCTSTR strURL,CString & strRet,BOOL bBaiduspider)
{
	//用Baiduspider 可以完全搜索，不然有网站被过滤掉的
	TCHAR* headers=_T("Accept:*/*\r\n")
		_T("Accept-Language:zh-cn\r\n")
		_T("User-Agent:Baiduspider\r\n");
	CString str,strTemp;
	CInternetSession session;
	CInternetFile* file = NULL;
	strRet.Empty(); strTemp.Empty();
#if 1
	try
	{
		file = (CInternetFile*) session.OpenURL(strURL,1,INTERNET_FLAG_RELOAD|INTERNET_FLAG_TRANSFER_ASCII);//INTERNET_FLAG_IDN_DIRECT);  //www.dragon-2008.com/GetIP.asp
	}
	catch (CInternetException* m_pException)
	{
		// 如果有错误的话，置文件为空
		file = NULL; 
		m_pException->Delete();
		//		return ;
	}
#else
	try
	{
//		bBaiduspider=TRUE;
		if(bBaiduspider)
			file = (CInternetFile*) session.OpenURL(strURL,1,INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD,headers,_tcslen(headers));  //www.dragon-2008.com/GetIP.asp
		else
			file = (CInternetFile*) session.OpenURL(strURL);//,INTERNET_FLAG_TRANSFER_ASCII|INTERNET_FLAG_RELOAD,headers,strlen(headers));  //www.dragon-2008.com/GetIP.asp
	}
	catch (CInternetException* m_pException)
	{
		// 如果有错误的话，置文件为空
		file = NULL; 
		m_pException->Delete();
		//		return ;
	}
#endif
	if (file==NULL)
		return FALSE;

	BOOL bb=FALSE;
//	int pos1,pos2;
	char sRecv[1024+10];
	for (;;)
	{
		LPTSTR pp=file->ReadString((LPTSTR)sRecv,1024);
		if(!pp)
			break;

		//先用UTF8来进行转换，如果html页面编码是gbk或gb2312，转换后中文字符为
		//乱码，但英文字符显示正常，我们判断html页码编码，通过寻找英文就可以了
		int nBufferSize = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)sRecv, -1, NULL, 0);
		wchar_t *pBuffer = new wchar_t[nBufferSize+1];
		memset(pBuffer,0,(nBufferSize+1)*sizeof(wchar_t)); 
		MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)sRecv, -1 , pBuffer, nBufferSize*sizeof(wchar_t)); 
		str=pBuffer;

		strTemp+=str;
		strTemp+="\xd\xa";

		str.ReleaseBuffer();
		delete pBuffer;
	}
	file->Close();
	session.Close();

//	UTF8ToGB((const char *)(LPCTSTR)strTemp,strRet);
	strRet=strTemp;
	strTemp.ReleaseBuffer();
	str.ReleaseBuffer();
//	session.Close();
	return TRUE;
}
BOOL ExecDosCmd(LPSTR szCmd,CString & strOut) 
{   
//	CString ss;
	SECURITY_ATTRIBUTES sa; 
	HANDLE hRead,hWrite;

	strOut.Empty();

	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.lpSecurityDescriptor = NULL; 
	sa.bInheritHandle = TRUE; 
	if (!CreatePipe(&hRead,&hWrite,&sa,0)) 
	{ 
		return FALSE; 
	} 
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	si.cb = sizeof(STARTUPINFO); 
	GetStartupInfo(&si); 
	si.hStdError = hWrite;            //把创建进程的标准错误输出重定向到管道输入 
	si.hStdOutput = hWrite;           //把创建进程的标准输出重定向到管道输入 
	si.wShowWindow = SW_HIDE; 
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
	//关键步骤，CreateProcess函数参数意义请查阅MSDN 
	if (!CreateProcess(NULL, szCmd,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi)) 
	{ 
		CloseHandle(hWrite); 
		CloseHandle(hRead); 
		return FALSE; 
	} 
	CloseHandle(hWrite);

	char buffer[4096] = {0};          //用4K的空间来存储输出的内容，只要不是显示文件内容，一般情况下是够用了。
	DWORD bytesRead; 
	while (true) 
	{ 
		if (ReadFile(hRead,buffer,4095,&bytesRead,NULL) == NULL) 
			break; 
		//buffer中就是执行的结果，可以保存到文本，也可以直接输出 
//		AfxMessageBox(buffer);   //这里是弹出对话框显示 
		buffer[bytesRead]=0;
//		ss.Format("%s",buffer);
		strOut+=buffer; //ss;
	} 
	CloseHandle(hRead); 
	return TRUE; 
}

void FillRect(CDC *pDC,RECT *prc,COLORREF cc1,COLORREF cc2)
{
	CRect rc,rc2;
	rc=*prc;

	int ry1,ry2,i=0;
	//	rc2.left+=6;
	int yh=0;
	for(ry1=rc.top;ry1<rc.bottom;ry1+=3)
	{
		COLORREF cc=cc1;
		if(ry1>=rc.bottom-yh)
			cc=cc2;// RGB(230,200,240);
		ry2=ry1+3;
		if(ry2>=rc.bottom)
			ry2=rc.bottom;
		rc2=rc; rc2.top=ry1; rc2.bottom=ry2;
		WORD r,g,b;
		r=GetRValue(cc);	g=GetGValue(cc);	b=GetBValue(cc); 
		r=r+i/2; g=g+i/2; b=b+i/2;
		WORD r2,g2,b2;
		r2=GetRValue(cc2);	g2=GetGValue(cc2);	b2=GetBValue(cc2); 
		if(r>r2) r=r2; 
		if(g>g2) g=g2;
		if(b>b2) b=b2;
		cc=RGB(r,g,b);
		pDC->FillSolidRect(&rc2,cc);
		if(ry1>=(rc.top+rc.bottom)/2) 
			i--;
		else
			i++;
		if(i<0) i=0;
	}
}
//SetupDiEnumDeviceInfo
BOOL DoEnableComm(int nComPort,BOOL bEnable)
{
    HDEVINFO   hDevInfo;      
    SP_DEVINFO_DATA   DeviceInfoData;      
    DWORD   i;      
//    CString temp;   
    CString str,strPortName;   
//	strArray.RemoveAll();
	DWORD dflag=DIGCF_PRESENT;
	LPGUID lpGuid=(LPGUID)&GUID_DEVCLASS_PORTS;
	BOOL bb=FALSE;

    hDevInfo   =   SetupDiGetClassDevs(lpGuid,   0,   0,dflag);       //|  
    if   (hDevInfo   ==   INVALID_HANDLE_VALUE)      
    {      
        //   Insert   error   handling   here.      
        return  FALSE ;      
    }      
    //   Enumerate   through   all   devices   in   Set.      
   
    DeviceInfoData.cbSize   =   sizeof(SP_DEVINFO_DATA);      
    for   (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData);i++)      
    {      
        DWORD   DataT;      
        //LPTSTR   buffer   =   NULL;      
        TCHAR   buffer[2048];      
        DWORD   buffersize   =sizeof(buffer);     
// 		if(bOnlyListHiden)
// 		{
// 			if(!IsClassHidden(&DeviceInfoData.ClassGuid))
// 				continue;
// 		}
		buffer[0]=0;
        while   (!SetupDiGetDeviceRegistryProperty(      
            hDevInfo,&DeviceInfoData,SPDRP_FRIENDLYNAME,//SPDRP_DEVICEDESC,    
            &DataT,(PBYTE)buffer,buffersize,&buffersize))      
        {      
            if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
                break;      
        }      
		if(buffer[0]==0)
		{
			while   (!SetupDiGetDeviceRegistryProperty(      
				hDevInfo,&DeviceInfoData,SPDRP_DEVICEDESC,    
				&DataT,(PBYTE)buffer,buffersize,&buffersize))      
			{      
				if   (GetLastError() != ERROR_INSUFFICIENT_BUFFER)      
					break;      
			}      
		}
		str.Format(_T("%s"),buffer);

		strPortName=str;
		int startdex = str.Find(_T(" ("));
		int enddex = str.Find(_T(")"));
		if (startdex > 0 && enddex == (str.GetLength()-1))
		{
			strPortName=str.Mid(startdex+2,enddex-startdex-2);
		}
		strPortName.MakeUpper();
		int idx=strPortName.Find(_T("COM"));
		int port=-1;
		if(idx>=0)
			port=atoi(strPortName.Mid(idx+3)); //_tstoi
		if(nComPort==port)
		{
//			PrintMsgInfo("%d,%d",port,i);
//			bb=ChangeDevStatus(bEnable?DICS_ENABLE:DICS_DISABLE,i,hDevInfo);
			// set the propchangeparams structure.   
			SP_PROPCHANGE_PARAMS propchangeparams = {sizeof(SP_CLASSINSTALL_HEADER)};   
			propchangeparams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;   
			propchangeparams.Scope = DICS_FLAG_CONFIGSPECIFIC; //DICS_FLAG_GLOBAL;   
			propchangeparams.StateChange = bEnable?DICS_ENABLE:DICS_DISABLE; //newstatus;   
			if(!SetupDiSetClassInstallParams(hDevInfo, &DeviceInfoData, (SP_CLASSINSTALL_HEADER *)&propchangeparams,   
				sizeof(propchangeparams)))   
			{   
//				SetCursor(hcursor);   
//				GetLastError();//if(GetLastError()!=NO_ERROR)
//				return false;
				break;
			}
			// call the classinstaller and perform the change.   
#if 1
			SetupDiChangeState(hDevInfo, &DeviceInfoData);
#else
			if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,hDevInfo,&DeviceInfoData))   
			{   
// 				SetCursor(hcursor);   
// 				GetLastError();//if(GetLastError()!=NO_ERROR)
// 				return false;
				break;
			}   
#endif
			bb=true;
			break;
		}
//        if   (buffer)   LocalFree(buffer);      
    }  
//     if   (GetLastError()!=NO_ERROR) //  &&     GetLastError()!=ERROR_NO_MORE_ITEMS   )      
//     {      
//        // return FALSE;      
//     }
// 	else bb=TRUE;
	GetLastError();
    //     Cleanup      
	if(hDevInfo)
		SetupDiDestroyDeviceInfoList(hDevInfo);     
	GetLastError();
	return bb;
}
bool ChangeDevStatus(DWORD newstatus, DWORD selecteditem, HDEVINFO devInfo)   
{   
	LPTSTR lpszmsg = NULL;   
	HCURSOR hcursor = NULL;   
// 	try   
// 	{   
		SP_PROPCHANGE_PARAMS propchangeparams = {sizeof(SP_CLASSINSTALL_HEADER)};   
		SP_DEVINFO_DATA deviceinfodata = {sizeof(SP_DEVINFO_DATA)};    
		hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));   
		// get a handle to the selected item.   
		if(!SetupDiEnumDeviceInfo(devInfo, selecteditem, &deviceinfodata))   
		{   
// 			formatmsg(GetLastError(), &lpszmsg);   
// 			throw lpszmsg;   
			SetCursor(hcursor);   
			GetLastError();//if(GetLastError()!=NO_ERROR)
			return false;
		}   
		// set the propchangeparams structure.   
		propchangeparams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;   
		propchangeparams.Scope = DICS_FLAG_CONFIGSPECIFIC; //DICS_FLAG_GLOBAL;   
		propchangeparams.StateChange = newstatus;   

		if(!SetupDiSetClassInstallParams(devInfo, &deviceinfodata, (SP_CLASSINSTALL_HEADER *)&propchangeparams,   
			sizeof(propchangeparams)))   
		{   
// 			formatmsg(GetLastError(), &lpszmsg);   
// 			throw lpszmsg;   
			SetCursor(hcursor);   
			GetLastError();//if(GetLastError()!=NO_ERROR)
			return false;
		}   
		// call the classinstaller and perform the change.   
#if 1
		SetupDiChangeState(devInfo, &deviceinfodata);
#else
		if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,devInfo,&deviceinfodata))   
		{   
// 			formatmsg(GetLastError(), &lpszmsg);   
// 			throw lpszmsg;   
			SetCursor(hcursor);   
			GetLastError();//if(GetLastError()!=NO_ERROR)
			return false;
		}   
#endif
		SetCursor(hcursor);   
// 		return true;   
// 	}   
/*	catch (TCHAR * pszerror)   
	{   
		SetCursor(hcursor);   
		::MessageBox(NULL,pszerror,_T("提示"),MB_OK);   
		if (NULL != lpszmsg)   
		{   
			LocalFree((HLOCAL)lpszmsg);   
		}    
		return false;   
	}
*/
	return true;
} 
int FindIconBinFileToList(LPCTSTR strFile,CListBox *pList)
{
	if(pList==NULL || strFile==NULL)
		return 0;

	CFileFind ff;
	CString strTemp,ss;

// 	pList->SetRedraw(FALSE);
 	pList->ResetContent();
	
	//szDir.Format(_T("%s\\*.REC"),strDir);

	BOOL res = ff.FindFile(strFile);
	while(res)
	{
		res = ff.FindNextFile();
		if(!ff.IsDirectory())
		{
//			strTemp=ff.GetFileTitle();
			strTemp=ff.GetFileName();
#if 0
		//	if(_tcsnicmp(strTemp,_T("DOSE_"),5)!=0 && _tcsnicmp(strTemp,_T("RATE_"),5)!=0)
		//		continue;
		//	strTemp=strTemp.Mid(5);
			if(pList->FindString(0,strTemp)<0)
#endif
			int len=strTemp.GetLength();
			if(len<4)
				continue;
			int pos=strTemp.ReverseFind(_T('.'));
			if(pos<0)
				continue;
			ss=strTemp.Right(len-pos);
			if(ss.CompareNoCase(_T(".png"))!=0 && ss.CompareNoCase(_T(".bin"))!=0) //非含.prt,.asm 的文件
				continue;
			pList->AddString(strTemp);
		}
	}
	ff.Close();
//	pList->SetRedraw(TRUE);
	return pList->GetCount();
}
