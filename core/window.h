#ifndef VSB_SEMESTRAL_PROJECT_WINDOW_H
#define VSB_SEMESTRAL_PROJECT_WINDOW_H

#include <opencv2/core/types.hpp>
#include <memory>
#include "template.h"

namespace tless {
    /**
     * @brief Contains location of windows that passed objectness detection
     */
    class Window {
    public:
        int x;
        int y;
        int width;
        int height;
        int edgels;
        std::vector<std::shared_ptr<Template>> candidates;

        // Constructors
        explicit Window(int x = 0, int y = 0, int width = 0, int height = 0, int edgels = 0)
                : x(x), y(y), width(width), height(height), edgels(edgels) {}

        // Methods
        cv::Point tl();
        cv::Point tr();
        cv::Point bl();
        cv::Point br();
        cv::Size size();
        bool hasCandidates();

        // Overloads
        bool operator==(const Window &rhs) const;
        bool operator!=(const Window &rhs) const;
        bool operator<(const Window &rhs) const;
        bool operator>(const Window &rhs) const;
        bool operator<=(const Window &rhs) const;
        bool operator>=(const Window &rhs) const;
        friend std::ostream &operator<<(std::ostream &os, const Window &w);
    };
}

#endif
