#pragma once

#define IDC_VIEWER_SHOW 4001
#define IDC_VIEWER_MARK 4002


class CTMatView;
class CDetector;
class CEdgebaseTemplateMatchingDlg : public CDialogEx
{
// 생성입니다.
public:
	CEdgebaseTemplateMatchingDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDGEBASETEMPLATEMATCHING_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


private:
	CTMatView* m_pViewShow;
	CTMatView* m_pViewMark;

	CDetector* m_pDetector;


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLoadMark();
	afx_msg void OnBnClickedButtonMatching();
	afx_msg void OnBnClickedButtonLoadImage();
};
