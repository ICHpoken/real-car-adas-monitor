#include <iostream>
#include <opencv2/opencv.hpp>

#include "dms_monitor.h"
#include "dms_hud.h"

int main() {
    std::cout << "=== DMS Monitor Test ===" << std::endl;
    std::cout << "Controls: 'Q' or ESC to exit" << std::endl;

    // Инициализация монитора (пути к моделям)
    DMSMonitor dms("../models/deploy.prototxt", 
                   "../models/res10_300x300_ssd_iter_140000.caffemodel",
                   "../models/haarcascade_eye.xml");

    if (!dms.isLoaded()) {
        std::cerr << "Critical Error: DMS models failed to load!\n";
        return 1;
    }

    // Открытие веб-камеры (0 - камера по умолчанию)
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open webcam!\n";
        return 1;
    }

    DMSHUD hud;
    cv::Mat frame;

    std::cout << "Webcam opened. Starting analysis...\n";

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Анализ кадра
        DriverState state = dms.analyze(frame);

        // Отрисовка HUD поверх кадра
        hud.draw(frame, state);

        // Показ окна
        cv::imshow("Driver Monitoring System (DMS)", frame);

        // Обработка клавиш
        int key = cv::waitKey(30) & 0xFF; // Ждем 30 мс (~30 FPS)
        if (key == 'q' || key == 'Q' || key == 27) { // 27 - код клавиши ESC
            std::cout << "Exiting DMS test...\n";
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}