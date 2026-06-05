#include <iostream>
#include <map>
#include <iomanip> // Для форматирования таблицы
#include "obd_parser.h"
#include "onnx_classifier.h"

int main() {
    std::cout << "=== Real Car ADAS Monitor ===" << std::endl;
    std::cout << "Version: 0.3.0 (ONNX Classifier)" << std::endl;
    std::cout << "-----------------------------" << std::endl;

    // 1. Загружаем данные
    OBDParser parser;
    int count = parser.load("../data/obd_data.csv");
    if (count < 0) {
        std::cerr << "Critical error: cannot load CSV file!\n";
        return 1;
    }
    std::cout << "Successfully loaded records: " << count << "\n\n";

    // 2. Инициализируем классификатор
    try {
        ONNXClassifier classifier("../models/driver_classifier.onnx", "../models/normalization_params.json");
        std::cout << "ONNX Model loaded successfully!\n\n";

        // 3. Тестируем на первых 20 записях
        int limit = std::min(20, count);
        int correctPredictions = 0;

        std::cout << std::left << std::setw(6) << "True" 
                  << std::setw(12) << "Predicted" 
                  << std::setw(12) << "Confidence" << std::endl;
        std::cout << "--------------------------------------" << std::endl;

        for (int i = 0; i < limit; i++) {
            auto rec = parser.getRecord(i);
            
            // Формируем вектор признаков (строго в том же порядке, как в Colab!)
            std::vector<float> features = {
                rec.speed_kmh,
                rec.engine_rpm,
                rec.throttle_pos,
                rec.coolant_temp,
                rec.fuel_level,
                rec.intake_air_temp
            };

            // Получаем предсказание
            ClassificationResult pred = classifier.predict(features);

            // Считаем точность
            if (pred.label == rec.label) {
                correctPredictions++;
            }

            // Выводим строку таблицы
            std::cout << std::left << std::setw(6) << rec.label
                      << std::setw(12) << pred.label
                      << std::fixed << std::setprecision(2) << (pred.confidence * 100.0f) << "%" << std::endl;
        }

        // 4. Итоговая статистика
        double accuracy = (static_cast<double>(correctPredictions) / limit) * 100.0;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "Accuracy on first 20 records: " << std::fixed << std::setprecision(1) << accuracy << "%" << std::endl;
        std::cout << "\nBuild successful!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}