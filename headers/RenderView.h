/*
 *  RenderView.h
 *  3dModellingKinect
 *
 *  Created by Jeff Nicholson on 11-11-03.
 *  Copyright 2011 University of Calgary. All rights reserved.
 *
 */

#ifndef _RENDER_VIEW_H
#define _RENDER_VIEW_H

#include <QtOpenGL>
#include "ObjWriter.h"
#include "MyFreenectDevice.h"
#include "ObjectDetector.h"
#include "Primitives.h"

struct marker {
	vector<QPoint> points;
	void clear(){points.clear();}
};

typedef enum {
	LIVE,
	PAUSED,
	DETECTED,
	SELECTED,
	RENDERED
} APP_STATE;

class RenderView : public QGLWidget {

	Q_OBJECT

public:
	// constants
	const static float REFRESH_RATE = 1000/30;
	const static int WIDGET_WIDTH = 1280;		// width in pixels of widget
	const static int WIDGET_HEIGHT = 640;		// height in pixels of widget
	const static int RES_WIDTH = 640;				// width in pixels of rgb and depth camera resolution
	const static int RES_HEIGHT = 480;			// height in pixels of rgb and depth camera resolution
	const static int RES_PIXELS = 640*480;		// total number of pixels in rgb and depth camera resolution
	const static int CACHE_SIZE = 10;

	RenderView();
	~RenderView();
	void setStatusBar(QStatusBar* status);

	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	bool ctrlPressed;
	void clearMarkerList();
	void detect();
	void renderMesh();
	void toggleTexDisplay();
	void toggleColorDisplay();
	void increaseTilt();
	void decreaseTilt();
	void resetTilt();

public slots:
	void pause(bool);
	void exportObj();
	void exportPly();
	void exportXyz();
	void exportPcd();
	void setRearCutoff(int);
	void setFrontCutoff(int);
	void ctrlDown();
	void ctrlUp();
	void dump();

signals:
	void pausePlease();

protected:
	void initializeGL();
	void paintGL();
	QSize sizeHint();
	QSize minimumSizeHint();
	void resizeGL(int w, int h);
   void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void setXRotation(int);
	void setYRotation(int);
	void setZRotation(int);
	void setZoom(int);
	void updateStatusBar(APP_STATE st);

	MyFreenectDevice* m_device;
	freenect_video_format requested_format;
	double freenect_angle;

private:	
	Freenect::Freenect freenect;
	GLuint gl_depth_tex;
	GLuint gl_rgb_tex;
	bool displayTex;
	QPoint lastPos;
	int xRot;
	int yRot;
	int zRot;
	int zoom;
	int rearCutoff;
	int frontCutoff;
	double tiltAngle;
	QTimer *timer;

	std::vector<uint16_t> depth;
	std::vector< std::vector<uint16_t> > depth_cache;
	std::vector<uint8_t> rgb;
	std::vector<uint8_t> depth_rgb;
	uint8_t saveColor[3];
	bool displayColor;

	vector<marker> markerList;
	marker currentMarker;
	void renderMarkerList();
	void renderMarker(marker*);
	float xToWorldCoord(int x);
	float yToWorldCoord(int y);
	void worldCoordToPixelCoord(float x, float y, int& i, int& j);

	ObjectDetector detector;
	int nobjects;
	int selectedObject;
	std::vector<int> objects;
	
	std::vector<boundRect> objBound;
	std::vector<uint16_t> grid_depth;
	std::vector<vertex> vertexList;
	std::vector<tri_face> faceList;
	
	uint16_t findMaxDepthOfObject(int object);
	uint16_t findMinDepthOfObject(int object);
	void flattenObjectsBoundingArea(int object);
	void calcAvgOfSquare(int iTopLeft, int jTopLeft, int h, int w, boundRect br);
	void setDepthOrAvg(int i, int j, uint16_t depth);

	void renderSurface();
	void renderPointCloud();
	void renderRGB_DepthColor_Textures();

	void getDepthAndUpdateCache();

	float xWindowBound;

	QStatusBar* statusBar;
	APP_STATE state;
};

#endif /* _RENDER_VIEW_H */
