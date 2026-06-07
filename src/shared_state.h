#pragma once

#include <mutex>
#include <atomic>
#include <utility>
#include "obd_parser.h"
#include "onnx_classifier.h"

struct SharedState {
    OBDRecord current_record;
    ClassificationResult current_classification;
    int alert_drowsy_count = 0;
    int alert_distracted_count = 0;
    int alert_aggressive_count = 0;
    std::atomic<bool> running{true};
    std::mutex mtx;
    size_t current_index = 0;
    size_t total_records = 0;

    void update(size_t idx, const OBDRecord& rec, const ClassificationResult& cls) {
        std::lock_guard<std::mutex> lock(mtx);
        current_index = idx;
        current_record = rec;
        current_classification = cls;
        if (cls.label == 2) alert_aggressive_count++;
    }

    std::pair<OBDRecord, ClassificationResult> get() {
        std::lock_guard<std::mutex> lock(mtx);
        return {current_record, current_classification};
    }
};