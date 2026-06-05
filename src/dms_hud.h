#pragma once
#include <opencv2/opencv.hpp>
#include "dms_monitor.h"

// Класс для отрисовки HUD (Head-Up Display) состояния водителя
class DMSHUD {
public:
    // Рисует состояние водителя поверх кадра с камеры
    void draw(cv::Mat& frame, const DriverState& state);
};