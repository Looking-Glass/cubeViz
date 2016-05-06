#pragma once
#include "Label.h"

enum {DISPPROGRESS_NO,DISPPROGRESS_ONLYPERCENT,DISPPROGRESS_ALL};
// CWaitingDlg 对话框

class CWaitingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWaitingDlg)
	
public:
	CWaitingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWaitingDlg();

// 对话框数据
	CLabel m_LabelWorkInfo;
	CLabel m_LabelInfo1;
	CLabel m_LabelInfo2;
	CLabel m_LabelInfo3;
	enum { IDD = IDD_WAITING_DLG };
private:
	CFont m_Font1,m_Font2,m_Font3;
	int m_nProgressMin;
	int m_nProgressMax;
	int m_nProgressPos;
	int m_nDispProgressMode;
	BOOL m_bMoveBarMode;
	CString m_strS1,m_strS2;
public:
	void SetDispProgressMode(int m);
	void DrawInfo(CDC *pD=NULL);
	void SetProgressRange(int nMin,int nMax);
	void SetProgressPos(int nValue);
	void SetLabelText(int idx,LPCTSTR strText);

	void StartMoveBar();
	void EndMoveBar();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
//	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
};
