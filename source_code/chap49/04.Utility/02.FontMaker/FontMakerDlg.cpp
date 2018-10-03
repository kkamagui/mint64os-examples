/**
 *  file    FontMakerDlg.cpp
 *  date    2009/09/09
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   폰트를 추출하는 Font Maker의 핵심 소스 파일
 */
// FontMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FontMaker.h"
#include "FontMakerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontMakerDlg dialog

CFontMakerDlg::CFontMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFontMakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFontMakerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFontMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFontMakerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFontMakerDlg, CDialog)
	//{{AFX_MSG_MAP(CFontMakerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_FONTSELECT, OnButtonFontselect)
	ON_BN_CLICKED(IDC_BUTTON_FONTMAKE, OnButtonFontmake)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFontMakerDlg message handlers

void func1()
{

}

BOOL CFontMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	m_iFontSize = 0;

	func1();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFontMakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFontMakerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
	// 윈도우를 그릴 때 영문자와 한글을 출력하여 셈플을 보여줌
	else
	{
        CPaintDC dc( this );

        CString clsData;
        SIZE stHangulSize;
		SIZE stEnglishSize;
		CPen clPen( PS_DOT, 1, RGB( 255, 0, 0 ) );
		CPen* pclOldPen;

		// 배경색을 투명하게 설정하여 문자를 표시할 때 윈도우 배경을 지우지 않도록 함
		dc.SetBkMode( TRANSPARENT );

		// 생성한 폰트 설정
        dc.SelectObject( &m_clFont );

		// 펜을 설정
		pclOldPen = dc.SelectObject( &clPen );

		// 폰트 크기 저장
		GetTextExtentPoint32( dc.m_hDC, "A", 1, &stEnglishSize );
        GetTextExtentPoint32( dc.m_hDC, "가", 2, &stHangulSize );

		// 아래 위 기준선 출력
		dc.MoveTo( 0, 5 );
		dc.LineTo( 1000, 5 );
		dc.MoveTo( 0, 5 + stEnglishSize.cy - 1 );
		dc.LineTo( 1000, 5 + stEnglishSize.cy - 1 );
		// 영문자 출력
        dc.TextOut( 0, 5, "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
		
		// 아래 위 기준선 출력
		dc.MoveTo( 0, 30 );
		dc.LineTo( 1000, 30 );
		dc.MoveTo( 0, 30 + stHangulSize.cy - 1 );
		dc.LineTo( 1000, 30 + stHangulSize.cy - 1 );
		// 한글 출력
        dc.TextOut( 0, 30, "ㄱㄴㄷㄹㅁㅂㅅㅇㅈㅊㅋㅌㅍㅎ 가나다라마바사아자차카타파하" );

		// 폰트의 크기를 출력
        clsData.Format( "English Font Pixel %dx%d, Hangul Font Pixel %dx%d", 
			stEnglishSize.cx, stEnglishSize.cy, stHangulSize.cx, stHangulSize.cy );
        dc.TextOut( 0, 60, clsData );

		// Pen을 복원
		dc.SelectObject( pclOldPen );
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFontMakerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFontMakerDlg::OnButtonFontselect() 
{
    CFontDialog dlg;
    LOGFONT stLogFont;
    CPaintDC dc( this );

    if( dlg.DoModal() != IDOK )
    {
        return ;
    }

    // 선택된 폰트를 반환
    dlg.GetCurrentFont( &stLogFont );

	// 폰트 사이즈가 16 이면 160으로 나옴
	stLogFont.lfHeight = dlg.GetSize() / 10;
	if( stLogFont.lfHeight > 16 )
	{
		AfxMessageBox( "폰트 크기는 16 크기 이상 생성할 수 없습니다" );
		return ;
	}

	m_iFontSize = stLogFont.lfHeight;

    m_clFont.DeleteObject();
    if( m_clFont.CreateFontIndirect( &stLogFont ) == FALSE )
    {
        TRACE( "폰트 생성 실패\n" );
    }

    // 윈도우를 새로 그림
    Invalidate( TRUE );
}

/**
    영문자와 완성형 한글을 모두 다 출력하여 폰트 파일 생성 
*/
void CFontMakerDlg::OnButtonFontmake() 
{
    CPaintDC dc( this );
    CDC clTempDc;
    CBitmap clBitmap;

    // DC를 생성해서 bitmap을 설정
    clTempDc.CreateCompatibleDC( &dc );
    clBitmap.CreateCompatibleBitmap( &clTempDc, 100, 100 );
    clTempDc.SelectObject( &clBitmap );
    clTempDc.SelectObject( &m_clFont );
    
    // 한글 폰트파일을 생성
    SaveHangulFont( &clTempDc );
    // 영문 폰트파일을 생성
    SaveEnglishFont( &clTempDc );

    AfxMessageBox( "폰트 생성이 끝났습니다." );
}

/**
    한글 폰트 파일을 생성
*/
void CFontMakerDlg::SaveHangulFont( CDC* pclTempDC )
{
    int i;
    int j;
    int iCount;
    char vcBuffer[ 2 ];

    // 폰트 파일을 생성
    if( CreateHangulFontFile() == FALSE )
    {
        return ;
    }

	//=========================================================================
	// 폰트 데이터를 생성한 파일에 저장
	//=========================================================================
    //자음/모음 각각 0xA4A1 ~ 0xA4D3 생성
    vcBuffer[ 0 ] = ( char )0xA4;
    for( i = 0xA1 ; i <= 0xD3 ; i++ )
    {
        vcBuffer[ 1 ] = i;
        pclTempDC->TextOut( 0, 0, vcBuffer, 2 );

        // 모든 픽셀에 대해서 비트마스크를 생성
        SaveBitMask( pclTempDC, TRUE );
    }

    iCount = 0;
    // 완성형 한글 0xB0A1에서 0xC8FE 생성
    for( j = 0xB0 ; j <= 0xC8 ; j++ )
    {
        vcBuffer[ 0 ] = j;
        for( i = 0xA1 ; i <= 0xFE ; i++  )
        {
            vcBuffer[ 1 ] = i;
            pclTempDC->TextOut( 0, 0, vcBuffer, 2 );

            // 모든 픽셀에 대해서 비트마스크를 생성
            SaveBitMask( pclTempDC, TRUE );
            iCount++;
        }
    }

    TRACE( "Total Count = %d\n", iCount );

    // 뒤로 3칸 이동하여 ,\r\n을 삭제
    m_clFile.Seek( -3, SEEK_CUR );
    WriteFontData( "};\r\n" );
    CloseFontFile();
}

/**
    영문 폰트 파일을 생성
*/
void CFontMakerDlg::SaveEnglishFont( CDC* pclTempDC )
{
    int i;
    int iCount;
    char cBuffer;

	//=========================================================================
	// 폰트 데이터를 생성한 파일에 저장
	//=========================================================================
    // 폰트 파일을 생성
    if( CreateEnglishFontFile() == FALSE )
    {
        return ;
    }

    iCount = 0;
	// 0~255 문자 생성
    for( i = 0 ; i <= 0xFF ; i++ )
    {
        cBuffer = i;
		pclTempDC->FillSolidRect( 0, 0, 20, 20, RGB( 255, 255, 255 ) );
        pclTempDC->TextOut( 0, 0, &cBuffer, 1 );

        // 모든 픽셀에 대해서 비트마스크를 생성
        SaveBitMask( pclTempDC, FALSE );
        iCount++;
    }

    TRACE( "Total Count = %d\n", iCount );

    // 뒤로 3칸 이동하여 ,\r\n을 지움
    m_clFile.Seek( -3, SEEK_CUR );
    WriteFontData( "};\r\n" );
    CloseFontFile();
}

/**
    비트 마스크를 저장
*/
void CFontMakerDlg::SaveBitMask( CDC* clTempDc, BOOL bHangul )
{
    int i;
    int j;
    COLORREF stColor;
    unsigned short usBitMask;
	int iFontWidth;

	// 한글이면 폰트 너비 만큼을 모두 검색하고 영문자이면 너비가 한글의 반이므로
	// 반으로 설정함
	if( bHangul == TRUE )
	{
		iFontWidth = m_iFontSize;
	}
	else
	{
		iFontWidth = m_iFontSize / 2;
	}

    // 폰트 비트맵을 생성하여 파일에 저장
    for( j = 0 ; j < m_iFontSize ; j++ )
    {
        usBitMask = 0;
        for( i = 0 ; i < iFontWidth ; i++ )
        {
            stColor = clTempDc->GetPixel( i, j );
            if( stColor != 0xFFFFFF )
            {
                usBitMask |= ( 0x01 << ( iFontWidth - 1 - i ) );
            }
        }
        // BitMask를 쓴다.
        WriteFontData( usBitMask, bHangul );
        WriteFontData( "," );
    }
    WriteFontData( "\r\n" );
}

/**
    한글 폰트 파일을 생성
        파일을 생성한 후, C언어 배열의 헤더를 추가
*/
BOOL CFontMakerDlg::CreateHangulFontFile()
{
    char* pcHeader = "// 자모 51개 + 완성형 2350개 폰트 데이터, %dx%d pixel \r\n"
        "unsigned short g_vusHangulFont[] = { \r\n";
    char vcBuffer[ 1024 ];

	sprintf( vcBuffer, pcHeader, m_iFontSize, m_iFontSize );
	// 파일 생성
    if( m_clFile.Open( "HangulFont.c", CFile::modeCreate | 
        CFile::modeReadWrite ) == FALSE )
    {
        AfxMessageBox( "파일 생성에 실패했습니다." );
        return FALSE;
    }
    m_clFile.Write( vcBuffer, strlen( vcBuffer ) );
    return TRUE;
}

/**
    영문 폰트 파일을 생성
        파일을 생성한 후, C언어 배열의 헤더를 추가
*/
BOOL CFontMakerDlg::CreateEnglishFontFile()
{
    char* pcHeader = "// 영문자 256개 폰트 데이터, %dx%d pixel \r\n"
        "unsigned char g_vucEnglishFont[] = { \r\n";
    char vcBuffer[ 1024 ];

	sprintf( vcBuffer, pcHeader, m_iFontSize / 2, m_iFontSize );
	// 파일 생성
    if( m_clFile.Open( "EnglishFont.c", CFile::modeCreate | 
        CFile::modeReadWrite ) == FALSE )
    {
        AfxMessageBox( "파일 생성에 실패했습니다." );
        return FALSE;
    }
    m_clFile.Write( vcBuffer, strlen( vcBuffer ) );
    return TRUE;
}


/**
    Font 파일에 데이터 저장
*/
void CFontMakerDlg::WriteFontData( unsigned short usData, BOOL bHangul )
{
    char vcBuffer[ 20 ];

    // 한글일 경우는 2byte로 만들고 영문일 경우는 1byte로 저장
	if( bHangul == TRUE )
	{
		sprintf( vcBuffer, "0x%04X", usData );
	}
	else
	{
		sprintf( vcBuffer, "0x%02X", usData & 0xFF );
	}

    m_clFile.Write( vcBuffer, strlen( vcBuffer ) );
}

/**
    Font 파일에 데이터를 씀
*/
void CFontMakerDlg::WriteFontData( char* pcData )
{
    m_clFile.Write( pcData, strlen( pcData ) );
}

/**
    Font 파일을 닫음
*/
void CFontMakerDlg::CloseFontFile()
{
    m_clFile.Close();
}
