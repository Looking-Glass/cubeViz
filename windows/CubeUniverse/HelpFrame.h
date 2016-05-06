#pragma once


// CHelpFrame 框架

class CHelpFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CHelpFrame)
public:
	CHelpFrame();           // 动态创建所使用的受保护的构造函数
	virtual ~CHelpFrame();

	CView	*m_pParentView;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
};


