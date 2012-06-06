/*
 *  Window.h
 *  a1
 *
 *  Created by Jeff Nicholson on 11-09-22.
 *  Copyright 2011 University of Calgary. All rights reserved.
 *
 */

#ifndef _WINDOW_H
#define _WINDOW_H

#include <QtGui>
#include "MyFreenectDevice.h"
#include "RenderView.h"

class Window : public QMainWindow
{

	Q_OBJECT

public:
	Window(QWidget *parent = NULL);
	~Window();

public slots:
	void setPaused() {
		pause_button->setChecked(true);
	}
	
private:
	QSlider *createCutoffSlider();

	QWidget *centralWidget;
	RenderView *renderView;
	QGridLayout *mainLayout;
	QPushButton *pause_button;
	QVBoxLayout *controlLayout;
	QPushButton *exportObj_button;
	QPushButton *exportPly_button;
	QPushButton *exportXyz_button;
	QPushButton *exportPcd_button;
	QPushButton *dump_button;
	QPushButton *pic_button;

protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
};

#endif /* _WINDOW_H */
