// Global includes
#include <iostream>
#include <vector>

// OpenCV includes
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Definitions
int fix_perspective(cv::Mat& fixed_frame, const cv::Mat& frame);
void filter_frame(cv::Mat& filtered_frame, const cv::Mat& frame);

cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b);
void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center);


/**
 * Main function to take a calibrated picture (fix perspective etc) and output
 * a grayscale JPEG image where 0 is intact wood and 1 is marker trace.
 *
 * For this two work, you need a rectangle shape beneath your print. You can
 * either use a quadrilateral-shaped wooden board, or put a white paper below
 * your wooden board.
 *
 * Pass an argument to this program to output JPEG data to the specified file.
 * Pass it "-" or nothing to output to stdout.
 *
 * @return 0 on success, error-specific returned value otherwise.
 */
int main(int argc, const char** argv)
{
    // Create a capture to get pictures from camera
    CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY);
    if (!capture) {
        std::cerr << "No camera detected." << std::endl;
        return -1;
    }

    // Take a picture with the camera
    cv::Mat frame = cvQueryFrame(capture);
    if (frame.empty()) {
        std::cerr << "Invalid captured frame." << std::endl;
        return -2;
    }

    // Fix perspective
    cv::Mat fixed_perspective_frame;
    int result = fix_perspective(fixed_perspective_frame, frame);
    if (result) {
        std::cerr << "Unable to fix perspective: Error " << result << "." << std::endl;
        return -3;
    }

    // Filter the marker to get a black and white output image
    cv::Mat filtered_frame;
    filter_frame(filtered_frame, frame);

    if ((1 == argc) || (argc > 1 && 0 == strcmp("-", argv[1]))) {
        std::vector<uchar> jpeg_image;
        cv::imencode("jpg", filtered_frame, jpeg_image);
        for (int i = 0; i < jpeg_image.size(); ++i) {
            std::cout << jpeg_image[i];
        }
    } else {
        cv::imwrite(argv[1], filtered_frame);
    }

    return 0;
}


/******************
 * Main functions *
 ******************/

/**
 * Fix the perspective of an image.
 *
 * This function is based on
 * http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 *
 * @param[out] fixed_frame is the image with the fixed perspective, updated by
 * this function.
 * @param[in] frame is the raw input frame.
 *
 * @return 0 on success, error-specific non-zero value otherwise.
 */
int fix_perspective(cv::Mat& fixed_frame, const cv::Mat& frame) {
    // Convert image to grayscale to detect lines and so on
    cv::Mat bw_frame;
    cvtColor(frame, bw_frame, CV_RGB2GRAY);

    // Use Canny operator to get the edge map
    cv::Mat detected_edges;
    cv::blur(bw_frame, detected_edges, cv::Size(3,3));
    cv::Canny(detected_edges, detected_edges, 100, 100, 3);

    // Detect lines with Hough transform
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(detected_edges, lines, 1, CV_PI / 180.0, 70, 30, 10);

    // Compute intersections between the lines
    std::vector<cv::Point2f> corners;
    for (int i = 0; i < lines.size(); ++i) {
        for (int j = i + 1; j < lines.size(); ++j) {
            cv::Point2f pt = computeIntersect(lines[i], lines[j]);
            if (pt.x >= 0 && pt.y >= 0) {
                corners.push_back(pt);
            }
        }
    }

    // Check that the approximated polygonal has 4 vertices
    std::vector<cv::Point2f> approx;
    cv::approxPolyDP(
            cv::Mat(corners),
            approx,
            cv::arcLength(cv::Mat(corners), true) * 0.02,
            true);

    if (approx.size() != 4) {
        return -1;
    }

    // Sort corners: 0 is top-left, 1 is top-right etc.
    cv::Point2f center(0,0);
    for (int i = 0; i < corners.size(); ++i) {
        center += corners[i];
    }
    center *= (1. / corners.size());
    sortCorners(corners, center);

    // Apply transformations
    std::vector<cv::Point2f> dest_corners;  // Corners of the destination image
    dest_corners.push_back(cv::Point2f(0, 0));
    dest_corners.push_back(cv::Point2f(frame.cols, 0));
    dest_corners.push_back(cv::Point2f(frame.cols, frame.rows));
    dest_corners.push_back(cv::Point2f(0, frame.rows));
    cv::Mat transmtx = cv::getPerspectiveTransform(corners, dest_corners);  // Get transformation matrix
    cv::warpPerspective(frame, fixed_frame, transmtx, frame.size());  // Apply perspective transformation

    return 0;
}


/**
 * Filter the image to get the marker trace.
 *
 * @param[out] filtered_frame is the image after filtering, updated by
 * this function.
 * @param[in] frame is the raw input frame.
 *
 * @return void
 */
void filter_frame(cv::Mat& filtered_frame, const cv::Mat& frame) {
    // TODO
    filtered_frame = frame;
}


/***********************
 * Auxiliary functions *
 ***********************/

/**
 * Compute intersection between two lines described by a two-element vector
 * (\rho, \theta).
 *
 * \rho is the distance from the coordinate origin (0,0) (top-left corner of
 * the image).
 * \theta is the line rotation angle in radians (0 \sim \textrm{vertical line},
 * \pi/2 \sim \textrm{horizontal line}).
 *
 * @param a A two-element vector describing a line.
 * @param b A two-element vector describing a line.
 *
 * @return Coordinates (x, y) of the intersection point.
 */
cv::Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b) {
    int x1 = a[0];
    int y1 = a[1];
    int x2 = a[2];
    int y2 = a[3];

    int x3 = b[0];
    int y3 = b[1];
    int x4 = b[2];
    int y4 = b[3];

    float d = (static_cast<float>(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4));
    if (d) {
        cv::Point2f pt;
        pt.x = ((x1 *y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4)) / d;
        pt.y = ((x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4)) / d;
        return pt;
    }
    return cv::Point2f(-1, -1);
}


/**
 * Sort corners.
 *
 * @param corners The vector of corners to sort, modified in place.
 * @param[in] center  The center of the polygon.
 *
 * @return void.
 */
void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center) {
    std::vector<cv::Point2f> top, bot;

    for (int i = 0; i < corners.size(); ++i) {
        if (corners[i].y < center.y) {
            top.push_back(corners[i]);
        } else {
            bot.push_back(corners[i]);
        }
    }

    cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

    corners.clear();
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
}
