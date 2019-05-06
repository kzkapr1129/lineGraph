#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <boost/circular_buffer.hpp>

template<int N>
struct Value {
    float x;
    float y[N];

    Value();
    Value<N>& setX(float x);

    template<class... Args>
    Value<N>& setY(Args... args);
};

template<int N>
class LineChart {
public:
    LineChart();

    void setWindowSize(int width, int height);
    void setXRange(float min, float max);
    void setYRange(float min, float max);
    void setLineColor(int axis, const cv::Scalar& c);
    void setAxisTitle(int axis, const std::string& title);
    void setEnableLegend(bool enable);

    void push(const Value<N>& value);

    const cv::Mat& draw();

private:
    int calcYpos(float value) const;
    boost::circular_buffer<Value<N>> mRingBuffer;

    struct {
        cv::Scalar color;
        std::string title;
    } mAxises[N];

    cv::Mat mFrame;
    cv::Mat mLegend;
    bool mNeedRedraw;
    float mXMin;
    float mXMax;
    float mXWidth;
    float mYMin;
    float mYMax;
    float mYWidth;
};

template<int N>
Value<N>::Value() : x(0) {
    memset(y, 0, sizeof(y));
}

template<int N>
Value<N>& Value<N>::setX(float _x) {
    x = _x;
    return *this;
}

template<int N>
template<class... Args>
Value<N>& Value<N>::setY(Args... args) {
    int i = 0;
    for (auto const& e : { args... }) {
        y[i++] = e;
        if (i == N) break;
    }

    return *this;
}

template<int N>
LineChart<N>::LineChart() : mNeedRedraw(false), mRingBuffer(5000) {
    setWindowSize(512, 512);
    setXRange(0, 2);
    setYRange(-1, 1);

    // 軸の初期化
    for (int i = 0; i < N; i++) {
        mAxises[i].color = cv::Scalar(255, 0, 0);
        mAxises[i].title = "axis: " + std::to_string(i);
    }
}

template<int N>
void LineChart<N>::setWindowSize(int width, int height) {
    mFrame = cv::Mat::ones(height, width, CV_8UC3);
    mNeedRedraw = true;
}

template<int N>
void LineChart<N>::setXRange(float min, float max) {
    mXMin = min;
    mXMax = max;
    mXWidth = fabs(max - min);
    mNeedRedraw = true;
}

template<int N>
void LineChart<N>::setYRange(float min, float max) {
    mYMin = min;
    mYMax = max;
    mYWidth = fabs(max - min);
    mNeedRedraw = true;
}

template<int N>
void LineChart<N>::setLineColor(int axis, const cv::Scalar& c) {
    mAxises[axis].color = c;
    mNeedRedraw = true;
}

template<int N>
void LineChart<N>::setAxisTitle(int axis, const std::string& title) {
    mAxises[axis].title = title;
    mNeedRedraw = true;
}

template<int N>
void LineChart<N>::setEnableLegend(bool enable) {
    if (!enable) {
        mLegend.release();
        return;
    }

    int marginX = 3;
    int marginY = 3;

	// 凡例の高さ
	float totalMarginHeight = marginY * (N+1);
	float legendContentHeight = ceil(mFrame.rows * 0.1);
	float legendLinesHeight = legendContentHeight - totalMarginHeight;

	int textTotalHeight = 0;

	// フォントの基本サイズ取得
	for (int i = 0; i < N; i++) {
		int baseline;
		cv::Size textSize = cv::getTextSize(mAxises[i].title.c_str(), cv::FONT_HERSHEY_SIMPLEX, 1, 1, &baseline);
		textTotalHeight += textSize.height + baseline;
	}

	// 凡例ウインドウの高さに応じてフォントサイズのスケール値を計算
	float scale = legendLinesHeight / (float)textTotalHeight;

	struct _textSize {
		cv::Size textSize;
		int baseline;
	};
	std::vector<_textSize> sizes;

	int maxLineWidth = 0;
	// スケール後のフォントのサイズ取得
	for (int i = 0; i < N; i++) {
		_textSize size;
		size.textSize = cv::getTextSize(mAxises[i].title.c_str(), cv::FONT_HERSHEY_SIMPLEX, scale, 1, &size.baseline);
		sizes.push_back(size);

		maxLineWidth = std::max(maxLineWidth, size.textSize.width);
	}

	int iconPadding = 2;
	int iconSize = sizes[0].textSize.height + sizes[0].baseline - iconPadding/*left*/ - iconPadding/*right*/;
	int legendContentWidth = iconSize + (iconPadding*2) + marginX + maxLineWidth + marginX;

	mLegend = cv::Mat::zeros(legendContentHeight, legendContentWidth, CV_8UC3);
	mLegend = cv::Scalar(128, 250, 250);

	int top = marginY;
	for (int i = 0; i < N; i++) {
		cv::rectangle(mLegend, cv::Point(iconPadding, top + iconPadding),
			cv::Point(iconPadding + iconSize, top + iconPadding + iconSize),
			mAxises[i].color, -1);

		int textLeft = iconPadding + iconSize + iconPadding + marginX;
		int textBottom = top + sizes[i].textSize.height;
		cv::putText(mLegend, mAxises[i].title.c_str(), cv::Point(textLeft, textBottom), cv::FONT_HERSHEY_SIMPLEX, scale, cv::Scalar(0,0,0), 1, cv::LINE_AA);
	
		top += sizes[i].textSize.height + sizes[i].baseline + marginY;
	}
}

template<int N>
void LineChart<N>::push(const Value<N>& value) {
    mRingBuffer.push_back(value);

    float startTime = mRingBuffer.back().x - mXWidth;
    int len = mRingBuffer.size();
    int i;
    for (i = 0; i < len; i++) {
        const Value<N>& value = mRingBuffer[i];
        if (startTime <= value.x) {
            break;
        }
    }

    if (1 < i) {
        int eraseEndIndex = i - 1;
        mRingBuffer.erase_begin(eraseEndIndex);
    }

    mNeedRedraw = true;
}

template<int N>
const cv::Mat& LineChart<N>::draw() {
    if (!mNeedRedraw) {
        return mFrame;
    }

    // クリア
    mFrame = cv::Scalar(255, 250, 250);

    // Y軸中心線の描画
    cv::line(mFrame, cv::Point(0, mFrame.rows/2), cv::Point(mFrame.cols, mFrame.rows/2), cv::Scalar(255, 128, 128));

    if (0 < mRingBuffer.size()) {
        float startTime = mRingBuffer.back().x - mXWidth;

        struct {
            int x;
            int y;
        } preVal[N];

        int len = mRingBuffer.size() - 1;
        for (int i = len; i >= 0; i--) {
            for (int axis = 0; axis < N; axis++) {
                const Value<N>& value = mRingBuffer[i];
                float xpos = ((value.x - startTime) / mXWidth) * mFrame.cols;
                float ypos = calcYpos(value.y[axis]);

                if (startTime <= value.x) {
                    if (i != len) {
                        // 線分描画
                        cv::line(mFrame, cv::Point(xpos, ypos), cv::Point(preVal[axis].x, preVal[axis].y), mAxises[axis].color, 1/*, cv::LINE_AA*/);
                    }

                    preVal[axis].x = xpos;
                    preVal[axis].y = ypos;

                } else {
                    float weight = preVal[axis].x / fabs(xpos - preVal[axis].x);
                    float diffY = ypos - preVal[axis].y;
                    float posX0y = diffY * weight + preVal[axis].y;
                    cv::line(mFrame, cv::Point(0, posX0y), cv::Point(preVal[axis].x, preVal[axis].y), mAxises[axis].color, 1/*, cv::LINE_AA*/);
                    break;
                }
            }
        }
    }

    if (!mLegend.empty()) {
        cv::Rect roiRect(5, 5, mLegend.cols, mLegend.rows);
        cv::Mat roi = mFrame(roiRect);
        mLegend.copyTo(roi);
    }

    mNeedRedraw = false;
    return mFrame;
}

template<int N>
int LineChart<N>::calcYpos(float value) const {
	float cropVal = std::max(mYMin, std::min(mYMax, value));
	float hWeight = (cropVal - mYMin) / mYWidth;
	return mFrame.rows - mFrame.rows * hWeight;
}