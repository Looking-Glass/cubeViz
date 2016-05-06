// HelpView.cpp : 实现文件
//

#include "stdafx.h"
#include "DfuTool.h"
#include "HelpView.h"


// CHelpView

IMPLEMENT_DYNCREATE(CHelpView, CScrollView)

CHelpView::CHelpView() //:/CFormView(CHelpView::IDD)
{

}

CHelpView::~CHelpView()
{
}

void CHelpView::DoDataExchange(CDataExchange* pDX)
{
	CScrollView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHelpView, CScrollView)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CHelpView 诊断

#ifdef _DEBUG
void CHelpView::AssertValid() const
{
	CScrollView::AssertValid();
}

#ifndef _WIN32_WCE
void CHelpView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif
#endif //_DEBUG


// CHelpView 消息处理程序

void CHelpView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

//	m_imgReadme.LoadFromResource()
	LoadImageFromResource(&m_imgReadme,IDB_PNG_README);

	CSize sizeTotal;
	sizeTotal.cx = m_imgReadme.GetWidth(); //200; //rc.right-rc.left;
	sizeTotal.cy = m_imgReadme.GetHeight(); //100; //rc.bottom-rc.top;
	SetScrollSizes(MM_TEXT, sizeTotal);

}

void CHelpView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CScrollView::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if(bShow)
	{
	}
}

BOOL CHelpView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style&=~WS_BORDER;

	return CScrollView::PreCreateWindow(cs);
}


void CHelpView::OnDraw(CDC* pDC)
{
	// TODO: 在此添加专用代码和/或调用基类
	m_imgReadme.Draw(pDC->m_hDC,0,0);
}
