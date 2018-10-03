// FontMakerDlg.h : header file
//

#if !defined(AFX_FONTMAKERDLG_H__F770EBC1_37BA_46EA_8517_E2A4D4479403__INCLUDED_)
#define AFX_FONTMAKERDLG_H__F770EBC1_37BA_46EA_8517_E2A4D4479403__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CFontMakerDlg dialog

class CFontMakerDlg : public CDialog
{
// Construction
public:
	CFontMakerDlg(CWnd* pParent = NULL);	// standard constructor
    CFont m_clFont;
    CFile m_clFile;
	int m_iFontSize;

protected:
    void SaveBitMask( CDC* clDc, BOOL bHangul );
    BOOL CreateHangulFontFile();
    BOOL CreateEnglishFontFile();
    void WriteFontData( unsigned short usData, BOOL bHangul );
    void WriteFontData( char* pcData );
    void CloseFontFile();
    void SaveHangulFont( CDC* pclTempDC );
    void SaveEnglishFont( CDC* pcTempDC );

// Dialog Data
	//{{AFX_DATA(CFontMakerDlg)
	enum { IDD = IDD_FONTMAKER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFontMakerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFontMakerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonFontselect();
	afx_msg void OnButtonFontmake();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FONTMAKERDLG_H__F770EBC1_37BA_46EA_8517_E2A4D4479403__INCLUDED_)
