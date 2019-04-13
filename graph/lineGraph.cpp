#include "lineGraph.h"

#define FRAME_WIDTH 512

const cv::Scalar AXIS_COLORS[] = {
	cv::Scalar(0, 0, 255),
	cv::Scalar(0, 255, 0),
	cv::Scalar(255, 0, 0),
	cv::Scalar(0, 255, 255),
	cv::Scalar(255, 255, 0),
	cv::Scalar(255, 0, 255),
};

LineGraphDrawer::LineGraphDrawer() 
		: mYMin(-1), mYMax(1),
		mAdapter(NULL), mNumXAxis(100) {
	setRange(-1, 1);
}

LineGraphDrawer::~LineGraphDrawer() {
}

void LineGraphDrawer::setRange(float ymin, float ymax, int numXAxis) {
	if (ymax <= ymin || ymax < -1 || ymin < -1 || 1 < ymax || 1 < ymin || numXAxis <= 0) {
		// INVALID param
		return;
	}

	mYMin = ymin;
	mYMax = ymax;
	mNumXAxis = numXAxis;
	mFrame = cv::Mat::zeros(cv::Size(FRAME_WIDTH, FRAME_WIDTH), CV_8UC3);
}

void LineGraphDrawer::setAdapter(IAdapter* adapter) {
	mAdapter = adapter;
}

const cv::Mat& LineGraphDrawer::draw() {
	if (mAdapter == NULL || mFrame.empty()) {
		return mFrame;
	}

	// Y軸の中心線描画
	cv::line(mFrame, cv::Point(0, mFrame.rows / 2), cv::Point(mFrame.cols, mFrame.rows / 2), cv::Scalar(255, 255, 255), 1);

	// 各軸の線グラフ描画
	const float deltaX = mFrame.cols / (float)mNumXAxis;
	const int numAxis = mAdapter->numAxis();
	for (int axis = 0; axis < numAxis; axis++) {

		float preVal = 0;
		int length = std::min(mNumXAxis, mAdapter->length(axis));
		for (int i = 0; i < length; i++) {
			float val = mAdapter->value(axis, i);

			if (0 < i) {
				// 2番目のデータから描画する
				int sposX = (i-1) * deltaX;
				int eposX = i * deltaX;
				int sposY = calcYpos(preVal);
				int eposY = calcYpos(val);

				cv::line(mFrame, cv::Point(sposX, sposY), cv::Point(eposX, eposY), AXIS_COLORS[axis]);
			}

			preVal = val;
		}
	}

	return mFrame;
}

int LineGraphDrawer::calcYpos(float value) const {
	const float height = fabs(mYMax - mYMin);
	float cropVal = std::max(mYMin, std::min(mYMax, value));
	float hWeight = (cropVal - mYMin) / height;
	return mFrame.rows - mFrame.rows * hWeight;
}