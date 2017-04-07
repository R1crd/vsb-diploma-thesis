#include <random>
#include "template_matcher.h"
#include "../utils/utils.h"
#include "objectness.h"
#include "../core/triplet.h"
#include "hasher.h"
#include "../core/template.h"

float TemplateMatcher::extractGradientOrientation(cv::Mat &src, cv::Point &point) {
    float dx = (src.at<float>(point.y, point.x - 1) - src.at<float>(point.y, point.x + 1)) / 2.0f;
    float dy = (src.at<float>(point.y - 1, point.x) - src.at<float>(point.y + 1, point.x)) / 2.0f;

    return cv::fastAtan2(dy, dx);
}

void TemplateMatcher::match(const cv::Mat &srcColor, const cv::Mat &srcGrayscale, const cv::Mat &srcDepth,
                     std::vector<Window> &windows, std::vector<TemplateMatch> &matches) {

}

void TemplateMatcher::train(std::vector<TemplateGroup> &groups) {
    // Generate canny and stable feature points
    generateFeaturePoints(groups);

    // Extract gradient orientations
    extractGradientOrientations(groups);
}

void TemplateMatcher::generateFeaturePoints(std::vector<TemplateGroup> &groups) {
    // Init engine
    typedef std::mt19937 engine;

    for (auto &group : groups) {
        for (auto &t : group.templates) {
            std::vector<cv::Point> cannyPoints;
            std::vector<cv::Point> stablePoints;
            cv::Mat canny, sobelX, sobelY, sobel, src_8uc1;

            // Convert to uchar and apply canny to detect edges
            cv::convertScaleAbs(t.src, src_8uc1, 255);

            // Apply canny to detect edges
            cv::blur(src_8uc1, src_8uc1, cv::Size(3, 3));
            cv::Canny(src_8uc1, canny, cannyThreshold1, cannyThreshold2, 3, false);

            // Apply sobel to get mask for stable areas
            cv::Sobel(src_8uc1, sobelX, CV_8U, 1, 0, 3);
            cv::Sobel(src_8uc1, sobelY, CV_8U, 0, 1, 3);
            cv::addWeighted(sobelX, 0.5, sobelY, 0.5, 0, sobel);

            // Get all stable and edge points based on threshold
            for (int y = 0; y < canny.rows; y++) {
                for (int x = 0; x < canny.cols; x++) {
                    if (canny.at<uchar>(y, x) > 0) {
                        cannyPoints.push_back(cv::Point(x, y));
                    }

                    if (src_8uc1.at<uchar>(y, x) > grayscaleMinThreshold && sobel.at<uchar>(y, x) <= sobelMaxThreshold) {
                        stablePoints.push_back(cv::Point(x, y));
                    }
                }
            }

            // There should be more than MIN points for each template
            assert(stablePoints.size() > featurePointsCount);
            assert(cannyPoints.size() > featurePointsCount);

            // Shuffle
            std::shuffle(stablePoints.begin(), stablePoints.end(), engine(1));
            std::shuffle(cannyPoints.begin(), cannyPoints.end(), engine(1));

            // Save random points into the template arrays
            for (int i = 0; i < featurePointsCount; i++) {
                int ri = (int) Triplet::random(0, stablePoints.size() - 1);
                t.stablePoints.push_back(stablePoints[ri]);

                // Randomize once more
                ri = (int) Triplet::random(0, cannyPoints.size() - 1);
                t.edgePoints.push_back(cannyPoints[ri]);
            }

#ifndef NDEBUG
//            // Visualize extracted features
//            cv::Mat visualizationMat;
//            cv::cvtColor(t.src, visualizationMat, CV_GRAY2BGR);
//
//            for (int i = 0; i < featurePointsCount; ++i) {
//                cv::circle(visualizationMat, t.edgePoints[i], 1, cv::Scalar(0, 0, 255), -1);
//                cv::circle(visualizationMat, t.stablePoints[i], 1, cv::Scalar(0, 255, 0), -1);
//            }
//
//            cv::imshow("TemplateMatcher::train Sobel", sobel);
//            cv::imshow("TemplateMatcher::train Canny", canny);
//            cv::imshow("TemplateMatcher::train Feature points", visualizationMat);
//            cv::waitKey(0);
#endif
        }
    }
}

void TemplateMatcher::extractGradientOrientations(std::vector<TemplateGroup> &groups) {
    // Checks
    assert(groups.size() > 0);

    for (auto &group : groups) {
        for (auto &t : group.templates) {
            // Quantize surface normal and gradient orientations and extract other features
            for (int i = 0; i < featurePointsCount; i++) {
                // Checks
                assert(!t.src.empty());
                assert(!t.srcHSV.empty());
                assert(!t.srcDepth.empty());

                // Save features to template
                t.features.orientationGradients[i] = quantizeOrientationGradients(extractGradientOrientation(t.src, t.edgePoints[i]));
                t.features.surfaceNormals[i] = Hasher::quantizeSurfaceNormals(Hasher::extractSurfaceNormal(t.srcDepth, t.stablePoints[i]));
                t.features.depth[i] = t.srcDepth.at<float>(t.stablePoints[i]);
                t.features.color[i] = t.srcHSV.at<cv::Vec3b>(t.stablePoints[i]);

                // Checks
                assert(t.features.orientationGradients[i] >= 0);
                assert(t.features.orientationGradients[i] < 5);
                assert(t.features.surfaceNormals[i] >= 0);
                assert(t.features.surfaceNormals[i] < 8);
            }
        }
    }
}

int TemplateMatcher::quantizeOrientationGradients(float deg) {
    // Checks
    assert(deg >= 0);
    assert(deg <= 360);

    // We work only in first 2 quadrants
    int degNormalized = static_cast<int>(deg) % 180;

    // Quantize
    if (degNormalized >= 0 && degNormalized < 36) {
        return 0;
    } else if (degNormalized >= 36 && degNormalized < 72) {
        return 1;
    } else if (degNormalized >= 72 && degNormalized < 108) {
        return 2;
    } else if (degNormalized >= 108 && degNormalized < 144) {
        return 3;
    } else {
        return 4;
    }
}

uint TemplateMatcher::getFeaturePointsCount() const {
    return featurePointsCount;
}

uchar TemplateMatcher::getCannyThreshold1() const {
    return cannyThreshold1;
}

uchar TemplateMatcher::getCannyThreshold2() const {
    return cannyThreshold2;
}

uchar TemplateMatcher::getSobelMaxThreshold() const {
    return sobelMaxThreshold;
}

uchar TemplateMatcher::getGrayscaleMinThreshold() const {
    return grayscaleMinThreshold;
}

void TemplateMatcher::setFeaturePointsCount(uint featurePointsCount) {
    assert(featurePointsCount > 0);
    this->featurePointsCount = featurePointsCount;
}

void TemplateMatcher::setCannyThreshold1(uchar cannyThreshold1) {
    assert(featurePointsCount > 0);
    assert(featurePointsCount < 256);
    this->cannyThreshold1 = cannyThreshold1;
}

void TemplateMatcher::setCannyThreshold2(uchar cannyThreshold2) {
    assert(featurePointsCount > 0);
    assert(featurePointsCount < 256);
    this->cannyThreshold2 = cannyThreshold2;
}

void TemplateMatcher::setSobelMaxThreshold(uchar sobelMaxThreshold) {
    assert(featurePointsCount > 0);
    assert(featurePointsCount < 256);
    this->sobelMaxThreshold = sobelMaxThreshold;
}

void TemplateMatcher::setGrayscaleMinThreshold(uchar grayscaleMinThreshold) {
    assert(featurePointsCount > 0);
    assert(featurePointsCount < 256);
    this->grayscaleMinThreshold = grayscaleMinThreshold;
}
