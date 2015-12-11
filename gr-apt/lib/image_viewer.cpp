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
	scrollArea = new QScrollArea;
	layout->addWidget(scrollArea, 1, 0, 1, 2);
	imageLabel = new QLabel;
	imageLabel->setBackgroundRole(QPalette::Base);
	imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	scrollArea->setWidget(imageLabel);
	imageLabel->setScaledContents(true);
	
	connect(this, SIGNAL(updateImage_fromNonGUI_toGUI(cv::Mat)), SLOT(updateImage(cv::Mat)));
	qRegisterMetaType<cv::Mat>("cv::Mat");
}

ImageViewer::~ImageViewer(){
}

void ImageViewer::updateImage_fromNonGUI(cv::Mat image){
	emit updateImage_fromNonGUI_toGUI(image.clone());
}

void ImageViewer::updateImage(cv::Mat image){
	currImage = QImage(image.cols, image.rows, QImage::Format_RGB888);
	currImage.fill(255);
	
	cv::Mat uchar_image;
	image.convertTo(uchar_image, CV_8UC1);

	std::vector<cv::Mat> channels;
	channels.push_back(uchar_image.clone());
	channels.push_back(uchar_image.clone());
	channels.push_back(uchar_image.clone());

	cv::Mat rgb_img;
	cv::merge(channels, rgb_img);

	memcpy(currImage.bits(), rgb_img.data, sizeof(uchar)*image.rows*image.cols*3);
	update();
}

void ImageViewer::paintEvent(QPaintEvent *evt){
	Q_UNUSED(evt);

	if (!currImage.isNull()){
		//scale QImage to current widget size
		QImage resImage = currImage.scaledToWidth(size().width());

		//update label with current pixmap
		this->imageLabel->setPixmap(QPixmap::fromImage(resImage));
		this->imageLabel->adjustSize();
	}
}
