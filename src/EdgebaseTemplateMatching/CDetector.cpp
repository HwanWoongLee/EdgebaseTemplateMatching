#include "pch.h"
#include "CDetector.h"



CDetector::CDetector() {

}
CDetector::~CDetector() {

}


BOOL CDetector::Detect(cv::Mat src, cv::Mat mark, double scale) {
	// image process
	cv::Mat edge;
	edge = PreProcess(src, scale);
	mark = PreProcess(mark, scale);

	if (edge.empty() || mark.empty())
		return FALSE;

	// create diatate image
	cv::Mat image_dil = cv::Mat::zeros(edge.size() + (mark.size() * 2), CV_8UC1);

#pragma omp parallel for
	for (int y = 0; y < edge.rows; ++y) {
		auto data = edge.ptr<uchar>(y);
		auto data2 = image_dil.ptr<uchar>(y + mark.rows);

		for (int x = 0; x < edge.cols; ++x) {
			if (data[x] != 0)
				data2[x + mark.cols] = data[x];
		}
	}

	// create mark 0~359
	MakeMarks(mark);

	// matching


	return TRUE;
}


cv::Mat CDetector::PreProcess(const cv::Mat& image, double scale) {
	// gray scale
	cv::Mat gray;
	if (image.channels() == 3)
		cv::cvtColor(gray, gray, cv::COLOR_BGR2GRAY);

	// resize
	cv::resize(gray, gray, cv::Size(image.cols * scale, image.rows * scale));

	// edge detect
	cv::Mat edge = CannyImage(gray, 220, 255);

	return edge;
}


void CDetector::MakeMarks(const cv::Mat& mark) {
	m_marks.clear();
	m_marks.resize(360);

#pragma omp parallel for
	for (int angle = 0; angle < 360; ++angle) {
		cv::Mat rotImage = RotateImage(mark, angle);
		cv::threshold(rotImage, rotImage, 120, 255, cv::THRESH_BINARY);

		m_marks[angle] = rotImage.clone();
	}
}


cv::Mat	CDetector::CannyImage(const cv::Mat& image, int iCanny1, int iCanny2) {
	cv::Mat canny;
	if (image.channels() == 3)
		cv::cvtColor(image, canny, cv::COLOR_BGR2GRAY);
	else
		canny = image.clone();

	cv::GaussianBlur(canny, canny, cv::Size(3, 3), 0);
	cv::Canny(canny, canny, iCanny1, iCanny2);

	return canny;
}


cv::Mat	CDetector::RotateImage(const cv::Mat& image, int iAngle) {
	cv::Mat rotateImage;
	cv::Mat R = cv::getRotationMatrix2D(cv::Point2f(image.cols / 2, image.rows / 2), (double)iAngle, 1);
	cv::warpAffine(image, rotateImage, R, image.size(), cv::INTER_LINEAR, cv::BORDER_REPLICATE, cv::Scalar::all(0));

	return rotateImage;
}
