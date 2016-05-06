// Label.cpp : implementation file
//

#include "stdafx.h"
#include "Label.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CLabel, CStatic)
	//{{AFX_MSG_MAP(CLabel)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CLabel::CLabel()
{
	m_crText = GetSysColor(COLOR_WINDOWTEXT);

// 1.1
	m_hBackBrush = NULL;


	m_crHiColor =		0;
	m_crLoColor	=		0;

	m_bTimer =			FALSE;
	m_bState =			FALSE;
	m_bTransparent =	FALSE;
	m_Link =			LinkNone;
	m_hCursor =			NULL;
	m_Type =			None;
	m_bFont3d =			FALSE;
	m_bNotifyParent =	FALSE;
	m_bToolTips =		FALSE;
	m_bRotation =		FALSE;
	m_fillmode =		Normal;
	m_cr3DHiliteColor =	RGB(255,255,255);
	
	m_hwndBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
}

CLabel::~CLabel()
{
	// Clean up
	m_font.DeleteObject();
	::DeleteObject(m_hwndBrush);

	// Stop Checking complaining
	if (m_hBackBrush)
		::DeleteObject(m_hBackBrush);
	
}

void CLabel::UpdateSurface()
{
	CRect (rc);
	GetWindowRect(rc);
	RedrawWindow();

	GetParent()->ScreenToClient(rc);
	GetParent()->InvalidateRect(rc,TRUE);
	GetParent()->UpdateWindow();
}

void CLabel::ReconstructFont()
{
	m_font.DeleteObject();
	BOOL bCreated = m_font.CreateFontIndirect(&m_lf);

	ASSERT(bCreated);
}

void CLabel::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	DWORD dwFlags = 0;

	CRect rc;
	GetClientRect(rc);
	CString strText;
	GetWindowText(strText);
	CBitmap bmp;
	CDC* pDCMem;
	CBitmap*	pOldBitmap = NULL;

	if (!m_bTransparent)
	{
		pDCMem = new CDC;
		pDCMem->CreateCompatibleDC(&dc);
		bmp.CreateCompatibleBitmap(&dc,rc.Width(),rc.Height());
		pOldBitmap = pDCMem->SelectObject(&bmp);
	}
	else
	{
		pDCMem = &dc;
	}

	UINT nMode = pDCMem->SetBkMode(TRANSPARENT);


	COLORREF crText = pDCMem->SetTextColor(m_crText);
	CFont *pOldFont = pDCMem->SelectObject(&m_font);


	// Fill in backgound if not transparent
	if (!m_bTransparent)
	{
		if (m_fillmode == Normal)
		{
			CBrush br;
			
			if (m_hBackBrush != NULL)
				br.Attach(m_hBackBrush);
			else
				br.Attach(m_hwndBrush);
					
			pDCMem->FillRect(rc,&br);

			br.Detach();
		}
		else // Gradient Fill
		{
			DrawGradientFill(pDCMem, &rc, m_crLoColor, m_crHiColor, 100);
		}

	}
	

	// If the text is flashing turn the text color on
	// then to the color of the window background.

	LOGBRUSH lb;
	ZeroMemory(&lb,sizeof(lb));

	// Stop Checking complaining
	if (m_hBackBrush)
		::GetObject(m_hBackBrush,sizeof(lb),&lb);


	// Something to do with flashing
	if (!m_bState && m_Type == Text)
		pDCMem->SetTextColor(lb.lbColor);

	DWORD style = GetStyle();
	
	switch (style & SS_TYPEMASK)
	{
		case SS_RIGHT: 
			dwFlags = DT_RIGHT | DT_WORDBREAK; 
			break; 
		
		case SS_CENTER: 
			dwFlags = SS_CENTER | DT_WORDBREAK;
			break;

		case SS_LEFTNOWORDWRAP: 
			dwFlags = DT_LEFT; 
			break;

		default: // treat other types as left
			case SS_LEFT: 
				dwFlags = DT_LEFT | DT_WORDBREAK; 
				break;
	}	

		
	// Added to expand tabs...
	if(strText.Find(_T('\t')) != -1)
		dwFlags |= DT_EXPANDTABS;

	// If the text centered make an assumtion that
	// the will want to center verticly as well
	if (style & SS_CENTERIMAGE)
	{
		dwFlags = DT_CENTER;

		// Apply 
		if (strText.Find(_T("\r\n")) == -1)
		{
			dwFlags |= DT_VCENTER;

			// And because DT_VCENTER only works with single lines
			dwFlags |= DT_SINGLELINE; 
		}

	}

	if (m_bRotation)
	{
		int nAlign = pDCMem->SetTextAlign (TA_BASELINE);

		CPoint pt;
		GetViewportOrgEx (pDCMem->m_hDC,&pt) ;
		SetViewportOrgEx (pDCMem->m_hDC,rc.Width() / 2, rc.Height() / 2, NULL) ;
		pDCMem->TextOut (0, 0, strText) ;
		SetViewportOrgEx (pDCMem->m_hDC,pt.x / 2, pt.y / 2, NULL) ;
		pDCMem->SetTextAlign (nAlign);
	}
	else
	{
		pDCMem->DrawText(strText,rc,dwFlags);
		if (m_bFont3d)
		{
			pDCMem->SetTextColor(m_cr3DHiliteColor);

			if (m_3dType == Raised)
				rc.OffsetRect(-1,-1);
			else
				rc.OffsetRect(1,1);

			pDCMem->DrawText(strText,rc,dwFlags);
			m_3dType;

		}
	}

	// Restore DC's State
	pDCMem->SetBkMode(nMode);
	pDCMem->SelectObject(pOldFont);
	pDCMem->SetTextColor(crText);

	if (!m_bTransparent){
		dc.BitBlt(0,0,rc.Width(),rc.Height(),pDCMem,0,0,SRCCOPY);
		// continue DC restore 
		pDCMem->SelectObject ( pOldBitmap ) ;
		delete pDCMem;
	}
}

void CLabel::OnTimer(UINT nIDEvent) 
{

	m_bState = !m_bState;

	UpdateSurface();
	
	CStatic::OnTimer(nIDEvent);
}

BOOL CLabel::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{

	if (m_hCursor)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}

	return CStatic::OnSetCursor(pWnd, nHitTest, message);
}

void CLabel::OnLButtonDown(UINT nFlags, CPoint point) 
{

	if (!m_bNotifyParent) // Fix
	{
		CString strLink;

		GetWindowText(strLink);
		if (m_Link == HyperLink)
		{
			ShellExecute(NULL,_T("open"),m_sLink.IsEmpty() ? strLink : m_sLink,NULL,NULL,SW_SHOWNORMAL);
		}
		if (m_Link == MailLink)
		{
			strLink = "mailto:" + strLink;
			ShellExecute( NULL, NULL,  strLink,  NULL, NULL, SW_SHOWNORMAL );
		}
	}
	else
	{
		// To use notification in parent window
		// Respond to a OnNotify in parent and disassemble the message
		//
		NMHDR nm;

		nm.hwndFrom = GetSafeHwnd();
		nm.idFrom  = GetDlgCtrlID();
		nm.code = NM_LINKCLICK;
		GetParent()->SendMessage(WM_NOTIFY,nm.idFrom,(LPARAM) &nm);
	}
		
	CStatic::OnLButtonDown(nFlags, point);
}

CLabel& CLabel::SetText(const CString& strText)
{
	if(IsWindow(this->GetSafeHwnd())) 
	{
		SetWindowText(strText);
		UpdateSurface();
	}

	return *this;
}

CLabel& CLabel::SetTextColor(COLORREF crText)
{

	m_crText = crText;
	
	UpdateSurface();
	return *this;
}

CLabel& CLabel::SetFontBold(BOOL bBold)
{	

	m_lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
	ReconstructFont();
	UpdateSurface();
	return *this;
}

CLabel& CLabel::SetFontUnderline(BOOL bSet)
{	
	m_lf.lfUnderline = bSet;
	ReconstructFont();
	UpdateSurface();

	return *this;
}

CLabel& CLabel::SetFontItalic(BOOL bSet)
{

	m_lf.lfItalic = bSet;
	ReconstructFont();
	UpdateSurface();

	return *this;	
}

CLabel& CLabel::SetSunken(BOOL bSet)
{

	if (!bSet)
		ModifyStyleEx(WS_EX_STATICEDGE,0,SWP_DRAWFRAME);
	else
		ModifyStyleEx(0,WS_EX_STATICEDGE,SWP_DRAWFRAME);
		
	return *this;	
}

CLabel& CLabel::SetBorder(BOOL bSet)
{

	if (!bSet)
		ModifyStyle(WS_BORDER,0,SWP_DRAWFRAME);
	else
		ModifyStyle(0,WS_BORDER,SWP_DRAWFRAME);
		
	return *this;	
}

CLabel& CLabel::SetFontSize(int nSize)
{

	CFont cf;
	LOGFONT lf;

	cf.CreatePointFont(nSize * 10, m_lf.lfFaceName);
	cf.GetLogFont(&lf);

	m_lf.lfHeight = lf.lfHeight;
	m_lf.lfWidth  = lf.lfWidth;

//	nSize*=-1;
//	m_lf.lfHeight = nSize;
	ReconstructFont();
	UpdateSurface();

	return *this;
}

CLabel& CLabel::SetBkColor(COLORREF crBkgnd, COLORREF crBkgndHigh , BackFillMode mode)
{

	m_crLoColor = crBkgnd;
	m_crHiColor = crBkgndHigh;

	m_fillmode = mode;

	if (m_hBackBrush)
		::DeleteObject(m_hBackBrush);

	
	if (m_fillmode == Normal)
		m_hBackBrush = ::CreateSolidBrush(crBkgnd);

	UpdateSurface();

	return *this;
}

CLabel& CLabel::SetFontName(const CString& strFont, BYTE byCharSet /* Default = ANSI_CHARSET */)
{	

	m_lf.lfCharSet = byCharSet;

	_tcscpy(m_lf.lfFaceName,strFont);
	ReconstructFont();
	UpdateSurface();

	return *this;
}

CLabel& CLabel::FlashText(BOOL bActivate)
{

	if (m_bTimer)
		KillTimer(1);

	if (bActivate)
	{
		m_bState = FALSE;
		
		m_bTimer = TRUE;
		
		SetTimer(1,500,NULL);

		m_Type = Text;
	}
	else
		m_Type = None; // Fix

	return *this;
}

CLabel& CLabel::FlashBackground(BOOL bActivate)
{

	if (m_bTimer)
		KillTimer(1);

	if (bActivate)
	{
		m_bState = FALSE;

		m_bTimer = TRUE;
		SetTimer(1,500,NULL);

		m_Type = Background;
	}

	return *this;
}


CLabel& CLabel::SetLink(BOOL bLink,BOOL bNotifyParent)
{

	if (bLink)
		m_Link = HyperLink;
	else
		m_Link = LinkNone;

	m_bNotifyParent = bNotifyParent;

	if (m_Link != LinkNone)
		ModifyStyle(0,SS_NOTIFY);
	else
		ModifyStyle(SS_NOTIFY,0);

  

	return *this;
}

CLabel& CLabel::SetLinkCursor(HCURSOR hCursor)
{

	m_hCursor = hCursor;
	return *this;
}

CLabel& CLabel::SetTransparent(BOOL bSet)
{

	m_bTransparent = bSet;
	ModifyStyleEx(0,WS_EX_TRANSPARENT); // Fix for transparency
	UpdateSurface();

	return *this;
}

CLabel& CLabel::SetFont3D(BOOL bSet,Type3D type)
{

	m_bFont3d = bSet;
	m_3dType = type;
	UpdateSurface();


	return *this;
}

void CLabel::OnSysColorChange() 
{

	if (m_hwndBrush)
		::DeleteObject(m_hwndBrush);

	m_hwndBrush = ::CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	
	UpdateSurface();

		
}

CLabel& CLabel::SetRotationAngle(UINT nAngle,BOOL bRotation)
{
	m_lf.lfEscapement = m_lf.lfOrientation = (nAngle * 10);
	m_bRotation = bRotation;
	
	ReconstructFont();
	
	UpdateSurface();


	return *this;
}

CLabel& CLabel::SetText3DHiliteColor(COLORREF cr3DHiliteColor)
{
	m_cr3DHiliteColor = cr3DHiliteColor;
	UpdateSurface();


	return *this;
}

void CLabel::PreSubclassWindow() 
{

	CStatic::PreSubclassWindow();

	CFont* cf = GetFont();
	if(cf !=NULL)
	{
		cf->GetObject(sizeof(m_lf),&m_lf);
	}
	else
	{
		GetObject(GetStockObject(SYSTEM_FONT),sizeof(m_lf),&m_lf);
	}

	ReconstructFont();
	
}

BOOL CLabel::PreCreateWindow(CREATESTRUCT& cs) 
{	
	return CStatic::PreCreateWindow(cs);
}

void CLabel::DrawGradientFill(CDC* pDC, CRect* pRect, COLORREF crStart, COLORREF crEnd, int nSegments)
{
	// Get the starting RGB values and calculate the incremental
	// changes to be applied.

	COLORREF cr;
	int nR = GetRValue(crStart);
	int nG = GetGValue(crStart);
	int nB = GetBValue(crStart);

	int neB = GetBValue(crEnd);
	int neG = GetGValue(crEnd);
	int neR = GetRValue(crEnd);

	if(nSegments > pRect->Width())
		nSegments = pRect->Width();

	int nDiffR = (neR - nR);
	int nDiffG = (neG - nG);
	int nDiffB = (neB - nB);

	int ndR = 256 * (nDiffR) / (max(nSegments,1));
	int ndG = 256 * (nDiffG) / (max(nSegments,1));
	int ndB = 256 * (nDiffB) / (max(nSegments,1));

	nR *= 256;
	nG *= 256;
	nB *= 256;

	neR *= 256;
	neG *= 256;
	neB *= 256;

	int nCX = pRect->Width() / max(nSegments,1), nLeft = pRect->left, nRight;
	pDC->SelectStockObject(NULL_PEN);

	for (int i = 0; i < nSegments; i++, nR += ndR, nG += ndG, nB += ndB)
	{
		// Use special code for the last segment to avoid any problems
		// with integer division.

		if (i == (nSegments - 1))
			nRight = pRect->right;
		else
			nRight = nLeft + nCX;

		cr = RGB(nR / 256, nG / 256, nB / 256);
		
		{
			CBrush br(cr);
			CBrush* pbrOld = pDC->SelectObject(&br);
			pDC->Rectangle(nLeft, pRect->top, nRight + 1, pRect->bottom);
			pDC->SelectObject(pbrOld);
		}

		// Reset the left side of the drawing rectangle.

		nLeft = nRight;
	}
}

CLabel& CLabel::SetFont(LOGFONT lf)
{
	CopyMemory(&m_lf, &lf, sizeof(m_lf));
	ReconstructFont();
	UpdateSurface();
	return *this;

}
CLabel& CLabel::SetFont(CFont *pFont)
{
	ASSERT(pFont);
	pFont->GetLogFont(&m_lf);
//	CopyMemory(&m_lf, &lf, sizeof(m_lf));
	ReconstructFont();
	UpdateSurface();
	return *this;
}

BOOL CLabel::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

CLabel& CLabel::SetMailLink(BOOL bEnable, BOOL bNotifyParent)
{
	if (bEnable)
		m_Link = MailLink;
	else
		m_Link = LinkNone;


	m_bNotifyParent = bNotifyParent;

	if (m_Link != LinkNone)
		ModifyStyle(0,SS_NOTIFY);
	else
		ModifyStyle(SS_NOTIFY,0);

	return *this;
}

CLabel& CLabel::SetHyperLink(const CString& sLink)
{
	m_sLink = sLink;

	return *this;
}
