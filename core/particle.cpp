#include "particle.h"
#include "../processing/processing.h"
#include <glm/ext.hpp>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <random>

namespace tless {
    float Particle::velocity(float w, float v, float x, float pBest, float gBest, float c1, float c2, float r1, float r2) {
        return w * v + (c1 * r1) * (pBest - x) + (c2 * r2) * (gBest - x);
    }

    Particle::Particle(float tx, float ty, float tz, float rx, float ry, float rz,
                       float v1, float v2, float v3, float v4, float v5, float v6) {
        this->tx = tx;
        this->ty = ty;
        this->tz = tz;
        this->rx = rx;
        this->ry = ry;
        this->rz = rz;
        this->v1 = v1;
        this->v2 = v2;
        this->v3 = v3;
        this->v4 = v4;
        this->v5 = v5;
        this->v6 = v6;

        // Init pBest
        updatePBest();
    };

    float Particle::nextR() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> d(0, 1.0f);

        return d(gen);
    }

    glm::mat4 Particle::model()  {
        glm::mat4 m;
        glm::vec3 t(tx, ty, tz);

        // Rotate
        m = glm::rotate(m, rx, glm::vec3(1, 0, 0));
        m = glm::rotate(m, ry, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rz, glm::vec3(0, 0, 1));

        // Translate
        return glm::translate(m, t);
    }

    void Particle::updatePBest() {
        std::memcpy(pBest.v, v, sizeof v);
        std::memcpy(pBest.pose, pose, sizeof pose);
        pBest.fitness = fitness;
    }

    void Particle::progress(float w, float c1, float c2, const Particle &gBest) {
        // Calculate new velocity
        for (int i = 0; i < 3; i++) {
            v[i] = velocity(w, v[i], pose[i], pBest.pose[i], gBest.pose[i], c1, c2, nextR(), nextR());
        }
        for (int i = 3; i < 6; i++) {
            v[i] = velocity(0.96f, v[i], pose[i], pBest.pose[i], gBest.pose[i], c1, c2, nextR(), nextR());
        }

        // Update current possition with new velocity
        for (int i = 0; i < 6; i++) {
            pose[i] = v[i] + pose[i];
        }

        for (int i = 3; i < 6; i++) {
            if (std::abs(v[i]) > 1) {
                std::cout << v[i] << std::endl;
            }
        }
    }

    float Particle::objFun(const cv::Mat &gt, const cv::Mat &gtNormals, const cv::Mat &gtEdges, const cv::Mat &pose,
                            const cv::Mat &poseNormals) {
        float sumD = 0, sumU = 0, sumE = 0;
        const float tD = 200;
        const float inf = std::numeric_limits<float>::max();

        // Compute edges
        cv::Mat poseT, poseEdges;
        cv::Laplacian(pose, poseEdges, -1);
        cv::threshold(poseEdges, poseEdges, 20, 255, CV_THRESH_BINARY_INV);
        poseEdges.convertTo(poseEdges, CV_8U);
        cv::distanceTransform(poseEdges, poseT, CV_DIST_L2, 3);

        cv::Mat matD = cv::Mat::zeros(gt.size(), CV_32FC1);
        cv::Mat matE = cv::Mat::zeros(gt.size(), CV_32FC1);
        cv::Mat matU = cv::Mat::zeros(gt.size(), CV_32FC1);

//        cv::normalize(poseT, poseT, 0, 1, CV_MINMAX);
//        cv::imshow("poseEdges", poseEdges);
//        cv::imshow("distance", poseT);
//        cv::waitKey(1);

        for (int y = 0; y < gt.rows; y++) {
            for (int x = 0; x < gt.cols; x++) {
                // Compute distance transform
                if (gtEdges.at<uchar>(y, x) > 0) {
                    sumE += 1 / (poseT.at<float>(y, x) + 1);
                    matE.at<float>(y, x) = 1 / (poseT.at<float>(y, x) + 1);
                }

//                // Skip invalid depth pixels for other tests pixels
//                if (pose.at<float>(y, x) <= 0) {
//                    continue;
//                }

                // Compute depth diff
                float dDiff = std::abs(gt.at<float>(y, x) - pose.at<float>(y, x));
                sumD += (dDiff > tD) ? (1 / (inf + 1)) : (1 / (dDiff + 1));
                matD.at<float>(y, x) = (dDiff > tD) ? (1 / (inf + 1)) : (1 / (dDiff + 1));

                // Compare normals
                float dot = std::abs(gtNormals.at<cv::Vec3f>(y, x).dot(poseNormals.at<cv::Vec3f>(y, x)));
                sumU += std::isnan(dot) ? (1 / (inf + 1)) : (1 / (dot + 1));
                matU.at<float>(y, x) = std::isnan(dot) ? (1 / (9999999 + 1)) : (1 / (dot + 1));
            }
        }

//        cv::normalize(matU, matU, 0, 1, CV_MINMAX);
//        cv::normalize(matE, matE, 0, 1, CV_MINMAX);
//        cv::normalize(matD, matD, 0, 1, CV_MINMAX);
//
//        cv::imshow("matU", matU);
//        cv::imshow("matE", matE);
//        cv::imshow("matD", matD);
//        cv::waitKey(1);

        return -sumD * 1 * sumE;
    }

    std::ostream &tless::operator<<(std::ostream &os, const Particle &particle) {
        os << "fitness: " << particle.fitness << " v1: " << particle.v1 << " v2: " << particle.v2 << " v3: "
           << particle.v3 << " v4: " << particle.v4 << " v5: " << particle.v5 << " v6: " << particle.v6 << " v: "
           << particle.v << " tx: " << particle.tx << " ty: " << particle.ty << " tz: " << particle.tz << " rx: "
           << particle.rx << " ry: " << particle.ry << " rz: " << particle.rz;
        return os;
    }
}