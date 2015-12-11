//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#include "image_viewer.h"
#include <QImage>
#include <QPainter>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <iostream>
using namespace std;

ImageViewer::ImageViewer(QWidget *parent) : QWidget(parent){
	//layout
	QGridLayout *layout = new QGridLayout(this);
	d_scroll_area = new QScrollArea;
	layout->addWidget(d_scroll_area, 1, 0, 1, 2);
	d_image_label = new QLabel;
	d_image_label->setBackgroundRole(QPalette::Base);
	d_image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	d_scroll_area->setWidget(d_image_label);
	d_image_label->setScaledContents(true);

	//connection
	connect(this, SIGNAL(updateImage_fromNonGUI_toGUI(cv::Mat)), SLOT(updateImage(cv::Mat)));
	qRegisterMetaType<cv::Mat>("cv::Mat");
}

void ImageViewer::updateImage_fromNonGUI(cv::Mat image){
	//signal to GUI thread that the image should be updated
	emit updateImage_fromNonGUI_toGUI(image.clone());
}

void ImageViewer::updateImage(cv::Mat image){
	//append image data
	d_image.push_back(image);
	update();
}

void ImageViewer::paintEvent(QPaintEvent *evt){
	Q_UNUSED(evt);

	//update displayed image
	QImage image((uchar*)d_image.data, d_image.cols, d_image.rows, d_image.step, QImage::Format_RGB888);
	this->d_image_label->setPixmap(QPixmap::fromImage(image.scaledToWidth(d_scroll_area->size().width())));
	this->d_image_label->adjustSize();
}
