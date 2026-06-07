#pragma once
#include <string>
#include <vector>

struct OBDRecord {
    float speed_kmh = 0.0f;
    float engine_rpm = 0.0f;
    float throttle_pos = 0.0f;
    float coolant_temp = 0.0f;
    float fuel_level = 0.0f;
    float intake_air_temp = 0.0f;
    int label = -1;
};

class OBDParser {
public:
    int load(const std::string& filename);
    OBDRecord getRecord(int index) const;
    size_t size() const;   // только объявление

private:
    std::vector<OBDRecord> records_;
    int labelToInt(const std::string& label) const;
};