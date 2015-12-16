#include "apt.h"

cv::Mat apt_image_channel(cv::Mat raw_image, int channel)
{
	if (channel > 1) {
		return cv::Mat();
	}

	return raw_image(cv::Rect(APT_SYNC_WIDTH + APT_SPC_WIDTH + channel*APT_CHANNEL_OFFSET, 0, APT_CHANNEL_WIDTH, raw_image.rows));
}

