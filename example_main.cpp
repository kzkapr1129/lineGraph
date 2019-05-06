#include "graph/lineChart.h"
#include <time.h>
#include <math.h>
#include <chrono>

int main() {

	LineChart<3> chart;
	chart.setWindowSize(1920, 1080);
	chart.setLineColor(0, cv::Scalar(0, 0, 255));
	chart.setLineColor(1, cv::Scalar(0, 255, 0));
	chart.setLineColor(2, cv::Scalar(255, 0, 0));
	chart.setXRange(0, 2);
	chart.setAxisTitle(0, "position");
	chart.setAxisTitle(1, "force");
	chart.setAxisTitle(2, "velocity");
	chart.setEnableLegend(true);

	int index = 0;
	while (true) {
		double t = index * 0.002;
		float pos = 1 * sin(2 * 2 * M_PI * t);
		float force = 0.5 * sin(1.5 * 2 * M_PI * t);
		float velocity = 0.8 * sin(0.8 * 2 * M_PI * t);

		chart.push(Value<3>().setX(t).setY(pos, force, velocity));

		std::chrono::system_clock::time_point start, end;

		start = std::chrono::system_clock::now();
		const cv::Mat& frame = chart.draw();
		end = std::chrono::system_clock::now();

		double msec = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		printf("msec: %lf\n", msec);
	
		cv::imshow("frame", frame);
		if (cv::waitKey(1) == 27) {
			break;
		}

		index++;
	}

	return 0;
}