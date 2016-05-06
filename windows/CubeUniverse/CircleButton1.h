#if !defined(AFX_CIRCLEBUTTON1_H__F7CE6C35_4AFF_4A55_8FC1_58CC53B81B19__INCLUDED_)
#define AFX_CIRCLEBUTTON1_H__F7CE6C35_4AFF_4A55_8FC1_58CC53B81B19__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CircleButton1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCircleButton window

class CCircleButton : public CButton
{
// Construction
public:
	CCircleButton();

// Attributes
private:
	COLORREF m_textcolor;
	UINT m_bitmapID; //位图ID
	CSize size;//按钮大小
	HRGN h_rgn;	//按钮显示区域
	bool isPressed; //按钮是否被按下

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCircleButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetButtonBitmap(UINT bitmapID);
	virtual ~CCircleButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCircleButton)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CIRCLEBUTTON1_H__F7CE6C35_4AFF_4A55_8FC1_58CC53B81B19__INCLUDED_)
