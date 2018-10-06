#include "globals.h"
int nesp32 = 0;
int accuracy = 0;
int x_min = COLUMN, y_min = ROW, x_max = 0, y_max = 0;
quint16 serverport = 5555;
QMap<int, ESP32> esp32devices;

bool isESP32(int row, int column)
{
    for(auto value : esp32devices)
    {
        if(value.getX() == column && value.getY() == row)
        {
            return true;
        }
    }
    return false;
}
bool inRange(int a, int b, int accuracy)
{
    return (abs(a - b) <= accuracy);
}

bool validBorder(int y, int x, int y_min, int y_max, int x_min, int x_max)
{
    return (y >= y_min && y <= y_max && x >= x_min && x <= x_max);
}

const vector<string> split(const string& s, const char& c)
{
    string buff{""};
    vector<string> v;

    for(auto n:s)
    {
        if(n != c) buff+=n; else
        if(n == c && buff != "") { v.push_back(buff); buff = ""; }
    }
    if(buff != "") v.push_back(buff);

    return v;
}
string generateHash(string src)
{
    QByteArray bytes = QByteArray::fromStdString(src);
    QString string = QString(QCryptographicHash::hash(bytes,QCryptographicHash::Md5).toHex());
    return string.toUtf8().constData();
}

double GetUTCOffsetForQDateTimeAxis()
{
  time_t zero = 24 * 60 * 60L;
  struct tm* timeptr;
  int gmtime_hours;

  // get the local time for Jan 2, 1900 00:00 UTC
  timeptr = localtime(&zero);
  gmtime_hours = timeptr->tm_hour;

  // if the local time is the "day before" the UTC, subtract 24 hours
  // from the hours to get the UTC offset
  if(timeptr->tm_mday < 2)
  {
    gmtime_hours -= 24;
  }

  return 24.0 + gmtime_hours;
}
