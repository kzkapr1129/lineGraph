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
		: mRangeX(-1, 1), mRangeY(-1, 1),
		mAdapter(NULL), mNumXAxis(100) {
	init();
}

LineGraphDrawer::~LineGraphDrawer() {
}

void LineGraphDrawer::setRange(float xmin, float xmax, float ymin, float ymax, int numXAxis) {
	if (xmax <= xmin || ymax <= ymin || numXAxis <= 0) {
		// INVALID param
		return;
	}

	mRangeX.first = xmin;
	mRangeX.second = xmax;
	mRangeY.first = ymin;
	mRangeY.second = ymax;
	mNumXAxis = numXAxis;
	init();
}

void LineGraphDrawer::setAdapter(IAdapter* adapter) {
	mAdapter = adapter;
}

const cv::Mat& LineGraphDrawer::draw() {
	if (mAdapter == NULL || mFrame.empty()) {
		return mFrame;
	}

	// Y軸の中心線描画
	cv::line(mFrame, cv::Point(0, mFrame.rows / 2), cv::Point(mFrame.cols, mFrame.rows / 2), cv::Scalar(255, 255, 255), 2);

	// X軸の中心線描画
	cv::line(mFrame, cv::Point(mFrame.cols / 2, 0), cv::Point(mFrame.cols / 2, mFrame.rows), cv::Scalar(255, 255, 255), 2);

	// 各軸の線グラフ描画
	const float deltaX = mFrame.cols / (float)mNumXAxis;
	const int numAxis = mAdapter->numAxis();
	for (int axis = 0; axis < numAxis; axis++) {

		float preVal = 0;
		int length = mAdapter->length(axis);
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

void LineGraphDrawer::init() {
	const float width = mRangeX.second - mRangeX.first;
	const float height = mRangeY.second - mRangeY.first;
	const float aspect = width / height;
	const float frameWidth = FRAME_WIDTH;
	const float frameHeight = FRAME_WIDTH / aspect;
	mFrame = cv::Mat::zeros(cv::Size(frameWidth, frameHeight), CV_8UC3);
}

int LineGraphDrawer::calcYpos(float value) const {
	const float height = fabs(mRangeY.second - mRangeY.first);
	float cropVal = std::max(mRangeY.first, std::min(mRangeY.second, value));
	float kval = cropVal + (height / 2);
	float val2 = kval / height;
	return mFrame.rows * val2;
}