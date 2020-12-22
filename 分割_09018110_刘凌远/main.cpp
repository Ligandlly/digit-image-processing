//! Encoding = UTF-8

// 图像分割
// 程序运行时间较长
// 因为编译出的文件在./build 文件夹中，所以输入输出图像都在上一层文件夹
// 输入图像: ../segImg.bmp
// 输出图像: ../SegOut.bmp

#include <algorithm>
#include <array>
#include <opencv2/opencv.hpp>
using namespace std;

inline bool isInImage(const cv::Mat &img, int i, int j) {
    // 判断点是否在图像中
    return (i < img.rows && i >= 0 && j < img.cols && j >= 0);
}

cv::Mat median_filter(const cv::Mat &img) {
    cv::Mat result = cv::Mat::zeros(img.size(), CV_8U);
    uchar window[9];

    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            int index = 0;
            for (int delta_i = -1; delta_i <= 1; ++delta_i) {
                for (int delta_j = -1; delta_j <= 1; ++delta_j) {
                    int current_i = i + delta_i;
                    int current_j = j + delta_j;
                    if (isInImage(img, current_i, current_j)) {
                        window[index++] = img.at<uchar>(current_i, current_j);
                    }
                }
            }
            sort(window, window + 9);
            result.at<uchar>(i, j) = window[4];
        }
    }

    return result;
}

uchar otsu(const cv::Mat &img) {
    enum { L = 256 };
    array<long double, L> p = {};

    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            p[img.at<uchar>(i, j)]++;
        }
    }

    int area = img.size().area();

    for (int i = 0; i < L; ++i) {
        p[i] = p[i] / area;
    }

    array<long double, L> P1 = {};
    P1[0] = p[0];
    for (int i = 1; i < L; ++i) {
        P1[i] = P1[i - 1] + p[i];
    }

    array<long double, L> m = {};
    for (int i = 1; i < L; ++i) {
        m[i] = m[i - 1] + i * p[i];
    }

    long double m_G = *m.rbegin();

    array<long double, L> sigma2_B = {};
    for (int i = 0; i < L; ++i) {
        long double tmp = P1[i] * (1 - P1[i]);
        sigma2_B[i] = tmp < 0.00000001 /* tmp == 0 */
                          ? 0
                          : (m_G * P1[i] - m[i]) * (m_G * P1[i] - m[i]) / tmp;
    }

    long double max_value = *(max_element(sigma2_B.begin(), sigma2_B.end()));
    int count = 0;
    long double sum_ = 0;
    for (int i = 0; i < L; ++i) {
        if (sigma2_B[i] == max_value) {
            count++;
            sum_ += i;
        }
    }

    long double result = sum_ / count;
    return (uchar)result;
}

cv::Mat min_(const cv::Mat &img, int radius) {
    cv::Mat result = cv::Mat::zeros(img.size(), CV_8U);
    int radius2 = radius * radius;

    uchar min_value = 255;

    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            for (int delta_i = -radius; delta_i < radius; ++delta_i) {
                for (int delta_j = -radius; delta_j < radius; ++delta_j) {
                    int current_i = i + delta_i;
                    int current_j = j + delta_j;
                    if (isInImage(img, current_i, current_j) &&
                        (delta_i * delta_i + delta_j * delta_j < radius2) &&
                        img.at<uchar>(current_i, current_j) < min_value) {
                        min_value = img.at<uchar>(current_i, current_j);
                    }  // end if
                }
            }
            result.at<uchar>(i, j) = min_value;
            min_value = 255;
        }
    }

    return result;
}

cv::Mat max_(cv::Mat &img, int radius) {
    cv::Mat result = cv::Mat::zeros(img.rows, img.cols, CV_8U);
    int radius2 = radius * radius;

    uchar max_value = 0;

    for (int i = 0; i < img.rows; ++i) {
        for (int j = 0; j < img.cols; ++j) {
            for (int delta_i = -radius; delta_i < radius; ++delta_i) {
                for (int delta_j = -radius; delta_j < radius; ++delta_j) {
                    int current_i = i + delta_i;
                    int current_j = j + delta_j;
                    if (isInImage(img, current_i, current_j) &&
                        (delta_i * delta_i + delta_j * delta_j < radius2) &&
                        img.at<uchar>(current_i, current_j) > max_value) {
                        max_value = img.at<uchar>(current_i, current_j);
                    }  // end if
                }
            }
            result.at<uchar>(i, j) = max_value;
            max_value = 0;
        }
    }

    return result;
}

int main() {
    cv::Mat image;
    image = cv::imread("../segImg.bmp", cv::IMREAD_GRAYSCALE);
    if (!image.data) {
        printf("No image data \n");
        return -1;
    }

    image = median_filter(image);
    const int radius = 30;
    cv::Mat tmp = min_(image, radius);  // 腐蚀
    tmp = max_(tmp, radius);  // 开操作
    cv::Mat result = image - tmp; // 顶帽变换

    // Otsu阈值处理
    uchar threshold = otsu(result);
    result.forEach<uchar>(
        [threshold](uchar &val, const int *position) { val = val < threshold ? 0 : 255; });

    cv::imwrite("../SegOut.bmp", result);
    return 0;
}