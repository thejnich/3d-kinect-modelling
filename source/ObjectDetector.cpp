/*
 * Detects objects in scene based on user entered markers
 * Based on OpenCV watershed.cpp example
 */

#include "ObjectDetector.h"

ObjectDetector::ObjectDetector()
{
}

ObjectDetector::~ObjectDetector()
{
}

void ObjectDetector::detect(std::vector<uint8_t>& map, std::vector<uint8_t>& imgGray, std::vector<uint8_t>& imgBW)
{
	// construct a cv::Mat from rgb data passed in 
	img = cv::Mat(480, 640, CV_8UC3, map.data());
	cv::Mat grayMat;
	// convert rgb image to gray scale
	cv::cvtColor(img, grayMat, CV_BGR2GRAY);

	/*
	 * create binary black and white image by filtering grayscale image
	 * pixels in range 1-175 are set to 255(white), everything else black(0)
	 * this binary image is necessary for finding countours
	 */
	cv::Mat bw (grayMat.size(), CV_8UC3);
	cv::inRange(grayMat, 1, 175, bw);

	/* copy the grayscale and bw image to vectors passed in, for display in render view */ 
	std::copy(bw.data, bw.data + (bw.size().width * bw.size().height), imgBW.begin());
	std::copy(grayMat.data, grayMat.data + (grayMat.size().width * grayMat.size().height), imgGray.begin());

	/* find contours based on the markers */
	std::vector< std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(bw, contours, hierarchy,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	if(contours.empty())
		return;

	/* draw the contours in the markers map */
	cv::Mat markers(img.size(), CV_8UC3);
	markers = cv::Scalar::all(0);
	int idx = 0, compCount = 0;
	int lineThickness = 1;//CV_FILLED;
	int lineType = 8; // 8-connected line (see CV docs for more details)
	cv::Scalar color( 255, 0,0 );
	/* iterate through top level contours, and draw to markers */ 
	for( ; idx >= 0; idx = hierarchy[idx][0], compCount++ ) {
		cv::drawContours(markers, contours, idx, color, lineThickness, lineType, hierarchy, INT_MAX);
	}
	//printf("%d\n", compCount);
	if( compCount == 0 )
		return;

	// copy the data back to map, for use by caller
	std::copy(markers.data, markers.data + (markers.size().width * markers.size().height * 3), map.begin());
}
