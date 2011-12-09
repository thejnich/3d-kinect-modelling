#ifndef _OBJECT_DETECTOR_H
#define _OBJECT_DETECTOR_H

#include <stdint.h>
#include <vector>
#include <opencv/cv.h>

class ObjectDetector
{
public:
    ObjectDetector();
    ~ObjectDetector();

    void init(std::vector<uint8_t>& rgb, int width, int height);

    void startMarkingRegion(int x, int y);
    void addMarkerToCurrentRegion(int x, int y);
    void stopMarkingRegion();

    void detect(int& n, std::vector<int>& objects, std::vector<uint8_t>& map);

private:
    cv::Point prevPt;
    cv::Mat   markerMask;
    cv::Mat   img;
    cv::Mat   img0;
    cv::Mat   imgGray;
};

#endif // _OBJECT_DETECTOR_H
