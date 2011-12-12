/*
 *  Window.cpp
 *
 *  Created by Jeff Nicholson on 11-10-17.
 *  Copyright 2011 University of Calgary. All rights reserved.
 */

#include "Window.h"

Window::Window(QWidget *parent) : QMainWindow(parent)
{
	setWindowTitle(tr("CPSC589 - 3d Modelling with Kinect"));
	
	renderView = new RenderView();

	mainLayout = new QGridLayout;

	controlLayout = new QVBoxLayout();
	pause_button = new QPushButton("Pause");
	pause_button->setCheckable(true);
	connect(pause_button, SIGNAL(toggled(bool)), renderView, SLOT(pause(bool)));
	connect(renderView, SIGNAL(pausePlease()), this, SLOT(setPaused()));

	exportObj_button = new QPushButton("Export .obj");
	connect(exportObj_button, SIGNAL(clicked()), renderView, SLOT(exportObj()));

	exportPly_button = new QPushButton("Export .ply");
	connect(exportPly_button, SIGNAL(clicked()), renderView, SLOT(exportPly()));


	/* front and rear depth buffer cutoffs */
	QSlider *rearCutoffPlane = createCutoffSlider();
	connect(rearCutoffPlane, SIGNAL(valueChanged(int)), renderView, SLOT(setRearCutoff(int)));

	QSlider *frontCutoffPlane = createCutoffSlider();
	connect(frontCutoffPlane, SIGNAL(valueChanged(int)), renderView, SLOT(setFrontCutoff(int)));

	controlLayout->addWidget(pause_button);
	controlLayout->addWidget(exportPly_button);
	controlLayout->addWidget(exportObj_button);
	controlLayout->addWidget(new QLabel(tr("Rear Cutoff Plane")));
	controlLayout->addWidget(rearCutoffPlane);
	controlLayout->addWidget(new QLabel(tr("Front Cutoff Plane")));
	controlLayout->addWidget(frontCutoffPlane);

	mainLayout->addWidget(renderView, 0, 1, 2, 2);
	mainLayout->addLayout(controlLayout, 0, 3, 1, 2);

	centralWidget = new QWidget;
	setCentralWidget(centralWidget);
	centralWidget->setLayout(mainLayout);

	rearCutoffPlane->setValue(10*100);
	frontCutoffPlane->setValue(0);

	renderView->setStatusBar(statusBar());
}

QSlider *Window::createCutoffSlider()
{
	QSlider *cutoffPlane = new QSlider(Qt::Horizontal);
	cutoffPlane->setRange(0, 10*100);       /* 0 - 10 meters */
	cutoffPlane->setSingleStep(10);         /* 10 centimeters */
	cutoffPlane->setPageStep(50);
	cutoffPlane->setTickInterval(100);
	cutoffPlane->setTickPosition(QSlider::TicksRight);
	return cutoffPlane;
}

Window::~Window() {
}

void Window::keyPressEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Escape)
		close();
	else if (e->key() == Qt::Key_Control)
		renderView->ctrlDown();
	else if (e->key() == Qt::Key_C)
		renderView->clearMarkerList();
	else if (e->key() == Qt::Key_D)
		renderView->detect();
	else if (e->key() == Qt::Key_R)
		renderView->renderMesh();
	else if (e->key() == Qt::Key_T)
		renderView->toggleTexDisplay();
	else if (e->key() == Qt::Key_M)
		renderView->toggleColorDisplay();
	else
		QWidget::keyPressEvent(e);
}

void Window::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == Qt::Key_Control)
		renderView->ctrlUp();
}

