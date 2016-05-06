// DfuHelpView.cpp : 实现文件
//

#include "stdafx.h"
#include "DfuTool.h"
#include "DfuHelpView.h"


// CDfuHelpView

IMPLEMENT_DYNCREATE(CDfuHelpView, CFormView)

CDfuHelpView::CDfuHelpView()
	: CFormView(CDfuHelpView::IDD)
{
	int fw,fh;
	fw=6; fh=15;
	m_Font1.CreateFont(fh, fw, 0, 0, FW_NORMAL-100, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
		_T("Arial"));
	m_image=NULL;
}

CDfuHelpView::~CDfuHelpView()
{
	m_Font1.DeleteObject();
	if(m_image)
		delete m_image;
}

void CDfuHelpView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_NOTE, m_LabelNote);
}

BEGIN_MESSAGE_MAP(CDfuHelpView, CFormView)
END_MESSAGE_MAP()


// CDfuHelpView 诊断

#ifdef _DEBUG
void CDfuHelpView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDfuHelpView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CDfuHelpView 消息处理程序


void CDfuHelpView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_LabelNote.SetFont(&m_Font1);
	m_LabelNote.SetTextColor(RGB(170,41,21));

	TCHAR strNote[]=_T("\r\n\
Cube not in DFU mode, please flip the cube upside-down and follow the \
following instructions to enter DFU Mode:\r\n\
1. Hold down BOTH buttons\r\n\
2. Release only the RESET button, while holding down the SETUP button.\r\n\
3. Wait for the LED to start flashing yellow (it will flash magenta first)\r\n\
4. Release the SETUP button\r\n\
");
// 	3.Now you can click AUTO to download the latest Firmware and demo loop and update to your Photo Automatically\r\n\
// 		Or you can download the firmware and demo loop mannually");
	m_LabelNote.SetWindowText(strNote);
	m_LabelNote.Invalidate();

	m_image = new ImageEx( _T("GIF"), _T("DFUMODE") );
//	m_image = new ImageEx( L"Dfu_Mode.gif" );
	if(m_image==NULL)
	{
//		PrintMsgInfo(_T("Have not"));
		return;
	}
#if 1
	CRect crc,rc;
	GetClientRect(crc);
	m_LabelNote.GetWindowRect(&rc);
	ScreenToClient(&rc);

	crc.top=rc.bottom+4;

	int cx = (crc.Width() - m_image->GetWidth()) / 2;
	if(cx<0)
		cx=0;
	m_image->InitAnimation(m_hWnd, CPoint(cx,crc.top));
//	PrintMsgInfo(_T("%d,%d"),m_image->GetWidth(),m_image->GetHeight());
#endif
}


BOOL CDfuHelpView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类
	cs.style&=~WS_BORDER;

	return CFormView::PreCreateWindow(cs);
}
