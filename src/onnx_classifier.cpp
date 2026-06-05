#include "onnx_classifier.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

// --- Простой парсер JSON (работает только с нашим конкретным форматом) ---
std::vector<float> ONNXClassifier::parseJsonArray(const std::string& json, const std::string& key) const {
    std::vector<float> result;
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return result;
    
    // Находим открывающую и закрывающую квадратные скобки
    auto bracketStart = json.find('[', pos);
    auto bracketEnd = json.find(']', bracketStart);
    std::string arrStr = json.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    
    std::stringstream ss(arrStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Убираем пробелы
        token.erase(std::remove(token.begin(), token.end(), ' '), token.end());
        if (!token.empty()) {
            try { result.push_back(std::stof(token)); } 
            catch (...) { /* Игнорируем ошибки парсинга */ }
        }
    }
    return result;
}

std::vector<std::string> ONNXClassifier::parseJsonStringArray(const std::string& json, const std::string& key) const {
    std::vector<std::string> result;
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return result;
    
    auto bracketStart = json.find('[', pos);
    auto bracketEnd = json.find(']', bracketStart);
    std::string arrStr = json.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
    
    size_t p = 0;
    while ((p = arrStr.find('"', p)) != std::string::npos) {
        auto end = arrStr.find('"', p + 1);
        result.push_back(arrStr.substr(p + 1, end - p - 1));
        p = end + 1;
    }
    return result;
}

// --- Конструктор ---
ONNXClassifier::ONNXClassifier(const std::string& modelPath, const std::string& paramsPath) {
    // 1. Читаем JSON файл с параметрами
    std::ifstream f(paramsPath);
    if (!f.is_open()) {
        throw std::runtime_error("Не удалось открыть файл параметров: " + paramsPath);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();
    
    // 2. Парсим массивы
    mean_ = parseJsonArray(json, "mean");
    std_ = parseJsonArray(json, "std");
    classes_ = parseJsonStringArray(json, "classes");
    
    if (mean_.size() != 6 || std_.size() != 6) {
        throw std::runtime_error("Неверный размер массивов нормализации в JSON. Ожидалось 6 элементов.");
    }
    
    // 3. Загружаем ONNX модель
    // ВАЖНО: На Windows ONNX Runtime требует путь в виде std::wstring (широкая строка)
    std::wstring wModelPath(modelPath.begin(), modelPath.end());
    
    Ort::SessionOptions sessionOptions;
    // sessionOptions.SetIntraOpNumThreads(1); // Можно ограничить потоки при необходимости
    
    try {
        session = Ort::Session(env, wModelPath.c_str(), sessionOptions);
    } catch (const Ort::Exception& e) {
        throw std::runtime_error(std::string("Ошибка загрузки ONNX модели: ") + e.what());
    }
}

// --- Нормализация (Z-score) ---
std::vector<float> ONNXClassifier::normalize(const std::vector<float>& x) const {
    std::vector<float> out(x.size());
    for (size_t i = 0; i < x.size(); i++) {
        out[i] = (x[i] - mean_[i]) / std_[i];
    }
    return out;
}

// --- Функция активации Softmax (превращает логиты в вероятности) ---
std::vector<float> ONNXClassifier::softmax(const std::vector<float>& logits) const {
    // Вычитаем максимальное значение для численной стабильности (чтобы не было переполнения exp)
    float maxVal = *std::max_element(logits.begin(), logits.end());
    std::vector<float> exps(logits.size());
    float sum = 0.0f;
    
    for (size_t i = 0; i < logits.size(); i++) {
        exps[i] = std::exp(logits[i] - maxVal);
        sum += exps[i];
    }
    
    // Нормируем, чтобы сумма вероятностей была равна 1.0
    for (auto& e : exps) {
        e /= sum;
    }
    return exps;
}

// --- Предсказание ---
ClassificationResult ONNXClassifier::predict(const std::vector<float>& features) {
    if (features.size() != 6) {
        throw std::runtime_error("Ожидалось ровно 6 признаков для классификации.");
    }
    
    // 1. Нормализуем входные данные
    std::vector<float> normFeatures = normalize(features);
    
    // 2. Создаем входной тензор ONNX
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    std::array<int64_t, 2> inputShape{1, 6}; // Batch size = 1, Features = 6
    
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, 
        normFeatures.data(), 
        normFeatures.size(), 
        inputShape.data(), 
        inputShape.size()
    );
    
    // 3. Имена входного и выходного узлов (должны совпадать с теми, что мы задали в Colab!)
    const char* inputNames[] = {"features"};
    const char* outputNames[] = {"class_scores"};
    
    // 4. Запускаем инференс
    auto outputTensors = session.Run(
        Ort::RunOptions{nullptr},
        inputNames, &inputTensor, 1,
        outputNames, 1
    );
    
    // 5. Извлекаем сырые логиты из выходного тензора
    float* logits = outputTensors[0].GetTensorMutableData<float>();
    std::vector<float> rawScores(logits, logits + 3);
    
    // 6. Применяем Softmax для получения вероятностей
    std::vector<float> probabilities = softmax(rawScores);
    
    // 7. Находим индекс класса с максимальной вероятностью
    int best_idx = std::max_element(probabilities.begin(), probabilities.end()) - probabilities.begin();
    
    ClassificationResult result;
    result.scores = probabilities;
    result.confidence = probabilities[best_idx];
    
    // 8. АДАПТАЦИЯ: Переводим предсказание модели в строгий формат задания
    // (SLOW=0, NORMAL=1, AGGRESSIVE=2), используя массив classes_ из JSON
    std::string predicted_class = classes_[best_idx];
    
    if (predicted_class == "SLOW") {
        result.label = 0;
    } else if (predicted_class == "NORMAL") {
        result.label = 1;
    } else if (predicted_class == "AGGRESSIVE") {
        result.label = 2;
    } else {
        // Fallback на случай непредвиденных ошибок
        result.label = best_idx; 
    }
    
    return result;
}