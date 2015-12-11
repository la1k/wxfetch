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

ImageViewer::ImageViewer(QWidget *parent): QWidget(parent){
	QGridLayout *layout = new QGridLayout(this);
	d_scroll_area = new QScrollArea;
	layout->addWidget(d_scroll_area, 1, 0, 1, 2);
	d_image_label = new QLabel;
	d_image_label->setBackgroundRole(QPalette::Base);
	d_image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	d_scroll_area->setWidget(d_image_label);
	d_image_label->setScaledContents(true);
	
	connect(this, SIGNAL(updateImage_fromNonGUI_toGUI(cv::Mat)), SLOT(updateImage(cv::Mat)));
	qRegisterMetaType<cv::Mat>("cv::Mat");
}

ImageViewer::~ImageViewer(){
}

void ImageViewer::updateImage_fromNonGUI(cv::Mat image){
	emit updateImage_fromNonGUI_toGUI(image.clone());
}

void ImageViewer::updateImage(cv::Mat image){
	d_curr_image = QImage(image.cols, image.rows, QImage::Format_RGB888);
	d_curr_image.fill(255);
	
	cv::Mat uchar_image;
	image.convertTo(uchar_image, CV_8UC1);

	std::vector<cv::Mat> channels;
	channels.push_back(uchar_image.clone());
	channels.push_back(uchar_image.clone());
	channels.push_back(uchar_image.clone());

	cv::Mat rgb_img;
	cv::merge(channels, rgb_img);

	memcpy(d_curr_image.bits(), rgb_img.data, sizeof(uchar)*image.rows*image.cols*3);
	update();
}

void ImageViewer::paintEvent(QPaintEvent *evt){
	Q_UNUSED(evt);

	if (!d_curr_image.isNull()){
		//scale QImage to current widget size
		QImage resImage = d_curr_image.scaledToWidth(size().width());

		//update label with current pixmap
		this->d_image_label->setPixmap(QPixmap::fromImage(resImage));
		this->d_image_label->adjustSize();
	}
}
