#include <gtest/gtest.h>
#include "obd_parser.h"
#include <fstream>
#include <iostream>

// Тест 1: Проверка конвертации меток (SLOW->0, NORMAL->1, AGGRESSIVE->2)
TEST(OBDParserTest, LabelConversion) {
    OBDParser parser;
    // Создаём временный CSV файл для теста
    {
        std::ofstream f("temp_test_labels.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "30,1500,10,85,50,20,SLOW\n";
        f << "70,2800,40,90,40,25,NORMAL\n";
        f << "120,5000,90,95,30,30,AGGRESSIVE\n";
    }
    
    // Проверяем, что загрузилось 3 записи
    ASSERT_EQ(parser.load("temp_test_labels.csv"), 3);
    // Проверяем правильность конвертации меток
    EXPECT_EQ(parser.getRecord(0).label, 0);  // SLOW -> 0
    EXPECT_EQ(parser.getRecord(1).label, 1);  // NORMAL -> 1
    EXPECT_EQ(parser.getRecord(2).label, 2);  // AGGRESSIVE -> 2
    
    // Удаляем временный файл
    std::remove("temp_test_labels.csv");
}

// Тест 2: Загрузка несуществующего файла должна вернуть -1
TEST(OBDParserTest, FileNotFound) {
    OBDParser parser;
    EXPECT_EQ(parser.load("nonexistent_file_12345.csv"), -1);
}

// Тест 3: getRecord() должен бросать исключение при неверном индексе
TEST(OBDParserTest, OutOfRangeException) {
    OBDParser parser;
    // Пытаемся получить запись из пустого парсера — должно быть исключение
    EXPECT_THROW(parser.getRecord(0), std::out_of_range);
    
    // Загружаем 1 запись и пробуем получить несуществующие индексы
    {
        std::ofstream f("temp_single.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "50,2000,20,90,50,20,NORMAL\n";
    }
    parser.load("temp_single.csv");
    EXPECT_THROW(parser.getRecord(1), std::out_of_range);   // Индекс за пределами
    EXPECT_THROW(parser.getRecord(-1), std::out_of_range);  // Отрицательный индекс
    
    std::remove("temp_single.csv");
}

// Тест 4: Парсинг корректного CSV файла
TEST(OBDParserTest, ParseValidCSV) {
    // Создаём временный файл с одной записью
    {
        std::ofstream f("temp_valid.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "87.5,3200.0,50.0,92.0,68.0,25.0,NORMAL\n";
    }
    OBDParser parser;
    ASSERT_EQ(parser.load("temp_valid.csv"), 1);
    
    // Проверяем, что все значения распарсились правильно
    auto rec = parser.getRecord(0);
    EXPECT_FLOAT_EQ(rec.speed_kmh, 87.5f);
    EXPECT_FLOAT_EQ(rec.engine_rpm, 3200.0f);
    EXPECT_FLOAT_EQ(rec.throttle_pos, 50.0f);
    EXPECT_FLOAT_EQ(rec.coolant_temp, 92.0f);
    EXPECT_FLOAT_EQ(rec.fuel_level, 68.0f);
    EXPECT_FLOAT_EQ(rec.intake_air_temp, 25.0f);
    EXPECT_EQ(rec.label, 1);
    
    std::remove("temp_valid.csv");
}

// Тест 5: Парсинг файла с некорректными строками (плохие строки пропускаются)
TEST(OBDParserTest, SkipBadLine) {
    // Создаём файл с хорошими и плохими строками
    {
        std::ofstream f("temp_bad_line.csv");
        f << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        f << "87,3200,50,92,68,25,NORMAL\n";          // Хорошая строка (будет загружена)
        f << "bad,data,here\n";                        // Плохая строка (мало колонок)
        f << "90,3500,55,93,60,22,UNKNOWN_LABEL\n";    // Плохая строка (неизвестная метка)
        f << "95,3600,60,94,65,23,SLOW\n";             // Хорошая строка (будет загружена)
    }
    OBDParser parser;
    // Должно загрузиться только 2 записи (1-я и 4-я)
    ASSERT_EQ(parser.load("temp_bad_line.csv"), 2);
    EXPECT_EQ(parser.getRecord(0).speed_kmh, 87.0f);
    EXPECT_EQ(parser.getRecord(0).label, 1);  // NORMAL
    EXPECT_EQ(parser.getRecord(1).speed_kmh, 95.0f);
    EXPECT_EQ(parser.getRecord(1).label, 0);  // SLOW
    
    std::remove("temp_bad_line.csv");
}