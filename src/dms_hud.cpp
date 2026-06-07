#include "dms_hud.h"

void DMSHUD::draw(cv::Mat& full_frame, const DriverState& state, const cv::Mat& camera_frame) {
    int hud_width = full_frame.cols / 2;
    int hud_height = full_frame.rows;
    cv::Mat camera_resized;
    cv::resize(camera_frame, camera_resized, cv::Size(hud_width, hud_height));
    camera_resized.copyTo(full_frame(cv::Rect(hud_width, 0, hud_width, hud_height)));

    cv::Mat hud_area = full_frame(cv::Rect(hud_width, 0, hud_width, hud_height));
    if (state.face_detected) {
        cv::Rect shifted_rect(state.face_rect.x - hud_width, state.face_rect.y,
                              state.face_rect.width, state.face_rect.height);
        shifted_rect = shifted_rect & cv::Rect(0, 0, hud_width, hud_height);
        cv::Scalar face_color = (state.alert_drowsy || state.alert_distracted) ?
                                cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
        cv::rectangle(hud_area, shifted_rect, face_color, 2);
    }

    cv::putText(hud_area, "Driver Status:", cv::Point(10,30), cv::FONT_HERSHEY_SIMPLEX,0.6, cv::Scalar(255,255,255),1);
    cv::Scalar eye_color = state.eyes_open ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255);
    cv::putText(hud_area, std::string("Eyes: ")+ (state.eyes_open?"OPEN":"CLOSED"), cv::Point(10,60), cv::FONT_HERSHEY_SIMPLEX,0.5, eye_color,1);
    cv::Scalar head_color = state.looking_forward ? cv::Scalar(0,255,0) : cv::Scalar(0,0,255);
    cv::putText(hud_area, std::string("Head: ")+ (state.looking_forward?"FORWARD":"TURNED"), cv::Point(10,85), cv::FONT_HERSHEY_SIMPLEX,0.5, head_color,1);

    char openness_text[50], turn_text[50];
    snprintf(openness_text, sizeof(openness_text), "Openness: %.2f", state.eye_openness);
    snprintf(turn_text, sizeof(turn_text), "Head turn: %.1f deg", state.head_turn_deg);
    cv::putText(hud_area, openness_text, cv::Point(10,110), cv::FONT_HERSHEY_SIMPLEX,0.5, cv::Scalar(255,255,255),1);
    cv::putText(hud_area, turn_text, cv::Point(10,135), cv::FONT_HERSHEY_SIMPLEX,0.5, cv::Scalar(255,255,255),1);

    if (state.alert_drowsy) {
        cv::rectangle(hud_area, cv::Point(0,0), cv::Point(hud_width, hud_height), cv::Scalar(0,0,255),5);
        cv::putText(hud_area, "DROWSINESS ALERT!", cv::Point(40, hud_height/2), cv::FONT_HERSHEY_SIMPLEX,0.9, cv::Scalar(0,0,255),2);
    }
    if (state.alert_distracted) {
        cv::rectangle(hud_area, cv::Point(0, hud_height-30), cv::Point(hud_width, hud_height), cv::Scalar(0,0,255), cv::FILLED);
        cv::putText(hud_area, "DISTRACTION", cv::Point(20, hud_height-10), cv::FONT_HERSHEY_SIMPLEX,0.5, cv::Scalar(255,255,255),1);
    }
}