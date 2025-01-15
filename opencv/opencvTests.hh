#ifndef OPENCVTESTS_h
#define OPENCVTESTS_h

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

void opencvTest0(cv::Mat &);
void opencvTest1(cv::Mat &);

void airex1(cv::Mat &, cv::Mat &, int match = cv::TM_CCOEFF_NORMED);


#endif
