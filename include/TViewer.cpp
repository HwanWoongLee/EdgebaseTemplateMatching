#include "pch.h"
#include "TMatView.h"
#include "TViewer.h"
#include "TMatView.h"

IMPLEMENT_DYNCREATE(TViewer, CFrameWnd)

TViewer::TViewer(CWnd* pParent)
{
    m_bLButton  = false;
    m_dZoom     = 1.0;
    m_pParent   = (CTMatView*)pParent;
}
TViewer::TViewer()
{
}
TViewer::~TViewer()
{
}
void TViewer::SetDrawImage(const cv::Mat& image) {
    m_drawImage = image.clone();
    m_showNavImage = m_orgImage.clone();

	if (image.cols > 8) {
		cv::resize(m_drawImage, m_drawImage, cv::Size(GDI_WIDTHBYTES(m_drawImage.cols * 8), m_drawImage.rows));     // 영상 가로길이는 4바이트의 배수여야한다...
		cv::resize(m_showNavImage, m_showNavImage, cv::Size(GDI_WIDTHBYTES(m_showNavImage.cols * 8), m_showNavImage.rows));
	}

    //BITMAPINFO bitmapInfo;
    m_bitmapInfo.bmiHeader.biYPelsPerMeter	= 0;
    m_bitmapInfo.bmiHeader.biXPelsPerMeter	= 0;
    m_bitmapInfo.bmiHeader.biBitCount		= 24;
    m_bitmapInfo.bmiHeader.biWidth			= m_drawImage.cols;
    m_bitmapInfo.bmiHeader.biHeight			= m_drawImage.rows;
    m_bitmapInfo.bmiHeader.biPlanes			= 1;
    m_bitmapInfo.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    m_bitmapInfo.bmiHeader.biCompression	= BI_RGB;
    m_bitmapInfo.bmiHeader.biClrImportant	= 0;
    m_bitmapInfo.bmiHeader.biSizeImage		= 0;

    if (m_drawImage.channels() == 3) {

    }
    else if (m_drawImage.channels() == 1) {
        cvtColor(m_drawImage, m_drawImage, cv::COLOR_GRAY2RGB);
    }
    else if (m_drawImage.channels() == 4) {
    }

    if (m_showNavImage.channels() == 3) {

    }
    else if (m_showNavImage.channels() == 1) {
        cvtColor(m_showNavImage, m_showNavImage, cv::COLOR_GRAY2RGB);
    }
    else if (m_showNavImage.channels() == 4) {

    }

    flip(m_drawImage, m_drawImage, 0);
    flip(m_showNavImage, m_showNavImage, 0);

    return;
}

void TViewer::DrawView(const cv::Mat& image) {
    m_orgImage = image.clone();
    if (m_orgImage.channels() == 4)
        cvtColor(m_orgImage, m_orgImage, cv::COLOR_BGRA2BGR);

    SetDrawImage(m_orgImage);

    m_rectZoom = CRect();
    m_rectView = CRect();

	Invalidate(FALSE);
}

cv::Mat TViewer::GetImage() {
    return m_orgImage.clone();
}

void TViewer::FitImage() {
    m_dZoom     = 1.0;
    m_ptOffset  = CPoint(0, 0);
    m_rectZoom  = m_rectView;

    Invalidate(FALSE);
}

cv::Point TViewer::ClientToImage(CPoint clientPt, CRect clientRect, cv::Mat image) {
    cv::Point ptImage(-1, -1);
    
    if (clientRect.IsRectEmpty() || image.empty())
        return ptImage;

    double dRateToImage_x = (double)image.cols / (double)clientRect.Width();
    double dRateToImage_y = (double)image.rows / (double)clientRect.Height();
    double dRateToImage = dRateToImage_x > dRateToImage_y ? dRateToImage_x : dRateToImage_y;

    clientPt -= clientRect.TopLeft();

    double dx = clientPt.x * dRateToImage;
    double dy = clientPt.y * dRateToImage;

    //dx = floor(dx + .5);
    //dy = floor(dy + .5);

    if (dx > m_orgImage.cols - 1) dx = m_orgImage.cols - 1;
    else if (dx < 0) dx = 0;

    if (dy > m_orgImage.rows - 1) dy = m_orgImage.rows - 1;
    else if (dy < 0) dy = 0;

    ptImage = cv::Point(dx, dy);

    return ptImage;
}


void TViewer::CalcZoomRect(CPoint pt) {
    double dw = m_rectView.Width() / m_dZoom;
    double dh = m_rectView.Height() / m_dZoom;

    double dZoomRate = m_rectZoom.Width() / dw;

    double dx = pt.x - (((double)pt.x - (double)m_rectZoom.left) / dZoomRate);
    double dy = pt.y - (((double)pt.y - (double)m_rectZoom.top)  / dZoomRate);

    dx += m_ptOffset.x / m_dZoom;
    dy += m_ptOffset.y / m_dZoom;

    if (dx < m_rectView.left) dx = m_rectView.left; 
    else if (dx + dw > m_rectView.right) dx = m_rectView.right - dw;
    
    if (dy < m_rectView.top) dy = m_rectView.top; 
    else if (dy + dh > m_rectView.bottom) dy = m_rectView.bottom - dh;
    
    m_rectZoom = CRect(round(dx), round(dy), round(dx + dw), round(dy + dh));

    m_ptOffset.x = 0;
    m_ptOffset.y = 0;
}


void TViewer::Zooming(short zDelta) {
    if (zDelta > 0) {           // zoom in
        if (MAX_ZOOM > m_dZoom) {
            m_dZoom += m_dZoom * RATE_ZOOMING;
            CalcZoomRect(m_ptZoom);
        }
        else
            m_dZoom = MAX_ZOOM;
    }
    else {                      // zoom out
        if (MIN_ZOOM < m_dZoom) {
            m_dZoom -= m_dZoom * RATE_ZOOMING;
            CalcZoomRect(m_ptZoom);
        }
        if (MIN_ZOOM > m_dZoom) {
            m_dZoom = MIN_ZOOM;
            FitImage();
        }
    }
}


BEGIN_MESSAGE_MAP(TViewer, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
END_MESSAGE_MAP()


void TViewer::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (!m_bLButton) {
        m_bLButton = true;
        m_ptLBStart = point + m_ptOffset;
    }

	CFrameWnd::OnLButtonDown(nFlags, point);
}


void TViewer::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (m_bLButton) {
        m_bLButton = false;
        CalcZoomRect(m_ptZoom);
        Invalidate(FALSE);
    }
	CFrameWnd::OnLButtonUp(nFlags, point);
}


void TViewer::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_orgImage.empty()) {
        if (m_bLButton)
            m_ptOffset = m_ptLBStart - point;

        m_ptView = point;
        CPoint pt = m_ptView - m_rectView.TopLeft();
        m_ptZoom = CPoint(floor((pt.x / m_dZoom) + .5), floor((pt.y / m_dZoom) + .5));
        m_ptZoom += CPoint(floor((m_ptOffset.x / m_dZoom) + .5), floor((m_ptOffset.y / m_dZoom) + .5));
        m_ptZoom += m_rectZoom.TopLeft();

        m_ptImage = ClientToImage(m_ptZoom, m_rectView, m_orgImage);

        if (m_ptImage != cv::Point(-1, -1)) {
            if (m_orgImage.channels() == 1) {
                m_imgColor = cv::Scalar(m_orgImage.at<uchar>(m_ptImage));
            }
            else if (m_orgImage.channels() == 3) {
                m_imgColor = cv::Scalar(m_orgImage.at<cv::Vec3b>(m_ptImage)[0],
                                        m_orgImage.at<cv::Vec3b>(m_ptImage)[1],
                                        m_orgImage.at<cv::Vec3b>(m_ptImage)[2]);
            }
        }

        Invalidate(FALSE);
        m_pParent->UpdateTool();
    }
	CFrameWnd::OnMouseMove(nFlags, point);
}


BOOL TViewer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (!m_orgImage.empty()) {
        CPoint ptClient = pt;
        ScreenToClient(&ptClient);
    
        if (PtInRect(m_clientRect, ptClient)) {
            Zooming(zDelta);
            Invalidate(FALSE);
        }
    }
	return CFrameWnd::OnMouseWheel(nFlags, zDelta, pt);
}


void TViewer::OnPaint()
{
	CPaintDC dc(this);
	GetClientRect(m_clientRect);

	CMemDC		memDC(dc, this);
	CDC&        pDC = memDC.GetDC();

	// background
	pDC.FillSolidRect(m_clientRect, COLOR_BACKGROUND);

    // draw image
	if (!m_orgImage.empty()) {
        // image to bitmap
        //cv::Mat drawImage = m_orgImage.clone();
        //
        //cv::resize(drawImage, drawImage, cv::Size(GDI_WIDTHBYTES(drawImage.cols * 8), drawImage.rows));     // 영상 가로길이는 4바이트의 배수여야한다...
        //
        //BITMAPINFO bitmapInfo;
        //bitmapInfo.bmiHeader.biYPelsPerMeter    = 0;
        //bitmapInfo.bmiHeader.biXPelsPerMeter    = 0;
        //bitmapInfo.bmiHeader.biBitCount         = 24;
        //bitmapInfo.bmiHeader.biWidth            = drawImage.cols;
        //bitmapInfo.bmiHeader.biHeight           = drawImage.rows;
        //bitmapInfo.bmiHeader.biPlanes           = 1;
        //bitmapInfo.bmiHeader.biSize             = sizeof(BITMAPINFOHEADER);
        //bitmapInfo.bmiHeader.biCompression      = BI_RGB;
        //bitmapInfo.bmiHeader.biClrImportant     = 0;
        //bitmapInfo.bmiHeader.biSizeImage        = 0;
        //
        //if (drawImage.channels() == 3) {
        //
        //}
        //else if (drawImage.channels() == 1) {
        //    cvtColor(drawImage, drawImage, cv::COLOR_GRAY2RGB);
        //}
        //else if (drawImage.channels() == 4) {
        //}
        //
        //flip(drawImage, drawImage, 0);

        double image_w      = m_drawImage.cols;
        double image_h      = m_drawImage.rows;
        double rect_w       = m_clientRect.Width();
        double rect_h       = m_clientRect.Height();

        double dRectRate    = rect_w / rect_h;
        double dImageRate   = image_w / image_h;
        
        // resize to fit
        int dx = 0, dy = 0, dw = 0, dh = 0;  // 그리는 좌표
        if (dRectRate > dImageRate) {
            dh = rect_h;
            dw = dh * image_w / image_h;
            dx = (rect_w - dw) / 2;
            dy = 0;
        }
        else {
            dw = rect_w;
            dh = dw * image_h / image_w;
            dy = (rect_h - dh) / 2;
            dx = 0;
        }

        ::SetStretchBltMode(pDC.m_hDC, COLORONCOLOR);		//모드 설정(안해주면 영상 축소 시 깨짐 현상)

        m_rectView = CRect(dx, dy, dx + dw, dy + dh);

        if (m_rectZoom.IsRectEmpty())
            m_rectZoom = m_rectView;

        if (m_dZoom > 1.0) {
            double x = m_rectZoom.left;
            double y = m_rectZoom.top;
        
            dx = dx - ((x - dx) * m_dZoom);
            dy = dy - ((y - dy) * m_dZoom);
            dw = m_dZoom * dw;
            dh = m_dZoom * dh;
        }

        // offset
        dx -= m_ptOffset.x;
        dy -= m_ptOffset.y;

        // draw bitmap
        ::StretchDIBits(pDC.m_hDC,			        //출력대상 핸들
            dx, dy, dw, dh,				            //출력대상 좌표
            0, 0, m_drawImage.cols, m_drawImage.rows,	//원본의 좌표
            m_drawImage.data,							//데이터 시작 주소
            &m_bitmapInfo,							//BITMAPINFO 구조체 시작 주소
            DIB_RGB_COLORS,
            SRCCOPY);

        // draw navigation view
        if (m_pParent->GetNaviCheck()) {
            DisplayNavi(pDC.m_hDC, m_bitmapInfo);
        }
	}
}


void TViewer::DisplayNavi(HDC& hdc, BITMAPINFO& bitmapInfo) {
    if (m_orgImage.empty())
        return;

    auto ClientToNavi = [&](CPoint ptClient, CRect rectNavi)->CPoint {
        double dW = (double)rectNavi.Width() / (double)m_rectView.Width();
        double dH = (double)rectNavi.Height() / (double)m_rectView.Height();
        double dRate = dW > dH ? dW : dH;

        ptClient -= m_rectView.TopLeft();

        double dx = ptClient.x * dRate;
        double dy = ptClient.y * dRate;

        CPoint ptNavi = CPoint(dx, dy);

        ptNavi += rectNavi.TopLeft();

        //if (dx > m_orgImage.cols - 1) dx = m_orgImage.cols - 1;
        //else if (dx < 0) dx = 0;
        //
        //if (dy > m_orgImage.rows - 1) dy = m_orgImage.rows - 1;
        //else if (dy < 0) dy = 0;
        //
        //ptImage = cv::Point2d(dx, dy);

        return ptNavi;
    };

    CRect rect;
    GetWindowRect(rect);
    ScreenToClient(rect);

    ::StretchDIBits(hdc,
        rect.right - m_rectView.Width() * 0.3,
        rect.bottom - m_rectView.Height() * 0.3,
        m_rectView.Width() * 0.3,
        m_rectView.Height() * 0.3,
        0, 0, m_showNavImage.cols, m_showNavImage.rows,
        m_showNavImage.data,
        &bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY);

    CRect rectNavi(rect.right - m_rectView.Width() * 0.3,
        rect.bottom - m_rectView.Height() * 0.3,
        rect.right,
        rect.bottom);

    CPoint lt = ClientToNavi(m_rectZoom.TopLeft()       + CPoint(m_ptOffset.x / m_dZoom, m_ptOffset.y / m_dZoom), rectNavi);
    CPoint br = ClientToNavi(m_rectZoom.BottomRight()   + CPoint(m_ptOffset.x / m_dZoom, m_ptOffset.y / m_dZoom), rectNavi);
    CPoint pt = ClientToNavi(m_ptZoom, rectNavi);

    HBRUSH brush    = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN pen        = (HPEN)CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
    HPEN oldPen     = (HPEN)SelectObject(hdc, pen);

    ::Rectangle(hdc, lt.x, lt.y, br.x, br.y);
    ::Ellipse(hdc, pt.x - 1, pt.y - 1, pt.x + 1, pt.y + 1);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}