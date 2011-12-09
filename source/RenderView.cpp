/*
 *  RenderView.cpp
 *  3dModellingKinect
 *
 *  Created by Jeff Nicholson on 11-11-03.
 *  Copyright 2011 University of Calgary. All rights reserved.
 *
 */

#include <QDebug>
#include <QtGui>
#include <QtOpenGL>

#include "RenderView.h"

RenderView::RenderView()
{
	WIDGET_WIDTH = 1280;
	WIDGET_HEIGHT = 640;
	zoom = 3 * 120;

	m_device = &freenect.createDevice<MyFreenectDevice>(0);

	requested_format = FREENECT_VIDEO_RGB;

	m_device->startVideo();
	m_device->startDepth();
	
	m_device->setVideoFormat(requested_format);

	depth = std::vector<uint16_t>(640*480);
	rgb = std::vector<uint8_t>(640*480*4);
	depth_rgb = std::vector<uint8_t>(640*480*3);
	xRot = -30 * 16;
	yRot = -30 * 16;
	zRot = 0;

	ctrlPressed = false;
	markerList.clear();

	xWindowBound = 4.0f/3.0f-2.0f;
	
	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
	timer->start(REFRESH_RATE);

	nobjects = selectedObject = 0;
	saveColor[0] = 0;
	saveColor[1] = 0;
	saveColor[2] = 0;
}
RenderView::~RenderView() 
{
	// emit signal, rather than stopping video/depth directly because
	// if already paused, calling stopDepth/Video will generate exception
	emit pausePlease();
}

void RenderView::setStatusBar(QStatusBar* status)
{
	if (status)
	{
		statusBar = status;
		updateStatusBar(LIVE);
	}
}

// from Steven Longay, keeps the aspect ratio when resizing window
void RenderView::resizeGL(int w, int h)
{
	// Switch to the propper matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set drawing to take up the entire window
	glViewport(0, 0, w, h);
	
	float ratio;
	if ( w > h ) // In this case the w/h ratio is > 1
	{
		ratio = (float)w/(float)h;
		glOrtho(-ratio, ratio, -1, 1, -2, 2);
	}
	else        // In this case the h/w ratio is > 1
	{
		ratio = (float)h/(float)w;
		glOrtho(-1, 1, -ratio, ratio, -2, 2);
	}
	
	//Switch back to modelview matrix
	glMatrixMode(GL_MODELVIEW);
}


QSize RenderView::sizeHint() const
{
	return QSize(WIDGET_WIDTH, WIDGET_HEIGHT);
}


QSize RenderView::minimumSizeHint() const
{
	return QSize(WIDGET_WIDTH, WIDGET_HEIGHT);
}


void RenderView::initializeGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);

	glGenTextures(1, &gl_depth_tex);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &gl_rgb_tex);
	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 4.0f / 3.0f, 0.001, 1000);
	glMatrixMode(GL_MODELVIEW);
}

void RenderView::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	m_device->getDepth(depth);
	m_device->getRGB(rgb);
	m_device->getDepthRGB(depth_rgb);

//	std::vector<uint8_t> markers(640 * 480, 0);
//	for (int i = 0; i < 500; ++i)
//		markers[i * i] = 255;
	//detector.detect(depth_rgb);

	glPushMatrix();
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
	glTranslatef(0.0f, 0.0f, -(float)zoom / 120.0f);

	glPointSize(2.);
	glBegin(GL_POINTS);
	float x = -1., y = 1.;
	for (int i = 0; i < 640 * 480; i++) {
		
		if( (depth[i] > frontCutoff && depth[i] < rearCutoff && (selectedObject > 0 && objects[i] == selectedObject)) ||
			(depth[i] > frontCutoff && depth[i] < rearCutoff && (selectedObject == 0)) ) {
			glVertex3f(x, y, (float)depth[i] / 100.0f);
			glColor3uiv((GLuint*)&rgb[3 * (i+640*30 - 7)]);

			/* linear interpolation
			glVertex3f(x + (1.f/240.f/4.0f), y, 0.75 * (float)depth[i] / 100.0f + 0.25 * (float)depth[i+1] / 100.0f);
			glVertex3f(x + (22.f/240.f/4.0f), y, 0.5 * (float)depth[i] / 100.0f + 0.5 * (float)depth[i+1] / 100.0f);
			glVertex3f(x + (3.f/240.f/4.0f), y, 0.25 * (float)depth[i] / 100.0f + 0.75 * (float)depth[i+1] / 100.0f);
			*/
		}

		if (i % 640 == 0) {
			y -= 1.f / 320.f;
			x = -1.f;
		} else {
			x += 1.f / 240.f;
		}
	}
	glEnd();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, &depth_rgb[0]);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glTexCoord2f(0, 0); glVertex3f(0 - 2 ,1,0);
	glTexCoord2f(1, 0); glVertex3f(xWindowBound,1,0);
	glTexCoord2f(1, 1); glVertex3f(xWindowBound,0,0);
	glTexCoord2f(0, 1); glVertex3f(0 - 2 ,0,0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);
	
	if (m_device->getVideoFormat() == FREENECT_VIDEO_RGB || m_device->getVideoFormat() == FREENECT_VIDEO_YUV_RGB)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, &rgb[0]);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 1, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &rgb[0]);
	
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glTexCoord2f(0, 0); glVertex3f(0 - 2, 0,0);
	glTexCoord2f(1, 0); glVertex3f(xWindowBound, 0,0);
	glTexCoord2f(1, 1); glVertex3f(xWindowBound,-1,0);
	glTexCoord2f(0, 1); glVertex3f(0 - 2 ,-1,0);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	renderMarkerList();

	glFlush();
}

/* trackball functionality */
static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360 * 16;
	while (angle > 360 * 16)
		angle -= 360 * 16;
}

void RenderView::setXRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != xRot) {
		xRot = angle;
		updateGL();
	}
}

void RenderView::setYRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != yRot) {
		yRot = angle;
		updateGL();
	}
}

void RenderView::setZRotation(int angle)
{
	qNormalizeAngle(angle);
	if (angle != zRot) {
		zRot = angle;
		updateGL();
	}
}

void RenderView::setZoom(int z)
{
	if (z <= 120 * 10 && z >= 0) {
		zoom = z;
		updateGL();
	}
}

void RenderView::updateStatusBar(APP_STATE st)
{
	state = st;
	switch(state)
	{
	case LIVE:
		statusBar->showMessage("To begin the modelling process, hit [Spacebar] or [Pause Button]");
		break;
	case PAUSED:
		statusBar->showMessage("To detect the objects in the scene, roughly mark regions with [Ctrl + Left Click] on the upper left view. Hit [D] when finished");
		break;
	case DETECTED:
		statusBar->showMessage("To select one object in the scene, [Ctrl + Left Click] on the upper left view");
		break;
	case SELECTED:
		statusBar->showMessage("The selected model is ready for exporting");
		break;
	}
}

void RenderView::wheelEvent(QWheelEvent *event)
{
	setZoom(event->delta() + zoom);
}

void RenderView::mousePressEvent(QMouseEvent *event)
{
	if(ctrlPressed && (nobjects == 0) && (state == PAUSED)) {
		if(xToWorldCoord(event->x()) < xWindowBound && yToWorldCoord(event->y()) > 0)
		{
			currentMarker.points.push_back(event->pos());
			int i = 0, j = 0;
			worldCoordToPixelCoord(event->x(), event->y(), i, j);
			detector.startMarkingRegion(i, j);
		}
	}
	else if(ctrlPressed && (nobjects > 0) && (state == DETECTED || state == SELECTED)) {
		if(xToWorldCoord(event->x()) < xWindowBound && yToWorldCoord(event->y()) > 0)
		{
			for (int p = 0; p < 640 * 480; p++)
			{
				if (objects.at(p) == selectedObject)
				{
					depth_rgb.at(p * 3) = saveColor[0];
					depth_rgb.at(p * 3 + 1) = saveColor[1];
					depth_rgb.at(p * 3 + 2) = saveColor[2];
				}
			}

			int i = 0, j = 0;
			worldCoordToPixelCoord(event->x(), event->y(), i, j);
			selectedObject = objects.at(j * 640 + i);
			updateStatusBar(SELECTED);

			bool first = true;
			for (int p = 0; p < 640 * 480; p++)
			{
				if (objects.at(p) == selectedObject)
				{
					if (first)
					{
						saveColor[0] = depth_rgb.at(p * 3);
						saveColor[1] = depth_rgb.at(p * 3 + 1);
						saveColor[2] = depth_rgb.at(p * 3 + 2);
						first = false;
					}
					depth_rgb.at(p * 3) = 0;
					depth_rgb.at(p * 3 + 1) = 0;
					depth_rgb.at(p * 3 + 2) = 255;
				}
			}

			qDebug() << "Selected object: " << selectedObject;
		}
	}
	else
		lastPos = event->pos();
}

void RenderView::mouseReleaseEvent(QMouseEvent *event)
{
	if (ctrlPressed && (state == PAUSED)) {
		markerList.push_back(currentMarker);
		currentMarker.clear();
		detector.stopMarkingRegion();
	}
}

void RenderView::mouseMoveEvent(QMouseEvent *event)
{
	int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();

	if (event->buttons() & Qt::LeftButton) {
		if(ctrlPressed && xToWorldCoord(event->x()) < xWindowBound && yToWorldCoord(event->y()) > 0 && (nobjects == 0) && (state == PAUSED)) {
			currentMarker.points.push_back(event->pos());
			int i = 0, j = 0;
			worldCoordToPixelCoord(event->x(), event->y(), i, j);
			detector.addMarkerToCurrentRegion(i, j);
		}
		else {
			setXRotation(xRot - 8 * dy);
			setYRotation(yRot - 8 * dx);
		}
	} 
	else if(event->buttons() & Qt::RightButton) {
		setZRotation(zRot - 8 * dy);
	}

	lastPos = event->pos();
}

void RenderView::setRearCutoff(int distance_cm)
{
	rearCutoff = distance_cm;
	updateGL();
}

void RenderView::setFrontCutoff(int distance_cm)
{
	frontCutoff = distance_cm;
	updateGL();
}

void RenderView::pause(bool paused)
{
	if (paused) {
		updateStatusBar(PAUSED);
		m_device->stopDepth();
		m_device->stopVideo();
		detector.init(depth_rgb, 640, 480);
	}
	else {
		updateStatusBar(LIVE);
		m_device->startVideo();
		m_device->startDepth();
		nobjects = selectedObject = 0;
	}
}

void RenderView::exportObj() 
{
	/* ensure no data capture happens when trying to export */
	emit pausePlease();
	if(!ObjWriter::exportAsObj(depth, objects, selectedObject, frontCutoff, rearCutoff)) {
		QMessageBox::warning(this, "Export error", "Export failed");
		return;
	}
}

void RenderView::ctrlDown() {
	if(!ctrlPressed) {
		ctrlPressed = true;
	}
}

void RenderView::ctrlUp() {
	if(ctrlPressed) {
		ctrlPressed = false;
	}
}

void RenderView::clearMarkerList() {
	markerList.clear();
}

void RenderView::detect()
{
	if (state == PAUSED)
	{
		detector.detect(nobjects, objects, depth_rgb);
		updateStatusBar(DETECTED);
		clearMarkerList();
	}
}

void RenderView::renderMarkerList() 
{
	if (state == PAUSED)
	{
		if(ctrlPressed) {
			renderMarker(&currentMarker);
		}

		for(int i = 0; i < (int)markerList.size(); ++i) {
			renderMarker(&markerList[i]);
		}
	}
}

void RenderView::renderMarker(marker* m)
{	
	glColor3f(1., 1., 1.);
	glLineWidth(2.0f);
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < (int)m->points.size(); ++i) {
		glVertex3f(xToWorldCoord(m->points[i].x()), yToWorldCoord(m->points[i].y()), 0);
	}
	glEnd();
}

float RenderView::xToWorldCoord(int x)
{
	return (float)x/(float)WIDGET_WIDTH * 4 - 2;
}

float RenderView::yToWorldCoord(int y)
{
	return (float)y/(float)WIDGET_HEIGHT * (-2) + 1;
}

void RenderView::worldCoordToPixelCoord(float x, float y, int &i, int &j)
{
	float r = fabs(xToWorldCoord(x)+2)/fabs(xWindowBound + 2.0f);
	i = (int)(r*640);
	r = fabs(1-yToWorldCoord(y))/1.0f;
	j = (int)(r*480);
}
