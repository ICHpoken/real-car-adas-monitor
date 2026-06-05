#pragma once
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h> // Главный заголовок ONNX Runtime

// Структура для хранения результата классификации
struct ClassificationResult {
    int label;                  // Предсказанный класс (0, 1 или 2)
    float confidence;           // Уверенность модели (от 0.0 до 1.0)
    std::vector<float> scores;  // Массив вероятностей для всех 3 классов
};

// Класс для инференса (запуска) нейросети
class ONNXClassifier {
public:
    // Конструктор: загружает модель и параметры нормализации
    ONNXClassifier(const std::string& modelPath, const std::string& paramsPath);
    
    // Метод предсказания: принимает 6 признаков, возвращает результат
    ClassificationResult predict(const std::vector<float>& features);

private:
    // Среда выполнения ONNX (должна жить всё время работы программы)
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "ADAS_Classifier"};
    Ort::Session session{nullptr}; // Сессия загруженной модели
    
    // Параметры нормализации
    std::vector<float> mean_;
    std::vector<float> std_;
    std::vector<std::string> classes_;

    // Вспомогательные методы
    std::vector<float> normalize(const std::vector<float>& x) const;
    std::vector<float> softmax(const std::vector<float>& logits) const;
    
    // Простой парсер JSON (без внешних библиотек)
    std::vector<float> parseJsonArray(const std::string& json, const std::string& key) const;
    std::vector<std::string> parseJsonStringArray(const std::string& json, const std::string& key) const;
};