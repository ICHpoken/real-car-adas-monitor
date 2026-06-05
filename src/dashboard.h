#pragma once

#include <opencv2/opencv.hpp>
#include <string>

// Структура для передачи данных на панель
struct DashboardData {
    float speed_kmh = 0.0f;       // 0-140 км/ч
    float rpm = 0.0f;             // 0-6000 RPM
    float coolant_temp = 90.0f;   // 0-120 °C
    float fuel_level = 50.0f;     // 0-100 %
    float throttle_pos = 30.0f;   // 0-100 %
    int driving_style = 1;        // 0=SLOW, 1=NORMAL, 2=AGGRESSIVE
};

class Dashboard {
public:
    Dashboard(int width = 640, int height = 480);
    
    // Главный метод: рисует панель на переданном изображении
    void draw(cv::Mat& image, const DashboardData& data);
    
private:
    int panel_width_, panel_height_;
    cv::Point center_;             // центр для круговых приборов
    int gauge_radius_;
    
    // Вспомогательные методы
    void drawGauge(cv::Mat& img, const cv::Point& center, int radius,
                   float value, float minVal, float maxVal,
                   const std::string& title, const std::string& unit,
                   const cv::Scalar& goodColor, const cv::Scalar& warnColor,
                   float warnThreshold);
    
    void drawLinearGauge(cv::Mat& img, const cv::Rect& rect,
                         float value, float minVal, float maxVal,
                         const std::string& title, const std::string& unit,
                         const cv::Scalar& goodColor, const cv::Scalar& warnColor,
                         float warnThreshold);
    
    void drawWarning(cv::Mat& img, const std::string& text, const cv::Scalar& color, const cv::Point& position);
};