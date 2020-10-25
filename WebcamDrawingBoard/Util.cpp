#include "Util.h"

cv::Scalar rgb2bgr(double r, double g, double b, double a)
{
	return cv::Scalar(b, g, r, a);
}