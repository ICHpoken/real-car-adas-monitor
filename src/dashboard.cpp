#include "dashboard.h"
#include <cmath>

Dashboard::Dashboard(int width, int height)
    : panel_width_(width), panel_height_(height) {
    center_ = cv::Point(panel_width_ / 2, panel_height_ / 3);
    gauge_radius_ = std::min(panel_width_, panel_height_) / 4;
}

void Dashboard::drawGauge(cv::Mat& img, const cv::Point& center, int radius,
                          float value, float minVal, float maxVal,
                          const std::string& title, const std::string& unit,
                          const cv::Scalar& goodColor, const cv::Scalar& warnColor,
                          float warnThreshold) {
    // Фон круга
    cv::circle(img, center, radius, cv::Scalar(50, 50, 50), -1);
    cv::circle(img, center, radius, cv::Scalar(100, 100, 100), 2);
    
    // Углы для шкалы (от -150 до +150 градусов)
    const float startAngle = -150.0f;
    const float endAngle = 150.0f;
    float angleRange = endAngle - startAngle;
    float angle = startAngle + (value - minVal) / (maxVal - minVal) * angleRange;
    
    // Цвет дуги
    cv::Scalar arcColor = (value >= warnThreshold) ? warnColor : goodColor;
    cv::ellipse(img, center, cv::Size(radius, radius), 0, startAngle, angle, arcColor, 8);
    
    // Рисуем деления шкалы (риски и подписи)
    int numTicks = 9;  // количество делений (0, 25, 50, 75, 100% от диапазона)
    for (int i = 0; i <= numTicks; ++i) {
        float tickValue = minVal + (maxVal - minVal) * i / numTicks;
        float tickAngle = startAngle + (tickValue - minVal) / (maxVal - minVal) * angleRange;
        float rad = tickAngle * CV_PI / 180.0f;
        
        // Длина риски
        int tickLength = (i % 2 == 0) ? 12 : 8;  // длинные на каждое второе деление
        cv::Point outer(center.x + (radius - 5) * cos(rad),
                        center.y + (radius - 5) * sin(rad));
        cv::Point inner(center.x + (radius - 5 - tickLength) * cos(rad),
                        center.y + (radius - 5 - tickLength) * sin(rad));
        cv::line(img, outer, inner, cv::Scalar(200, 200, 200), 2, cv::LINE_AA);
        
        // Подпись значения (только для чётных i, чтобы не загромождать)
        if (i % 2 == 0) {
            char label[16];
            snprintf(label, sizeof(label), "%.0f", tickValue);
            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.4;
            int thickness = 1;
            cv::Size textSize = cv::getTextSize(label, fontFace, fontScale, thickness, nullptr);
            cv::Point textPos(center.x + (radius - 22) * cos(rad) - textSize.width/2,
                              center.y + (radius - 22) * sin(rad) + textSize.height/2);
            cv::putText(img, label, textPos, fontFace, fontScale, cv::Scalar(220, 220, 220), thickness);
        }
    }
    
    // Стрелка
    float rad = angle * CV_PI / 180.0f;
    cv::Point tip(center.x + (radius - 12) * cos(rad),
                  center.y + (radius - 12) * sin(rad));
    cv::line(img, center, tip, cv::Scalar(200, 200, 200), 4, cv::LINE_AA);
    // Маленький кружок в центре
    cv::circle(img, center, 6, cv::Scalar(200, 200, 200), -1);
    cv::circle(img, center, 3, cv::Scalar(50, 50, 50), -1);
    
    // Текущее значение крупно
    char text[32];
    snprintf(text, sizeof(text), "%.0f %s", value, unit.c_str());
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.7;
    int thickness = 2;
    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, nullptr);
    cv::Point textPos(center.x - textSize.width/2, center.y + radius - 15);
    cv::putText(img, text, textPos, fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
    
    // Название прибора
    cv::putText(img, title, cv::Point(center.x - 35, center.y - radius - 5),
                fontFace, 0.5, cv::Scalar(200, 200, 200), 1);
}

void Dashboard::drawLinearGauge(cv::Mat& img, const cv::Rect& rect,
                                float value, float minVal, float maxVal,
                                const std::string& title, const std::string& unit,
                                const cv::Scalar& goodColor, const cv::Scalar& warnColor,
                                float warnThreshold) {
    // Фон полосы
    cv::rectangle(img, rect, cv::Scalar(80, 80, 80), -1);
    cv::rectangle(img, rect, cv::Scalar(150, 150, 150), 1);
    
    int fillWidth = static_cast<int>((value - minVal) / (maxVal - minVal) * rect.width);
    fillWidth = std::clamp(fillWidth, 0, rect.width);
    cv::Rect fillRect(rect.x, rect.y, fillWidth, rect.height);
    cv::Scalar color = (value >= warnThreshold) ? warnColor : goodColor;
    cv::rectangle(img, fillRect, color, -1);
    
    // Текст
    char text[32];
    snprintf(text, sizeof(text), "%s: %.0f %s", title.c_str(), value, unit.c_str());
    cv::putText(img, text, cv::Point(rect.x, rect.y - 5),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
}

void Dashboard::draw(cv::Mat& image, const DashboardData& data) {
    // Очищаем левую половину
    cv::Rect leftRect(0, 0, panel_width_, panel_height_);
    cv::rectangle(image, leftRect, cv::Scalar(30, 30, 40), -1);
    
    // Уменьшаем радиус круговых приборов
    int radius = gauge_radius_ * 0.75;  // было 0.9
    
    // Разносим центры по горизонтали
    int margin = radius + 30;  // отступ от края
    cv::Point speedCenter(margin, panel_height_ / 3);
    cv::Point rpmCenter(panel_width_ - margin, panel_height_ / 3);
    
    drawGauge(image, speedCenter, radius, data.speed_kmh, 0, 140,
              "Speed", "km/h", cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), 90.0f);
    
    drawGauge(image, rpmCenter, radius, data.rpm, 0, 6000,
              "RPM", "rpm", cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), 4500.0f);
    
    // Линейные шкалы – смещаем ниже
    int gaugeY = panel_height_ / 2 + 60;  // было +30
    int barHeight = 20;
    int barWidth = panel_width_ - 80;     // уже, с отступами по бокам
    int startX = 40;                      // левый отступ
    
    cv::Rect tempRect(startX, gaugeY, barWidth, barHeight);
    drawLinearGauge(image, tempRect, data.coolant_temp, 0, 120,
                    "Coolant", "C", cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), 100.0f);
    gaugeY += barHeight + 15;
    
    cv::Rect fuelRect(startX, gaugeY, barWidth, barHeight);
    drawLinearGauge(image, fuelRect, data.fuel_level, 0, 100,
                    "Fuel", "%", cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), 15.0f);
    gaugeY += barHeight + 15;
    
    cv::Rect throttleRect(startX, gaugeY, barWidth, barHeight);
    drawLinearGauge(image, throttleRect, data.throttle_pos, 0, 100,
                    "Throttle", "%", cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255), 80.0f);
    
    // Стиль вождения (оставляем внизу слева)
    std::string styleText;
    cv::Scalar styleColor;
    switch (data.driving_style) {
        case 0: styleText = "SLOW"; styleColor = cv::Scalar(255, 200, 0); break;
        case 1: styleText = "NORMAL"; styleColor = cv::Scalar(0, 255, 0); break;
        case 2: styleText = "AGGRESSIVE"; styleColor = cv::Scalar(0, 0, 255); break;
        default: styleText = "UNKNOWN"; styleColor = cv::Scalar(200, 200, 200);
    }
    cv::putText(image, "Driving Style: " + styleText,
                cv::Point(20, panel_height_ - 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, styleColor, 2);
    
    // Предупреждения
    if (data.coolant_temp > 100.0f) {
        cv::putText(image, "HIGH TEMP!", cv::Point(panel_width_ - 130, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
    }
    if (data.fuel_level < 15.0f) {
        cv::putText(image, "LOW FUEL!", cv::Point(panel_width_ - 130, 60),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
    }
}

void Dashboard::drawWarning(cv::Mat& img, const std::string& text, const cv::Scalar& color, const cv::Point& position) {
    cv::putText(img, text, position, cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2, cv::LINE_AA);
}