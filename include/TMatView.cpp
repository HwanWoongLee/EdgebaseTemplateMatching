#include "pch.h"
#include "TMatView.h"
#include "TViewer.h"

IMPLEMENT_DYNAMIC(CTMatView, CWnd)

BEGIN_MESSAGE_MAP(CTMatView, CWnd)
    ON_WM_CREATE()
    ON_WM_SHOWWINDOW()
    ON_WM_PAINT()
    ON_WM_MOUSEWHEEL()
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


CTMatView::CTMatView() {
    InitMatView();
}

CTMatView::CTMatView(cv::Mat image) {
    InitMatView();
    SetImage(image);
}

CTMatView::CTMatView(cv::Mat image, CRect rect) {
    InitMatView();
    SetImage(image);
    SetRectArea(rect);
}

CTMatView::~CTMatView() {

    m_brush.DeleteObject();
}

void CTMatView::InitMatView() {
    for (int i = 0; i < eRECT_NUM; ++i) {
        m_rect[i] = CRect();
    }

    m_bLBDown   = false;
    m_bShowTool = true;
}

void CTMatView::UpdateTool() {
    InvalidateRect(m_rect[eRECT_COORD]);
    InvalidateRect(m_rect[eRECT_ZOOM_RATE]);
}

void CTMatView::ShowTool(bool bShow) {
    if (m_rect[eRECT_WND].IsRectEmpty())
        return;

    m_bShowTool = bShow;

    SetRectArea(m_rect[eRECT_WND]);

    CreateView();
    CreateMenu();

    Invalidate(FALSE);
}

void CTMatView::SetImage(cv::Mat image) {
    if (!image.empty()) {
        if (m_pViewer) {
            m_pViewer->DrawView(image);
            m_pViewer->FitImage();
       }
    }
    else {
        MessageBox(_T("Failed to open image"));
    }
}

cv::Mat CTMatView::GetImage() {
    if (m_pViewer) {
        return m_pViewer->GetImage();
    }
}

void CTMatView::GetImage(cv::Mat& image) {
    if (m_pViewer) {
        image = m_pViewer->GetImage();
    }
}

void CTMatView::MoveWindow(CRect rect) {
    SetParentWnd();
    SetRectArea(rect);

    CreateView();
    CreateMenu();
}

void CTMatView::SetParentWnd() {
    this->m_pwndParent = this->GetParent();
}

void CTMatView::SetRectArea(CRect rect) {
    //Set RectArea
    double dW = rect.Width();
    double dH = rect.Height();

    m_rect[eRECT_WND]   = rect;
    if (m_bShowTool) {
        m_rect[eRECT_MENU] = CRect(0, 0, dW, 25);
        m_rect[eRECT_VIEW] = CRect(0, 25, dW, dH);
    }
    else {
        m_rect[eRECT_MENU] = CRect(0, 0, 0, 0);
        m_rect[eRECT_VIEW] = CRect(0, 0, dW, dH);
    }

    //Set window size    
    SetWindowPos(NULL,
        rect.left, rect.top,
        dW, dH,
        SWP_NOREPOSITION);
}


void CTMatView::CreateView() {
    if (m_pViewer) {
        delete m_pViewer;
        m_pViewer = nullptr;
    }


    if (!m_pViewer) {
        m_pViewer = new TViewer(this);
        m_pViewer->Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(), this);
        m_pViewer->MoveWindow(m_rect[eRECT_VIEW]);
    }
}

void CTMatView::CreateMenu() {
    CRect rect = m_rect[eRECT_MENU];

    double dh = rect.Height();
    //double dw = rect.Width();

    double dButtonWidth = 30.0;

    CRect rectLoad          = CRect(dButtonWidth * 0, 0, dButtonWidth * 2, dh);
    CRect rectSave          = CRect(dButtonWidth * 2, 0, dButtonWidth * 4, dh);
    CRect rectFit           = CRect(dButtonWidth * 4, 0, dButtonWidth * 5, dh);
    m_rect[eRECT_ZOOM_RATE] = CRect(dButtonWidth * 5, 0, dButtonWidth * 7, dh);
    CRect rectNav           = CRect(rect.right - (dButtonWidth * 2), 0, rect.right, dh);
    m_rect[eRECT_COORD]     = CRect(m_rect[eRECT_ZOOM_RATE].right, 0, rectNav.left - 5, dh);

    m_btnLoad.DestroyWindow();
    m_btnSave.DestroyWindow();
    m_btnFit.DestroyWindow();
    m_checkBox.DestroyWindow();

    CreateButton(m_btnLoad, rectLoad, eBTN_LOAD, _T("Load"));
    CreateButton(m_btnSave, rectSave, eBTN_SAVE, _T("Save"));
    CreateButton(m_btnFit,  rectFit,  eBTN_FIT, _T("Fit"));

    if (!m_checkBox.Create(_T("Nav"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, rectNav, this, IDC_CHECK_BOX)) {
        MessageBox(_T("CheckBox create failed"));
    }
    else {
        SetWindowTheme(m_checkBox.m_hWnd, _T(""), _T(""));
        m_checkBox.SetCheck(m_bShowTool);
    }
}

void CTMatView::CreateButton(CMFCButton& button, CRect rect, eBTN_ID btnID, LPCTSTR str) {
    button.Create(NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        rect, this, btnID);

    button.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
    button.SetWindowTextW(str);

    //버튼 문자열이 없으면 이미지.
    if (str == _T("")) {
        CImage btnImg;
        CString cstrIconPath;
        HRESULT hres;

        switch (btnID) {
            case eBTN_LOAD:
                cstrIconPath = _T("..\\icon\\open.ico");
                break;
            case eBTN_SAVE:
                cstrIconPath = _T("..\\icon\\save.ico");
                break;
        }
        //CImage로 불러옴.
        hres = btnImg.Load(cstrIconPath);
        if (hres != S_OK)
            return;

        //CImage -> BITMAP
        HBITMAP hbit;
        hbit = btnImg.Detach();
        BITMAP bit;
        ::GetObject(hbit, sizeof(BITMAP), &bit);

        //BITMAP -> Mat(크기변경을 위해...)
        Mat tempMat;
        tempMat.create(bit.bmWidth, bit.bmHeight, CV_8UC4);
        memcpy(tempMat.data, bit.bmBits, tempMat.cols * tempMat.rows * tempMat.channels());
        flip(tempMat, tempMat, 0);
        resize(tempMat, tempMat, cv::Size(rect.Width(), rect.Height()));

        //Mat-> BITMAP

        button.SetImage(hbit);
    }
}

BOOL CTMatView::LoadImageFile() {
    CString szFilter = _T("Image (*.BMP, *.PNG, *.JPG) | *.BMP;*.PNG;*.JPG;*.jpg | All Files(*.*)|*.*||");
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

    Mat loadImg;

    if (IDOK == dlg.DoModal()) {
        CString cstrPath = dlg.GetPathName();
        CT2CA pszConvertedAnsiString(cstrPath);
        string strPath(pszConvertedAnsiString);

        // loadImg = imread(strPath, IMREAD_UNCHANGED);
		loadImg = imread(strPath);
    }

    if (loadImg.empty()) {
        return FALSE;
    }
    else {
        SetImage(loadImg);
    }

    Invalidate(FALSE);

    return TRUE;
}

BOOL CTMatView::SaveImageFile() {
    cv::Mat image = m_pViewer->GetImage().clone();
    if (image.empty()) {
        MessageBox(_T("Not have image"));
        return FALSE;
    }

    CString szFilter = _T("Image (*.BMP, *.PNG, *.JPG) | *.BMP;*.PNG;*.JPG;*.jpg | All Files(*.*)|*.*||");
    CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, szFilter);

    if (IDOK == dlg.DoModal()) {
        CString cstrPath = dlg.GetPathName();
        if (-1 == cstrPath.ReverseFind('.')) {
            cstrPath += ".jpg";
        }
        CT2CA pszConvertedAnsiString(cstrPath);
        string strPath(pszConvertedAnsiString);

        imwrite(strPath, image);
    }

    return TRUE;
}


/////////////////////////////////////////////   Message


int CTMatView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_brush.CreateSolidBrush(COLOR_MENU);

    return 0;
}

void CTMatView::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CWnd::OnShowWindow(bShow, nStatus);

    if (bShow) {
        Invalidate(FALSE);
    }
}

void CTMatView::OnPaint()
{
    CPaintDC dc(this);
    CRect rectClient;
    GetClientRect(rectClient);

    CMemDC		memDC(dc, this);
    CDC&        pDC = memDC.GetDC();

    // background
    CRect rect = rectClient;
    pDC.FillSolidRect(rect, COLOR_BACKGROUND);

    // menu
    rect = CRect(0, 0, m_rect[eRECT_MENU].Width(), m_rect[eRECT_MENU].Height());
    pDC.FillSolidRect(rect, COLOR_MENU);
    
    // zoom
    CString str;
    str.Format(_T("%.1lf"), m_pViewer->GetZoomRate());
    pDC.Rectangle(m_rect[eRECT_ZOOM_RATE]);
    pDC.SetTextColor(RGB(0, 0, 0));
    pDC.SetBkColor(RGB(255, 255, 255));
    pDC.DrawText(str, m_rect[eRECT_ZOOM_RATE], DT_CENTER | DT_TABSTOP | DT_VCENTER | DT_SINGLELINE);
    
    // Coordtrans
    auto ptImage = m_pViewer->GetImagePts();
    auto ptView = m_pViewer->GetViewPts();
    auto imgColor = m_pViewer->GetImageColor();

    str.Format(_T("Image [%d, %d] / View [%d, %d] / Color [%.0lf %.0lf %.0lf]"), ptImage.x, ptImage.y, ptView.x, ptView.y, imgColor[0], imgColor[1], imgColor[2]);
    pDC.Rectangle(m_rect[eRECT_COORD]);
    pDC.SetTextColor(RGB(0, 0, 0));
    pDC.SetBkColor(RGB(255, 255, 255));
    pDC.DrawText(str, m_rect[eRECT_COORD] + CRect(-5, 0, 0, 0), DT_LEFT | DT_TABSTOP | DT_VCENTER | DT_SINGLELINE);

    return;
}

BOOL CTMatView::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
        case eBTN_SAVE: {
            SaveImageFile();
            break;
        }
        case eBTN_LOAD: {
            LoadImageFile();
            break;
        }
        case eBTN_FIT: {
            m_pViewer->FitImage();
            //FitImage();
            break;
        }
        case IDC_CHECK_BOX: {
            Invalidate(FALSE);
            break;
        }
    }
    return CWnd::OnCommand(wParam, lParam);
}

BOOL CTMatView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    CPoint ptClient = pt;
    ScreenToClient(&ptClient);

    if (PtInRect(m_rect[eRECT_VIEW], ptClient)) {
        InvalidateRect(m_rect[eRECT_ZOOM_RATE]);
        InvalidateRect(m_rect[eRECT_COORD]);
    }
  
    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}


HBRUSH CTMatView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
    
    if (pWnd->GetDlgCtrlID() == IDC_CHECK_BOX) {
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkMode(TRANSPARENT);

        hbr = (HBRUSH)m_brush;
    }

    return hbr;
}

