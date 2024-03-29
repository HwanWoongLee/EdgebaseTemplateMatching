#include "pch.h"
#include "CDetector.h"



CDetector::CDetector() {
	m_iCanny1 = 100;
	m_iCanny2 = 200;
	m_iRotate = 0;
}
CDetector::~CDetector() {

}


BOOL CDetector::Detect(cv::Mat src, cv::Mat mark, cv::Mat& dst, double scale) {
	if (m_iRotate != 0) {
		cv::Mat M = cv::getRotationMatrix2D(cv::Point2f(src.cols / 2, src.rows / 2), m_iRotate, 1.0);
		cv::warpAffine(src, src, M, src.size());
	}

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
	RESULT_DATA result_data;
	result_data.score = 0;
	result_data.angle = 0;

#pragma omp parallel for
	for (int angle = -180; angle <= 180; ++angle) {
		int index = angle;
		if (index < 0)
			index += 360;

		cv::Mat mark_tilt = m_marks[index];

		cv::Mat temp;
		cv::matchTemplate(image_dil, mark_tilt, temp, cv::TM_CCOEFF_NORMED);

		double score;
		cv::Point pos;
		cv::minMaxLoc(temp, 0, &score, 0, &pos);
		
		if (score > result_data.score) {
			result_data.angle = angle;
			result_data.score = score;
			result_data.pt = pos - cv::Point(mark.cols, mark.rows);
			result_data.mark = mark_tilt.clone();
		}
	}
	if (result_data.score <= 0)
		return FALSE;

	result_data.score *= 100; 

	// draw result
	dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);
	cv::resize(dst, dst, edge.size());

	cv::rectangle(dst, cv::Rect(result_data.pt.x, result_data.pt.y, mark.cols, mark.rows), cv::Scalar(0, 255, 0));

	std::vector<cv::Point> pts;
	cv::findNonZero(result_data.mark, pts);

#pragma omp parallel for
	for (int i = 0; i < pts.size(); ++i) {
		auto loc = result_data.pt + pts[i];

		if (loc.x < 0 || loc.y < 0)
			continue;
		else if (loc.x >= dst.cols || loc.y >= dst.rows)
			continue;

		dst.at<cv::Vec3b>(loc)[0] = 0;
		dst.at<cv::Vec3b>(loc)[1] = 0;
		dst.at<cv::Vec3b>(loc)[2] = 255;
	}

	return TRUE;
}


cv::Mat CDetector::PreProcess(const cv::Mat& image, double scale) {
	// gray scale
	cv::Mat gray;
	if (image.channels() == 3)
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

	// resize
	cv::resize(gray, gray, cv::Size(image.cols * scale, image.rows * scale));

	cv::normalize(gray, gray, 0, 255, cv::NORM_MINMAX);

	// edge detect
	cv::Mat edge = CannyImage(gray, m_iCanny1, m_iCanny2);

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
