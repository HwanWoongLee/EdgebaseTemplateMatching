#pragma once


class CDetector
{
public:
	CDetector();
	~CDetector();

	BOOL Detect(cv::Mat src, cv::Mat mark, double scale = 0.5);


private:
	void MakeMarks(const cv::Mat& mark);

	cv::Mat PreProcess(const cv::Mat& image, double scale);
	cv::Mat CannyImage(const cv::Mat& image, int iCanny1, int iCanny2);
	cv::Mat	RotateImage(const cv::Mat& image, int iAngle);

private:
	std::vector<cv::Mat> m_marks;
};

