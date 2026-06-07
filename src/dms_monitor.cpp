#include "dms_monitor.h"
#include <cmath>
#include <iostream>

DMSMonitor::DMSMonitor() {}

bool DMSMonitor::initialize(const std::string& face_prototxt,
                            const std::string& face_model,
                            const std::string& eyes_cascade_path) {
    net = cv::dnn::readNetFromCaffe(face_prototxt, face_model);
    if (net.empty()) {
        std::cerr << "ERROR: Face detection model not loaded\n";
        return false;
    }
    if (!eyes_cascade.load(eyes_cascade_path)) {
        std::cerr << "ERROR: Eyes cascade not loaded\n";
        return false;
    }
    return true;
}

DriverState DMSMonitor::analyze(cv::Mat& frame) {
    DriverState state;
    state.face_rect = detectFace(frame);
    state.face_detected = (state.face_rect.width > 0 && state.face_rect.height > 0);
    if (state.face_detected) {
        state.eye_openness = estimateEyeOpenness(frame, state.face_rect);
        state.eyes_open = (state.eye_openness > 0.3f);
        state.head_turn_deg = estimateHeadTurn(state.face_rect, frame.size());
        state.looking_forward = (std::abs(state.head_turn_deg) < 20.0f);
        eye_history.push_back(!state.eyes_open);
        if (eye_history.size() > HISTORY_SIZE) eye_history.pop_front();
        int closed_count = std::count(eye_history.begin(), eye_history.end(), true);
        state.alert_drowsy = (closed_count >= CLOSED_THRESHOLD);
        state.alert_distracted = !state.looking_forward;
    }
    return state;
}

cv::Rect DMSMonitor::detectFace(const cv::Mat& frame) {
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300,300),
                                          cv::Scalar(104.0,177.0,123.0));
    net.setInput(blob);
    cv::Mat detections = net.forward();
    cv::Rect bestRect;
    for (int i = 0; i < detections.size[2]; ++i) {
        float* data = (float*)detections.data + i*7;
        float confidence = data[2];
        if (confidence > 0.5) {
            int x1 = int(data[3]*frame.cols);
            int y1 = int(data[4]*frame.rows);
            int x2 = int(data[5]*frame.cols);
            int y2 = int(data[6]*frame.rows);
            bestRect = cv::Rect(x1, y1, x2-x1, y2-y1);
            break;
        }
    }
    return bestRect;
}

float DMSMonitor::estimateEyeOpenness(const cv::Mat& frame, const cv::Rect& face) {
    if (face.width<=0 || face.height<=0) return 0.0f;
    cv::Mat faceROI = frame(face);
    cv::Rect eyesROI(0,0,faceROI.cols, faceROI.rows/2);
    cv::Mat eyesArea = faceROI(eyesROI);
    std::vector<cv::Rect> eyes;
    eyes_cascade.detectMultiScale(eyesArea, eyes);
    return float(eyes.size()) / 2.0f;
}

float DMSMonitor::estimateHeadTurn(const cv::Rect& face, const cv::Size& frame_size) {
    if (frame_size.width==0) return 0.0f;
    int frame_center_x = frame_size.width/2;
    int face_center_x = face.x + face.width/2;
    int offset = face_center_x - frame_center_x;
    float turn_deg = (offset/150.0f)*45.0f;
    turn_deg = std::max(-45.0f, std::min(45.0f, turn_deg));
    return turn_deg;
}