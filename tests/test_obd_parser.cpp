#include <gtest/gtest.h>
#include "obd_parser.h"
#include <fstream>

TEST(OBDParserTest, LabelConversion) {
    OBDParser parser;
    std::ofstream f("tmp.csv");
    f << "speed,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_temp,label\n";
    f << "30,1500,10,85,50,20,SLOW\n70,2800,40,90,40,25,NORMAL\n120,5000,90,95,30,30,AGGRESSIVE\n";
    f.close();
    ASSERT_EQ(parser.load("tmp.csv"), 3);
    EXPECT_EQ(parser.getRecord(0).label, 0);
    EXPECT_EQ(parser.getRecord(1).label, 1);
    EXPECT_EQ(parser.getRecord(2).label, 2);
    std::remove("tmp.csv");
}

TEST(OBDParserTest, FileNotFound) {
    OBDParser parser;
    EXPECT_EQ(parser.load("nonexist.csv"), -1);
}

TEST(OBDParserTest, OutOfRange) {
    OBDParser parser;
    EXPECT_THROW(parser.getRecord(0), std::out_of_range);
    std::ofstream f("tmp2.csv");
    f << "a,b,c,d,e,f,g\n1,2,3,4,5,6,NORMAL\n";
    f.close();
    parser.load("tmp2.csv");
    EXPECT_THROW(parser.getRecord(1), std::out_of_range);
    std::remove("tmp2.csv");
}

TEST(OBDParserTest, ValidCSV) {
    std::ofstream f("valid.csv");
    f << "a,b,c,d,e,f,g\n87.5,3200,50,92,68,25,NORMAL\n";
    f.close();
    OBDParser parser;
    parser.load("valid.csv");
    auto r = parser.getRecord(0);
    EXPECT_FLOAT_EQ(r.speed_kmh, 87.5f);
    EXPECT_EQ(r.label, 1);
    std::remove("valid.csv");
}

TEST(OBDParserTest, SkipBadLines) {
    std::ofstream f("bad.csv");
    f << "a,b,c,d,e,f,g\n87,3200,50,92,68,25,NORMAL\nbad,line\n90,3500,55,93,60,22,UNKNOWN\n95,3600,60,94,65,23,SLOW\n";
    f.close();
    OBDParser parser;
    int cnt = parser.load("bad.csv");
    EXPECT_EQ(cnt, 2);
    std::remove("bad.csv");
}