#include <iostream>
#include <map>
#include "obd_parser.h"

int main() {
    std::cout << "=== Real Car ADAS Monitor ===" << std::endl;
    std::cout << "Version: 0.2.0 (OBD Parser)" << std::endl;
    std::cout << "-----------------------------" << std::endl;

    OBDParser parser;
    // Загружаем CSV файл (путь относительно папки build/)
    int count = parser.load("../data/obd_data.csv");
    
    if (count < 0) {
        std::cerr << "Critical error: cannot load CSV file!\n";
        std::cerr << "Expected path: ../data/obd_data.csv\n";
        std::cerr << "Make sure you run the program from the build/ folder.\n";
        return 1;
    }
    
    // Выводим информацию о загруженных данных
    std::cout << "Successfully loaded records: " << count << "\n\n";
    
    // Выводим первые 5 записей для проверки
    std::cout << "First 5 records:" << std::endl;
    std::cout << "Speed(km/h) | RPM   | Throttle | Coolant | Fuel | Label" << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    for (int i = 0; i < 5 && i < count; i++) {
        auto r = parser.getRecord(i);
        std::cout << r.speed_kmh << "        | " 
                  << r.engine_rpm << " | "
                  << r.throttle_pos << "       | "
                  << r.coolant_temp << "     | "
                  << r.fuel_level << "  | "
                  << r.label << "\n";
    }
    
    // Подсчитываем статистику по классам стиля вождения
    std::map<int, int> stats;
    stats[0] = 0; // SLOW
    stats[1] = 0; // NORMAL
    stats[2] = 0; // AGGRESSIVE
    
    for (size_t i = 0; i < parser.size(); i++) {
        stats[parser.getRecord(i).label]++;
    }
    
    // Выводим итоговую статистику
    std::cout << "\nDriving style statistics:" << std::endl;
    std::cout << "SLOW (0):       " << stats[0] << " records" << std::endl;
    std::cout << "NORMAL (1):     " << stats[1] << " records" << std::endl;
    std::cout << "AGGRESSIVE (2): " << stats[2] << " records" << std::endl;
    
    std::cout << "\nBuild successful!" << std::endl;
    return 0;
}