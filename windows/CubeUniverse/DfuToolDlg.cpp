
// DfuToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DfuTool.h"
#include "DfuToolDlg.h"
#include <afxinet.h>
#include <io.h>
#include "registry.h"
#include "WaitingDlg.h"
//#define USE_LIBUSBK
//#include "DfuHelpView.h"

#define MAX_AUTOREFRESH_TIME	3600

#define BINICON_W	112
#define BINICON_H	112

#define BINICON_GAP	16

#define MAX_BINFILE_LEN	(108*1024)
//#define LIBUSB0_VER_INT64 0x0001000200060000l		//libusb0 v1.2.6.0
#ifdef USE_LIBUSBK
static WORD s_wLibusb0[4]={0,7,0,3};	//libusb0 v1.2.6.0
#else
static WORD s_wLibusb0[4]={0,6,2,1};	//libusb0 v1.2.6.0
#endif
#define STR_INNAME		"Maple"
//#define STR_INNAME		"502"
//#define USE_MYDEBUG
/*
在DFU模式下，调用附件dfu-util.exe, 然后用命令dfu-util -a 1 -D binary.bin 把程序写进单片机。
*/
//// http://6x.cubetube.org/apps/

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#ifdef USE_ONEKEY_DOWNLOAD
#include "comm.h"	//加串口是为切换到DFU模式

#define BPS_SWITCHTODFU		9600
CComm gl_Comm;		//用于切换到DFU,以14400波特率打开串口，实现切换


static wdi_device_info s_mydev_Dfumode={
	NULL,
	0x1EAF,0x0003,	//vid,pid
	false,			//BOOL is_composite
	0,				//unsigned char mi;	
	DESCNAME_DFUMODE,//"Maple 003",		//char* desc;
	NULL,			//"libusb0" //char* driver;
	"USB\\VID_1EAF&PID_0003\\LLM_003",//char* device_id;
	"USB\\VID_1EAF&PID_0003&REV_0200",	//char* hardware_id;
	"USB\\Class_FE&SubClass_01&Prot_02", //char* compatible_id;
	NULL,						//char* upper_filter;
	0		//UINT64 driver_version;
};
static wdi_device_info s_mydev_Usbser={
	NULL,
	0x1EAF,0x0004,	//vid,pid
	false,			//BOOL is_composite
	0,				//unsigned char mi;	
	DESCNAME_USBSER,//"Maple",		//char* desc;
	NULL,			//"usbser" //char* driver;
	"USB\\VID_1EAF&PID_0004\\5&2AF2411A&0&2",//char* device_id;
	"USB\\VID_1EAF&PID_0004&REV_0200",	//char* hardware_id;
	"USB\\Class_02&SubClass_02&Prot_01", //char* compatible_id;
	NULL,						//char* upper_filter;
	0		//UINT64 driver_version;
};
/*
libwdi:debug [wdi_create_list] Hardware ID: USB\VID_1EAF&PID_0004&REV_0200
libwdi:debug [wdi_create_list] Compatible ID: USB\Class_02&SubClass_02&Prot_01
libwdi:warning [wdi_create_list] could not read driver version
libwdi:debug [wdi_create_list] usbser USB device (7): USB\VID_1EAF&PID_0004\5&2AF2411A&0&2
libwdi:debug [wdi_create_list] Device description: 'Maple'
*/
/*
 * Converts a name + ext UTF-8 pair to a valid MS filename.
 * Returned string is allocated and needs to be freed manually
 */
char* to_valid_filename(char* name, char* ext);
char* to_valid_filename(char* name, char* ext)
{
	size_t i, j, k;
	BOOL found;
	char* ret;
	wchar_t unauthorized[] = L"\x0001\x0002\x0003\x0004\x0005\x0006\x0007\x0008\x000a"
		L"\x000b\x000c\x000d\x000e\x000f\x0010\x0011\x0012\x0013\x0014\x0015\x0016\x0017"
		L"\x0018\x0019\x001a\x001b\x001c\x001d\x001e\x001f\x007f\"*/:<>?\\|,";
	wchar_t to_underscore[] = L" \t";
	wchar_t *wname, *wext, *wret;

	if ((name == NULL) || (ext == NULL)) {
		return NULL;
	}

	if (strlen(name) > WDI_MAX_STRLEN) return NULL;

	// Convert to UTF-16
	wname = utf8_to_wchar(name);
	wext = utf8_to_wchar(ext);
	if ((wname == NULL) || (wext == NULL)) {
		safe_free(wname); safe_free(wext); return NULL;
	}

	// The returned UTF-8 string will never be larger than the sum of its parts
	wret = (wchar_t*)calloc(2*(wcslen(wname) + wcslen(wext) + 2), 1);
	if (wret == NULL) {
		safe_free(wname); safe_free(wext); return NULL;
	}
	wcscpy(wret, wname);
	safe_free(wname);
	wcscat(wret, wext);
	safe_free(wext);

	for (i=0, k=0; i<wcslen(wret); i++) {
		found = FALSE;
		for (j=0; j<wcslen(unauthorized); j++) {
			if (wret[i] == unauthorized[j]) {
				found = TRUE; break;
			}
		}
		if (found) continue;
		found = FALSE;
		for (j=0; j<wcslen(to_underscore); j++) {
			if (wret[i] == to_underscore[j]) {
				wret[k++] = '_';
				found = TRUE; break;
			}
		}
		if (found) continue;
		wret[k++] = wret[i];
	}
	wret[k] = 0;
	ret = wchar_to_utf8(wret);
	safe_free(wret);
	return ret;
}
#endif

static char *s_strCmd="dfu-util -a 1 -D ";
static char s_strCurCmd[MAX_PATH]="";
static char s_strCmdPath[MAX_PATH]="";
static char s_strCmdParam[MAX_PATH]="";

#define MPTMSG_WORKINFO		11

#define PRINTPROMPT(...) do { char ss[2048];\
	sprintf(ss, __VA_ARGS__); SendMessage(MY_PRINT_MESSAGE,1,(LPARAM)ss); } while (0)

#define PRINTWORKINFO(...) do { static TCHAR ss[2048];\
	sprintf(ss, __VA_ARGS__); SendMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)ss); } while (0)

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDfuToolDlg dialog


CDfuToolDlg::CDfuToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDfuToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	int fw,fh;
// 	m_Font1.CreateFont(14,6, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
// 		OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
// 		DEFAULT_PITCH | FF_DONTCARE, _T("Arial")); //Microsoft Sans Serif
#ifdef USE_ONEKEY_DOWNLOAD
	fw=10; fh=20;
#else
	fw=6; fh=15;
#endif
	m_Font1.CreateFont(fh, fw, 0, 0, FW_NORMAL-100, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
// 	m_Font2.CreateFont(20,10, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
// 		OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
// 		DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));
#ifdef USE_ONEKEY_DOWNLOAD
	fw=16; fh=32;
#else
	fw=14; fh=30;
#endif
	fw=1; fh=1;
	m_Font2.CreateFont(fh,fw, 0, 0, FW_MEDIUM, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
	m_Font3.CreateFont(16, 8, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Courier New"));

//	m_pHelpFrame=NULL;m_pDfuHelpFrame=NULL;

	m_bExit=FALSE;
//	m_bDownloading=FALSE;

//	m_bUpdating=FALSE;
	m_bAutoUpdating=FALSE;

	m_nDfuMode=MDMSG_NO;
	m_szDfuModeInfo[0]=0;

	m_nCurWebFileTotalLen=0;	//当前要从网上下载的文件总长度

	m_hWaitEvent=CreateEvent(NULL,TRUE,FALSE,NULL);	

#ifdef USE_ONEKEY_DOWNLOAD
	memset(&m_stIdOptions,0,sizeof(m_stIdOptions));
	m_pstDevice=NULL; m_pstListDev=NULL;
	memset(&m_stClOptions,0,sizeof(m_stClOptions));
	m_nCurDevIndex=-1;
#endif	
	m_bmpUnknow.LoadBitmap(IDB_BITMAP_UNKNOW);
//	m_imageBK.LoadFromResource(AfxGetInstanceHandle(),IDB)
	LoadImageFromResource(&m_imageBK,IDB_PNG_BK);

	m_pWaitingDlg=NULL;
//	m_bShowWorkInfo=FALSE;
	m_bRefreshListBinWorking=FALSE;

	m_tLastRefresh=-1;
}

void CDfuToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_EDIT_PROMPT, m_EditPrompt);

	//	DDX_Control(pDX, IDC_BTN_DFULAMP, m_btnLampDfu);
	// 	for(int i=0;i<1+MAX_UPDATE_NUM;i++)
	// 	{
	// 		DDX_Control(pDX, IDC_BTN_AUTOLAMP+i, m_btnLampUpdate[i]);
	// 	}
#ifdef USE_ONEKEY_DOWNLOAD
	DDX_Control(pDX, IDC_SYSLINK1, m_LinkCtrl1);
#endif	
	DDX_Control(pDX, IDC_LIST1, m_ListCtrlBin);
	DDX_Control(pDX, IDC_STATIC_WORKINFO, m_LabelWorkInfo);
	DDX_Control(pDX, IDC_LIST_FILE, m_ListFile);
}

BEGIN_MESSAGE_MAP(CDfuToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP

#if 0
	ON_COMMAND(IDM_DFUHELP,OnDfuHelp)
	ON_COMMAND(IDM_INSTALLLIBUSBHELP,OnInstallLibusbHelp)
#endif
	ON_MESSAGE(MY_PRINT_MESSAGE, OnMyPrintMessage)
	ON_MESSAGE(MY_PROGRESS_MESSAGE, OnMySetProgressMessage)
	ON_MESSAGE(MY_DFUMODE_MESSAGE, OnMyDfuModeMessage)
#ifdef USE_ONEKEY_DOWNLOAD
//	ON_BN_CLICKED(IDC_BTN_FIXCUBE, OnBnClickedBtnFixcube)
	ON_BN_CLICKED(IDC_BTN_LISTALL, OnBnClickedBtnListAll)
	ON_BN_CLICKED(IDC_BTN_INSTALL, OnBnClickedBtnInstall)
	ON_BN_CLICKED(IDC_BTN_TODFUMODE, OnBnClickedBtnToDfuMode)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, OnNMClickSyslink)
//	ON_NOTIFY(NM_CLICK, IDC_SYSLINK2, OnNMClickSyslink)
#endif
	ON_MESSAGE(MY_FINISH_MESSAGE, OnMyFinishMessage)
	ON_WM_TIMER()
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnNMClickListCtrlBin)
	ON_WM_ERASEBKGND()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDfuToolDlg::OnNMDblclkListBin)
	ON_BN_CLICKED(IDC_BTN_INSTALLDFU, &CDfuToolDlg::OnBnClickedBtnInstalldfu)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_REFRESHLISTBIN, &CDfuToolDlg::OnRefreshListbin)
	ON_COMMAND(IDM_UPDATECUBE, &CDfuToolDlg::OnUpdateCube)
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()


// CDfuToolDlg message handlers

BOOL CDfuToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_bX64=is_x64();
	PrintWinVersion(m_nWinVer);
//	PrintMsgToFile("my.txt","%d,%d",m_nWinVer,m_bX64);
	CDC *pDC=GetDC();
	m_MemDC.CreateCompatibleDC(pDC);
	m_bmpBin.CreateCompatibleBitmap(pDC,BINICON_W,BINICON_H);
	m_pOldBmp=m_MemDC.SelectObject(&m_bmpBin);
	ReleaseDC(pDC);


#ifdef USE_ONEKEY_DOWNLOAD
//	GetDlgItem(IDC_BTN_FIXCUBE)->ShowWindow(SW_SHOW);
//	GetDlgItem(IDC_SYSLINK1)->ShowWindow(SW_SHOW);//	GetDlgItem(IDC_SYSLINK2)->ShowWindow(SW_SHOW);
//	m_LinkCtrl1.SetWindowText("Cube Doctor is released by LookingGlass Co.,Ltd.<a href=\"http://www.cubetube.org\">http://www.cubetube.org</a>");
//	m_LinkCtrl2.SetWindowText("Cube Doctor is released under the GPLv3. Source is available from <a href=\"https://github.com/enjrolas/CubeDoctor\">https://github.com/enjrolas/CubeDoctor</a>");
//	CButton *pbtn=(CButton *)GetDlgItem(IDC_BTN_FIXCUBE);
//	pbtn->SetFont(&m_Font1);
//	m_LabelWorkInfo.ShowWindow(SW_SHOW);
	m_LabelWorkInfo.SetWindowText(NULL);//_T("OK"));
//	GetDlgItem(IDC_STATIC_CURFILE)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CURFILE)->SetWindowText(NULL);//_T("OK"));
#ifdef USE_MYDEBUG
	UINT idv[]={IDC_COMBO_DEVLIST,IDC_COMBO_COMM,IDC_BTN_LISTALL,IDC_BTN_TODFUMODE,IDC_BTN_INSTALL,IDC_BTN_INSTALLDFU};
	ShowControl(this,idv,sizeof(idv)/sizeof(UINT),TRUE);
#endif
#endif
//	m_LabelTitle.SetFont(&m_Font2);
//	m_LabelTitle.SetTextColor(RGB(41,21,170));
// 	m_LabelNote.SetFont(&m_Font1);
// 	m_LabelNote.SetTextColor(RGB(170,41,21));
//	m_EditPrompt.SetFont(&m_Font3);
	m_LabelWorkInfo.SetFont(&m_Font3);
	m_LabelWorkInfo.SetTextColor(RGB(170,41,21));
	m_LabelWorkInfo.SetBkColor(RGB(30,31,41));
//	m_LabelNote.SetWindowText(strNote);

//	SetLamp(&m_btnLampAuto,0);
// 	for (int i=0;i<MAX_UPDATE_NUM+1;i++)
// 		SetLamp(&m_btnLampUpdate[i],IDB_LAMPGRAY);
//	SetLamp(&m_btnLampDfu,m_nDfuMode==MDMSG_DFU?IDB_LAMPYELLOW24:IDB_LAMPGRAY24);
	
	UpdateParam(FALSE);
//	SetTimer(1,1000,NULL);
#ifdef USE_ONEKEY_DOWNLOAD
	OnBnClickedBtnListAll();
#endif
	m_ImageListBin.Create(BINICON_W,BINICON_H,ILC_COLOR24,1,1);
	m_ListCtrlBin.SetFont(&m_Font2);
	m_ListCtrlBin.SetBkColor(0);

//	RefreshListBin();
	SetTimer(1,500,NULL);
//	DoRefreshListBin();
	CRect wrc,crc;
	GetWindowRect(&wrc);
	GetClientRect(&crc);
	ClientToScreen(&crc);
	int ww=m_imageBK.GetWidth()+wrc.Width()-crc.Width();
	int hh=m_imageBK.GetHeight()+wrc.Height()-crc.Height();
	crc.left=(wrc.left+wrc.right-ww)/2; 
	if(crc.left<0) crc.left=0;
	crc.right=crc.left+ww;
	crc.top=(wrc.top+wrc.bottom-hh)/2; 
	if(crc.top<0) crc.top=0;
	crc.bottom=crc.top+hh;
	MoveWindow(&crc);
	
	crc=CRect(440,30,440+BINICON_W*3+2*BINICON_GAP+40,30+BINICON_H*3+2*BINICON_GAP+10-4);
	m_ListCtrlBin.MoveWindow(&crc);
	CenterWindow();
	return TRUE;  // return TRUE  unless you set the focus to a control
}
#ifdef USE_ONEKEY_DOWNLOAD
//在列表框里显示有pszHasNameIn串的设备,若pszHasNameIn=NULL,则显示所有,返回列表框项数
int CDfuToolDlg::display_devices(struct wdi_device_info *list,char *pszHasNameIn)
{
	char szDev[MAX_PATH];
	char szName[MAX_PATH];
//	CString strTemp;
	struct wdi_device_info *dev;
	int index = -1;
	szName[0]=0;
	if(pszHasNameIn)
	{
//		strName.Format(_T("%s"),pszHasNameIn);	strName.MakeLower();
		safe_strcpy(szName,MAX_PATH-1,pszHasNameIn);
		strlwr(szName);
	}
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	if(pCombo==NULL)
		return 0;
	pCombo->ResetContent();
	m_nCurDevIndex=-1;
	if(list==NULL)
		return 0;
	for (dev = list; dev != NULL; dev = dev->next) {
		// Compute the width needed to accommodate our text
		if (dev->desc == NULL) {
//			dprintf("error: device description is empty");
			break;
		}
		BOOL bFind=FALSE;
		if(pszHasNameIn)
		{
//			strTemp.Format(_T("%s"),dev->desc);	//desc里有汉字的,后面用到Find(strName)可能出错
			safe_strcpy(szDev,MAX_PATH-1,dev->desc);
			//当desc里有汉字的,下面可能出错
// 			strTemp.MakeLower();
// 			if(strTemp.Find(strName)<0)	//没找到串则不加入
// 				continue;
			strlwr(szDev);
			if(strstr(szDev,szName)==NULL)
				continue;
 			bFind=TRUE;
		}
// 		strTemp.Format(_T("%s"),dev->desc);
// 		index = pCombo->AddString(strTemp);
		safe_strcpy(szDev,MAX_PATH-1,dev->desc);
		index = pCombo->AddString(szDev);
		if(index<0)
			continue;
		if(bFind || pszHasNameIn==NULL)
			m_nCurDevIndex=index;
		pCombo->SetItemData(index,(LPARAM)dev);
		// Select by Hardware ID if one's available
// 		if (safe_strcmp(current_device_hardware_id, dev->hardware_id) == 0) {
// 			current_device_index = index;
// 			safe_free(current_device_hardware_id);
// 		}
	}
	if(pCombo->GetCount()==0)
		return 0;
#if 0
	index=pCombo->FindString(0,"Photon with WiFi");
	if(index<0)
		index=pCombo->FindString(0,"Photon");
	if(index>=0)
		m_nCurDevIndex=index;
//	PrintMsgInfo("%d",index);
// 	_IGNORE(ComboBox_SetCurSel(hDeviceList, current_device_index));
	// Set the width to computed value
// 	SendMessage(hDeviceList, CB_SETDROPPEDWIDTH, max_width, 0);
#endif
	pCombo->SetCurSel(m_nCurDevIndex);
	return pCombo->GetCount();
//	return index;
}
void CDfuToolDlg::OnBnClickedBtnListAll()
{
	DoListAll();
	DoScanComm();
// 	if(IsExistLibusbk())
// 		PrintMsgInfo("Is Exist Libusbk");
}
int CDfuToolDlg::DoListAll()
{
	m_stClOptions.list_all=true;
	m_stClOptions.trim_whitespaces=true;
	if(m_pstListDev!=NULL)
	{
		wdi_destroy_list(m_pstListDev);
	}
//	PrintMsgToFile("2.txt","1");
	m_nCurDevIndex=-1;
	int r = wdi_create_list(&m_pstListDev, &m_stClOptions);
	if (r != WDI_SUCCESS) 
		return 0;
	int nm=display_devices(m_pstListDev,STR_INNAME);//PrintMsgInfo(_T("%s"),list->driver);

#ifdef USE_MYDEBUG
	CString strTemp,ss;
	wdi_device_info *device=GetItemDevice(m_nCurDevIndex);
	if(device==NULL)
		return nm;
	BOOL replace_driver = (device->driver != NULL);
	if (replace_driver) {
		if (device->driver_version == 0) {
			strTemp=device->driver;
		} else {
			strTemp.Format(_T("%s (v%d.%d.%d.%d)"), device->driver,
				(int)((device->driver_version>>48)&0xffff), (int)((device->driver_version>>32)&0xffff),
				(int)((device->driver_version>>16)&0xffff), (int)(device->driver_version & 0xffff));
		}
	} else {
		strTemp=_T("(NONE)");
	}
	// Display the VID,PID,MI
	ss.Format(_T(" VID=%04X,PID=%04X"),device->vid,device->pid);
	strTemp+=ss;
	if(device->is_composite)
	{
		ss.Format(_T(",MI=%02X"),device->mi);
		strTemp+=ss;
	}
//	SetDlgItemText(IDC_STATIC_WORKINFO,strTemp);
	PrintMsgToFile("my.txt",strTemp);
#endif
	return nm;
}
int CDfuToolDlg::DoScanComm() 
{
	int i,m=0;
	CString strTemp;
	m_arrSI.RemoveAll();
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_COMM);
	if(pCombo==NULL)
		return 0;
	pCombo->ResetContent();
	// Populate the list of serial ports.
	if(EnumSerialPorts(m_arrSI,FALSE/*include all*/))
	{
		int n=m_arrSI.GetSize();
		for (i=0; i<n; i++) 
		{
			//				strTemp.Format(_T("COM%d"),asi[i].nComPort);
			strTemp=m_arrSI[i].strFriendlyName.MakeLower();
//			PrintMsgInfo(_T("<%s>%s"),strTemp,m_arrSI[i].strPortName);
			if(m_nWinVer<WINDOWS_8)
			{
// 				if(strTemp.Find(_T("maple r3"))<0)	//有photon 标识的串口
// 					continue;
			}
			if(m_arrSI[i].strHID.Find(_T("VID_1EAF"))<0)
				continue;
			int idx=pCombo->AddString(m_arrSI[i].strPortName); //strTemp);
			pCombo->SetItemData(idx,m_arrSI[i].nComPort);
//					PrintMsgInfo(_T("<%s>\n%s"),m_arrSI[i].strFriendlyName,m_arrSI[i].strHID);
			m++;
		}
	}
	if(m>0) // theApp.m_nCommPort==m_CombSelComm.GetItemData(i))
	{
		pCombo->SetCurSel(0);
	}
	return m;
}
//切换到DFU模式
BOOL CDfuToolDlg::SwitchToDfuMode()
{
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_COMM);
	int i,n=pCombo->GetCount();
	if(n==0)
		return FALSE;
	int nComPort;
	TCHAR ss[100];
	CString strTemp;
	BOOL bb=FALSE;
	for (i=0; i<n; i++) 
	{
		nComPort=pCombo->GetItemData(i);
		strTemp.Format("%d,n,8,1",BPS_SWITCHTODFU);
		sprintf(ss,"COM%d",nComPort);
		if(!gl_Comm.OpenComm(ss,strTemp))
		{
//			DoEnableComm(nComPort,FALSE);
			DoEnableComm(nComPort,FALSE);
			Sleep(200);
			DoEnableComm(nComPort,TRUE);
//				continue;
			if(!gl_Comm.OpenComm(ss,strTemp))
				continue; //return FALSE;
		}
		Sleep(100);
		gl_Comm.SetDtr(TRUE);
		Sleep(10);
		gl_Comm.SetDtr(FALSE);
		Sleep(50);
		gl_Comm.SetDtr(TRUE);
		Sleep(200);
		gl_Comm.Write((BYTE*)"1EAF",4);
		Sleep(500);
		bb=TRUE;
	}
	gl_Comm.CloseCommPort();
	return TRUE;
}
void CDfuToolDlg::OnBnClickedBtnToDfuMode()
{
	BOOL bDfuMode=FALSE;
	bDfuMode=DoCheckDfuMode(FALSE);
	if(!bDfuMode)
	{
		DoListAll();	
		int n=DoScanComm();	//扫描串口
//		PrintMsgInfo("%d",n);
		if(n>0)
		{
			if(SwitchToDfuMode())
			{
//				PrintMsgInfo("ooo");
				Sleep(1000);
				DoListAll();	
				n=DoScanComm();	//扫描串口
				bDfuMode=DoCheckDfuMode(FALSE);
			}
		}
	}
	if(bDfuMode)
	{
		PrintMsgInfo(_T("Now Dfu Mode."));
	}
}
void CDfuToolDlg::OnBnClickedBtnInstall()
{
	wdi_device_info *dev=GetItemDevice(m_nCurDevIndex);
/*
PrintMsgToFile("xx.txt","pid=%04X,vid=%04X\xd\xa",dev->pid,dev->vid);
PrintMsgToFile("xx.txt","is_composite=%d  mi=%d\xd\xa",dev->is_composite,(int)dev->mi);
PrintMsgToFile("xx.txt","driver=%s\xd\xa device_id=%s\xd\xa",dev->driver,dev->device_id);
PrintMsgToFile("xx.txt","desc=%s\xd\xa hardware_id=%s\xd\xa",dev->desc,dev->hardware_id);
PrintMsgToFile("xx.txt","compatible_id=%s\xd\xa",dev->compatible_id);
PrintMsgToFile("xx.txt","upper_filter=%s\xd\xa driver_version=%X\xd\xa",dev->upper_filter,dev->driver_version);
pid=0003,vid=1EAF is_composite=0  mi=0 driver=libusb0
device_id=USB\VID_1EAF&PID_0003\LLM_003
desc=Maple 003
hardware_id=USB\VID_1EAF&PID_0003&REV_0200
compatible_id=USB\Class_fe&SubClass_01&Prot_02
upper_filter=(null) driver_version=60000
*/
	if(dev==NULL) //		return;
		dev=&s_mydev_Dfumode;
	DoInstallDriver(dev);
}
//检查是否有Dfu模式的驱动，无则安装
BOOL CDfuToolDlg::CheckInstallDfuModeDriver()
{
// 	wdi_device_info *dev=GetItemDevice(m_nCurDevIndex);
	const char* driver_display_name[WDI_NB_DRIVERS] = { "WinUSB", "libusb0", "libusbK", "Custom (extract only)" , "Custom (extract only)2" }; //libusb-win32
//	char descname[]="Photon with WiFi";
	char descname[]=DESCNAME_DFUMODE; //"Maple 003";
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	CString strTemp;
	int nm=pCombo->GetCount();
//	PrintMsgToFile("1.txt","nm=%d",nm);
	for (int i=0;i<nm;i++)
	{
		dev = (struct wdi_device_info*)pCombo->GetItemData(i);
		if(dev==NULL)
			continue;
		WORD *pW=(WORD *)&dev->driver_version;
//		if(memcmp(&dev->driver_version,s_wLibusb0,sizeof(s_wLibusb0))==0)
// 		if(dev->driver!=NULL)
// 			PrintMsgToFile("1.txt","%s:driver=%s,%X,%X,%X,%X",dev->desc,dev->driver,pW[3],pW[2],pW[1],pW[0]);
		if(strcmp(dev->desc,descname)!=0)
			continue;
		//#define LIBUSB0_VER_INT64 0x0001000200060000
		//没有则安装,版本不对也安装
		if(dev->driver==NULL|| memcmp(&dev->driver_version,s_wLibusb0,sizeof(s_wLibusb0))!=0)// (dev->driver_version>>32) !=LIBUSB0_VER_INT64_H|| (dev->driver_version&0xFFFFFFFF) !=LIBUSB0_VER_INT64_L) //v1.2.6.0
		{
			return DoInstallDriver(dev);
		}
		strTemp=dev->driver;
//		PrintMsgToFile("t.txt","driver=%s,%X,%X",dev->driver,dev->driver_version>>32,dev->driver_version&0xffffffff);
#ifdef USE_LIBUSBK
		if(strTemp.Find(driver_display_name[2])<0)
#else
		if(strTemp.Find(driver_display_name[1])<0)
#endif
		{
			return DoInstallDriver(dev);
		}
	}
	return FALSE;
}
//	char descname[]="Photon with WiFi";
int CDfuToolDlg::CheckDescNameIsInDevList(char descname[],UINT64 *pVer64)
{
	//	char descname[]="Photon DFU Mode";
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	CString strTemp;
	int nm=pCombo->GetCount();
	int m=0;
	if(pVer64)
		*pVer64=0;
	for (int i=0;i<nm;i++)
	{
		dev = (struct wdi_device_info*)pCombo->GetItemData(i);
		if(dev==NULL)
			continue;
		if(strcmp(dev->desc,descname)!=0)
			continue;
		m++;
//		return TRUE;
		if(pVer64)
			*pVer64=dev->driver_version;
	}
	return m;
}
//检查是否有Usb2com的驱动，无则安装
BOOL CDfuToolDlg::CheckInstallUsb2comDriver()
{
	const char* driver_display_name="usbser";
	char descname[]=DESCNAME_USBSER;
//	char descname[]="Photon DFU Mode";
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	CString strTemp;
	int nm=pCombo->GetCount();
	int m=0;

//	if(m_nWinVer!=-1&&m_nWinVer>=WINDOWS_8 && m_bX64)// >WINDOWS_8_1_OR_LATER)	//Win10以上Usb2com驱动因系统自带，不用安装 bywbj20160222
//		return TRUE;
//	PrintMsgToFile("1.txt","nWinVer=%d",nWinVer);
	for (int i=0;i<nm;i++)
	{
		dev = (struct wdi_device_info*)pCombo->GetItemData(i);
		if(dev==NULL)
			continue;
		if(strcmp(dev->desc,descname)!=0)
			continue;
		if(dev->driver==NULL)	//没有则安装
		{
//			PrintMsgToFile("1.txt","A:dev%d=NULL",i+1);
			return DoInstallDriver(dev);
		}
		strTemp=dev->driver;
		if(strTemp.Find(driver_display_name)<0)
		{
//			PrintMsgToFile("1.txt","B:dev%d=%s",i+1,dev->driver);
			return DoInstallDriver(dev);
		}
		m++;
	}
	if(m>0)
	{
		pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_COMM);
		if(pCombo->GetCount()==0)	//这里没列出也重安装
			return DoInstallDriver(dev);
	}
	return FALSE;
}
//安装(Photon with WiFi时)usb2com或(Photon DFU Mode时的)libusbk驱动
BOOL CDfuToolDlg::DoInstallDriver(wdi_device_info *dev)
{
	char extraction_path[MAX_PATH];
	const char* driver_display_name[WDI_NB_DRIVERS] = { "WinUSB", "libusb-win32", "libusbK", "Custom (extract only)","Custom (extract only)2" };
	const char* driver_name[WDI_NB_DRIVERS-2] = { "WinUSB", "libusb0", "libusbK" };
	struct wdi_options_prepare_driver pd_options = { 0 };
//	struct wdi_options_install_driver id_options = { 0 };
	int r;
	BOOL bOK=FALSE;
	char *inf_name;
	if(strcmp(dev->desc,DESCNAME_USBSER)==0)	//安装USB2COM驱动
	{
//		PRINTWORKINFO(_T("Installing the usbser driver,for switching to DFU mode..."));
		PRINTWORKINFO(_T("The usb serial auto installing,it may take a few minutes,please waiting..."));
		pd_options.driver_type=WDI_USER;
		inf_name= to_valid_filename("maple_serial", ".inf");
//		PrintMsgInfo("%s",inf_name); return;
	}
	else
	{
//		PRINTWORKINFO(_T("Installing the libusbK driver,for updating the firmware..."));
		PRINTWORKINFO(_T("The libusb auto installing,it may take a few minutes,please waiting..."));
#ifdef USE_LIBUSBK
		pd_options.driver_type=WDI_LIBUSBK;
#else
		pd_options.driver_type=WDI_LIBUSB0; //WDI_USER2; //WDI_LIBUSBK;
#endif
		inf_name= to_valid_filename(dev->desc, ".inf"); //dev->desc //"maple003"
	}
	if (inf_name == NULL) 
		return FALSE;
	//	pd_options.driver_type=WDI_LIBUSBK;
	pd_options.disable_cat=FALSE;
	pd_options.use_wcid_driver=FALSE;
	pd_options.disable_signing=(m_nWinVer<WINDOWS_8); //FALSE; //TRUE;	这里win8需要签名才行
	pd_options.cert_subject=NULL;
	char *tmp =getenvU("USERPROFILE");
	safe_sprintf(extraction_path, sizeof(extraction_path), "%s\\usb_driver", tmp);
//	safe_sprintf(extraction_path, sizeof(extraction_path), "usb_driver");
//	PrintMsgInfo("type=%d",pd_options.driver_type);
	r = wdi_prepare_driver(dev, extraction_path, inf_name, &pd_options);
	if (r == WDI_SUCCESS) 
	{ //return;
//		dsprintf("Successfully extracted driver files.");
		// Perform the install if not extracting the files only
		if (1) //(pd_options.driver_type != WDI_USER ) ) 
		{
// 			if ( (get_driver_type(dev) == DT_SYSTEM)
// 				&& (MessageBoxA(hMain, "You are about to modify a system driver.\n"
// 				"Are you sure this is what you want?", "Warning - System Driver",
// 				MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDNO) ) {
// 					r = WDI_ERROR_USER_CANCEL; goto out;
// 			}
			//开始安装驱动 bywbj
//			dsprintf("Installing driver. Please wait...");
#ifdef USE_MYDEBUG
			m_stIdOptions.hWnd =m_hWnd;
#else
			m_stIdOptions.hWnd =NULL; //m_hWnd;
#endif
			r = wdi_install_driver(dev, extraction_path, inf_name, &m_stIdOptions);
			// Switch to non driverless-only mode and set hw ID to show the newly installed device
// 			current_device_hardware_id = (dev != NULL)?safe_strdup(dev->hardware_id):NULL;
// 			if ((r == WDI_SUCCESS) && (!cl_options.list_all) && (!pd_options.use_wcid_driver)) {
// 				toggle_driverless(FALSE);
// 			}
//			PostMessage(WM_DEVICECHANGE, 0, 0);	// Force a refresh
			bOK=(r == WDI_SUCCESS);
			safe_free(inf_name);
		}
	} else {
//		dsprintf("Could not extract files");
	}
	safe_free(inf_name);
	if(bOK)
	{
		PRINTWORKINFO(_T("The driver installation successful."));
	}
	else
	{
#ifdef USE_MYDEBUG
		PRINTWORKINFO(_T("The driver installation failed!"));
#endif
//		PRINTWORKINFO(_T(" "));
	}
	return bOK;
}
struct wdi_device_info* CDfuToolDlg::GetItemDevice(int nIdx)
{
	struct wdi_device_info *dev = NULL;
	CComboBox *pCombo=(CComboBox *)GetDlgItem(IDC_COMBO_DEVLIST);
	if(pCombo==NULL || pCombo->GetCount()==0 || nIdx<0||nIdx>=pCombo->GetCount())
		return dev;
	dev = (struct wdi_device_info*)pCombo->GetItemData(nIdx);
	return dev;
}

#endif

void CDfuToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDfuToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDfuToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDfuToolDlg::SetLamp(CCircleButton *pBtn,int nLampID)
{
	if(pBtn==NULL)
		return;
	if(nLampID!=0)
		pBtn->SetButtonBitmap(nLampID);
	else
		pBtn->SetButtonBitmap(IDB_LAMPGREEN16);
	pBtn->Invalidate();
}

#if 0
void CDfuToolDlg::DoDfuHelp()
{
	if(m_pDfuHelpFrame==NULL)
	{
		CSize m_sizeBrowseDataWin(480+20,680);

		m_pDfuHelpFrame = new CHelpFrame;
		CCreateContext cc;
		cc.m_pCurrentDoc = NULL;
		cc.m_pCurrentFrame = m_pDfuHelpFrame;
		cc.m_pNewViewClass = RUNTIME_CLASS(CDfuHelpView);

		CRect wrc,rc;
		GetDesktopWindow()->GetWindowRect(&wrc);
//		m_sizeBrowseDataWin.cy=wrc.Height();
		//rc.left=(wrc.left+wrc.right-m_sizeBrowseDataWin.cx)/2; 
		rc.right=rc.left+m_sizeBrowseDataWin.cx;
		rc.top=(wrc.top+wrc.bottom-m_sizeBrowseDataWin.cy)/2; rc.bottom=rc.top+m_sizeBrowseDataWin.cy;
//			WS_MINIMIZEBOX 
		m_pDfuHelpFrame->Create(NULL, _T("DfuMode Help"), WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_BORDER,rc, NULL, NULL, 0, &cc); // CFrameWnd::rectDefault
		//	pFrame->GetActiveView()->OnInitialUpdate();
		m_pDfuHelpFrame->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
		m_pDfuHelpFrame->ShowWindow(SW_SHOW);
		m_pDfuHelpFrame->UpdateWindow();
		m_pDfuHelpFrame->MoveWindow(&rc);
		//		m_pDfuHelpFrame->CenterWindow();
	}
	else
	{
		if(m_pDfuHelpFrame->IsIconic())
		{
			m_pDfuHelpFrame->ShowWindow(SW_NORMAL);
		}
		else
		{
			if(!m_pDfuHelpFrame->IsWindowVisible())
				m_pDfuHelpFrame->ShowWindow(SW_SHOW);
			m_pDfuHelpFrame->BringWindowToTop();
		}
	}
}
void CDfuToolDlg::OnDfuHelp()
{
	DoDfuHelp();
}
#endif
#if 0
void CDfuToolDlg::OnInstallLibusbHelp()
{
	DoHelp();
}
void CDfuToolDlg::DoHelp()
{
	if(m_pHelpFrame==NULL)
	{
		CSize m_sizeBrowseDataWin(700,600);

		m_pHelpFrame = new CHelpFrame;
		CCreateContext cc;
		cc.m_pCurrentDoc = NULL;
		cc.m_pCurrentFrame = m_pHelpFrame;
		cc.m_pNewViewClass = RUNTIME_CLASS(CHelpView);

		CRect wrc,rc;
		GetDesktopWindow()->GetWindowRect(&wrc);
		m_sizeBrowseDataWin.cy=wrc.Height();
		//rc.left=(wrc.left+wrc.right-m_sizeBrowseDataWin.cx)/2; 
		rc.right=rc.left+m_sizeBrowseDataWin.cx;
		rc.top=(wrc.top+wrc.bottom-m_sizeBrowseDataWin.cy)/2; rc.bottom=rc.top+m_sizeBrowseDataWin.cy;
		m_pHelpFrame->Create(NULL, _T("Readme"), WS_OVERLAPPEDWINDOW,rc, NULL, NULL, 0, &cc); // CFrameWnd::rectDefault
		//	pFrame->GetActiveView()->OnInitialUpdate();
		m_pHelpFrame->SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);
		m_pHelpFrame->ShowWindow(SW_SHOW);
		m_pHelpFrame->UpdateWindow();
		m_pHelpFrame->MoveWindow(&rc);
		//		m_pHelpFrame->CenterWindow();
	}
	else
	{
		if(m_pHelpFrame->IsIconic())
		{
			m_pHelpFrame->ShowWindow(SW_NORMAL);
		}
		else
		{
			if(!m_pHelpFrame->IsWindowVisible())
				m_pHelpFrame->ShowWindow(SW_SHOW);
			m_pHelpFrame->BringWindowToTop();
		}
	}


}
#endif

/*
void CDfuToolDlg::OnBnClickedBtnBrowse3()
{
	DoBrowseFile(2);
}
*/

/*
void CDfuToolDlg::OnBnClickedBtnDownload3()
{
	if(m_bDownloading)
		m_bExit=TRUE;
	else
		DoDownload(2);
}
*/

BOOL CDfuToolDlg::MakeUpdateCmd()
{
	TCHAR szFileName[MAX_PATH];
	TCHAR szExtName[MAX_PATH];
	CString strPath,strFile;
	s_strCurCmd[0]=0;
	strPath=GetFolderFromFullpath(m_szFixDemoFile,szFileName,szExtName);
	strFile.Format(_T("%s%s"),szFileName,szExtName);
	//	PrintMsgInfo("%s\xd\xa %s",strPath,strFile);
	if(strFile.GetLength()==0)
	{
//		m_EditPrompt.SetWindowText("No update file! Please select a file to update.");
		PRINTPROMPT("No update file! Please select a file to update.\n");
		return FALSE;
	}
	if(strPath.GetLength()==0)	//空的就用当前目录
		strPath=theApp.m_szRunDir;
	SetCurrentDirectory(strPath);	//更换到当前目录
// 	strcpy(s_strCurCmd,s_strCmd);
// 	strcat(s_strCurCmd,strFile);
// 	strcat(s_strCurCmd," -R >my.txt");
//	sprintf(s_strCurCmd,"%s\\%s%s -R >my.txt",theApp.m_szRunDir,s_strCmd,strFile);
	sprintf(s_strCmdPath,"%s\\dfu-util.exe",theApp.m_szRunDir);
	sprintf(s_strCmdParam,"-a 1 -D %s -R",strFile); //
	sprintf(s_strCurCmd,"%s %s",s_strCmdPath,s_strCmdParam);
	return TRUE;
}

/*
void CDfuToolDlg::OnBnClickedBtnUpdate3()
{
	if(m_bUpdating)
		m_bExit=TRUE;
	else
		DoUpdate(2);
}
*/
void CDfuToolDlg::ClearItemInfo(int nItem)
{
	int t1,t2;
	int i;
	if(nItem<=-1)
	{	
		t1=0;t2=MAX_UPDATE_NUM; 
//		SetLamp(&m_btnLampUpdate[0],IDB_LAMPGRAY24);
#ifdef USE_ONEKEY_DOWNLOAD
		m_LabelWorkInfo.SetWindowText(NULL);//_T("OK"));
		GetDlgItem(IDC_STATIC_CURFILE)->SetWindowText(NULL);//_T("OK"));
#endif
	}
	else
	{	t1=nItem; t2=t1+1; }
//	m_EditPrompt.SetWindowText(NULL);
	for (i=t1;i<t2;i++)
	{
//		SetLamp(&m_btnLampUpdate[i+1],IDB_LAMPGRAY);
//		SetDlgItemText(IDC_STATIC_LENINFO1+i,NULL);
//		SetDlgItemText(IDC_STATIC_LENINFO4+i,NULL);
	}
}
BOOL IsDirectoryExists(TCHAR *dirName)
{
	WIN32_FILE_ATTRIBUTE_DATA dataDirAttrData;
	if (!::GetFileAttributesEx(dirName, GetFileExInfoStandard, &dataDirAttrData))
	{
		DWORD lastError = ::GetLastError();
		if (lastError == ERROR_PATH_NOT_FOUND || lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_NOT_READY)
			return FALSE;
	}
	return (dataDirAttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
#if 1
/*
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{ECFB0CFD-74C4-4F52-BBF7-343461CD72AC}]
"Class"="libusbk devices"
"ClassDesc"="libusbK USB Devices"
@="libusbK USB Devices"
"Icon"="-20"
"IconPath"=hex(7):43,00,3a,00,5c,00,57,00,69,00,6e,00,64,00,6f,00,77,00,73,00,\
5c,00,73,00,79,00,73,00,74,00,65,00,6d,00,33,00,32,00,5c,00,53,00,45,00,54,\
00,55,00,50,00,41,00,50,00,49,00,2e,00,64,00,6c,00,6c,00,2c,00,2d,00,32,00,\
30,00,00,00,00,00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{ECFB0CFD-74C4-4F52-BBF7-343461CD72AC}\0000]
"InfPath"="oem90.inf"
"InfSection"="LUsbK_Device"
"InfSectionExt"=".NTAMD64"
"ProviderName"="libusbK"
"DriverDateData"=hex:00,40,eb,bf,f9,0c,d0,01
"DriverDate"="12-1-2014"
"DriverVersion"="3.0.7.0"
"MatchingDeviceId"="usb\\vid_2b04&pid_c006"
"DriverDesc"="Photon with WiFi"
*/
#endif
BOOL CDfuToolDlg::DoCheckDfuMode(BOOL bPrompt)
{
#if 0
	int m=GetDfuMode(m_hWnd,m_szDfuModeInfo,sizeof(m_szDfuModeInfo)-1);
//	PrintMsgInfo("%d\xd\xa%s",m,m_szDfuModeInfo);
	if(m<0)
	{
		if(bPrompt)
			PrintMsgWarning(_T("The driver not installed! Please install."));
		return FALSE;
	}
	else if(m!=MDMSG_DFU)
	{
		if(bPrompt)
			PrintMsgWarning(_T("The photon is not in DFU mode!"));
		return FALSE;
	}
	return TRUE;
#endif
	CString strTemp;
	TCHAR strCurCmd[MAX_PATH];
	SetCurrentDirectory(theApp.m_szRunDir);	//更换到当前目录
	sprintf(strCurCmd,"%s\\dfu-util.exe -l",theApp.m_szRunDir);
	if(ExecDosCmd(strCurCmd,strTemp))
	{
//		PrintMsgToFile("m.txt","%s",strTemp);
		if(strTemp.Find("Found DFU: [0x1eaf:0x0003]")>=0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

int CDfuToolDlg::AutoUpdateDemoFile()
{
//	CString strFile;
	static TCHAR szInfo[MAX_PATH];
	GetDlgItem(IDC_STATIC_CURFILE)->SetWindowText(m_szFixDemoFile);
// 	strFile.Format("%s\\%s",theApp.m_szRunDir,s_strDemoFile[m_nDemoIdx]);
// 	pEditFile->SetWindowText(strFile);

	//		m_nUpdateItem=i;
//	DoUpdate(i,FALSE);

	PRINTPROMPT("\n");
	MakeUpdateCmd();//	DoDfuUtil(m_hWnd,s_strCurCmd);
//	system(s_strCurCmd);
// 	int status = 0;  
// 	status = system(s_strCurCmd);  
// //	if (-1 != status && status == 0)  
// 	{  
// 		PrintMsgInfo("%d",status);// dir successfully!\n");  
// 	}  
//	PrintMsgInfo("%s\xd\xa %s",s_strCmdPath,s_strCmdParam);
#if 1
	int rt=0;
	BOOL bOK=FALSE;
	CString strTemp;
	strcpy(szInfo,"Starting update file ...\n");
	if(m_pWaitingDlg)
	{
		m_pWaitingDlg->SetLabelText(0,_T("Updating"));
		m_pWaitingDlg->SetProgressRange(0,20);
		m_pWaitingDlg->StartMoveBar();
	}
	PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);

	if(ExecDosCmd(s_strCurCmd,strTemp))
	{
		if(strTemp.Find("finished!")>0)
		{
			if(strTemp.Find("Resetting USB to switch back to runtime mode")>0)
				strcpy(szInfo,"Update file finished.Resetting USB to switch back to runtime mode.\n");
			else
				strcpy(szInfo,"Update file finished.\n");
			bOK=TRUE;
		}
		else
		{
			strcpy(szInfo,"Update file failed!\n");
			rt=1;
		}
	}
	else
	{
		strcpy(szInfo,"Failed to run dfu-util.exe!\n");
		rt=2;
	}
	PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
//	PrintMsgToFile("1.txt","%s",strTemp);

#else
	HINSTANCE rt=ShellExecute(NULL,_T("open"),s_strCmdPath,s_strCmdParam,NULL,SW_SHOWNORMAL); 
	if((int)rt>32)
	{
//		PostMessage(WM_COMMAND,IDM_INSTALLLIBUSBHELP);
		strcpy(szInfo,"Update file OK.\n");
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
	}
#endif
//	PrintMsgInfo("%d",rt);
	int dnum,cnum;
	if(bOK)
	{
		if(m_pWaitingDlg)
			m_pWaitingDlg->SetLabelText(0,_T("Resetting"));
	}
	else
		Sleep(3000);

	for (int i=0; bOK && i<10;i++)
	{
// 	if(m_bExit)	//有用户中断,则退出安装
// 		break;
		Sleep(1000);
		dnum=DoListAll();
		cnum=DoScanComm();
		if(dnum>0 && cnum>0 && CheckDescNameIsInDevList(DESCNAME_USBSER))
		{
			PRINTWORKINFO(_T("The L3D Cube is in Usbser mode."));
			WaitForSingleObject(m_hWaitEvent,1000);	
			break;
		}
	}
	//m_ListCtrlBin.EnableWindow();
// 	CloseWaitingWnd();
// 	if(m_pWaitingDlg)
// 		m_pWaitingDlg->EndMoveBar();
// 	PostMessage(MY_FINISH_MESSAGE,rt);
	return rt;
}
/*
dfu-util - (C) 2007-2008 by OpenMoko Inc.
This program is Free Software and has ABSOLUTELY NO WARRANTY

Opening USB Device 0x0000:0x0000...
Found Runtime: [0x1eaf:0x0003] devnum=1, cfg=0, intf=0, alt=1, name="DFU Program FLASH 0x08005000"
Setting Configuration 1...
Claiming USB DFU Interface...
Setting Alternate Setting ...
Determining device status: state = dfuIDLE, status = 0
dfuIDLE, continuing
Transfer Size = 0x0400
bytes_per_hash=371
Starting download: [##################################################] finished!
state(8) = dfuMANIFEST-WAIT-RESET, status(0) = No error condition is present
Done!
Resetting USB to switch back to runtime mode

*/

void CDfuToolDlg::UpdateParam(BOOL bSaveToTheApp)
{
	int i;
	CString strTemp;
#if 0
	if(bSaveToTheApp)
	{
		for(i=0;i<MAX_UPDATE_NUM;i++)
		{
			m_EditUpdateFile[i].GetWindowText(strTemp);
			theApp.m_strUpdateFile[i]=strTemp.Trim();
		}
//		PrintMsgInfo(theApp.m_strUpdateFile[2]);
	}
	else
	{
		for(i=0;i<MAX_UPDATE_NUM;i++)
		{
			m_EditUpdateFile[i].SetWindowText(theApp.m_strUpdateFile[i]);
		}
//		PrintMsgInfo(theApp.m_strUpdateFile[2]);
	}
#endif
}

void CDfuToolDlg::OnOK()
{
	OnCancel();
	//	CDialog::OnOK();
}

void CDfuToolDlg::OnCancel()
{
	if(MessageBox(_T("Are you sure to exit?(Y/N)"),_T("Exit"),MB_ICONQUESTION|MB_YESNO)!=IDYES)
		return;

	UpdateParam(TRUE);
#ifndef USE_DEMO
	//	if(usb_handle)
	//		Connect_Complete();
	//	usb_handle=NULL;
#endif
	CDialog::OnCancel();
}
//	int	m_nCurWebFileTotalLen;	//当前要从网上下载的文件总长度
int CDfuToolDlg::GetWebFileLen(CString & strFileURL)
{
	CInternetSession	netSession;
	CStdioFile			*fTargFile=NULL;
//	CString strTemp,strFile;
//	static TCHAR szInfo[MAX_PATH];
	int nDownloaded=0;
	int flen=0;
	CFile fDestFile;

	BOOL bOpen=FALSE;
	try
	{
		nDownloaded = 1;
//		sprintf(szInfo,"Verifying Web site: %s\n",strFileURL); //"正在校验下载地址..."
		fTargFile = netSession.OpenURL(strFileURL,1,INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);
		if(fTargFile==NULL)
			return -1;
		flen = fTargFile->SeekToEnd();
		fTargFile->SeekToBegin();
	}
	catch(CInternetException *IE)
	{
//		strcpy(szInfo,"Download error!\n");
		nDownloaded = 0;
		delete fTargFile; fTargFile=NULL;
		IE->Delete();					// 删除异常对象，以防止泄漏
		flen=-2;
	}
exitdf0:
	if(fTargFile)
	{
		fTargFile->Close();					// 关闭远程文件
		delete fTargFile;					// 删除CStdioFile对象，以防止泄漏
		fTargFile=NULL;
	}
	return flen;
}
#if 0
BOOL CDfuToolDlg::DownloadFile(BOOL bCheckIsExist)
{
	if(m_nDownloadItem<0||m_nDownloadItem>=MAX_UPDATE_NUM)
		return FALSE;
	CEdit *pEditFile=&m_EditUpdateFile[m_nDownloadItem];

	CString		strFileURL,strOutFile,strFile; //KBin,Perc,

	UpdateParam(TRUE);

	BOOL bDo=FALSE;
	if(theApp.m_strUpdateFile[m_nDownloadItem].GetLength()==0)	//空则要下载
	{
		strFile.Format("%s\\%s",theApp.m_szRunDir,g_strUpdateFile[m_nDownloadItem]);
//		pEditFile->SetWindowText(strFile);
		SetFileNameNoCurrentDir(pEditFile,strFile);
		bDo=TRUE;
	}
	else
	{
		strFile=theApp.m_strUpdateFile[m_nDownloadItem];
	}
	CString strPath=GetFolderFromFullpath(strFile);//,szFileName,szExtName);
//	strFile.Format(_T("%s%s"),szFileName,szExtName);
	if(strPath.GetLength()==0)	//空的就用当前目录
	{
		strPath.Format("%s\\%s",theApp.m_szRunDir,strFile);
		strFile=strPath;
		strPath=theApp.m_szRunDir;
	}
//	SetCurrentDirectory(strPath);	//更换到当前目录
	if(bCheckIsExist && !bDo)	//当充许检测存在,且不空时,检查是否存在本地，如有则不下载
	{
		if(_access((const char *)strFile,0)==0)	//文件存在
		{
			return FALSE;
		}
	}
	strFileURL.Format(_T("%s/%s"),g_strWebAddr,g_strUpdateFile[m_nDownloadItem]);
	strOutFile=strFile; //.Empty();
	return DownloadFile(strFileURL,strOutFile);
}
#endif

BOOL CDfuToolDlg::DownloadFile(CString & strFileURL,CString & strOutFile,BOOL bPrompt)
{
	char				filebuf[512];
	CInternetSession	netSession;
	CStdioFile			*fTargFile=NULL;
	int					outfs;
	CString strTemp;
	static TCHAR szInfo[MAX_PATH];
	int nDownloaded=0;

	PRINTPROMPT("\n");
	BOOL bWrote=FALSE;
	CFile fDestFile;
	BOOL bOpen=FALSE;
	try
	{
		nDownloaded = 1;
//		GetDlgItemText(IDC_EDIT1,szFile);
		sprintf(szInfo,"Verifying Web site: %s\n",strFileURL); //"正在校验下载地址..."
		if(bPrompt)
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
		fTargFile = netSession.OpenURL(strFileURL,1,INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD);

		COleDateTime dlStart = COleDateTime::GetCurrentTime();
		int filesize = fTargFile->SeekToEnd();
		fTargFile->SeekToBegin();
		outfs = filesize / 1024;		// 计算文件大小（千字节）
//		FileSize.Format("%d",outfs);	// 以KB为单位格式文件大小
		
		//strOutFile=fTargFile->GetFileName();
//		strOutFile.Format("%s\\%s",theApp.m_szRunDir,fTargFile->GetFileName());

		// 在当前目录创建新的目标文件
		bOpen=fDestFile.Open(strOutFile,CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
		if(!bOpen)
		{
			strcpy(szInfo,"Create file failed!\n");
			if(bPrompt)
			PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
			goto exitdf;
		}
		int byteswrite;		// 写入文件的字节数
		int pos = 0;		// 当前进度条的位置
		int nperc,kbrecv;	// 进度条的百分比,获取到的数据大小（Kbs为单位）
		double secs,kbsec;	// 记录秒数, 速度（KB/秒）

//		SetDlgItemText(IDC_STATIC_PROMPT,"正在下载...");
//		strcpy(szInfo,"Downloading...\n");
		sprintf(szInfo,"Downloading from Web site: %s\n",strFileURL); //"正在校验下载地址..."
		if(bPrompt)
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
		if(m_pWaitingDlg)
			m_pWaitingDlg->SetProgressRange(0,filesize);
		while (byteswrite = fTargFile->Read(filebuf, 512))	// 读取文件
		{
			if(m_bExit)						// 如果点击取消下载
			{
//				SetDlgItemText(IDC_STATIC_PROMPT,"下载时已被用户取消！"); // Set satus bar text
				strcpy(szInfo,"User break!\n");
				PostMessage(MY_PRINT_MESSAGE,1,(LPARAM)szInfo);
//				AfxEndThread(0);					// 结束下载线程
				nDownloaded=0;
				break;
			}
			// 根据开始时间与当前时间比较，获取秒数
			COleDateTimeSpan dlElapsed = COleDateTime::GetCurrentTime() - dlStart;
			secs = dlElapsed.GetTotalSeconds();
			pos = pos + byteswrite;					// 设置新的进度条位置
			fDestFile.Write(filebuf, byteswrite);	// 将实际数据写入文件
			bWrote=TRUE;
// 			kbrecv = pos / 1024;					// 获取收到的数据
// 			kbsec = kbrecv / secs;					// 获取每秒下载多少（KB）

//			Perc.Format("%d",nperc);				// 格式化进度百分比
//			KBin.Format("%d",kbrecv);				// 格式化已下载数据大小（KB）
//			KBsec.Format("%d",(int)kbsec);			// 格式化下载速度（KB/秒）
			SetProgressInfo(pos); //m_nDownloadItem,
		}
	}
	catch(CInternetException *IE)
	{
// 		TCHAR error[255];
// 		IE->GetErrorMessage(error,255); // 获取错误消息
//		sprintf(szInfo,"%s\n",strerror);
		strcpy(szInfo,"Download error!\n");
		if(bPrompt)
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
//		SetDlgItemText(IDB_BTN_STOP,"Exit");
		nDownloaded = 0;
		delete fTargFile; fTargFile=NULL;
		IE->Delete();					// 删除异常对象，以防止泄漏
	}
exitdf:
	if(bOpen)
	{
		fDestFile.Close();	bOpen=FALSE;	// 关闭目标文件
	}
	if(fTargFile)
	{
		fTargFile->Close();					// 关闭远程文件
		delete fTargFile;					// 删除CStdioFile对象，以防止泄漏
		fTargFile=NULL;
	}
	if(nDownloaded == 1)
	{
//		SetDlgItemText(IDC_STATIC_PROMPT,"下载完成！");
		strcpy(szInfo,"Download file OK.\n");
		if(bPrompt)
		PostMessage(MY_PRINT_MESSAGE,MPTMSG_WORKINFO,(LPARAM)szInfo);
//		bStart->EnableWindow(TRUE);
	}
	else if(strOutFile.GetLength()>0) //不成功时删除目标文件
	{
//		PrintMsgInfo(strOutFile);
		if(bWrote)
		{
			if(_access((const char *)strOutFile,0)==0)	//文件存在
				CFile::Remove(strOutFile); //删除不成功的文件
		}
//		pEditFile->SetWindowText(NULL);
	}
//	PRINTPROMPT("\n");
	return nDownloaded;
}
void CDfuToolDlg::SetProgressInfo(int pos)
{
#if 0
	CString strTemp;
	int nMin,nMax;
	m_ProgressCtrl.GetRange(nMin,nMax);
	if(nMax==0)
		return;
	m_ProgressCtrl.SetPos(pos);
	int	nperc = pos * 100 / nMax;			// 进度百分比
	int	kbrecv = (pos+1023)/1024;					// 获取收到的数据
//	strTemp.Format(_T("%d/%d KB %d%%"),kbrecv,(nMax+1023)/1024,nperc);
//	SetDlgItemText(IDC_STATIC_LENINFO1+nItem,strTemp);
#endif
	if(m_pWaitingDlg)
		m_pWaitingDlg->SetProgressPos(pos);
}
LRESULT CDfuToolDlg::OnMyFinishMessage(WPARAM wParam,LPARAM lParam)
{
	if(wParam==MYFM_OK)
	{
	}
	else if(wParam==1)
	{
	}
	else if(wParam==2)
	{
	}
	EnableWindow();
	CloseWaitingWnd();
	if(wParam==1)
	{
		PrintMsgWarning(_T("Update file failed!"));
	}
	if(wParam==2)	
	{
		PrintMsgWarning(_T("Failed to run dfu-util.exe!"));
	}
	if(wParam==MYFM_GETFILELEN)
	{
		int flen=lParam;
		if(flen<=0)
			PrintMsgWarning(_T("Cannot get web file length from the web site!"));
		if(flen>MAX_BINFILE_LEN)
			PrintMsgWarning(_T("File length is %dk(>=%dk),cannot download!"),(flen+1023)/1024,MAX_BINFILE_LEN/1024);
	}
	if(wParam==MYFM_LOADFILE)
	{
		PrintMsgWarning(_T("Loading file failed!"));
	}
	m_ListCtrlBin.SetFocus();
	return TRUE;
}
LRESULT CDfuToolDlg::OnMyPrintMessage(WPARAM wParam,LPARAM lParam)
{
//	CEdit *pEdit=(CEdit *)GetDlgItem(IDC_EDIT_PROMPT);
	TCHAR *pp=(TCHAR *)lParam;
	CString strTemp,ss;
#ifdef USE_ONEKEY_DOWNLOAD
	CWnd *pWnd;
//	if(wParam==MPTMSG_WORKINFO)
	{
//		pWnd=GetDlgItem(IDC_STATIC_WORKINFO);
		if(pp)
		{
			ss=pp;	m_LabelWorkInfo.SetText(ss);
		}
		if(m_pWaitingDlg && pp)
			m_pWaitingDlg->SetLabelText(3,pp);
		return TRUE;
	}
#endif
#if 0
	m_EditPrompt.GetWindowText(strTemp);
	ss=pp;
	ss.Replace(_T("\n"),_T("\xd\xa"));
	strTemp+=ss;
	m_EditPrompt.SetWindowText(strTemp);
//	m_EditPrompt.SetSel(-1,-1);
	m_EditPrompt.LineScroll(m_EditPrompt.GetLineCount(),0);  
//	PrintMsgInfo("OK");
#endif
	return TRUE;
}

LRESULT CDfuToolDlg::OnMyDfuModeMessage(WPARAM wParam,LPARAM lParam)
{
	if(wParam==MDMSG_DFU)
	{
	}
	else if(wParam==MDMSG_RUNTIME)
	{
	}
	else	//MDMSG_NO
	{

	}
	m_nDfuMode=wParam;
	UINT idv=IDB_LAMPGRAY24;
	if(m_nDfuMode==MDMSG_DFU)
		idv=IDB_LAMPYELLOW24;
	if(m_nDfuMode==MDMSG_RUNTIME)
		idv=IDB_LAMPBLUE24;
//	SetLamp(&m_btnLampDfu,idv);
	return TRUE;
}

LRESULT CDfuToolDlg::OnMySetProgressMessage(WPARAM wParam,LPARAM lParam)
{
	if(wParam==MPMSG_RANGE)
	{
		if(m_pWaitingDlg)
			m_pWaitingDlg->SetProgressRange(0,lParam);
//		SetProgressInfo(m_nUpdateItem+MAX_UPDATE_NUM,0);
		SetProgressInfo(0);
//		SetLamp(&m_btnLampUpdate[m_nUpdateItem+1],IDB_LAMPYELLOW16);
	}
	if(wParam==MPMSG_CURPOS)
	{
		SetProgressInfo(lParam); //m_nUpdateItem+MAX_UPDATE_NUM,
	}
	return TRUE;
}
static char szCmd[100]="dfu-util -l";

void CDfuToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent==1)
	{
//		KillTimer(1);
		time_t tv=time(NULL);
		if(m_tLastRefresh==-1||tv-m_tLastRefresh>=MAX_AUTOREFRESH_TIME)
			DoRefreshListBin();
//		if(!m_bUpdating)
		{
//			static int m=0; m=(m+1)%3;
 //			int m=GetDfuMode(m_hWnd,m_szDfuModeInfo,sizeof(m_szDfuModeInfo)-1);
 //			PostMessage(MY_DFUMODE_MESSAGE,m,0);
// 			static char ss[100];
// 			strcpy(ss,szCmd);
// 			DoDfuUtil(m_hWnd,ss);
		}
	}
	CDialog::OnTimer(nIDEvent);
}
void CDfuToolDlg::EnableControls()
{
	BOOL bb=FALSE;
	UINT idv[]={
		IDC_LIST1,
	};
	bb=!(m_bAutoUpdating||m_bRefreshListBinWorking);
//	::EnableControl(this,idv,sizeof(idv)/sizeof(UINT),bb);
	if(bb)
		m_ListCtrlBin.Invalidate();
}
//将文件名中的当前程序目录名去掉，置到pWnd标题,以便简洁
void CDfuToolDlg::SetFileNameNoCurrentDir(CWnd *pWnd,LPCTSTR strFileName)
{
	CString strFile,strPath;
	TCHAR szFileName[MAX_PATH];
	TCHAR szExtName[MAX_PATH];
	strPath=GetFolderFromFullpath(strFileName,szFileName,szExtName);
	if(strPath.CompareNoCase(theApp.m_szRunDir)==0)
	{
		strFile.Format("%s%s",szFileName,szExtName); //=g_strUpdateFile[i];	
		pWnd->SetWindowText(strFile);
	}
	else
		pWnd->SetWindowText(strFileName);
}

#ifdef USE_ONEKEY_DOWNLOAD
//一键下载
UINT CDfuToolDlg::AutoFixcubeThread(LPVOID pParam)
{
	CDfuToolDlg *pParent=(CDfuToolDlg *)pParam;
	pParent->m_bAutoUpdating=TRUE;
	pParent->EnableControls();
	BOOL bb=FALSE;
	CString strUrl,strFile;
	strUrl=pParent->m_szFileUrl;
	strFile=pParent->m_szFixDemoFile;
	if(strUrl.GetLength()>0)
	{
		int flen=pParent->GetWebFileLen(strUrl);
		if(flen<=0||flen>MAX_BINFILE_LEN)
		{
			// 		if(pParent->m_pWaitingDlg)
			// 			pParent->m_pWaitingDlg->EndMoveBar();
			pParent->PostMessage(MY_FINISH_MESSAGE,MYFM_GETFILELEN,flen);
			pParent->m_bAutoUpdating=FALSE;
			pParent->EnableControls();
			return 0;
		}
		if(pParent->m_pWaitingDlg)
			pParent->m_pWaitingDlg->EndMoveBar();
		bb=pParent->DownloadFile(strUrl,strFile);
	}
	else if(strFile.GetLength()>0)
	{
		if(pParent->m_pWaitingDlg)
			pParent->m_pWaitingDlg->EndMoveBar();
		bb=TRUE;
	}
	if(bb)
	{
		pParent->ClearItemInfo();
		pParent->DoFixcube();
	}
	else
	{
// 		if(pParent->m_pWaitingDlg)
// 			pParent->m_pWaitingDlg->EndMoveBar();
		pParent->PostMessage(MY_FINISH_MESSAGE,MYFM_LOADFILE);
		pParent->m_LabelWorkInfo.ShowWindow(SW_SHOW);	//m_bShowWorkInfo=TRUE;
	}
	pParent->m_bAutoUpdating=FALSE;
	pParent->EnableControls();
//	pParent->m_bDownloading=FALSE;
	return 0;
}
BOOL CDfuToolDlg::DoFixcube()
{
	BOOL bOK=FALSE;
	int dnum=0,cnum=0;
	int nPleaseInsertMapleN=0;
	int nSwitchToDfuN=0;
	int nScanMapleN=0;
	BOOL bErr=FALSE;
	while(1)
	{
		PRINTWORKINFO(_T("Scanning maple..."));
		dnum=DoListAll();
		cnum=DoScanComm();
		bOK=DoCheckDfuMode(FALSE); 
		if(bOK)	//是DFU mode 则退出循环,进行更新固件
		{
			PRINTWORKINFO(_T("The L3D Cube is in DFU mode."));
			break;
		}
		if(m_bExit)	//有用户中断,则退出安装
			break;
		if(dnum==0)	//列表中没有
		{
			//加提示请插入Maple
			PRINTWORKINFO(_T("Please insert the L3D Cube!"));
			if(nPleaseInsertMapleN>=40)	//等待1分钟
			{	bErr=TRUE; break; }
			//等待一会
			WaitForSingleObject(m_hWaitEvent,1500);	//30
			nPleaseInsertMapleN++;
//			PrintMsgInfo("dnum=0");
			continue;
		}
		nPleaseInsertMapleN=0;
		if(!bOK && dnum>0 && cnum>0 && CheckDescNameIsInDevList(DESCNAME_USBSER))
		{
//			PrintMsgInfo("%d,%d,%d",bOK,dnum,cnum);
			PRINTWORKINFO(_T("Switching to DFU mode,waiting..."));
			nSwitchToDfuN++;
			if(SwitchToDfuMode())
			{
				WaitForSingleObject(m_hWaitEvent,1000);	
//				PrintMsgToFile("1.txt","A");
				dnum=DoListAll();
//				PrintMsgToFile("1.txt","B %d",dnum);
				bOK=CheckInstallDfuModeDriver();	//PrintMsgInfo("%d",bOK);
				if(bOK)
					WaitForSingleObject(m_hWaitEvent,1000);	
				bOK=DoCheckDfuMode(FALSE); 
				if(bOK)
				{
					PRINTWORKINFO(_T("The L3D Cube is in DFU mode."));
//					WaitForSingleObject(m_hWaitEvent,1000);	
					break;
				}
//				EnableWindow();
//				PrintMsgToFile("1.txt","2");
				if(nSwitchToDfuN<4)
					continue;
			}
			if(nSwitchToDfuN>=4)
			{
				if(MessageBox(_T("Not switching to DFU mode,\ndo you want to install the driver?(Y/N)"),_T("Exit"),MB_ICONQUESTION|MB_YESNO)==IDYES)
				{
					DoInstallDriver(&s_mydev_Dfumode);
				}
				bErr=TRUE; break;
			}
		}
		//检查是否有Dfu模式的驱动，无则安装
		bOK=CheckInstallDfuModeDriver();	//PrintMsgInfo("%d",bOK);
		if(!bOK)
		{
			//检查是否有Usb2com的驱动，无则安装
			bOK=CheckInstallUsb2comDriver();
		}
		WaitForSingleObject(m_hWaitEvent,1000);	//30
	}
	int ret=0;
	if(!m_bExit && !bErr)
	{
		ret=AutoUpdateDemoFile(); //AutoUpdateFile();
	}
	if(m_pWaitingDlg)
		m_pWaitingDlg->EndMoveBar();
	PostMessage(MY_FINISH_MESSAGE,ret);
	m_LabelWorkInfo.ShowWindow(SW_SHOW);	//m_bShowWorkInfo=TRUE;
	return !bErr; //TRUE;
}


void CDfuToolDlg::OnNMClickSyslink(NMHDR *pNMHDR, LRESULT *pResult)
{
	PNMLINK pNMLink = (PNMLINK) pNMHDR;   
	ShellExecuteW(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);  //在浏览器中打开        
	*pResult = 0;  
}
#endif


//DoDownloadFile_FixDemo(strFileUrl,strOutFile);

void CDfuToolDlg::DoDownloadFile_FixDemo(LPCTSTR strFileUrl,LPCTSTR strOutFile) //DoFixDemo(LPCTSTR lpszFile)
{
	if(m_bAutoUpdating)// ||m_bUpdating)
	{
		return;
	}
	m_bExit=FALSE;
	if(strFileUrl)
		strcpy(m_szFileUrl,strFileUrl);
	else
		m_szFileUrl[0]=0;
	if(strOutFile)
		strcpy(m_szFixDemoFile,strOutFile);
	else
		m_szFixDemoFile[0]=0;
	//	UpdateParam(TRUE);
	//	SetLamp(&m_btnLampUpdate[0],IDB_LAMPYELLOW16);
	AfxBeginThread(AutoFixcubeThread,this);
}

//[{"id":1, "title": "demo application", "binary":"http://6x.cubetube.org/binaries/demo.bin", "icon": "http://6x.cubetube.org/images/demo.png", "lastModified": "2016-01-21 14:21:13"}, {"id":2, "title": "music pack", "binary":"http://6x.cubetube.org/binaries/music.bin", "icon": "http://6x.cubetube.org/images/music.png", "lastModified": "2016-01-19 10:15:09"}, ]

BOOL CDfuToolDlg::GetCubeBinInfo(CString & strInfo,CUBEBININFO *pstCBI)
{
	CString strValue;
	TCHAR *strField[CBIFIELD_NUM]={"\"id\":","\"title\":","\"binary\":","\"icon\":","\"lastModified\":"};
	int m=0;
	int p1,p2,pos=0;
	memset(pstCBI,0,sizeof(CUBEBININFO));
	for(int i=0;i<CBIFIELD_NUM;i++)
	{
		pos=0;
		p1=strInfo.Find(strField[i],pos);
		if(p1<0)
			return FALSE;
		p1+=strlen(strField[i]);
		if(i==CBIFIELD_ID)
		{
			pstCBI->id=atoi((LPCTSTR)strInfo+p1);
			m++;
			continue;
		}
		pos=p1;
		p1=strInfo.Find('\"',pos);
		if(p1>=0)
			pos=p1+1;
		p1=pos;
		p2=strInfo.Find('\"',pos);
		if(p2<0)
		{
			p2=strInfo.Find('}',pos);
			if(p2<0)
				p2=strInfo.Find(',',pos);
			if(p2<0)
				continue;
		}
		int ln=p2-p1;
//		strValue=strInfo.Mid(p1,ln);
		if(i==CBIFIELD_TITLE)
		{
			if(ln>=sizeof(pstCBI->szTitle)) 
				ln=sizeof(pstCBI->szTitle)-1;
			strValue=strInfo.Mid(p1,ln);
			strcpy(pstCBI->szTitle,(LPCTSTR)strValue);
		}
		else if(i==CBIFIELD_BIN)
		{
			if(ln>=sizeof(pstCBI->szBinUrl)) 
				ln=sizeof(pstCBI->szBinUrl)-1;
			strValue=strInfo.Mid(p1,ln);
			strcpy(pstCBI->szBinUrl,(LPCTSTR)strValue);
		}
		else if(i==CBIFIELD_ICON)
		{
			if(ln>=sizeof(pstCBI->szIconUrl)) 
				ln=sizeof(pstCBI->szIconUrl)-1;
			strValue=strInfo.Mid(p1,ln);
			strcpy(pstCBI->szIconUrl,(LPCTSTR)strValue);
		}
		else
		{
			if(ln>=sizeof(pstCBI->szLastModified)) 
				ln=sizeof(pstCBI->szLastModified)-1;
			strValue=strInfo.Mid(p1,ln);
			strcpy(pstCBI->szLastModified,(LPCTSTR)strValue);
		}
//		PrintMsgToFile("111.txt","m=%d,%s",m+1,strValue);
		m++;
	}
//	PrintMsgInfo("m=%d",m);
	return m==CBIFIELD_NUM; //TRUE;
}
int CDfuToolDlg::FindCubeBinInfo(CString & strInfo,CArray<CUBEBININFO,CUBEBININFO&> &cbi)
{
	CString strTemp;
	int m=0;
	int p1,p2,pos=0;
	CUBEBININFO stCBI;
	cbi.RemoveAll();
	while(1) {
		p1=strInfo.Find(_T('{'),pos);
		if(p1<0)
			break;
		p1++;
		pos=p1;
		p2=strInfo.Find(_T('}'),pos);
		if(p2<0)
			break;
		strTemp=strInfo.Mid(p1,p2-p1); 
		//滤掉回车转行符
		strTemp.Replace(_T("\xd"),_T("")); strTemp.Replace(_T("\xa"),_T(""));
		pos=p2+1;
		if(!GetCubeBinInfo(strTemp,&stCBI))
			continue;
		cbi.Add(stCBI);
		m++;
	}
	return m;
}
int CDfuToolDlg::ToLocalBinInfo(CArray<CUBEBININFO,CUBEBININFO&> &cbi)
{
	TCHAR ss[MAX_PATH];
	CUBEBININFO *pstCBI;
	int nm=cbi.GetSize();
	for (int i=0;i<nm;i++)
	{
		pstCBI=&cbi.GetAt(i);
		char *pp=strrchr(pstCBI->szBinUrl,'/');
		if(pp)
		{
			pp++;
			sprintf(ss,"%s\\%s",theApp.m_strCubeBinDir,pp);
			strcpy(pstCBI->szBinUrl,ss);
		}
		pp=strrchr(pstCBI->szIconUrl,'/');
		if(pp)
		{
			pp++;
			sprintf(ss,"%s\\%s",theApp.m_strCubeBinDir,pp);
			strcpy(pstCBI->szIconUrl,ss);
		}
	}
	return nm;
}
//在本地找出已下载png和bin文件
int CDfuToolDlg::FindLocalCubeBinInfo(CArray<CUBEBININFO,CUBEBININFO&> &cbi)
{
	int nm;
	CString strTemp;
	CUBEBININFO stCBI;
	strTemp.Format(_T("%s\\*.*"),theApp.m_strCubeBinDir);
	nm=FindIconBinFileToList(strTemp,&m_ListFile);
	if(nm==0)
		return 0;
	cbi.RemoveAll();
	int m=0;
	TCHAR ss[MAX_PATH];
	for (int i=0;i<nm;)
	{
		memset(&stCBI,0,sizeof(stCBI));
		stCBI.id=m+1;
		m_ListFile.GetText(i,ss);
		char *pp=_tcsrchr(ss,_T('.'));
		if(pp==NULL||pp==ss)
		{
			i++; continue;
		}
		int ln=pp-ss;
		if(ln>sizeof(stCBI.szTitle)-1)
			ln=sizeof(stCBI.szTitle)-1;
		strncpy(stCBI.szTitle,ss,ln); stCBI.szTitle[ln]=0; 
		int k=i+1;
		while(1)
		{
			if(pp>ss)
			{
				if(_tcsicmp(pp,_T(".bin"))==0)
				{
					sprintf(stCBI.szBinUrl,"%s\\%s",theApp.m_strCubeBinDir,ss);
				}
				else if(_tcsicmp(pp,_T(".png"))==0)
				{
					sprintf(stCBI.szIconUrl,"%s\\%s",theApp.m_strCubeBinDir,ss);
				}
			}
			if(stCBI.szIconUrl[0]!=0&&stCBI.szBinUrl[0]!=0)
				break;
			if(k>=nm)
				break;
			m_ListFile.GetText(k,ss);
			pp=_tcsrchr(ss,_T('.'));
//			if(pp==NULL||pp==ss)
//				continue;
			if(pp && strnicmp(stCBI.szTitle,ss,pp-ss)!=0)	//找不到Title相同的
			{
				break;
			}
			k++;
		}
		if(stCBI.szTitle[0]!=0)
		{
			cbi.Add(stCBI); m++;
		}
		i=k;
	}
	return m;
}

void CDfuToolDlg::OnNMClickListCtrlBin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	DoUpdateFromListBinItem(pNMItemActivate->iItem);
}
void CDfuToolDlg::OnNMDblclkListBin(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	//	PrintMsgInfo(_T("%d"),pNMItemActivate->iItem);
// 	CWaitingDlg dlg;
// 	dlg.SetDispProgressMode(DISPPROGRESS_ALL);
// 	EnableWindow(FALSE);
// 	dlg.DoModal();
// 	EnableWindow(TRUE);
//	return;
	//	PrintMsgInfo(strTemp);
	DoUpdateFromListBinItem(pNMItemActivate->iItem);
}
void CDfuToolDlg::DoUpdateFromListBinItem(int nItemNo)
{
	CString strTemp;
	int nm=m_cbi.GetSize();
	if(nm==0||nItemNo>=nm||nItemNo<0)
		return;
	if(m_bAutoUpdating||m_bRefreshListBinWorking)// ||m_bUpdating)
	{
		return;
	}

	CString strFileUrl,strOutFile;
	CUBEBININFO *pstCBI=&m_cbi.GetAt(nItemNo);

	char *pp=strrchr(pstCBI->szBinUrl,'/');
	if(pp)
	{
		pp++;
		strFileUrl=pstCBI->szBinUrl;
		strOutFile.Format("%s\\%s",theApp.m_strCubeBinDir,pp);
	}
	else	//用本地文件
	{
		strFileUrl.Empty();
		strOutFile.Format(_T("%s"),pstCBI->szBinUrl);
	}
/*
	int flen=GetWebFileLen(strFileUrl);
	if(flen<=0)
	{
		PrintMsgWarning(_T("Cannot get web file length from the web site!"));
		return;
	}
	if(flen>MAX_BINFILE_LEN)
	{
		PrintMsgWarning(_T("File length is %dk(>=%dk),cannot download!"),(flen+1023)/1024,MAX_BINFILE_LEN/1024);
		return;
	}
*/
//	return;
	//m_ListCtrlBin.
	CreateWaitingWnd();
	m_LabelWorkInfo.ShowWindow(SW_HIDE);
	EnableWindow(FALSE);
	strTemp.Format(_T("You have selected to play <%s>"),pstCBI->szTitle);
	m_pWaitingDlg->SetLabelText(1,strTemp);
	if(m_pWaitingDlg)
	{
		m_pWaitingDlg->SetLabelText(0,_T("LOADING"));
		m_pWaitingDlg->SetProgressRange(0,20);
		m_pWaitingDlg->StartMoveBar();
	}
/*	BOOL bb;
	bb=DownloadFile(strFileUrl,strOutFile);
	if(bb)
		DoFixDemo(strOutFile);
	else
	{
		//m_ListCtrlBin.
		EnableWindow();
		CloseWaitingWnd();
	}
*/
	DoDownloadFile_FixDemo(strFileUrl,strOutFile);
//	PrintMsgToFile("1.txt","%s",strOutFile);
	//	PrintMsgInfo("%d",nm);
}
void CDfuToolDlg::DoRefreshListBin()
{
	if(m_bAutoUpdating||m_bRefreshListBinWorking)// ||m_bUpdating)
	{
		return;
	}
	m_tLastRefresh=time(NULL);
//	theApp.BeginWaitCursor();
//	m_bExit=FALSE;
	CreateWaitingWnd();
	EnableWindow(FALSE);
	CString strTemp="Searching file ...";
//	strcpy(szInfo,"Searching file ...\n");
	if(m_pWaitingDlg)
	{
		CRect rc;
		m_pWaitingDlg->GetWindowRect(&rc);
//		ScreenToClient(&rc);
		rc.bottom=rc.top+rc.Height()*2/3;
		m_pWaitingDlg->MoveWindow(&rc);
		m_pWaitingDlg->GetDlgItem(IDC_STATIC_INFO2)->ShowWindow(SW_HIDE);

		m_pWaitingDlg->SetLabelText(1,strTemp);
		m_pWaitingDlg->SetLabelText(0,_T("Searching"));
		m_pWaitingDlg->SetProgressRange(0,20);
		m_pWaitingDlg->StartMoveBar();
	}
	AfxBeginThread(RefreshListBinThread,this);
}
UINT CDfuToolDlg::RefreshListBinThread(LPVOID pParam)
{
	CDfuToolDlg *pParent=(CDfuToolDlg *)pParam;
	pParent->m_bRefreshListBinWorking=TRUE;
	pParent->EnableControls();
	pParent->RefreshListBin();
	pParent->m_bRefreshListBinWorking=FALSE;
	pParent->EnableControls();
//	pParent->PostMessage()
	//	pParent->m_bDownloading=FALSE;
//	theApp.EndWaitCursor();
	if(pParent->m_pWaitingDlg)
		pParent->m_pWaitingDlg->EndMoveBar();
	pParent->PostMessage(MY_FINISH_MESSAGE,0);
	return 0;
}
//
int CDfuToolDlg::RefreshListBin()
{
	CImage iMage;
	CString strTemp;
	CString strUrl="http://6x.cubetube.org/apps/";
	CString strLocalFile;
	int nm=0;
	m_cbi.RemoveAll();
	strLocalFile.Format(_T("%s\\apps.info"),theApp.m_strCubeBinDir);
	if(GetURLReturnStr(strUrl,strTemp))
	{
		nm=FindCubeBinInfo(strTemp,m_cbi);
//		PrintMsgToFile(_T("11.txt"),_T("%d \xd\xa %s"),nm,strTemp);
//		PrintMsgInfo(strTemp);
// 		for (int i=0;i<nm;i++)
// 		{
// 			PrintMsgToFile(_T("12.txt"),_T("%s"),m_cbi.GetAt(i).szIconUrl);
// 		}
		SaveToFile(strLocalFile,(void *)(LPCTSTR)strTemp,strTemp.GetLength());
	}

	if(nm==0)	//网络上没找到时，则找本地的
	{
		LoadFileToString(strLocalFile,strTemp);
		if(strTemp.GetLength()>0)
		{
			nm=FindCubeBinInfo(strTemp,m_cbi);
			if(nm>0)
				ToLocalBinInfo(m_cbi);
//			PrintMsgInfo(strTemp); 	PrintMsgInfo("%d",nm); return 0;
		}
		if(nm==0)
			nm=FindLocalCubeBinInfo(m_cbi);
		if(nm==0)
			return 0;
	}
// 	PrintMsgInfo("%d",nm);
// 	PrintMsgInfo(m_cbi.GetAt(0).szBinUrl);	PrintMsgInfo(m_cbi.GetAt(0).szIconUrl);
// 	PrintMsgInfo(m_cbi.GetAt(1).szBinUrl);	PrintMsgInfo(m_cbi.GetAt(1).szIconUrl);
// 	return 0;

	m_ImageListBin.DeleteImageList();
	m_ImageListBin.Create(BINICON_W,BINICON_H,ILC_COLOR24,1,1);
	m_ImageListBin.Add(&m_bmpUnknow,RGB(255,255,255));

// 	DWORD dwStyle=::GetWindowLong(m_ListCtrlBin.GetSafeHwnd(),GWL_STYLE) ;
// 	dwStyle =dwStyle^CS_VREDRAW;
// 	::SetWindowLong(m_ListCtrlBin.GetSafeHwnd(),GWL_STYLE,dwStyle|WS_VISIBLE );

	m_ListCtrlBin.DeleteAllItems();
	m_ListCtrlBin.SetImageList(&m_ImageListBin,LVSIL_NORMAL );
//	m_ListCtrlBin.SetIconSpacing(BINICON_W+16,BINICON_W+16*2+8);
	m_ListCtrlBin.SetIconSpacing(BINICON_W+BINICON_GAP,BINICON_H+BINICON_GAP);
//	m_ListCtrlBin.EnableScrollBarCtrl(0,FALSE);

	CString strFileUrl,strOutFile;
	nm=m_cbi.GetSize();
	BOOL bb;
	CUBEBININFO *pstCBI;
	for (int i=0;i<nm;i++)
	{
		//if(i<nm) 
			pstCBI=&m_cbi.GetAt(i);
		char *pp=strrchr(pstCBI->szIconUrl,'/');
		bb=FALSE;
		if(pp)
		{
			pp++;
			strFileUrl=pstCBI->szIconUrl;
			strOutFile.Format("%s\\%s",theApp.m_strCubeBinDir,pp);
//			PrintMsgInfo(strOutFile);
			bb=DownloadFile(strFileUrl,strOutFile,FALSE);
//			PrintMsgToFile(_T("2.txt"),_T("%d, %s\xd\xa%s"),bb,strFileUrl,strOutFile);
			if(!bb)
			{
				sprintf(pstCBI->szIconUrl,"%s",strOutFile);
				pp=strrchr(pstCBI->szBinUrl,'/');
				if(pp)
				{
					pp++;
					strFileUrl.Format("%s\\%s",theApp.m_strCubeBinDir,pp);
					sprintf(pstCBI->szBinUrl,"%s",strFileUrl);
				}
				else
					pstCBI->szBinUrl[0]=0;
				bb=TRUE;
			}
		}
		else
		{
			if(pstCBI->szIconUrl[0]==0)
			{
				strOutFile.Empty(); bb=FALSE;
			}
			else
			{
				strOutFile.Format(_T("%s"),pstCBI->szIconUrl);	bb=TRUE;
			}
		}
		int idxbmp=0;
		if(bb)
		{
			COLORREF bc=RGB(255,255,255); //RGB(35,31,32);	//
			CRect rc(0,0,BINICON_W,BINICON_H);
			iMage.Load(strOutFile);
//			FillRect(&m_MemDC,&rc,RGB(60+i%4*40,100+i%10*10,130+i%10*10),RGB(100+i%5*20,50+i%20*8,120+i%20*5));
			iMage.Draw(m_MemDC.m_hDC,0,0,BINICON_W,BINICON_H);
//			iMage.TransparentBlt(m_MemDC.m_hDC,0,0,BINICON_W,BINICON_H,bc);

			m_ImageListBin.Add(&m_bmpBin,bc);
			idxbmp=i+1;
//			PrintMsgToFile(_T("2.txt"),_T("%d, %s\xd\xa%s"),idxbmp,strFileUrl,strOutFile);
		}
//		int k=m_ListCtrlBin.InsertItem(i,pstCBI->szTitle,idxbmp);
		int k=m_ListCtrlBin.InsertItem(i,_T(""),idxbmp);
		WaitForSingleObject(m_hWaitEvent,50);
//		PrintMsgToFile("1.txt","%d,%d,%d",bb,i+1,m_ImageListBin.GetImageCount());
	}
	return nm;
}
/*
CRgn rgn;
rgn.CreateEllipticRgn(0,0,rc.Width(),rc.Height()); 
// 			HBITMAP hBmpOld;
// 			hBmpOld = (HBITMAP)SelectObject(hMemDC,m_hBmpKB64);//m_hBmpKB48);
// 			BitBlt(hDC,0,0,m_nKBIconW,m_nKBIconH,hMemDC,0,0,SRCCOPY);
// 			SelectObject(hMemDC,hBmpOld);
// 			DeleteObject(h_rgn);
CRgn *poldr=(CRgn *)m_MemDC.SelectClipRgn(&rgn);
m_MemDC.SelectClipRgn(poldr);
rgn.DeleteObject();

*/

BOOL CDfuToolDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect crc;
	GetClientRect(&crc);
	m_imageBK.Draw(pDC->m_hDC,crc);
	return TRUE; //CDialog::OnEraseBkgnd(pDC);
}
void CDfuToolDlg::CreateWaitingWnd()
{
	CloseWaitingWnd();
	if(m_pWaitingDlg==NULL)
	{
		m_pWaitingDlg=new CWaitingDlg(this);
		m_pWaitingDlg->Create(IDD_WAITING_DLG);
		m_pWaitingDlg->SetDispProgressMode(DISPPROGRESS_ONLYPERCENT);
	}
	CRect wrc,rc;
	m_pWaitingDlg->GetWindowRect(&rc);
	GetWindowRect(&wrc);
//	PrintMsgWarning(_T("%d,%d"),rc.Width(),rc.Height());
	int x,y;
	x=(wrc.left+wrc.right-rc.Width())/2;
	y=(wrc.top+wrc.bottom-rc.Height())/2;
	m_pWaitingDlg->SetWindowPos(NULL,x,y,0,0,SWP_NOSIZE);
//	wrc.left=x; wrc.top=y; wrc.right=wrc.left+rc.Width(); wrc.bottom=wrc.top+rc.Height();
//	m_pWaitingDlg->MoveWindow(&wrc);
	m_pWaitingDlg->ShowWindow(SW_SHOW);
	m_pWaitingDlg->SetProgressPos(0);
}
void CDfuToolDlg::CloseWaitingWnd()
{
	if(m_pWaitingDlg==NULL)
		return;
	m_pWaitingDlg->DestroyWindow();
	delete m_pWaitingDlg;
	m_pWaitingDlg	= NULL;
}




void CDfuToolDlg::OnBnClickedBtnInstalldfu()
{
	DoInstallDriver(&s_mydev_Dfumode);
}


void CDfuToolDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if(m_bAutoUpdating||m_bRefreshListBinWorking) 
		return;
	CMenu menu;
	menu.LoadMenu(IDR_POPUPMENU1);
	CMenu *pMenu; 
	CRect wrc;
	m_ListCtrlBin.GetWindowRect(&wrc);
//	ScreenToClient(&wrc);
	if(wrc.PtInRect(point) && m_ListCtrlBin.GetSelectedCount()>0)
	{
		pMenu=menu.GetSubMenu(1);
	}
	else
		pMenu=menu.GetSubMenu(0);
// 	if(m_bAutoUpdating||m_bRefreshListBinWorking) 
// 		pMenu->EnableMenuItem(IDM_REFRESHLISTBIN,MF_BYCOMMAND|MF_GRAYED);
	pMenu->TrackPopupMenu (TPM_LEFTALIGN|TPM_RIGHTBUTTON,point.x,point.y,this);

	pMenu->DestroyMenu();
}


void CDfuToolDlg::OnRefreshListbin()
{
	DoRefreshListBin();
}


BOOL CDfuToolDlg::PreTranslateMessage(MSG* pMsg)
{
	if(WM_KEYDOWN == pMsg->message){
		switch(pMsg->wParam){
		case VK_RETURN:
			if(pMsg->hwnd==m_ListCtrlBin.GetSafeHwnd())
			{
				if(m_ListCtrlBin.GetSelectedCount()>0)
				{
					PostMessage(WM_COMMAND,IDM_UPDATECUBE);
				}
				return TRUE;
			}
			break;
		default:
			break;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CDfuToolDlg::OnUpdateCube()
{
	if(m_ListCtrlBin.GetItemCount()==0)
		return;
	if(m_ListCtrlBin.GetSelectedCount()==0)
		return;
	for (int i=0;i<m_ListCtrlBin.GetItemCount();i++)
	{
		if(m_ListCtrlBin.GetItemState(i,LVIS_SELECTED) & LVIS_SELECTED)
		{
//			PrintMsgInfo("%d",i+1);
			DoUpdateFromListBinItem(i);
			break;
		}
	}
}


void CDfuToolDlg::OnSetFocus(CWnd* pOldWnd)
{
	CDialog::OnSetFocus(pOldWnd);

	m_ListCtrlBin.SetFocus();
}
