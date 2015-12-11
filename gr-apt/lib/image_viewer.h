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

/**
 * Widget for displaying and updating an image line-by-line, with interface for updating the image with data from any thread.
 **/
class ImageViewer : public QWidget {
	Q_OBJECT
	public:
		ImageViewer(QWidget *parent = NULL);

		/**
		 * Update displayed image with new image lines. Appends to current image.
		 * Starts internal signalling for GUI to update the internal QImage, so that memory violations from non-gui thread
		 * is avoided. 
		 *
		 * \param image Input image of format CV_8UC3
		 **/
		void updateImage_fromNonGUI(cv::Mat image);
	protected:
		void paintEvent(QPaintEvent *evt);

	private slots:
		void updateImage(cv::Mat image); 

	private:
		QScrollArea *d_scroll_area; //scrollarea for image
		QLabel *d_image_label; //displayed image
		cv::Mat d_image; //accumulated image data
	
	signals:
		void updateImage_fromNonGUI_toGUI(cv::Mat image);
};


#endif
