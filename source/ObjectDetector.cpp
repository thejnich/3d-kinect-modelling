#include "ObjectDetector.h"
#include <QDebug>
#include "opencv2/highgui/highgui.hpp"

ObjectDetector::ObjectDetector() :
    prevPt(-1, 1)
{
}

ObjectDetector::~ObjectDetector()
{
}

void ObjectDetector::init(std::vector<uint8_t> &rgb, int width, int height)
{
    img0 = cv::Mat(height, width, CV_8UC3, rgb.data());
    if (img0.empty())
    {
        return;
    }
    img0.copyTo(img);
    cv::cvtColor(img, markerMask, CV_BGR2GRAY);
    cv::cvtColor(markerMask, imgGray, CV_GRAY2BGR);
    markerMask = cv::Scalar::all(0);
}

void ObjectDetector::startMarkingRegion(int x, int y)
{
    prevPt = cv::Point(x, y);
}

void ObjectDetector::addMarkerToCurrentRegion(int x, int y)
{
    cv::Point pt(x, y);
    if( prevPt.x < 0 )
    {
        prevPt = pt;
    }
    cv::line( markerMask, prevPt, pt, cv::Scalar::all(255), 5, 8, 0 );
    cv::line( img, prevPt, pt, cv::Scalar::all(255), 5, 8, 0 );
    prevPt = pt;
}

void ObjectDetector::stopMarkingRegion()
{
    prevPt = cv::Point(-1, -1);
}

void ObjectDetector::detect(int& n, std::vector<int>& objects, std::vector<boundRect>& objBound, std::vector<uint8_t>& map)
{
    /* find contours based on the markers */
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(markerMask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    if(contours.empty())
        return;

    /* draw the contours in the markers map */
    cv::Mat markers(markerMask.size(), CV_32S);
    markers = cv::Scalar::all(0);
    int idx = 0, compCount = 0;
    for( ; idx >= 0; idx = hierarchy[idx][0], compCount++ )
        cv::drawContours(markers, contours, idx, cv::Scalar::all(compCount+1), -1, 8, hierarchy, INT_MAX);

    if( compCount == 0 )
        return;

    std::vector<cv::Vec3b> colorTab;
    for(int i = 0; i < compCount; i++ )
    {
        int b = cv::theRNG().uniform(0, 255);
        int g = cv::theRNG().uniform(0, 255);
        int r = cv::theRNG().uniform(0, 255);
        colorTab.push_back(cv::Vec3b((uchar)b, (uchar)g, (uchar)r));
    }

    /* running the watershed detection based on the image and the markers */
    watershed(img0, markers);

    /* retrieve the objects */
    n = compCount;
    objects = std::vector<int>(markers.cols * markers.rows, 0);
	for (int i = 0; i < compCount; ++i) {
		boundRect b = {0,640,0,480}; // imax, imin, jmax, jmin
		objBound.push_back(b);
	}
    cv::Mat wshed(markers.size(), CV_8UC3);
	 printf("marker rows: %d\nmarker cols: %d\n", markers.rows, markers.cols);
	 for(int j = 0; j < markers.rows; j++ )
	 {
		 for(int i = 0; i < markers.cols; i++ )
		 {
			 int idx = markers.at<int>(j,i);


			 objects.at(j * markers.cols + i) = idx;
			 if( idx == -1 )
				 wshed.at< cv::Vec3b >(j,i) = cv::Vec3b(255,255,255);
			 else if( idx <= 0 || idx > compCount )
				 wshed.at< cv::Vec3b >(j,i) = cv::Vec3b(0,0,0);
			 else {
				 wshed.at< cv::Vec3b >(j,i) = colorTab[idx - 1];

				 if(i > objBound[idx-1].imax)
					 objBound[idx-1].imax = i;
				 else if(i < objBound[idx-1].imin)
					 objBound[idx-1].imin = i;

				 if(j > objBound[idx-1].jmax)
					 objBound[idx-1].jmax = j;
				 else if(j < objBound[idx-1].jmin)
					 objBound[idx-1].jmin = j;
			 }
		  }
    }
    wshed = wshed*0.5 + imgGray*0.5;
    std::copy(wshed.data, wshed.data + (wshed.size().width * wshed.size().height * 3), map.begin());
}
