#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "server.h"
#include "globals.h"
#include <mutex>
#include "dbmanager.h"
#include <QtCharts>
#include <QDateTimeAxis>
using namespace QtCharts;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void dBExitThreadSignal();
    void doUpdateGridSignal(int, int, QString);
    void doExecuteQuerySignal(QListPacket, int, int);
    void doSelectQuerySignal(uint from, uint to);
    void doUpdateChartSeriesSignal(uint time);
public slots:
   void handleRealTimeModeButtonSlot();
   void handleDBModeButtonSlot();
   void receiveDataSlot(QListPacket);
   void updateUISlot();
   void updateCellSlot(int row, int column, QString tooltip);
   void updateChartSeriesSlot(uint time);
   void updateGridLayoutSlot(QListPacket);
private:
    Ui::MainWindow *ui;
    mutex m;
    QMap<int, QList<Packet>> concurrentMap;
    QList<string> distinctdevices;
    QMap<string,QList<Packet>> hashmap;
    QMap<uint, QList<Packet>> packetsForTimestamp;
    QList<uint> Xaxis;
    Server server;
    DbManager db;
    QChart *chart;
    QChartView* chartView;
    QLineSeries *series;
    QDateTimeAxis *axisX;
    QValueAxis *axisY;
    void SetupChartLayout();
    void SetupGridLayout();
    void UpdateGridLayout();
    void ClearGridLayout();
    void RealTimeDataCollection();
    void UpdateChartLayout(QDateTime from, QDateTime to, QMap<uint,int> map);
};

#endif // MAINWINDOW_Hs
