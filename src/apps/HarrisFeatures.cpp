/**
 * @file HarrisFeatures.cpp
 *
 * @brief Task 7: robust feature detection using Harris corners
 *
 * Implements the full Harris pipeline:
 *   1. Compute the Harris response H at each pixel
 *   2. Threshold H to remove weak responses
 *   3. Non-maxima suppression to keep one point per corner
 *   4. Order by H value, keep the top N
 * The threshold and N can be changed live for experimentation.
 *
 * Usage: HarrisFeatures [camera-index | video-file]
 *
 * @author Yu Cao
 * @date 2026-03-12
 */
#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "AppUtils.h"

namespace
{
    const char *WINDOW_NAME = "Harris Features";

    /**
     * @brief Harris detector settings
     */
    struct HarrisParams
    {
        int block_size = 2;      // neighborhood size for the covariance matrix A
        int ksize = 3;           // Sobel kernel size for the gradients
        double k = 0.04;         // alpha in H = det(A) - alpha * trace(A)^2
        float threshold = 0.05f; // Step 2: minimum response, as a fraction of the maximum
        int top_n = 500;         // Step 4: keep only the N strongest corners
    };

    /**
     * @brief Detect Harris corners and draw them as red circles on the frame
     *
     *        H = det(A) - alpha * trace(A)^2 = l0*l1 - alpha*(l0+l1)^2
     *        where A is the covariance matrix of the gradients over a window and l0, l1 are
     *        its eigenvalues.
     *
     * @param[in,out] frame   BGR input frame; corners are drawn on it in place
     * @param[in]     params  Detector settings
     * @return the number of corners drawn
     */
    int detectAndDrawHarris(cv::Mat &frame, const HarrisParams &params)
    {
        cv::Mat gray, response, local_max;

        // Harris works on the image gradients Ix, Iy
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        gray.convertTo(gray, CV_32F);

        // Step 1: compute H at every pixel. H is large and positive at corners, negative
        // along edges and close to 0 in flat regions.
        cv::cornerHarris(gray, response, params.block_size, params.ksize, params.k);

        // Step 2: discard pixels whose response is below threshold * the strongest response.
        // The threshold is taken from the raw H: rescaling H to [0, 255] first would map the
        // most negative edge response to 0 and let flat regions pass.
        double max_response = 0.0;
        cv::minMaxLoc(response, nullptr, &max_response);
        const float threshold = static_cast<float>(std::max(params.threshold * max_response, 0.0));

        // Step 3: non-maxima suppression. A corner region has many pixels with a good
        // response, but only one point per corner is wanted, so keep only pixels equal to the
        // maximum of their 3x3 neighborhood (the dilated response)
        cv::dilate(response, local_max, cv::Mat());
        std::vector<std::pair<float, cv::Point>> candidates;
        for (int r = 0; r < response.rows; r++)
        {
            const float *h = response.ptr<float>(r);
            const float *h_max = local_max.ptr<float>(r);
            for (int c = 0; c < response.cols; c++)
            {
                if (h[c] > threshold && h[c] >= h_max[c])
                {
                    candidates.push_back(std::make_pair(h[c], cv::Point(c, r)));
                }
            }
        }

        // Step 4: order by H value, keep only the top N
        std::sort(candidates.begin(), candidates.end(),
                  [](const std::pair<float, cv::Point> &a, const std::pair<float, cv::Point> &b) {
                      return a.first > b.first;
                  });

        int draw_count = std::min(static_cast<int>(candidates.size()), params.top_n);
        for (int i = 0; i < draw_count; i++)
        {
            cv::circle(frame, candidates[i].second, 4, cv::Scalar(0, 0, 255), 1);
        }

        // show the current settings and the feature counts
        char label[128];
        std::snprintf(label, sizeof(label), "Threshold: %.3f | Top N: %d | Found: %d | Shown: %d",
                      params.threshold, params.top_n, static_cast<int>(candidates.size()),
                      draw_count);
        cv::putText(frame, label, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7,
                    cv::Scalar(0, 255, 255), 2);

        return draw_count;
    }
} // namespace

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::printf("Usage: HarrisFeatures [camera-index | video-file]  (default: camera 0)\n");
        return 1;
    }

    // open the source before changing directory, so relative video paths still work
    cv::VideoCapture capture;
    if (!openVideoSource(argc == 2 ? argv[1] : "", capture))
    {
        std::printf("Error: cannot open video source %s\n", argc == 2 ? argv[1] : "0");
        return 1;
    }
    chdirToExecutableDir();

    std::printf("Controls:\n");
    std::printf("  'i' - increase threshold (fewer candidates)\n");
    std::printf("  'd' - decrease threshold (more candidates)\n");
    std::printf("  'n' - increase top N\n");
    std::printf("  'm' - decrease top N\n");
    std::printf("  's' - save a screenshot\n");
    std::printf("  'q' - quit\n\n");

    HarrisParams params;
    cv::Mat frame;
    while (true)
    {
        capture >> frame;
        if (frame.empty())
        {
            break;
        }

        detectAndDrawHarris(frame, params);

        cv::Mat display = frame.clone();
        drawHelpText(display, "[i/d] threshold +/-   [n/m] top N +/-   [s] screenshot   [q] quit");
        cv::imshow(WINDOW_NAME, display);

        int key = cv::waitKey(10);
        if (key == 'q' || key == 27)
        {
            break;
        }
        else if (key == 'i')
        {
            params.threshold = std::min(params.threshold + 0.01f, 0.5f);
            std::printf("Threshold: %.3f\n", params.threshold);
        }
        else if (key == 'd')
        {
            params.threshold = std::max(params.threshold - 0.01f, 0.001f);
            std::printf("Threshold: %.3f\n", params.threshold);
        }
        else if (key == 'n')
        {
            params.top_n += 100;
            std::printf("Top N: %d\n", params.top_n);
        }
        else if (key == 'm')
        {
            params.top_n = std::max(params.top_n - 100, 100);
            std::printf("Top N: %d\n", params.top_n);
        }
        else if (key == 's')
        {
            // the settings are part of the name, e.g. harris_thresh50_top500_001.png
            std::string prefix = "harris_thresh" +
                                 std::to_string(static_cast<int>(params.threshold * 1000)) +
                                 "_top" + std::to_string(params.top_n);
            std::string path = saveImage(prefix, frame);
            if (!path.empty())
            {
                std::printf("Screenshot saved to %s\n", path.c_str());
            }
        }
    }

    cv::destroyAllWindows();
    return 0;
}
