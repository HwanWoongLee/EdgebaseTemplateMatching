#include "pch.h"
#include "framework.h"
#include "EdgebaseTemplateMatching.h"
#include "EdgebaseTemplateMatchingDlg.h"
#include "afxdialogex.h"

#include "TMatView.h"
#include "CDetector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEdgebaseTemplateMatchingDlg 대화 상자



CEdgebaseTemplateMatchingDlg::CEdgebaseTemplateMatchingDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDGEBASETEMPLATEMATCHING_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pViewMark = nullptr;
	m_pViewShow = nullptr;
	m_pDetector = nullptr;
	m_pThread = nullptr;
	m_bMatching = false;
}

void CEdgebaseTemplateMatchingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_CANNY_1, m_sliderCanny1);
	DDX_Control(pDX, IDC_SLIDER_CANNY_2, m_sliderCanny2);
	DDX_Control(pDX, IDC_SLIDER_ROTATE, m_sliderRotate);
	DDX_Control(pDX, IDC_EDIT_CANNY_1, m_editCanny1);
	DDX_Control(pDX, IDC_EDIT_CANNY_2, m_editCanny2);
	DDX_Control(pDX, IDC_EDIT_ROTATE, m_editRotate);
}

BEGIN_MESSAGE_MAP(CEdgebaseTemplateMatchingDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOAD_MARK, &CEdgebaseTemplateMatchingDlg::OnBnClickedButtonLoadMark)
	ON_BN_CLICKED(IDC_BUTTON_MATCHING, &CEdgebaseTemplateMatchingDlg::OnBnClickedButtonMatching)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMAGE, &CEdgebaseTemplateMatchingDlg::OnBnClickedButtonLoadImage)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CEdgebaseTemplateMatchingDlg 메시지 처리기

BOOL CEdgebaseTemplateMatchingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	this->SetWindowTextW(_T("Edgebase Template Matching"));

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.


	CRect rect;
	CWnd* pWnd;

	// Creat Viewer
	if (!m_pViewShow) {
		m_pViewShow = new CTMatView();
		if (!m_pViewShow->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(), this, IDC_VIEWER_SHOW)) {
			AfxMessageBox(_T("Failed Create Mark View"));
			return FALSE;
		}

		pWnd = GetDlgItem(IDC_STATIC_VIEW_SHOW);
		pWnd->GetWindowRect(rect);
		ScreenToClient(rect);
		m_pViewShow->MoveWindow(rect);
		m_pViewShow->ShowTool(false);
	}
	if (!m_pViewMark) {
		m_pViewMark = new CTMatView();
		if (!m_pViewMark->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(), this, IDC_VIEWER_MARK)) {
			AfxMessageBox(_T("Failed Create Mark View"));
			return FALSE;
		}
		pWnd = GetDlgItem(IDC_STATIC_VIEW_MARK);
		pWnd->GetWindowRect(rect);
		ScreenToClient(rect);
		m_pViewMark->MoveWindow(rect);
		m_pViewMark->ShowTool(false);
	}

	// Create Detector
	if (!m_pDetector) {
		m_pDetector = new CDetector();
	}

	// Slider Control
	m_sliderCanny1.SetRange(0, 255);
	m_sliderCanny2.SetRange(0, 255);
	m_sliderRotate.SetRange(0, 360);

	m_sliderCanny1.SetPos(100);
	m_sliderCanny2.SetPos(200);
	m_sliderRotate.SetPos(0);

	m_editCanny1.SetWindowText(_T("100"));
	m_editCanny2.SetWindowText(_T("200"));
	m_editRotate.SetWindowText(_T("0"));

	m_canny1 = 100;
	m_canny2 = 200;
	m_rotate = 0;

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CEdgebaseTemplateMatchingDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CEdgebaseTemplateMatchingDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CEdgebaseTemplateMatchingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEdgebaseTemplateMatchingDlg::OnBnClickedButtonLoadMark()
{
	if (!m_pViewMark)
		return;

	CString szFilter = _T("Image (*.BMP, *.PNG, *.JPG) | *.BMP;*.PNG;*.JPG;*.jpg | All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	Mat loadImg;
	if (IDOK == dlg.DoModal()) {
		CString cstrPath = dlg.GetPathName();
		CT2CA pszConvertedAnsiString(cstrPath);
		string strPath(pszConvertedAnsiString);

		loadImg = imread(strPath);
	}

	if (!loadImg.empty()) {
		m_pViewMark->SetImage(loadImg);
	}
}


void CEdgebaseTemplateMatchingDlg::OnBnClickedButtonMatching()
{
	if (!m_pDetector) {
		AfxMessageBox(_T("Not have detector"));
		return;
	}

	m_bMatching = !m_bMatching;

	if (!m_pThread) {
		m_pThread = new std::thread([&]() {
			while (true) {
				if (m_bMatching)
					Matching();
			}
		});
	}
}

void CEdgebaseTemplateMatchingDlg::Matching() {
	cv::Mat image, mark, dst;

	image = m_pViewShow->GetImage();
	mark = m_pViewMark->GetImage();

	if (image.empty() || mark.empty())
		return;

	m_pDetector->m_iCanny1 = m_canny1;
	m_pDetector->m_iCanny2 = m_canny2;
	m_pDetector->m_iRotate = m_rotate;

	if (!m_pDetector->Detect(image, mark, dst, 0.3)) {
		// AfxMessageBox(_T("Detect Failed..."));
		return;
	}

	if (dst.size() != image.size())
		cv::resize(dst, dst, image.size());

	cv::Mat show_image;
	cv::addWeighted(image, 0.8, dst, 1.0, 0, show_image);

	cv::imshow("Edge Detect", show_image);
	cv::waitKey(10);
}

void CEdgebaseTemplateMatchingDlg::OnBnClickedButtonLoadImage()
{
	if (!m_pViewShow)
		return;

	CString szFilter = _T("Image (*.BMP, *.PNG, *.JPG) | *.BMP;*.PNG;*.JPG;*.jpg | All Files(*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

	Mat loadImg;
	if (IDOK == dlg.DoModal()) {
		CString cstrPath = dlg.GetPathName();
		CT2CA pszConvertedAnsiString(cstrPath);
		string strPath(pszConvertedAnsiString);

		loadImg = imread(strPath);
	}

	if (!loadImg.empty()) {
		m_pViewShow->SetImage(loadImg);
	}
}


void CEdgebaseTemplateMatchingDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int iPos = 0;
	CString strPos = _T("");

	if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_CANNY_1) {
		iPos = m_sliderCanny1.GetPos();

		strPos.Format(_T("%d"), iPos);
		m_editCanny1.SetWindowTextW(strPos);
		
		m_canny1 = iPos;
	}
	else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_CANNY_2) {
		iPos = m_sliderCanny2.GetPos();

		strPos.Format(_T("%d"), iPos);
		m_editCanny2.SetWindowTextW(strPos);

		m_canny2 = iPos;
	}
	else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_ROTATE) {
		iPos = m_sliderRotate.GetPos();

		strPos.Format(_T("%d"), iPos);
		m_editRotate.SetWindowTextW(strPos);

		m_rotate = iPos;
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
