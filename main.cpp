#include "graph/lineGraph.h"
#include <random>

class MyAdapter : public IAdapter {
public:
	MyAdapter() : mt(rnd()) {
	}

	int numAxis() const {
		return 1;
	}

	float length(int axis) {
		return 100;
	}

	float value(int axis, int index) const {
		return (index - 50) / 50.f;
	}

private:
	std::random_device rnd;
	std::mt19937 mt;
};

int main() {

	MyAdapter adapter;
	LineGraphDrawer drawer;
	drawer.setAdapter(&adapter);
	const cv::Mat& frame = drawer.draw();

	cv::imshow("frame", frame);
	cv::waitKey();


	return 0;
}