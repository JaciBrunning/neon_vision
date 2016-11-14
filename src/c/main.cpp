#include <iostream>
#include <inttypes.h>
#include <sys/time.h>
#include <time.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include <vector>

extern "C" void memcpy_threshold_asm(uint8_t *dest, const uint8_t *src, int count, int minimum);

long long current_time_millis() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)((unsigned long long)(tv.tv_sec) * 1000 + (unsigned long long)(tv.tv_usec) / 1000);
}

void memcpy_threshold_c(uint8_t *dest, const uint8_t *src, int count, int minimum) {
    for (int i = 0; i < count; i++) {
        dest[i] = src[i] > minimum ? src[i] : 0;
    }
}

cv::Mat cv_img(cv::Size(640, 480), CV_8UC1);
std::vector<std::vector<cv::Point> > _contours;
std::vector<std::vector<cv::Point> > _filt_contours;
std::vector<std::vector<cv::Point> > _filt_hulls;
std::vector<cv::Rect> _rects;
int active_contour;

void standard_process() {
    _contours.clear();
    _filt_contours.clear();
    _filt_hulls.clear();
    _rects.clear();

    findContours(cv_img, _contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);
    double largest_area = 0.0;
    int i;
    for (i = 0; i < _contours.size(); i++) {
        std::vector<cv::Point> *contour = &_contours[i];
        cv::Rect r = boundingRect(*contour);
        double area = contourArea(*contour);
        if (area > 300.0) {
            std::vector<cv::Point> hull;
            convexHull(*contour, hull);
            double solidity = 100 * area / contourArea(hull);

            if (solidity < 60.0) {
                if (area > largest_area) {
                    largest_area = area;
                    active_contour = _filt_contours.size();
                }
                _filt_contours.push_back(*contour);
                _filt_hulls.push_back(hull);
                _rects.push_back(r);
            }
        }
    }
}

int main() {
    uint8_t *source = (uint8_t*)malloc(640*480);

    // Random fill
    for (int i = 0; i < 640*480; i++) {
        source[i] = rand() % 255;
    }

    uint8_t *destination = (uint8_t*)malloc(640*480);
    uint8_t *destination_simd = (uint8_t*)malloc(640*480);

    long long start_c = current_time_millis();
    for (int i = 0; i < 30; i++) {
        memcpy_threshold_c(destination, source, 640*480, 99);
        cv_img.data = destination;
        standard_process();
    }
    long long end_c = current_time_millis();
    std::cout << "C Time: " << (end_c - start_c) << std::endl;

    cv::Scalar hsl_low(0, 100, 0), hsl_high(255, 255, 255);
    cv_img.data = source;
    long long start_cv = current_time_millis();
    for (int i = 0; i < 30; i++) {
        cv::inRange(cv_img, hsl_low, hsl_high, cv_img);
        standard_process();
    }
    long long end_cv = current_time_millis();
    std::cout << "CV Time: " << (end_cv - start_cv) << std::endl;

    long long start = current_time_millis();
    for (int i = 0; i < 30; i++) {
        memcpy_threshold_asm(destination_simd, source, 640*480, 99);
        cv_img.data = destination_simd;
        standard_process();
    }
    long long end = current_time_millis();
    std::cout << "SIMD Time: " << (end - start) << std::endl;

    free(source);
    free(destination);
    free(destination_simd);
    return 0;
}