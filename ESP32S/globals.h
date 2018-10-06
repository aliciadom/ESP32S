#ifndef GLOBALS_H
#define GLOBALS_H
#define ROW 70
#define COLUMN 70
#include <QtWidgets>
#include <vector>
#include <map>
#include <cmath>

#include "esp.h"
#define ROW 70
#define COLUMN 70
using namespace std;
extern int nesp32;
extern int accuracy;
extern int x_min, y_min, x_max, y_max;
extern quint16 serverport;
extern QMap<int, ESP32> esp32devices;
typedef QList<Packet> QListPacket;

bool isESP32(int row, int column);
bool inRange(int a, int b, int accuracy);
bool validBorder(int y, int x, int y_min, int y_max, int x_min, int x_max);
const vector<string> split(const string& s, const char& c);
string generateHash(string src);
double GetUTCOffsetForQDateTimeAxis();
#endif // GLOBALS_H
