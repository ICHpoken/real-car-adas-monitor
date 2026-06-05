#include "dms_monitor.h"
#include <iostream>
#include <algorithm>

// Конструктор: загружает модели
DMSMonitor::DMSMonitor(const std::string& faceProto, const std::string& faceModel,
                       const std::string& eyeCascade) {
    try {
        // Загрузка DNN модели для лиц
        faceNet_ = cv::dnn::readNetFromCaffe(faceProto, faceModel);
        // Загрузка каскада Хаара для глаз
        eyeCascade_.load(eyeCascade);
        
        if (faceNet_.empty() || eyeCascade_.empty()) {
            std::cerr << "Warning: DMS models failed to load correctly!\n";
            loaded_ = false;
        } else {
            loaded_ = true;
            std::cout << "DMS Models loaded successfully.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading DMS models: " << e.what() << "\n";
        loaded_ = false;
    }
}

// Поиск лица на кадре
cv::Rect DMSMonitor::detectFace(const cv::Mat& frame) {
    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, cv::Size(300, 300),
                                          cv::Scalar(104, 177, 123));
    faceNet_.setInput(blob);
    cv::Mat detections = faceNet_.forward();

    cv::Mat detectionMat(detections.size[2], detections.size[3], CV_32F, detections.ptr<float>());
    
    cv::Rect bestFace;
    float bestConf = 0.5f; // Минимальная уверенность 50%

    for (int i = 0; i < detectionMat.rows; i++) {
        float confidence = detectionMat.at<float>(i, 2);
        if (confidence > bestConf) {
            bestConf = confidence;
            int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
            int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
            int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
            int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);
            bestFace = cv::Rect(x1, y1, x2 - x1, y2 - y1);
        }
    }
    return bestFace;
}

// Оценка открытости глаз (используем каскад Хаара в верхней половине лица)
float DMSMonitor::estimateEyeOpenness(const cv::Mat& faceROI) {
    cv::Mat grayFace;
    cv::cvtColor(faceROI, grayFace, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(grayFace, grayFace); // Выравнивание гистограммы для лучшего поиска

    // Берем только верхнюю половину лица (где находятся глаза)
    cv::Rect upperHalf(0, 0, grayFace.cols, grayFace.rows / 2);
    cv::Mat eyesROI = grayFace(upperHalf);

    std::vector<cv::Rect> eyes;
    // Ищем глаза
    eyeCascade_.detectMultiScale(eyesROI, eyes, 1.1, 2, 0, cv::Size(30, 30));

    if (eyes.empty()) {
        return 0.1f; // Глаза не найдены -> скорее всего закрыты
    }

    // Если глаза найдены, считаем их площадь как показатель открытости
    float totalEyeArea = 0.0f;
    for (const auto& eye : eyes) {
        totalEyeArea += eye.area();
    }
    
    // Нормализуем относительно размера лица
    float openness = totalEyeArea / (eyesROI.cols * eyesROI.rows);
    return std::min(1.0f, openness * 15.0f); // Масштабируем для удобства
}

// Оценка поворота головы (по смещению лица от центра кадра)
float DMSMonitor::estimateHeadTurn(const cv::Rect& faceRect, int frameWidth) {
    float faceCenterX = faceRect.x + faceRect.width / 2.0f;
    float frameCenterX = frameWidth / 2.0f;
    
    // Смещение от -1.0 (крайний левый) до 1.0 (крайний правый)
    float deviation = (faceCenterX - frameCenterX) / frameCenterX;
    
    // Переводим в примерные градусы (максимум около 45 градусов)
    return deviation * 45.0f;
}

// Главный метод анализа кадра
DriverState DMSMonitor::analyze(const cv::Mat& frame) {
    DriverState state;
    
    if (!loaded_ || frame.empty()) {
        return state;
    }

    // 1. Ищем лицо
    state.face_rect = detectFace(frame);
    state.face_detected = (state.face_rect.area() > 0);

    if (state.face_detected) {
        // 2. Оцениваем глаза
        cv::Mat faceROI = frame(state.face_rect);
        state.eye_openness = estimateEyeOpenness(faceROI);
        state.eyes_open = (state.eye_openness > 0.3f); // Порог открытости

        // 3. Оцениваем поворот головы
        state.head_turn_deg = estimateHeadTurn(state.face_rect, frame.cols);
        state.looking_forward = (std::abs(state.head_turn_deg) < 20.0f); // Допуск 20 градусов
    }

    // 4. Обновляем историю для детекции усталости (дремоты)
    eyeHistory_.push_back(state.eyes_open);
    if (eyeHistory_.size() > 15) {
        eyeHistory_.pop_front(); // Храним только последние 15 кадров
    }

    // Алерт усталости: если в 10 из 15 последних кадров глаза были закрыты
    int closedCount = 0;
    for (bool open : eyeHistory_) {
        if (!open) closedCount++;
    }
    state.alert_drowsy = (closedCount >= 10);

    // Алерт отвлечения: лицо найдено, но смотрит не прямо
    state.alert_distracted = (state.face_detected && !state.looking_forward);

    return state;
}