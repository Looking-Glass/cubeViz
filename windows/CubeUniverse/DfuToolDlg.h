
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

	int	m_nCurWebFileTotalLen;	//��ǰҪ���������ص��ļ��ܳ���
	//ȡnItem��Ҫ���������ص��ļ�����,����-1,-2�򲻳ɹ�
	int GetWebFileLen(CString & strFileURL);
	//
	void SetLamp(CCircleButton *pBtn,int nLampID);

	void UpdateParam(BOOL bSaveToTheApp=TRUE);
	void SetProgressInfo(int pos);
	void ClearItemInfo(int nItem=-1);
	void EnableControls();
	//���ļ����еĵ�ǰ����Ŀ¼��ȥ�����õ�pWnd����,�Ա���
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
	//���б������ʾ��pszHasNameIn�����豸,��pszHasNameIn=NULL,����ʾ����,�����б������
	int display_devices(struct wdi_device_info *list,char *pszHasNameIn=NULL);
	struct wdi_device_info* GetItemDevice(int nIdx);

	struct wdi_options_install_driver m_stIdOptions;
	struct wdi_device_info *m_pstDevice, *m_pstListDev;
	struct wdi_options_create_list m_stClOptions; // = { 0 };
	int m_nCurDevIndex;
	int DoListAll();

	CArray<SSerInfo,SSerInfo&> m_arrSI;
	int DoScanComm();
	BOOL SwitchToDfuMode();		//�л���DFUģʽ
	//��װ(Photon with WiFiʱ)usb2com��(Photon DFU Modeʱ��)libusbk����
	BOOL DoInstallDriver(wdi_device_info *dev);
	//����Ƿ���Dfuģʽ������������װ
	BOOL CheckInstallDfuModeDriver();
	//����Ƿ���Usb2com������������װ
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
	//�ڱ����ҳ�������png��bin�ļ�
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
