#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "obd_parser.h"
#include "onnx_classifier.h"
#include "dashboard.h"
#include "dms_monitor.h"
#include "dms_hud.h"
#include "shared_state.h"

int main() {
    try {
        std::cout << "=== ADAS Final Integration ===\n" << std::flush;

        // 1. OBD парсер
        OBDParser parser;
        if (parser.load("../data/obd_data.csv") < 0) {
            std::cerr << "Failed to load OBD data\n";
            return 1;
        }
        std::cout << "Loaded " << parser.size() << " OBD records\n" << std::flush;

        // 2. ONNX классификатор
        ONNXClassifier classifier("../models/driver_classifier.onnx", "../models/normalization_params.json");
        std::cout << "ONNX classifier loaded\n" << std::flush;

        // 3. DMS
        DMSMonitor dms;
        if (!dms.initialize("../models/deploy.prototxt",
                            "../models/res10_300x300_ssd_iter_140000.caffemodel",
                            "../models/haarcascade_eye.xml")) {
            std::cerr << "DMS initialization failed\n";
            return 1;
        }
        std::cout << "DMS initialized\n" << std::flush;

        // 4. Dashboard и HUD
        Dashboard dash(640, 480);
        DMSHUD hud;

        // 5. Камера
        cv::VideoCapture cap(0);
        if (!cap.isOpened()) {
            std::cerr << "Cannot open webcam\n";
            return 1;
        }

        // 6. Видео запись
        cv::VideoWriter writer;
        bool recording = writer.open("../output/result_situation2.mp4",
                                     cv::VideoWriter::fourcc('m','p','4','v'),
                                     15.0, cv::Size(1280, 480));
        if (!recording) std::cerr << "Warning: cannot record video\n";

        // 7. Лог алертов
        std::ofstream logfile("../output/dms_alerts.log", std::ios::trunc);
        if (!logfile.is_open()) std::cerr << "Warning: cannot open log\n";

        // 8. Состояние и поток OBD
        SharedState state;
        state.total_records = parser.size();

        std::thread obd_thread([&]() {
            size_t idx = 0;
            size_t total = parser.size();
            if (total == 0) return;
            while (state.running) {
                OBDRecord rec = parser.getRecord(idx % total);
                std::vector<float> features = {
                    rec.speed_kmh, rec.engine_rpm, rec.throttle_pos,
                    rec.coolant_temp, rec.fuel_level, rec.intake_air_temp
                };
                ClassificationResult cls = classifier.predict(features);
                state.update(idx, rec, cls);
                idx = (idx + 1) % total;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });

        // 9. Главный цикл
        cv::Mat frame, display;
        DriverState lastDriverState;
        bool paused = false;
        std::cout << "System running. Press Q to quit, SPACE to pause, S for screenshot.\n" << std::flush;

        while (state.running) {
            if (!paused) {
                cap >> frame;
                if (frame.empty()) break;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            DriverState driverState = dms.analyze(frame);

            if (driverState.alert_drowsy && !lastDriverState.alert_drowsy) {
                auto t = std::time(nullptr);
                logfile << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << " - DROWSINESS_ALERT\n";
                state.alert_drowsy_count++;
            }
            if (driverState.alert_distracted && !lastDriverState.alert_distracted) {
                auto t = std::time(nullptr);
                logfile << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << " - DISTRACTION_ALERT\n";
                state.alert_distracted_count++;
            }
            lastDriverState = driverState;

            auto [obdRecord, classification] = state.get();

            DashboardData dashData;
            dashData.speed_kmh = obdRecord.speed_kmh;
            dashData.rpm = obdRecord.engine_rpm;
            dashData.coolant_temp = obdRecord.coolant_temp;
            dashData.fuel_level = obdRecord.fuel_level;
            dashData.throttle_pos = obdRecord.throttle_pos;
            dashData.driving_style = classification.label;

            display = cv::Mat(480, 1280, CV_8UC3, cv::Scalar(30, 30, 40));
            cv::Mat leftHalf = display(cv::Rect(0, 0, 640, 480));
            dash.draw(leftHalf, dashData);
            hud.draw(display, driverState, frame);

            cv::imshow("ADAS Monitor", display);
            if (recording) writer.write(display);

            char key = cv::waitKey(1);
            if (key == 'q' || key == 'Q') state.running = false;
            else if (key == ' ') paused = !paused;
            else if (key == 's' || key == 'S') cv::imwrite("../output/screenshot.png", display);
        }

        obd_thread.join();
        if (recording) writer.release();
        logfile.close();
        cap.release();
        cv::destroyAllWindows();

        std::cout << "\n========== Final Statistics ==========\n";
        std::cout << "Aggressive driving alerts: " << state.alert_aggressive_count << "\n";
        std::cout << "Drowsiness alerts: " << state.alert_drowsy_count << "\n";
        std::cout << "Distraction alerts: " << state.alert_distracted_count << "\n";
        std::cout << "======================================\n";

    } catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}