#include "obd_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Конвертирует строковую метку стиля вождения в числовой код
int OBDParser::labelToInt(const std::string& label) const {
    if (label == "SLOW") return 0;        // Медленный стиль
    if (label == "NORMAL") return 1;      // Нормальный стиль
    if (label == "AGGRESSIVE") return 2;  // Агрессивный стиль
    return -1; // Неизвестная метка
}

// Загружает CSV файл с телеметрией
int OBDParser::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << filename << "\n";
        return -1;
    }

    std::string line;
    // Пропускаем первую строку (заголовки столбцов)
    if (!std::getline(file, line)) {
        return 0; // Файл пустой
    }

    int lineNum = 1;
    while (std::getline(file, line)) {
        lineNum++;
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;

        // Разбиваем строку по запятым
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        // Проверяем, что в строке ровно 7 колонок
        if (tokens.size() != 7) {
            std::cerr << "Warning: skipping line " << lineNum 
                      << " (wrong number of columns: " << tokens.size() << ")\n";
            continue;
        }

        try {
            OBDRecord rec;
            // Парсим числовые значения из строковых токенов
            rec.speed_kmh = std::stof(tokens[0]);
            rec.engine_rpm = std::stof(tokens[1]);
            rec.throttle_pos = std::stof(tokens[2]);
            rec.coolant_temp = std::stof(tokens[3]);
            rec.fuel_level = std::stof(tokens[4]);
            rec.intake_air_temp = std::stof(tokens[5]);
            rec.label = labelToInt(tokens[6]);

            // Если метка неизвестная — пропускаем строку
            if (rec.label == -1) {
                std::cerr << "Warning: skipping line " << lineNum 
                          << " (unknown label: " << tokens[6] << ")\n";
                continue;
            }

            // Добавляем успешную запись в вектор
            records_.push_back(rec);
        } catch (const std::exception& e) {
            // Ловим ошибки парсинга чисел (std::invalid_argument, std::out_of_range)
            std::cerr << "Warning: skipping line " << lineNum 
                      << " (parse error: " << e.what() << ")\n";
        }
    }

    return static_cast<int>(records_.size());
}

// Возвращает запись по индексу с проверкой границ
OBDRecord OBDParser::getRecord(int index) const {
    if (index < 0 || index >= static_cast<int>(records_.size())) {
        throw std::out_of_range("Index out of range in OBD records");
    }
    return records_[index];
}