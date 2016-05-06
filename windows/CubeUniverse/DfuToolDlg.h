
// DfuToolDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Label.h"
#include "CircleButton1.h"
#include <getopt.h>
#include <main.h>
//#include "HelpFrame.h"
//#include "HelpView.h"
#ifdef USE_ONEKEY_DOWNLOAD
#include "libwdi.h"
#include "msapi_utf8.h"
#endif
#include "afxlistctrl.h"
#include "WaitingDlg.h"

#define MY_FINISH_MESSAGE	WM_USER+106	
enum {MYFM_OK=0, MYFM_GETFILELEN=3,MYFM_LOADFILE=4};

typedef struct tagCUBEBININFO {
	int id;
	TCHAR szTitle[128];
	TCHAR szBinUrl[MAX_PATH];
	TCHAR szIconUrl[MAX_PATH];
	TCHAR szLastModified[30];
} CUBEBININFO;
enum {CBIFIELD_ID,CBIFIELD_TITLE,CBIFIELD_BIN,CBIFIELD_ICON,CBIFIELD_TIME,CBIFIELD_NUM};

// CDfuToolDlg dialog
class CDfuToolDlg : public CDialog
{
// Construction
public:
	CDfuToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef USE_ONEKEY_DOWNLOAD
	enum { IDD = IDD_DFUTOOL_DIALOG_ONEKEY };
#else
	enum { IDD = IDD_DFUTOOL_DIALOG };
#endif
	CLabel m_LabelWorkInfo;
//	CLabel m_LabelNote;
//	CString m_strEditUpdateFile[MAX_UPDATE_NUM];
//	CEdit m_EditPrompt;
//	CCircleButton m_btnLampDfu;
//	CCircleButton m_btnLampUpdate[1+MAX_UPDATE_NUM];	//0:AUTO,1-3:UPDATE1-3
#ifdef USE_ONEKEY_DOWNLOAD
	CLinkCtrl m_LinkCtrl1;//,m_LinkCtrl2;
#endif
	CListBox m_ListFile;
	CFont	m_Font1,m_Font2,m_Font3;
//	CHelpFrame *m_pHelpFrame;
//	CHelpFrame *m_pDfuHelpFrame;

	HANDLE m_hWaitEvent;
	int m_nDfuMode;
	TCHAR m_szDfuModeInfo[MAX_PATH];

	int	m_nCurWebFileTotalLen;	//当前要从网上下载的文件总长度
	//取nItem项要从网上下载的文件长度,返回-1,-2则不成功
	int GetWebFileLen(CString & strFileURL);
	//
	void SetLamp(CCircleButton *pBtn,int nLampID);

	void UpdateParam(BOOL bSaveToTheApp=TRUE);
	void SetProgressInfo(int pos);
	void ClearItemInfo(int nItem=-1);
	void EnableControls();
	//将文件名中的当前程序目录名去掉，置到pWnd标题,以便简洁
	void SetFileNameNoCurrentDir(CWnd *pWnd,LPCTSTR strFileName);

	BOOL DoCheckDfuMode(BOOL bPrompt=TRUE);
	BOOL MakeUpdateCmd();
#if 0
	void DoHelp();
#endif

	volatile BOOL m_bExit;
	BOOL DownloadFile(CString & strFileURL,CString & strOutFile,BOOL bPrompt=TRUE);


//	BOOL UpdateFile();
//	volatile int m_nUpdateItem;
	volatile BOOL m_bAutoUpdating;
	volatile int m_nAutoUpdateItemN;
#ifdef USE_ONEKEY_DOWNLOAD
	static UINT AutoFixcubeThread(LPVOID pParam);
	BOOL DoFixcube();
//	BOOL AutoUpdateFile();
	//在列表框里显示有pszHasNameIn串的设备,若pszHasNameIn=NULL,则显示所有,返回列表框项数
	int display_devices(struct wdi_device_info *list,char *pszHasNameIn=NULL);
	struct wdi_device_info* GetItemDevice(int nIdx);

	struct wdi_options_install_driver m_stIdOptions;
	struct wdi_device_info *m_pstDevice, *m_pstListDev;
	struct wdi_options_create_list m_stClOptions; // = { 0 };
	int m_nCurDevIndex;
	int DoListAll();

	CArray<SSerInfo,SSerInfo&> m_arrSI;
	int DoScanComm();
	BOOL SwitchToDfuMode();		//切换到DFU模式
	//安装(Photon with WiFi时)usb2com或(Photon DFU Mode时的)libusbk驱动
	BOOL DoInstallDriver(wdi_device_info *dev);
	//检查是否有Dfu模式的驱动，无则安装
	BOOL CheckInstallDfuModeDriver();
	//检查是否有Usb2com的驱动，无则安装
	BOOL CheckInstallUsb2comDriver();
#define DESCNAME_DFUMODE	"Maple 003"
#define DESCNAME_USBSER		"Maple"
	int CheckDescNameIsInDevList(char descname[],UINT64 *pVer64=NULL);
#endif
	TCHAR m_szFileUrl[MAX_PATH];
	TCHAR m_szFixDemoFile[MAX_PATH];
//	void DoFixDemo(LPCTSTR lpszFile);
	void DoDownloadFile_FixDemo(LPCTSTR strFileUrl,LPCTSTR strOutFile);
	int AutoUpdateDemoFile();

	CWaitingDlg *m_pWaitingDlg;
	void CreateWaitingWnd();
	void CloseWaitingWnd();
	int m_nWinVer;
	BOOL m_bX64;
//	BOOL m_bShowWorkInfo;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	virtual void OnOK();
	virtual void OnCancel();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
#if 0
	afx_msg void OnDfuHelp();
	afx_msg void OnInstallLibusbHelp();
#endif
	afx_msg LRESULT OnMyPrintMessage(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnMySetProgressMessage(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnMyDfuModeMessage(WPARAM wParam,LPARAM lParam);

#ifdef USE_ONEKEY_DOWNLOAD
	afx_msg void OnBnClickedBtnListAll();
	afx_msg void OnBnClickedBtnToDfuMode();
	afx_msg void OnBnClickedBtnInstall();
	afx_msg void OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult);
#endif
	afx_msg LRESULT OnMyFinishMessage(WPARAM wParam,LPARAM lParam);
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMClickListCtrlBin(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnNMDblclkListBin(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnInstalldfu();

	CListCtrl m_ListCtrlBin;
	CImageList m_ImageListBin;
	CArray<CUBEBININFO,CUBEBININFO&> m_cbi;

	BOOL GetCubeBinInfo(CString & strInfo,CUBEBININFO *pstCBI);
	int FindCubeBinInfo(CString & strInfo,CArray<CUBEBININFO,CUBEBININFO&> &cbi);
	
	int ToLocalBinInfo(CArray<CUBEBININFO,CUBEBININFO&> &cbi);
	//在本地找出已下载png和bin文件
	int FindLocalCubeBinInfo(CArray<CUBEBININFO,CUBEBININFO&> &cbi);

	time_t m_tLastRefresh;


	CDC m_MemDC;
	CBitmap m_bmpBin,*m_pOldBmp;
	CBitmap m_bmpUnknow;
	CImage m_imageBK;;
public:
	void DoUpdateFromListBinItem(int nItemNo);

	volatile BOOL m_bRefreshListBinWorking;
	void DoRefreshListBin();
	static UINT RefreshListBinThread(LPVOID pParam);
	int RefreshListBin();

	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnRefreshListbin();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnUpdateCube();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
};
