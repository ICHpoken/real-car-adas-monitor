#include "dms_hud.h"
#include <cstdio>

// Отрисовка угловых скобок (вместо полного прямоугольника)
static void drawCornerBox(cv::Mat& frame, cv::Rect r, const cv::Scalar& color, int thickness) {
    int cornerLen = 30;
    // Верхний левый угол
    cv::line(frame, cv::Point(r.x, r.y), cv::Point(r.x + cornerLen, r.y), color, thickness, cv::LINE_AA);
    cv::line(frame, cv::Point(r.x, r.y), cv::Point(r.x, r.y + cornerLen), color, thickness, cv::LINE_AA);
    // Верхний правый угол
    cv::line(frame, cv::Point(r.br().x, r.y), cv::Point(r.br().x - cornerLen, r.y), color, thickness, cv::LINE_AA);
    cv::line(frame, cv::Point(r.br().x, r.y), cv::Point(r.br().x, r.y + cornerLen), color, thickness, cv::LINE_AA);
    // Нижний левый угол
    cv::line(frame, cv::Point(r.x, r.br().y), cv::Point(r.x + cornerLen, r.br().y), color, thickness, cv::LINE_AA);
    cv::line(frame, cv::Point(r.x, r.br().y), cv::Point(r.x, r.br().y - cornerLen), color, thickness, cv::LINE_AA);
    // Нижний правый угол
    cv::line(frame, cv::Point(r.br().x, r.br().y), cv::Point(r.br().x - cornerLen, r.br().y), color, thickness, cv::LINE_AA);
    cv::line(frame, cv::Point(r.br().x, r.br().y), cv::Point(r.br().x, r.br().y - cornerLen), color, thickness, cv::LINE_AA);
}

void DMSHUD::draw(cv::Mat& frame, const DriverState& state) {
    // Определяем основной цвет рамки лица
    cv::Scalar faceColor = cv::Scalar(0, 220, 0); // Зеленый по умолчанию
    if (state.alert_drowsy) faceColor = cv::Scalar(0, 165, 255);   // Оранжевый
    if (state.alert_distracted) faceColor = cv::Scalar(0, 0, 255); // Красный

    // 1. Рисуем угловые скобки вокруг лица
    if (state.face_detected) {
        drawCornerBox(frame, state.face_rect, faceColor, 3);
    }

    // 2. Панель статуса в левом верхнем углу
    int y = 30;
    int x = 20;
    auto drawLine = [&](const std::string& text, const cv::Scalar& col) {
        cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.6, col, 2, cv::LINE_AA);
        y += 30;
    };

    drawLine("Face: " + std::string(state.face_detected ? "DETECTED" : "NOT FOUND"), 
             state.face_detected ? cv::Scalar(0, 220, 0) : cv::Scalar(0, 0, 255));
             
    drawLine("Eyes: " + std::string(state.eyes_open ? "OPEN" : "CLOSED"), 
             state.eyes_open ? cv::Scalar(0, 220, 0) : cv::Scalar(0, 0, 255));
             
    char buf[64];
    snprintf(buf, sizeof(buf), "Head turn: %.1f deg", state.head_turn_deg);
    drawLine(buf, cv::Scalar(200, 200, 200));

    drawLine("Status: " + std::string(state.alert_drowsy || state.alert_distracted ? "ALERT!" : "NORMAL"),
             (state.alert_drowsy || state.alert_distracted) ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 220, 0));

    // 3. Глобальные алерты (плашки)
    if (state.alert_drowsy) {
        cv::rectangle(frame, cv::Rect(0, frame.rows / 2 - 40, frame.cols, 80), 
                      cv::Scalar(0, 165, 255), -1, cv::LINE_AA);
        cv::putText(frame, "DROWSINESS ALERT!", cv::Point(frame.cols / 2 - 150, frame.rows / 2 + 10),
                    cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(255, 255, 255), 3, cv::LINE_AA);
    }

    if (state.alert_distracted) {
        cv::rectangle(frame, cv::Rect(0, frame.rows - 60, frame.cols, 60), 
                      cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
        cv::putText(frame, "DISTRACTION DETECTED", cv::Point(frame.cols / 2 - 160, frame.rows - 20),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    }
}