#pragma once
#include <string>
#include <vector>
#include <stdexcept>

// Структура для хранения одной записи телеметрии из OBD-II
struct OBDRecord {
    float speed_kmh;        // Скорость в км/ч
    float engine_rpm;       // Обороты двигателя
    float throttle_pos;     // Положение дроссельной заслонки (%)
    float coolant_temp;     // Температура охлаждающей жидкости (°C)
    float fuel_level;       // Уровень топлива (%)
    float intake_air_temp;  // Температура всасываемого воздуха (°C)
    int label;              // Метка стиля вождения: 0 = SLOW, 1 = NORMAL, 2 = AGGRESSIVE
};

// Класс для парсинга CSV файлов с телеметрией автомобиля
class OBDParser {
public:
    // Загружает CSV файл, возвращает количество записей или -1 при ошибке
    int load(const std::string& filename);
    
    // Возвращает запись по индексу
    OBDRecord getRecord(int index) const;
    
    // Возвращает общее количество загруженных записей
    size_t size() const { return records_.size(); }

private:
    std::vector<OBDRecord> records_;  // Вектор всех загруженных записей
    
    // Вспомогательный метод для конвертации строковой метки в число
    int labelToInt(const std::string& label) const;
};