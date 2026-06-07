#ifndef DMS_MONITOR_H
#define DMS_MONITOR_H

#include <opencv2/opencv.hpp>
#include <deque>

struct DriverState {
    bool face_detected = false;
    bool eyes_open = false;
    bool looking_forward = true;
    float eye_openness = 0.0f;
    float head_turn_deg = 0.0f;
    bool alert_drowsy = false;
    bool alert_distracted = false;
    cv::Rect face_rect;
};

class DMSMonitor {
public:
    DMSMonitor();
    bool initialize(const std::string& face_prototxt,
                    const std::string& face_model,
                    const std::string& eyes_cascade_path);
    DriverState analyze(cv::Mat& frame);

private:
    cv::dnn::Net net;
    cv::CascadeClassifier eyes_cascade;
    std::deque<bool> eye_history;
    const int HISTORY_SIZE = 15;
    const int CLOSED_THRESHOLD = 10;

    cv::Rect detectFace(const cv::Mat& frame);
    float estimateEyeOpenness(const cv::Mat& frame, const cv::Rect& face);
    float estimateHeadTurn(const cv::Rect& face, const cv::Size& frame_size);
};

#endif