#pragma once


typedef struct RESULT_DATA {
	int			angle;
	double		score;
	cv::Point	pt;
	cv::Rect	rect;

	cv::Mat		mark;
} RESULT_DATA;


class CDetector
{
public:
	CDetector();
	~CDetector();

	BOOL Detect(cv::Mat src, cv::Mat mark, cv::Mat& dst, double scale = 0.5);

	int m_iCanny1;
	int m_iCanny2;
	int m_iRotate;

private:
	void MakeMarks(const cv::Mat& mark);

	cv::Mat PreProcess(const cv::Mat& image, double scale);
	cv::Mat CannyImage(const cv::Mat& image, int iCanny1, int iCanny2);
	cv::Mat	RotateImage(const cv::Mat& image, int iAngle);

private:
	std::vector<cv::Mat> m_marks;
};

