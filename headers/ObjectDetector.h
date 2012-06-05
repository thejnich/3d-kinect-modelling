#ifndef _OBJECT_DETECTOR_H
#define _OBJECT_DETECTOR_H

#include <stdint.h>
#include <vector>
#include <opencv/cv.h>
#include "Primitives.h"
#include <QDebug>
#include "opencv2/highgui/highgui.hpp"

class ObjectDetector
{
public:
	ObjectDetector();
	~ObjectDetector();

	void detect(std::vector<uint8_t>& map, std::vector<uint8_t>& imgGray, std::vector<uint8_t>& imgBW);

private:
	cv::Mat   markerMask;
	cv::Mat   img;
	std::vector<cv::Vec3b> colorTab;
};

#endif // _OBJECT_DETECTOR_H
