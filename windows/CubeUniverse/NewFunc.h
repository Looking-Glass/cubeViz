
#include <afxtempl.h>

#pragma once

#ifndef GET_CHANBIT
#define GET_CHANBIT(bitbuf,chan)	(bitbuf[(chan)/8]&(1<<((chan)%8)))
#define SET_CHANBIT(bitbuf,chan)	(bitbuf[(chan)/8]|=(1<<((chan)%8)))
#define CLR_CHANBIT(bitbuf,chan)	(bitbuf[(chan)/8]&=~(1<<((chan)%8)))
#endif

#ifndef GET_BYTEBIT
#define GET_BYTEBIT(bv,n)			((bv)&(1<<n))
#define SET_BYTEBIT(bv,n)			(bv)|=(1<<n)
#define	CLR_BYTEBIT(bv,n)			(bv)&=~(1<<n)
#endif

//Color List
#ifndef LTCYAN
#define BLACK	 RGB(0,0,0)	  //0
#define BLUE	 RGB(0,0,255)	  //1
#define GREEN	 RGB(0,255,0)	  //2
#define CYAN	 RGB(0,255,255)   //3
#define RED	 RGB(255,0,0)	  //4
#define MAGENTA  RGB(128,0,0)	  //5
#define BROWN	 RGB(128,128,0)   //6
#define LTGRAY	 RGB(192,192,192) //7
#define DARKGRAY RGB(128,128,128) //8
#define LTBLUE	 RGB(0,0,128)	  //9
#define LTGREEN  RGB(0,128,0)	  //10
#define LTCYAN	 RGB(0,128,128)   //11
#define LTRED	 RGB(255,0,255)   //12
#define LTMAGENTA RGB(128,0,128)  //13
#define YELLOW	 RGB(255,255,0)   //14
#define WHITE	 RGB(255,255,255) //15
#define GRAY	 DARKGRAY	  //8
#define ORANGE	 BROWN		  //6
#endif

enum {TIMEMODE_HOUR=0,TIMEMODE_MIN=1,TIMEMODE_SEC=2,
	TIMEMODE_MARK_UNIT1=0x8000,	//只用最简缩写(一个字符)表示单位
	TIMEMODE_MARK_UNIT2=0x4000	//只用正常缩写(一个或多字符)表示单位
};
LPTSTR ToTimeStr(time_t tv,int nTimeMode,LPTSTR szOutStr);
LPTSTR ToTimeStr(time_t tv,int nTimeMode,CString & strOut);

void AddStrToFile(LPCTSTR lpfname,LPCTSTR szStr,BOOL bAddCurTime=TRUE);

//将nValue赋给串str,当nValue=0:清空,否则转换int值
void SetStringInt(CString & str,int nValue);
//转换串str为UINT值,为负数时,用绝对值
UINT GetStringUInt(CString & str);

//取日期数字串，调用GetTimeDigitStr,m=0
char *GetDateStr(time_t tTime,char sdate[]);
//取时间的数字串,存于szTime[],m=0:日期,如:"070203"(2007-02-03),m=1:日期时间,"070203123000"
//m=2:没有秒的日期时间 "0702031230"
char *GetTimeDigitStr(time_t tTime,char szTime[],int m);
//取当前时间串，用的格式"%Y-%m-%d %H:%M:%S"
CString GetCurrentTimeStr();
CString GetCurrentTimeStr(time_t tTime);
//返回ttime的日期时间串(按"%Y-%m-%d %H:%M:%S"格式) 
char *GetDateTimeStr(time_t ttime,char *str);

int PrintMsgWarning(LPCTSTR fmt, ...);
int PrintMsgError(LPCTSTR fmt, ...);
int PrintMsgInfo(LPCTSTR fmt, ...);
//void PrintPrompt(LPCTSTR fmt, ...);	
void PrintMsgToFile(LPCTSTR fname,LPCTSTR fmt, ...);

//取cBitByte[]位数,nMaxBitNum:最大位数
int GetBitNum(BYTE cBitByte[],int nMaxBitNum);
//取位表中有效字节数(到最后一个有1的位占的字节数),nMaxBitNum:最大位数
int GetValidBitByteNum(BYTE cBitByte[],int nMaxBitNum);

void PrintHexMessage(LPCTSTR lpBuf,int nlen);
CString ToHexStr(BYTE buf[],int len);

void EnableControl(CWnd *pWnd,UINT id[],int num,BOOL bEnable=TRUE);
void ShowControl(CWnd *pWnd,UINT id[],int num,BOOL bShow=TRUE);

void LocateWindow(CWnd *pWnd,CWnd *pLocationWnd,CWnd *pParent);
void LocateWindow(UINT nID,UINT nLocationID,CWnd *pParent);


CString OpenDir(LPCTSTR  cDlgName);  
BOOL IsFileNameValid(LPCTSTR lpFileName);
int Touch(LPCTSTR lpPath, BOOL bValidate);
//void BackupOldChlDateFile(const char *pszChlDateFile);
int round(double fv);

LPITEMIDLIST ParsePidlFromPath(LPCSTR path);

//将szStr串中有','号隔开的数取出，存于fResult[]中,nMaxN:最多取的个数,返回实际个数
//如 1.23,45.6,12.45 等
int GetFloatFromStr(IN LPCTSTR szStr,OUT float fResult[],IN int nMaxN);
//将fResult[]中的浮点数转换在串存于szStr中(用','号隔开),返回实际个数,szStr[]最大MAX_PATH长,超长度则返回0
//如 1.23,45.6,12.45 等
int SetFloatToStr(IN float fResult[],IN int nNum,OUT LPTSTR szStr);

//从文件全名从取文件夹名称(目录名)
CString GetFolderFromFullpath(LPCTSTR lpFullpath,OUT TCHAR *pFileName=NULL,OUT TCHAR *pExtName=NULL);
//取文件名串lpszStr路径后面的文件名,返回该地址,名长度(不包括扩展名)在pLen中
TCHAR *GetFileName(LPCTSTR lpszStr,int *pLen);

/* Helper functions to access DLLs */
static __inline HMODULE GetDLLHandle(char* szDLLName)
{
	HANDLE h = GetModuleHandleA(szDLLName);
	if (h == NULL) {
		// coverity[alloc_fn][var_assign]
		h = LoadLibraryA(szDLLName);
	}
	return (HMODULE)h;
}

BOOL is_x64(void);

// Windows versions
enum WindowsVersion {
	WINDOWS_UNDEFINED = -1,
	WINDOWS_UNSUPPORTED = 0,
	WINDOWS_XP = 0x51,
	WINDOWS_2003 = 0x52,	// Also XP x64
	WINDOWS_VISTA = 0x60,
	WINDOWS_7 = 0x61,
	WINDOWS_8 = 0x62,
	WINDOWS_8_1_OR_LATER = 0x63,
	WINDOWS_MAX
};

#define safe_free(p) do {if ((void*)p != NULL) {free((void*)p); p = NULL;}} while(0)
#define safe_min(a, b) min((size_t)(a), (size_t)(b))
#define safe_strcp(dst, dst_max, src, count) do {memcpy(dst, src, safe_min(count, dst_max)); \
	((char*)dst)[safe_min(count, dst_max)-1] = 0;} while(0)
#define safe_strcpy(dst, dst_max, src) safe_strcp(dst, dst_max, src, safe_strlen(src)+1)
#define safe_strncat(dst, dst_max, src, count) strncat(dst, src, safe_min(count, dst_max - safe_strlen(dst) - 1))
#define safe_strcat(dst, dst_max, src) safe_strncat(dst, dst_max, src, safe_strlen(src)+1)
#define safe_strcmp(str1, str2) strcmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_stricmp(str1, str2) _stricmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2))
#define safe_strncmp(str1, str2, count) strncmp(((str1==NULL)?"<NULL>":str1), ((str2==NULL)?"<NULL>":str2), count)
#define safe_closehandle(h) do {if (h != INVALID_HANDLE_VALUE) {CloseHandle(h); h = INVALID_HANDLE_VALUE;}} while(0)
#define safe_sprintf(dst, count, ...) do {_snprintf(dst, count, __VA_ARGS__); (dst)[(count)-1] = 0; } while(0)
#define safe_strlen(str) ((((char*)str)==NULL)?0:strlen(str))
#define safe_strdup _strdup
#define MF_CHECK(cond) ((cond)?MF_CHECKED:MF_UNCHECKED)
#define IGNORE_RETVAL(expr) do { (void)(expr); } while(0)

#if defined(_MSC_VER)
#define safe_vsnprintf(buf, size, format, arg) _vsnprintf_s(buf, size, _TRUNCATE, format, arg)
#else
#define safe_vsnprintf vsnprintf
#endif

const char* PrintWinVersion(OUT int & nWindowsVersion);

bool LoadImageFromResource(IN CImage* pImage,IN UINT nResID, IN LPCTSTR lpTyp=_T("PNG"));

// Struct used when enumerating the available serial ports
// Holds information about an individual serial port.
struct SSerInfo {
	SSerInfo() : bUsbDevice(FALSE) {}
	CString strDevPath;          // Device path for use with CreateFile()
	CString strPortName;         // Simple name (i.e. COM1)
	CString strFriendlyName;     // Full name to be displayed to a user
	BOOL bUsbDevice;             // Provided through a USB connection?
	CString strPortDesc;         // friendly name without the COMx
	int		nComPort;			 // 1~
	CString strHID;
};
BOOL EnumSerialPorts(CArray<SSerInfo,SSerInfo&> &asi, BOOL bIgnoreBusyPorts);

BOOL DoEnableComm(int nComPort,BOOL bEnable);

void ListCtrlInit(CListCtrl *pList);
void ListCtrlBuildColumns(CListCtrl *pList,int nCols, int * nWidth, LPCTSTR pFieldName[]);

BOOL LoadFileToString(LPCSTR strFile,CString & strOut);
BOOL SaveToFile(LPCTSTR strFile,void *buff,int len);

BOOL GetURLReturnStr(LPCTSTR strURL,CString & strRet,BOOL bBaiduspider=FALSE);

BOOL ExecDosCmd(LPSTR szCmd,CString & strOut);

void FillRect(CDC *pDC,RECT *prc,COLORREF cc1,COLORREF cc2);

int FindIconBinFileToList(LPCTSTR strFile,CListBox *pList);
