//=======================================================================================================
// Copyright 2015 Asgeir Bjorgan
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
// http://opensource.org/licenses/MIT)
//=======================================================================================================

#ifndef IMAGE_VIEWER_H_DEFINED
#define IMAGE_VIEWER_H_DEFINED

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QGridLayout>
#include <QColor>
#include <QMutex>
#include <opencv2/core/core.hpp>

class QImage;

class ImageViewer : public QWidget {
	Q_OBJECT
	public:
		ImageViewer(QWidget *parent = NULL);

		//update displayed image with new image from non-gui thread
		void updateImage_fromNonGUI(cv::Mat image);
	protected:
		void paintEvent(QPaintEvent *evt);

	private slots:
		void updateImage(cv::Mat image); 

	private:
		QScrollArea *d_scroll_area; //contains the image
		QLabel *d_image_label; //displayed image
		QImage d_curr_image; //container for current image data
	
	signals:
		void updateImage_fromNonGUI_toGUI(cv::Mat image);
};


#endif
