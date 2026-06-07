#include "obd_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>

int OBDParser::labelToInt(const std::string& label) const {
    if (label == "SLOW") return 0;
    if (label == "NORMAL") return 1;
    if (label == "AGGRESSIVE") return 2;
    return -1;
}

int OBDParser::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << filename << "\n";
        return -1;
    }
    std::string line;
    std::getline(file, line); // заголовок
    int lineNum = 1;
    while (std::getline(file, line)) {
        lineNum++;
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, ',')) tokens.push_back(token);
        if (tokens.size() != 7) {
            std::cerr << "Warning: line " << lineNum << " wrong columns\n";
            continue;
        }
        try {
            OBDRecord rec;
            rec.speed_kmh = std::stof(tokens[0]);
            rec.engine_rpm = std::stof(tokens[1]);
            rec.throttle_pos = std::stof(tokens[2]);
            rec.coolant_temp = std::stof(tokens[3]);
            rec.fuel_level = std::stof(tokens[4]);
            rec.intake_air_temp = std::stof(tokens[5]);
            rec.label = labelToInt(tokens[6]);
            if (rec.label == -1) {
                std::cerr << "Warning: line " << lineNum << " unknown label\n";
                continue;
            }
            records_.push_back(rec);
        } catch (const std::exception& e) {
            std::cerr << "Warning: line " << lineNum << " parse error\n";
        }
    }
    return static_cast<int>(records_.size());
}

OBDRecord OBDParser::getRecord(int index) const {
    if (index < 0 || index >= static_cast<int>(records_.size()))
        throw std::out_of_range("Index out of range");
    return records_[index];
}

size_t OBDParser::size() const {
    return records_.size();
}