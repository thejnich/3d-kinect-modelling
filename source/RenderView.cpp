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
	zoom = 3 * 120;
	tiltAngle = 10;

	m_device = &freenect.createDevice<MyFreenectDevice>(0);

	m_device->startVideo();
	m_device->startDepth();
	m_device->setLed(LED_YELLOW);
	m_device->setTiltDegrees(tiltAngle);

	requested_format = FREENECT_VIDEO_RGB;
	m_device->setVideoFormat(requested_format);

	depth = std::vector<uint16_t>(RES_PIXELS);
	depth_cache = std::vector< std::vector<uint16_t> >();
	rgb = std::vector<uint8_t>(RES_PIXELS*4);
	depth_rgb = std::vector<uint8_t>(RES_PIXELS*3);
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

	objBound = std::vector<boundRect>();
	grid_depth = std::vector<uint16_t>(RES_PIXELS, USHRT_MAX);
	vertexList = std::vector<vertex>();
	faceList = std::vector<tri_face>();

	displayTex = true;
	displayColor = true;
	detector.init(depth_rgb, RES_WIDTH, RES_HEIGHT);
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
	glEnable(GL_NORMALIZE);

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

	//m_device->getDepth(depth);
	getDepthAndUpdateCache();
	m_device->getRGB(rgb);
	m_device->getDepthRGB(depth_rgb);

	glPushMatrix();
	glRotatef(xRot / 16.0, 1.0, 0.0, 0.0);
	glRotatef(yRot / 16.0, 0.0, 1.0, 0.0);
	glRotatef(zRot / 16.0, 0.0, 0.0, 1.0);
	glTranslatef(0.0f, 0.0f, -(float)zoom / 120.0f);

	if(state == DETECTING) {
		detector.detect(nobjects, objects, objBound, depth_rgb);
	}

	/*
	 * Render the generated mesh if it exists, or the point cloud
	 */
	if(faceList.size()>0) {
		renderSurface();
	}
	else {
		renderPointCloud();
	}
	glPopMatrix();

	// show the rgb video stream and colored depth map as textures
	if (displayTex)
	{
		renderRGB_DepthColor_Textures();
		if(state !=DETECTING)
			renderMarkerList();
	}

	glFlush();
}

/*
 * Cache last ten depth frames, so they can be dumped for analysis
 */
void RenderView::getDepthAndUpdateCache() {
	std::vector<uint16_t> prev;
	prev.assign(depth.begin(), depth.end());
	if(m_device->getDepth(depth)) {
		if( (int)depth_cache.size() < CACHE_SIZE ) 
			depth_cache.push_back(prev);
		else {
			depth_cache.erase(depth_cache.begin());
			depth_cache.push_back(prev);
		}
	}
}

void RenderView::renderRGB_DepthColor_Textures() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gl_depth_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, RES_WIDTH, RES_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, &depth_rgb[0]);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glTexCoord2f(0, 0); glVertex3f(0 - 2 ,1,0);
	glTexCoord2f(1, 0); glVertex3f(xWindowBound,1,0);
	glTexCoord2f(1, 1); glVertex3f(xWindowBound,0,0);
	glTexCoord2f(0, 1); glVertex3f(0 - 2 ,0,0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, gl_rgb_tex);

	if (m_device->getVideoFormat() == FREENECT_VIDEO_RGB || m_device->getVideoFormat() == FREENECT_VIDEO_YUV_RGB)
		glTexImage2D(GL_TEXTURE_2D, 0, 3, RES_WIDTH, RES_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, &rgb[0]);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, 1, RES_WIDTH, RES_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &rgb[0]);

	glBegin(GL_TRIANGLE_FAN);
	glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
	glTexCoord2f(0, 0); glVertex3f(0 - 2, 0,0);
	glTexCoord2f(1, 0); glVertex3f(xWindowBound, 0,0);
	glTexCoord2f(1, 1); glVertex3f(xWindowBound,-1,0);
	glTexCoord2f(0, 1); glVertex3f(0 - 2 ,-1,0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
}

void RenderView::renderSurface() {
	glEnable(GL_LIGHTING);
	GLfloat specular[] = {1.0f, 2.0f, 1.0f, 1.0f};
	GLfloat specularpos[] = {0.0f, 1.0f, 1.0f, 1.0f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, specularpos);
	glEnable(GL_LIGHT0);
	GLfloat ambient[] = {0.5f, 0.5f, 0.5f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	for(int i = 0; i < (int)faceList.size(); ++i) {

		//glBegin(GL_LINE_LOOP);
		glBegin(GL_TRIANGLES);
		glVertex3f(vertexList[faceList[i].v1].x, vertexList[faceList[i].v1].y, vertexList[faceList[i].v1].z);
		glVertex3f(vertexList[faceList[i].v2].x, vertexList[faceList[i].v2].y, vertexList[faceList[i].v2].z);
		glVertex3f(vertexList[faceList[i].v3].x, vertexList[faceList[i].v3].y, vertexList[faceList[i].v3].z);

		float v1x = vertexList[faceList[i].v2].x - vertexList[faceList[i].v1].x;
		float v1y = vertexList[faceList[i].v2].y - vertexList[faceList[i].v1].y;
		float v1z = vertexList[faceList[i].v2].z - vertexList[faceList[i].v1].z;
		float v2x = vertexList[faceList[i].v3].x - vertexList[faceList[i].v1].x;
		float v2y = vertexList[faceList[i].v3].y - vertexList[faceList[i].v1].y;
		float v2z = vertexList[faceList[i].v3].z - vertexList[faceList[i].v1].z;

		float nx = v1y * v2z - v1z * v2y;
		float ny = v1z * v2x - v1x * v2z;
		float nz = v1x * v2y - v1y * v2x;

		glNormal3f(nx,ny,nz);

		glEnd();
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

void RenderView::renderPointCloud() {
	glPointSize(2.);
	glBegin(GL_POINTS);
	float x = -1., y = 1.;
	for (int i = 0; i < RES_PIXELS; i++) {

		if( (depth[i] > frontCutoff && depth[i] < rearCutoff && (selectedObject > 0 && objects[i] == selectedObject)) ||
				(depth[i] > frontCutoff && depth[i] < rearCutoff && (selectedObject == 0)) ) {
			glVertex3f(x, y, (float)depth[i] / 100.0f);
			if (displayColor)
			{
				glColor3uiv((GLuint*)&rgb[3 * (i+RES_WIDTH*30 - 7)]);
			}
			else
			{
				glColor3f(1., 1., 1.);
			}

		}

		if (i % RES_WIDTH == 0) {
			y -= 1.f / (RES_WIDTH/2.f);
			x = -1.f;
		} else {
			x += 1.f / (RES_HEIGHT/2.0f);
		}
	}
	glEnd();

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
			statusBar->showMessage("The selected model is ready for exporting as a point cloud (out.ply). Press R to render a mesh.");
			break;
		case RENDERED:
			statusBar->showMessage("The mesh is ready for exporting as an obj file (out.obj)");
			break;
	}
}

void RenderView::wheelEvent(QWheelEvent *event)
{
	setZoom(event->delta() + zoom);
}

void RenderView::mousePressEvent(QMouseEvent *event)
{
	if(ctrlPressed && (nobjects == 0)) {
		if(mouseInsideMarkingRegion(event))
		{
			currentMarker.points.push_back(event->pos());
			int i = 0, j = 0;
			worldCoordToPixelCoord(event->x(), event->y(), i, j);
			detector.startMarkingRegion(i, j);
		}
	}
	else if(ctrlPressed && (nobjects > 0) && (state == DETECTED || state == SELECTED || state == DETECTING)) {
		if(mouseInsideMarkingRegion(event))
		{
			for (int p = 0; p < RES_PIXELS; p++)
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
			selectedObject = objects.at(j * RES_WIDTH + i);
		//	updateStatusBar(SELECTED);

			bool first = true;
			for (int p = 0; p < RES_PIXELS; p++)
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

bool RenderView::mouseInsideMarkingRegion(QMouseEvent *event) {
	return (xToWorldCoord(event->x()) < xWindowBound && yToWorldCoord(event->y()) > 0);
}

void RenderView::mouseReleaseEvent(QMouseEvent *event)
{
	if (ctrlPressed) {
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
		if(ctrlPressed && mouseInsideMarkingRegion(event) && (nobjects == 0)) {
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
	}
	else {
		updateStatusBar(LIVE);
		m_device->startVideo();
		m_device->startDepth();
		nobjects = selectedObject = 0;
		clearMarkerList();
		faceList.clear();
		vertexList.clear();
	}
}

void RenderView::exportObj() 
{
	/* ensure no data capture happens when trying to export */
	emit pausePlease();
	if(!ObjWriter::exportAsObj(vertexList, faceList)) {
		QMessageBox::warning(this, "Export error", "Export failed");
		return;
	}
}

void RenderView::exportPly()
{
	/* ensure no data capture happens when trying to export */
	emit pausePlease();
	if(!ObjWriter::exportAsPly(depth, objects, selectedObject, frontCutoff, rearCutoff)) {
		QMessageBox::warning(this, "Export error", "Export failed");
		return;
	}

}

void RenderView::exportXyz()
{
	/* ensure no data capture happens when trying to export */
	emit pausePlease();
	if(!ObjWriter::exportAsXyz(depth, objects, selectedObject, frontCutoff, rearCutoff)) {
		QMessageBox::warning(this, "Export error", "Export failed");
		return;
	}
}

void RenderView::exportPcd()
{
	/* ensure no data capture happens when trying to export */
	emit pausePlease();
	if(!ObjWriter::exportAsPcd(depth, objects, selectedObject, frontCutoff, rearCutoff)) {
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
	if (state != DETECTING)
	{
		state = DETECTING;

		//updateStatusBar(DETECTED);
		//clearMarkerList();
	}
	else
	{
		state = LIVE;
	}
}

void RenderView::renderMarkerList() 
{
	if(ctrlPressed) {
		renderMarker(&currentMarker);
	}

	for(int i = 0; i < (int)markerList.size(); ++i) {
		renderMarker(&markerList[i]);
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

void RenderView::toggleTexDisplay()
{
	displayTex = !displayTex;
}

void RenderView::toggleColorDisplay()
{
	displayColor = !displayColor;
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
	i = (int)(r*RES_WIDTH);
	r = fabs(1-yToWorldCoord(y))/1.0f;
	j = (int)(r*RES_HEIGHT);
}

uint16_t RenderView::findMaxDepthOfObject(int object) {
	uint16_t maxDepth = 0;
	boundRect b = objBound[object-1];
	for(int i = b.imin; i <= b.imax; ++i) {
		for(int j = b.jmin; j <= b.jmax; ++j) {
			if((objects[j*RES_WIDTH+i] == object) && (depth[j*RES_WIDTH+i]>maxDepth) && (depth[j*RES_WIDTH+i] < 65517))
				maxDepth = depth[j*RES_WIDTH+i];
		}
	}

	qDebug() << "max depth: " << maxDepth;
	return maxDepth;
}

uint16_t RenderView::findMinDepthOfObject(int object) {
	uint16_t minDepth = 65517;
	boundRect b = objBound[object-1];
	for(int i = b.imin; i <= b.imax; ++i) {
		for(int j = b.jmin; j <= b.jmax; ++j) {
			if((objects[j*RES_WIDTH+i] == object) && (depth[j*RES_WIDTH+i]<minDepth) && (depth[j*RES_WIDTH+i] > 0))
				minDepth = depth[j*RES_WIDTH+i];
		}
	}

	qDebug() << "min depth: " << minDepth;
	return minDepth;
}

void RenderView::flattenObjectsBoundingArea(int object) {
	uint16_t maxDepth = findMaxDepthOfObject(object);
	uint16_t minDepth = findMinDepthOfObject(object);

	if (maxDepth > minDepth + 50)
	{
		maxDepth = minDepth + 30;
	}

	//maxDepth = 100;
	boundRect b = objBound[object-1];

	for(int i = b.imin; i <= b.imax; ++i) {
		for(int j = b.jmin; j <= b.jmax; ++j) {
			if(depth[j*RES_WIDTH+i] > maxDepth)
				depth[j*RES_WIDTH+i] = maxDepth;
			else if (depth[j*RES_WIDTH+i] < minDepth)
				depth[j*RES_WIDTH+i] = minDepth;
		}
	}

}

void RenderView::renderMesh() {
	if(selectedObject <= 0)
		return;
	qDebug() << "attempting to render mesh...";

	flattenObjectsBoundingArea(selectedObject);
	int squareDim = 10;

	boundRect b = objBound[selectedObject-1];

	for(int i = b.imin; b.imax-i >= squareDim; i+=squareDim) {
		for(int j = b.jmin; b.jmax-j >= squareDim; j+=squareDim) {
			calcAvgOfSquare(i,j,squareDim,squareDim,b);
		}
	}

	// fill faceList and vertexList
	//uint8_t min = findMinDepthOfObject(selectedObject);
	//uint8_t max = findMaxDepthOfObject(selectedObject);
	// add all vertices
	vertexList.clear();
	faceList.clear();
	for(int j = b.jmin; b.jmax-j >= squareDim; j+=squareDim) {
		for(int i = b.imin; b.imax-i >= squareDim; i+=squareDim) {
			vertex v = {xToWorldCoord(i), yToWorldCoord(j), (float)grid_depth[j*RES_WIDTH+i]/100.0f};
			vertexList.push_back(v);
		}
		// last col
		//vertex v = {xToWorldCoord(b.imax), yToWorldCoord(j), (float)grid_depth[j*640+b.imax]/100.0f};
		//vertexList.push_back(v);
	}

	// last row
	//for(int i = b.imin, j = b.jmax; i < b.imax-squareDim; i+=squareDim) {
	//	vertex v = {xToWorldCoord(i), yToWorldCoord(j), (float)grid_depth[j*640+i]/100.0f};
	//	vertexList.push_back(v);
	//}
	// bottom right vertex
	//vertex v = {xToWorldCoord(b.imax), yToWorldCoord(b.jmax), (float)grid_depth[b.jmax*640+b.imax]/100.0f};
	//vertexList.push_back(v);

	int vertPerRow = (b.imax - b.imin)/squareDim;
	for(int i = 0; i < (int)vertexList.size() - vertPerRow; ++i) {
		tri_face f = {i, i+vertPerRow, i+1};
		faceList.push_back(f);
		tri_face f1 = {i+vertPerRow, i+vertPerRow+1, i+1};
		faceList.push_back(f1);
	}
	updateStatusBar(RENDERED);	
	qDebug() << "finished mesh render!";
	qDebug() << "numFaces: " << faceList.size();
}

// use w and h in case we end up using rectangles
// i,j coords of top left corner
void RenderView::calcAvgOfSquare(int iTopLeft, int jTopLeft, int h, int w, boundRect br) {

	double sum = 0;
	uint16_t avgDepth;

	for(int i = iTopLeft; i <= iTopLeft + w; ++i) {
		for(int j = jTopLeft; j <= jTopLeft+h; ++j) {
			if( (i < br.imax) && (i > br.imin) && (j < br.jmax) && (j > br.jmin))
				sum += depth[j*RES_WIDTH+i];
		}
	}

	// calculate average for this square
	avgDepth = (uint16_t)(sum / (w*h));

	// check if each of the four corners has been visited
	// if not, set to avgDepth.
	// if yes, set to avg between avgDepth and current value in grid_depth

	int iRight = iTopLeft + w;
	int jBot = jTopLeft + h;
	if(iRight > br.imax)
		iRight = br.imax;
	if(jBot > br.jmax)
		jBot = br.jmax;

	// top left
	setDepthOrAvg(iTopLeft, jTopLeft, avgDepth);

	// top right
	setDepthOrAvg(iRight, jTopLeft, avgDepth);

	// bottom right
	setDepthOrAvg(iRight, jBot, avgDepth);

	// bottom left
	setDepthOrAvg(iTopLeft, jBot, avgDepth);
}


void RenderView::setDepthOrAvg(int i, int j, uint16_t depth) {
	if(grid_depth[j*RES_WIDTH+i] == USHRT_MAX){
		grid_depth[j*RES_WIDTH+i] = depth;
	}
	else {
		grid_depth[j*RES_WIDTH+i] = (uint16_t)(( (int)(grid_depth[j*RES_WIDTH+i]) + (int)depth ) / 2);	
	}
}

void RenderView::increaseTilt() {
	tiltAngle++;
	if(tiltAngle > 30)
		tiltAngle = 30;
	m_device->setTiltDegrees(tiltAngle);
}

void RenderView::decreaseTilt() {
	tiltAngle--;
	if(tiltAngle < -30)
		tiltAngle = -30;
	m_device->setTiltDegrees(tiltAngle);
}

void RenderView::resetTilt() {
	tiltAngle = 0;
	m_device->setTiltDegrees(tiltAngle);
}

void RenderView::dump() {
	emit pausePlease();
	float x = -1., y = 1.;
	for (int i = 0; i < RES_PIXELS; i++) {
		if(depth[i] > frontCutoff && depth[i] < rearCutoff ) {
			printf("%.2f, %.2f", x,y);
			for(int j = 0; j<(int)depth_cache.size(); j++) {	
				printf(", %.5f", (float)depth_cache[j][i]);
			}
			printf("\n");
		}

		if (i % RES_WIDTH == 0) {
			y -= 1.f / (RES_WIDTH/2.f);
			x = -1.f;
		} else {
			x += 1.f / (RES_HEIGHT/2.0f);
		}
	}
	printf("done\n");
}
