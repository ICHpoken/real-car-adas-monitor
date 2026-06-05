#pragma once
#include <opencv2/opencv.hpp>
#include <deque>
#include <string>

// Структура для хранения состояния водителя
struct DriverState {
    bool face_detected = false;
    bool eyes_open = true;
    bool looking_forward = true;
    float eye_openness = 1.0f;   // 0.0 (закрыты) - 1.0 (открыты)
    float head_turn_deg = 0.0f;  // Угол поворота головы в градусах
    bool alert_drowsy = false;
    bool alert_distracted = false;
    cv::Rect face_rect;          // Координаты найденного лица
};

// Класс мониторинга состояния водителя
class DMSMonitor {
public:
    // Конструктор загружает модели детекции
    DMSMonitor(const std::string& faceProto, const std::string& faceModel,
               const std::string& eyeCascade);
    
    // Главный метод: анализирует кадр и возвращает состояние водителя
    DriverState analyze(const cv::Mat& frame);
    
    // Проверка успешной загрузки моделей
    bool isLoaded() const { return loaded_; }

private:
    cv::dnn::Net faceNet_;               // Нейросеть для лиц
    cv::CascadeClassifier eyeCascade_;   // Каскад Хаара для глаз
    std::deque<bool> eyeHistory_;        // История открытия глаз (последние 15 кадров)
    bool loaded_ = false;

    // Приватные методы
    cv::Rect detectFace(const cv::Mat& frame);
    float estimateEyeOpenness(const cv::Mat& faceROI);
    float estimateHeadTurn(const cv::Rect& faceRect, int frameWidth);
};