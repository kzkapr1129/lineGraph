#pragma once

#include <string>
#include <opencv2/opencv.hpp>

class IAdapter {
public:
	virtual ~IAdapter() {}
	virtual int numAxis() const = 0;
	virtual int length(int axis) = 0;

	// ~1から1の範囲の値を返却する
	virtual float value(int axis, int index) const = 0;
};

class LineGraphDrawer {
public:
	LineGraphDrawer();
	~LineGraphDrawer();

	// xmin,xmax,ymin,ymaxは-1 ~ 1の範囲で指定
	void setRange(float ymin, float ymax, int numXAxis = 100);
	void setAdapter(IAdapter* adapter);
	const cv::Mat& draw();

private:
	int calcYpos(float value) const;
	float mYMin;
	float mYMax;
	int mNumXAxis;
	cv::Mat mFrame;
	IAdapter* mAdapter;
};