#ifndef VSB_SEMESTRAL_PROJECT_MATCHER_H
#define VSB_SEMESTRAL_PROJECT_MATCHER_H

#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include "../core/window.h"
#include "../core/match.h"
#include "../core/group.h"
#include "../core/value_point.h"

/**
 * class Hasher
 *
 * Used to train HashTables and quickly verify what templates should be matched
 * per each window passed form objectness detection
 */
class Matcher {
private:
    uint pointsCount; // Number of feature points to extract for each template

    // Match thresholds
    float tMatch;
    float tOverlap;
    uchar tColorTest;

    // Extent of local neighbourhood
    cv::Range neighbourhood;

    // Methods
    cv::Vec3b normalizeHSV(const cv::Vec3b &hsv);
    void extractFeatures(std::vector<Group> &groups);
    void generateFeaturePoints(std::vector<Group> &groups);
    uchar quantizeOrientationGradient(float deg);
    void nonMaximaSuppression(std::vector<Match> &matches);
    void cherryPickFeaturePoints(std::vector<ValuePoint<float>> &points, double tMinDistance, uint pointsCount, std::vector<cv::Point> &out);

    // Tests
    bool testObjectSize(float scale); // Test I
    int testSurfaceNormal(uchar normal, Window &window, const cv::Mat &sceneDepth, const cv::Point &stable); // Test II
    int testGradients(uchar gradient, Window &window, const cv::Mat &sceneAngle, const cv::Mat &sceneMagnitude, const cv::Point &edge); // Test III
    int testDepth(int physicalDiameter, std::vector<int> &depths); // Test IV
    int testColor(cv::Vec3b HSV, Window &window, const cv::Mat &sceneHSV, const cv::Point &stable); // Test V
public:
    // Static methods
    static int median(std::vector<int> &values);

    // Constructor
    explicit Matcher(uint pointsCount = 100, float tMatch = 0.6f, float tOverlap = 0.1f, uchar tColorTest = 5, const cv::Range &neighbourhood = cv::Range(5, 5))
        : pointsCount(pointsCount), tMatch(tMatch), tOverlap(tOverlap), tColorTest(tColorTest), neighbourhood(neighbourhood) {}

    // Methods
    void match(const cv::Mat &sceneHSV, const cv::Mat &sceneGray, const cv::Mat &sceneDepth, std::vector<Window> &windows, std::vector<Match> &matches);
    void train(std::vector<Group> &groups);

    // Getters
    uint getPointsCount() const;
    float getTMatch() const;
    uchar getTColorTest() const;
    const cv::Range &getNeighbourhood() const;

    float getTOverlap() const;
    void setTOverlap(float tOverlap);

    // Setters
    void setPointsCount(uint count);
    void setTMatch(float tMatch);
    void setTColorTest(uchar tColorTest);
    void setNeighbourhood(const cv::Range &neighbourhood);
};

#endif //VSB_SEMESTRAL_PROJECT_MATCHER_H
