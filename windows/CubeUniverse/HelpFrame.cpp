// HelpFrame.cpp : 实现文件
//

#include "stdafx.h"
#include "DfuTool.h"
#include "HelpFrame.h"


// CHelpFrame

IMPLEMENT_DYNCREATE(CHelpFrame, CFrameWnd)

CHelpFrame::CHelpFrame()
{
	m_pParentView=NULL;
}

CHelpFrame::~CHelpFrame()
{
}


BEGIN_MESSAGE_MAP(CHelpFrame, CFrameWnd)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CHelpFrame 消息处理程序


void CHelpFrame::OnClose()
{
	ShowWindow(SW_HIDE);

//	CFrameWnd::OnClose();
}
