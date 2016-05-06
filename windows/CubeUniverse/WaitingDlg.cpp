// WaitingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DfuTool.h"
#include "WaitingDlg.h"
#include "afxdialogex.h"


// CWaitingDlg 对话框

IMPLEMENT_DYNAMIC(CWaitingDlg, CDialogEx)

CWaitingDlg::CWaitingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWaitingDlg::IDD, pParent)
{
	int fw,fh;
	fw=7; fh=fw*2;
	m_Font1.CreateFont(fh, fw, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
	fw=8; fh=fw*2+2;
	m_Font2.CreateFont(fh,fw, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
	fw=8; fh=fw*2;
	m_Font3.CreateFont(fh, fw, 0, 0, FW_MEDIUM+100, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
//		_T("Courier New"));
	m_nProgressMin=0;
	m_nProgressMax=100;
	m_nProgressPos=0;
	m_bMoveBarMode=FALSE;

	m_strS1.Empty();
	m_strS2.Empty();
}

CWaitingDlg::~CWaitingDlg()
{
	m_Font1.DeleteObject();
	m_Font2.DeleteObject();
	m_Font3.DeleteObject();
}

void CWaitingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	DDX_Control(pDX,IDC_STATIC_WORKING,m_LabelWorkInfo);
	DDX_Control(pDX,IDC_STATIC_INFO1,m_LabelInfo1);
	DDX_Control(pDX,IDC_STATIC_INFO2,m_LabelInfo2);
	DDX_Control(pDX,IDC_STATIC_INFO3,m_LabelInfo3);
}
 

BEGIN_MESSAGE_MAP(CWaitingDlg, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CWaitingDlg 消息处理程序


BOOL CWaitingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_LabelWorkInfo.SetFont(&m_Font2); m_LabelWorkInfo.SetBkColor(WHITE);
	m_LabelInfo1.SetFont(&m_Font3); m_LabelInfo1.SetBkColor(WHITE);
	m_LabelInfo1.SetWindowText(NULL);
	m_LabelInfo2.SetFont(&m_Font2); m_LabelInfo2.SetBkColor(WHITE);
	//	m_LabelTitle.SetTextColor(RGB(41,21,170));
	// 	m_LabelNote.SetFont(&m_Font1);
	// 	m_LabelNote.SetTextColor(RGB(170,41,21));
//	SetTimer(1,100,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}




BOOL CWaitingDlg::OnEraseBkgnd(CDC* pDC)
{
//	return CDialogEx::OnEraseBkgnd(pDC);
	DrawInfo(pDC);
	return TRUE;
}
void CWaitingDlg::DrawInfo(CDC *pD)
{
	CDC *pDC;
	if(pD==NULL)
		pDC=GetDC();
	else pDC=pD;
/*
	GetClientRect(&crc);
	rc1=crc; rc2=crc;
	rc1.bottom=(crc.top+crc.bottom)/2;
	rc2.top=rc1.bottom;
	//	pDC->FillSolidRect(&ClientRect,RGB(95,95,93));
	FillRect(pDC,&crc,RGB(190,220,240),RGB(200,220,240));

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(BLUE);
	CFont *pOldF=pDC->SelectObject(&m_Font);
	pDC->DrawText((LPCTSTR)m_strInfo,-1,&rc1,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	pDC->DrawText((LPCTSTR)m_strInfo2,-1,&rc2,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	pDC->SelectObject(pOldF);
*/
	CRect crc,rc,rc1,rc2;
	COLORREF c1,c2,c3;
	GetClientRect(&crc);
	rc=crc;
	pDC->FillSolidRect(&rc,WHITE);
	c1=RGB(159,166,172);
	c2=RGB(35,31,32);
	pDC->Draw3dRect(&rc,c1,c1);
	rc.InflateRect(-1,-1);
	pDC->Draw3dRect(&rc,c2,c2);
	rc.InflateRect(-1,-1);
	pDC->Draw3dRect(&rc,c1,c1);

	CWnd *pWnd1=GetDlgItem(IDC_STATIC_WORKING);
	CWnd *pWnd2=GetDlgItem(IDC_STATIC_INFO1);
	if(pWnd1&&pWnd2)
	{
		pWnd1->GetWindowRect(&rc1);
		ScreenToClient(&rc1);
		pWnd2->GetWindowRect(&rc2);
		ScreenToClient(&rc2);
		rc=rc1;  //rc.right=crc.right-(rc.left-crc.left);
		rc.top=(rc1.bottom+rc2.top)/2;
		rc.bottom=rc.top+1;
		pWnd2=GetDlgItem(IDC_STATIC_INFO2);
		pWnd2->GetWindowRect(&rc2);
		ScreenToClient(&rc2);
		rc.left=rc2.left; rc.right=rc2.right;
		pDC->Draw3dRect(&rc,c2,c2);
		rc.InflateRect(0,1);
		pDC->Draw3dRect(&rc,c2,c2);
	}
	if(pD==NULL)
		ReleaseDC(pDC);
}
void CWaitingDlg::SetDispProgressMode(int m)
{
	if(m<DISPPROGRESS_NO)
		m=DISPPROGRESS_NO;
	if(m>DISPPROGRESS_ALL)
		m=DISPPROGRESS_ALL;
	m_nDispProgressMode=m;
}

void CWaitingDlg::SetProgressRange(int nMin,int nMax)
{
	if(nMin==nMax)
		nMax=nMin+1;
	m_nProgressMin=min(nMin,nMax);
	m_nProgressMax=max(nMin,nMax);
	m_nProgressPos=m_nProgressMin;
}
void CWaitingDlg::SetProgressPos(int nValue)
{
	COLORREF fc,bc;
	CRect crc,rc,rc1,rc2;
	if(nValue<m_nProgressMin)
		nValue=m_nProgressMin;
	if(nValue>m_nProgressMax)
		nValue=m_nProgressMax;
	m_nProgressPos=nValue;
	fc=RGB(45,171,226); bc=RGB(237,237,237);

	CWnd *pWnd=GetDlgItem(IDC_STATIC_PROGRESS);
	CDC *pDC=pWnd->GetDC();
	pWnd->GetClientRect(&crc);
	rc=crc; rc1=rc; rc2=rc;
	int w,w1;
	w=rc.Width();
	w1=m_nProgressPos*w/(m_nProgressMax-m_nProgressMin);
	if(w1>w)
		w1=w;
	rc1.right=rc1.left+w1;
	rc2.left=rc1.right;
	if(m_bMoveBarMode)
	{
		pDC->FillSolidRect(&rc,bc);
		rc1.left=rc1.right-16;
		if(rc1.left<rc.left)
			rc1.left=rc.left;
		if(rc1.Width()>0)
			pDC->FillSolidRect(&rc1,fc);
	}
	else
	{
		if(rc1.Width()>0)
			pDC->FillSolidRect(&rc1,fc);
		if(rc2.Width()>0)
			pDC->FillSolidRect(&rc2,bc);
		if(m_nDispProgressMode!=DISPPROGRESS_NO)
		{
			TCHAR str[100];
			CFont *pOldf=(CFont *)pDC->SelectObject(&m_Font1);
			pDC->SetTextColor(MAGENTA);
			int bm=pDC->SetBkMode(TRANSPARENT);
			int pn=m_nProgressPos*100/(m_nProgressMax-m_nProgressMin);
			if(m_nDispProgressMode==DISPPROGRESS_ONLYPERCENT)
				_stprintf(str,_T("%d%%"),pn);
			else //if(m_nDispProgressMode==DISPPROGRESS_ALL)
				_stprintf(str,_T("%d/%d(%d%%)"),m_nProgressPos,m_nProgressMax,pn);
			pDC->DrawText(str,-1,&rc,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
			pDC->SetBkMode(bm);
			pDC->SelectObject(pOldf);
		}
	}
	pWnd->ReleaseDC(pDC);
}
void CWaitingDlg::OnTimer(UINT_PTR nIDEvent)
{
//	if(nValue<=m_nProgressMax)
	SetProgressPos(m_nProgressPos);
	m_nProgressPos+=2;
	if(m_nProgressPos>m_nProgressMax)
		m_nProgressPos=0;
	CDialogEx::OnTimer(nIDEvent);
}
void CWaitingDlg::StartMoveBar()
{
	m_bMoveBarMode=TRUE;
	SetTimer(1,200,NULL);
}
void CWaitingDlg::EndMoveBar()
{
	KillTimer(1);
	SetProgressPos(m_nProgressMin);
	m_bMoveBarMode=FALSE;
}
void CWaitingDlg::SetLabelText(int idx,LPCTSTR strText)
{
	CString strTemp;//,s1,s2;
	CLabel * pLabel[4]={&m_LabelWorkInfo,&m_LabelInfo1,&m_LabelInfo2,&m_LabelInfo3};
	if(idx<0||idx>=4)
		return;
//	pLabel[idx]->SetWindowText(strText);
	strTemp=strText;
	if(idx!=1 || strTemp.Find(_T('<'))==-1)
	{
		pLabel[idx]->SetText(strTemp);
		if(idx==1) 
		{
			m_strS1.Empty(); m_strS2.Empty();
			pLabel[idx]->ShowWindow(SW_SHOW);
		}
		return;
	}
	//idx==1
	int ln,pos1,pos2;
	pos1=strTemp.Find(_T('<'));
	m_strS1=strTemp.Left(pos1);
	pos2=strTemp.Find(_T('>'));
	if(pos2>pos1)
		ln=pos2-pos1-1;
	else
		ln=strTemp.GetLength()-pos1-1;
	m_strS2=strTemp.Mid(pos1+1,ln);
	pLabel[idx]->ShowWindow(SW_HIDE);

//	s1="my "; s2="OK";
	CRect rc;
	pLabel[idx]->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(&rc);

// 	CDC *pDC=GetDC();
// 	ReleaseDC(pDC);
}

void CWaitingDlg::OnCancel()
{

//	CDialogEx::OnCancel();
}
void CWaitingDlg::OnOK()
{

	//	CDialogEx::OnOK();
}


void CWaitingDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
	CDC *pDC=&dc;

	CRect rc,rc1,rc2;
	CSize sz1,sz2;
	m_LabelInfo1.GetWindowRect(&rc);
	ScreenToClient(&rc);
	pDC->FillSolidRect(&rc,WHITE);
	if(m_strS1.IsEmpty()&&m_strS2.IsEmpty())
		return;

	rc1=rc; rc2=rc;
	CFont *pOFont=pDC->SelectObject(&m_Font3);
	int bm=pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(BLACK);

	sz1=pDC->GetTextExtent(m_strS1);
	sz2=pDC->GetTextExtent(m_strS2);
	int w=sz1.cx+sz2.cx;
	rc1.left=(rc1.left+rc1.right-w)/2;
	pDC->DrawText(m_strS1,&rc1,DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	pDC->SetTextColor(RGB(36,152,218));
	rc2.left=rc1.left+sz1.cx;
	pDC->DrawText(m_strS2,&rc2,DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	pDC->SetBkMode(bm);
	pDC->SelectObject(pOFont);
}
