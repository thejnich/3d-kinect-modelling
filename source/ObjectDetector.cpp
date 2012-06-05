/*
 * Detects objects in scene based on user entered markers
 * Based on OpenCV watershed.cpp example
 */

#include "ObjectDetector.h"

ObjectDetector::ObjectDetector() :
    prevPt(-1, 1)
{
}

ObjectDetector::~ObjectDetector()
{
}

void ObjectDetector::init(std::vector<uint8_t> &rgb, int width, int height)
{
	// create matrix from rgb data
   /* img = cv::Mat(height, width, CV_8UC3, rgb.data());
    if (img.empty())
    {
        return;
    }
*/
	 // simple way to initialize markerMask to the correct type of matrix
   // cv::cvtColor(img, markerMask, CV_BGR2GRAY);

	 // zero markerMask
    //markerMask = cv::Scalar::all(0);

	// generate random colors for objects found, not usually more than MAX_COLOR are found
	for(int i = 0; i < MAX_COLOR; i++ )
    {
        int b = cv::theRNG().uniform(0, 255);
        int g = cv::theRNG().uniform(0, 255);
        int r = cv::theRNG().uniform(0, 255);
        colorTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
    }

}

void ObjectDetector::clearMarkers() {
	//markerMask = cv::Scalar::all(0);
}

void ObjectDetector::startMarkingRegion(int x, int y)
{
    prevPt = cv::Point(x, y);
}

void ObjectDetector::addMarkerToCurrentRegion(int x, int y)
{
   // cv::Point pt(x, y);
   // if( prevPt.x < 0 )
   // {
   //     prevPt = pt;
   // }
   // cv::line( markerMask, prevPt, pt, cv::Scalar::all(255), 5, 8, 0 );
   // prevPt = pt;
}

void ObjectDetector::stopMarkingRegion()
{
    prevPt = cv::Point(-1, -1);
}

void ObjectDetector::detect(int& n, std::vector<int>& objects, std::vector<boundRect>& objBound, std::vector<uint8_t>& map, std::vector<uint8_t>& imgGray, std::vector<uint8_t>& imgBW)
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

	/* running the watershed detection based on the image and the markers */
//	watershed(img, markers);
//
//	/* retrieve the objects */
//	n = compCount;
//	objects = std::vector<int>(markers.cols * markers.rows, 0);
//	/*
//	for (int i = 0; i < compCount; ++i) {
//		boundRect b = {0,640,0,480}; // imax, imin, jmax, jmin
//		objBound.push_back(b);
//	}
//	*/
//	cv::Mat wshed(markers.size(), CV_8UC3);
//	for(int j = 0; j < markers.rows; j++ )
//	{
//		for(int i = 0; i < markers.cols; i++ )
//		{
//			int idx = markers.at<int>(j,i);
//
//
//			objects.at(j * markers.cols + i) = idx;
//			if( idx == -1 )
//				wshed.at< cv::Vec3b >(j,i) = cv::Vec3b(255,255,255);
//			else if( idx <= 0 || idx > compCount )
//				wshed.at< cv::Vec3b >(j,i) = cv::Vec3b(0,0,0);
//			else {
//				wshed.at< cv::Vec3b >(j,i) = cv::Vec3b(100,100,100);//colorTab[(idx - 1)%MAX_COLOR];
//				/*
//				if(i > objBound[idx-1].imax)
//					objBound[idx-1].imax = i;
//				else if(i < objBound[idx-1].imin)
//					objBound[idx-1].imin = i;
//
//				if(j > objBound[idx-1].jmax)
//					objBound[idx-1].jmax = j;
//				else if(j < objBound[idx-1].jmin)
//					objBound[idx-1].jmin = j;
//				*/
//			}
//		}
//	}

	// copy the data back to map, for use by caller
	//std::copy(wshed.data, wshed.data + (wshed.size().width * wshed.size().height * 3), map.begin());
	std::copy(markers.data, markers.data + (markers.size().width * markers.size().height * 3), map.begin());
}
