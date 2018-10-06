#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->nesp32label->setText(QString::number(nesp32));
    ui->serverportlabel->setValidator(new QIntValidator(0, 65536, this));
    ui->dateTimeFrom->setDate(QDate::currentDate().addDays(-1));
    ui->dateTimeTo->setDate(QDate::currentDate());
    SetupGridLayout();
    SetupChartLayout();
    connect(ui->startButton, SIGNAL (released()), this, SLOT (handleRealTimeModeButtonSlot()));
    connect(ui->buttonUpdate, SIGNAL (released()), this, SLOT (handleDBModeButtonSlot()));
    connect(&server, SIGNAL(sendData(QListPacket)), this, SLOT(receiveDataSlot(QListPacket)));
    connect(&server, SIGNAL(doUpdate()), this, SLOT(updateUISlot()));
    connect(this,SIGNAL(doUpdateGridSignal(int, int, QString)),this,SLOT(updateCellSlot(int, int, QString)));
    connect(this,SIGNAL(doUpdateChartSeriesSignal(uint)),this,SLOT(updateChartSeriesSlot(uint)));
    connect(this,SIGNAL(doExecuteQuerySignal(QListPacket,int, int)), &db, SLOT(executeQuery(QListPacket,int, int)));
    connect(this,SIGNAL(doSelectQuerySignal(uint,uint)), &db, SLOT(selectQuery(uint,uint)));
    connect(&db, SIGNAL(doUpdateGrid(QListPacket)), this, SLOT(updateGridLayoutSlot(QListPacket)));
    db.start();
}

MainWindow::~MainWindow()
{
    delete ui;
    db.quit();
}

void MainWindow::SetupGridLayout()
{
    QPalette empty, espcolor;
    empty.setColor(QPalette::Window, Qt::yellow);
    espcolor.setColor(QPalette::Window, Qt::black);
    for (int i = 0; i < ROW; i++)
    {
        for(int j = 0; j< COLUMN; j++)
        {
            QLabel *ql = new QLabel(centralWidget());
            QString s;
            s.append("POS[X,Y]: [").append(QString::number(j)).append(",").append(QString::number(i)).append("]\n");
            ql->setAutoFillBackground(true);
            ql->setFrameShape(QFrame::StyledPanel);
            ql->setPalette(empty);
            for (ESP32 esp32 : esp32devices)
            {
                double value = esp32.getCell(j,i).getrssi();
                s.append(QString::fromUtf8(esp32.getName().c_str())).append(" RSSI: ").append(QString::number(value)).append("\n");
                if(value==0.0)
                {
                    ql->setPalette(espcolor);
                }
            }
            ql->setToolTip(s);
            ql->setFixedSize(QSize(20,20));
            ui->gridLayout->addWidget(ql,i,j);
            ui->gridLayout->setSpacing(0);
        }
    }
}

void MainWindow::SetupChartLayout()
{
    QDateTime from = ui->dateTimeFrom->dateTime();
    QDateTime to = ui->dateTimeTo->dateTime();
    qreal defaultvalue = 0;
    axisY = new QValueAxis();
    axisX= new QDateTimeAxis();
    chart = new QChart();
    series = new QLineSeries();
    //Y
    axisY->setLabelsVisible(false);
    axisY->setTitleText("Numero dispositivi");
    axisY->setMin(0);
    axisY->setMax(5);
    //axisY->setLineVisible(false);

    //X

    axisX->setTickCount(2);
    axisX->setMin(from);
    axisX->setMax(to);
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText(to.date().toString());

    //series
    series->append(from.toMSecsSinceEpoch(),defaultvalue);
    series->append(to.toMSecsSinceEpoch(),defaultvalue);
    series->setPointLabelsClipping(false);
    series->setPointLabelsVisible(true);    // is false by default
    series->setPointLabelsColor(Qt::black);

    QString format = "@yPoint";
    series->setPointLabelsFormat(format); //18/09/18 09:40

    //chart
    chart->legend()->hide();
    chart->setTitle("Dati filtrati durante il periodo temporale");
    chart->addAxis(axisY, Qt::AlignLeft);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addSeries(series);
    // before attaching axises to ther serie you have to attach them to the chart
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView = new QChartView(chart);
    ui->verticalLayout->addWidget(chartView);
}

void MainWindow::UpdateGridLayout()
{
    for (int i = 0; i < ROW; i++)
    {
        for(int j = 0; j< COLUMN; j++)
        {
            if(!isESP32(i,j))
            {
                QString s;
                for (string hash : hashmap.keys())
                {
                    int count = 0;
                    for(Packet p : hashmap[hash])
                    {
                        int esp32id = p.getESP32();
                        ESP32 esp32 = esp32devices.find(esp32id).value();
                        double esp32rssi = esp32.getCell(j,i).getrssi();
                        if(inRange(p.getRSSI(),static_cast<int>(esp32rssi),accuracy) && validBorder(i,j,y_min,y_max,x_min,x_max))
                        {
                            count++;
                        }
                    }
                    if(count == nesp32)
                    {
                        Packet p = hashmap[hash][0];
                        string packethash = p.getHash();
                        uint unixtime = p.getTimestamp();
                        QDateTime time;
                        time.setTime_t(unixtime);
                        s.append("{\n\tMAC: ").append(QString::fromUtf8(p.getMAC().c_str())).append("\n\t:").append(time.toString()).append("\n}\n");
                        if(!distinctdevices.contains(packethash))
                            distinctdevices.push_back(packethash);
                        /**/
                        if(packetsForTimestamp.contains(unixtime))
                        {
                            packetsForTimestamp[unixtime].push_back(p);
                        }
                        else
                        {
                            QList<Packet> packets;
                            packets.push_back(p);
                            packetsForTimestamp.insert(unixtime,packets);
                        }
                        emit doUpdateGridSignal(i,j,s);
                        emit doUpdateChartSeriesSignal(unixtime);
                        emit doExecuteQuerySignal(hashmap[hash],i,j);
                    }
                }
            }

        }
    }
    ui->deviceslabel->setText(QString::number(distinctdevices.count()));
}

void MainWindow::ClearGridLayout()
{
    QPalette empty;
    empty.setColor(QPalette::Window, Qt::yellow);

    for (int i = 0; i < ROW; i++)
    {
        for(int j = 0; j< COLUMN; j++)
        {
            QWidget *qw = ui->gridLayout->itemAtPosition(i,j)->widget();
            QColor qcolor = qw->palette().color(QPalette::Window);
            if(qcolor==Qt::red)
            {
                QString s;
                s.append("POS[X,Y]: [").append(QString::number(j)).append(",").append(QString::number(i)).append("]\n");
                for (ESP32 esp32 : esp32devices)
                {
                    double value = esp32.getCell(j,i).getrssi();
                    s.append(QString::fromUtf8(esp32.getName().c_str())).append(" RSSI: ").append(QString::number(value)).append("\n");
                }
                qw->setToolTip(s);
                qw->setPalette(empty);
            }
        }
    }
}

/*
* this method is called when user presses 'AGGIORNA'
*/
void MainWindow::handleDBModeButtonSlot()
{
    QDateTime from = ui->dateTimeFrom->dateTime();
    QDateTime to = ui->dateTimeTo->dateTime();
    uint unixtimefrom = from.toTime_t();
    uint unixtimeto = to.toTime_t();
    ClearGridLayout();

    /**/
    packetsForTimestamp.clear();

    Xaxis.clear();
    series->clear();

    axisX->setTickCount(2);
    axisX->setMin(from);
    axisX->setMax(to);
    axisY->setMin(0);
    axisY->setMax(5);
    /**/
    ui->deviceslabel->setText(QString::number(0));
    distinctdevices.clear();
    emit doSelectQuerySignal(unixtimefrom,unixtimeto);
}

void MainWindow::handleRealTimeModeButtonSlot()
{
    if(ui->startButton->text()=="RICEVI IN TEMPO REALE")
    {
            ui->startButton->setText("FERMA ACQUISIZIONE TEMPO REALE");
            serverport = ui->serverportlabel->text().toUShort();
            ClearGridLayout();
            ui->deviceslabel->setText(QString::number(0));
            distinctdevices.clear();
            RealTimeDataCollection();
    }
    else
    {
        ui->startButton->setText("RICEVI IN TEMPO REALE");
        emit server.stopRunning();
    }

}

void MainWindow::RealTimeDataCollection()
{
    server.startServer();
}

void MainWindow::receiveDataSlot(QListPacket packets)
{
    qDebug() << "receiveData()";
    lock_guard<mutex> ul(m);
    for (Packet p : packets)
    {
        int esp32id = p.getESP32();
        if(!concurrentMap.contains(esp32id))
        {
                QList<Packet> l;
                l.push_back(p);
                concurrentMap.insert(esp32id,l);
        }
        else
        {
            concurrentMap[esp32id].push_back(p);
        }
    }
}
void MainWindow::updateUISlot()
{
    qDebug() << "updateUi()";
    hashmap.clear();
    for(int i : concurrentMap.keys())
    {
        for(Packet p : concurrentMap[i])
        {
            string hash = p.getHash();
            if(!hashmap.contains(hash))
            {
                QList<Packet> packets;
                packets.push_back(p);
                hashmap.insert(hash,packets);
            }
            else
            {
                hashmap[hash].push_back(p);
            }
            qDebug() << "MainWindow::UpdateUI " << QString::fromStdString(p.toString());
        }
    }
    concurrentMap.clear();
    UpdateGridLayout();
}
void MainWindow::updateCellSlot(int row, int column, QString tooltip)
{
    QPalette qp;
    qp.setColor(QPalette::Window, Qt::red);
    QWidget *qw = ui->gridLayout->itemAtPosition(row,column)->widget();
    QString qwstring = qw->toolTip();
    QPalette qwpalette = qw->palette();
    qwstring.append(tooltip);
    qw->setToolTip(qwstring);
    qw->setPalette(qp);
}

void MainWindow::updateChartSeriesSlot(uint unixtime)
{
    QDateTime time;
    time.setTime_t(unixtime);
    if(!Xaxis.contains(unixtime))
    {
        qDebug() << "Ok i am here";
        Xaxis.push_back(unixtime);
        axisX->setTickCount(3);
        series->append(time.toMSecsSinceEpoch(),1);
    }
    /*
    else
    {
        int X = Xaxis.indexOf(unixtime);
        double ndistinctdevices = series->at(X).x();
        series->remove(unixtime,ndistinctdevices);
        series->append(time.toMSecsSinceEpoch(),ndistinctdevices++);
    }
    */
}

/*
* this slot method receive data from a  db thread.
* this solt method is invoked directly from:
*       - db thread (signal/slot).
* this slot method is invoked indirectly from:
*       - handleDbModeButton()
*/
void MainWindow::updateGridLayoutSlot(QListPacket packets)
{
    QMap<uint,int> map;
    distinctdevices.clear();
    int avoidDuplicate_row = 0, avoidDuplicate_column = 0;
    for(Packet p : packets)
    {
        QString s;
        string hash = p.getHash();
        string mac = p.getMAC();
        uint unixtime = p.getTimestamp();
        int row = p.getY();
        int column = p.getX();

        QDateTime time;
        time.setTime_t(unixtime);

        if(!distinctdevices.contains(hash))
        {
            distinctdevices.push_back(hash);
            avoidDuplicate_column = column;
            avoidDuplicate_row = row;
            s.append("{\n\tMAC: ").append(QString::fromUtf8(mac.c_str())).append("\n\tDATA:").append(time.toString()).append("\n}\n");
            if(!map.contains(unixtime))
                map.insert(unixtime,1);
            else
                map[unixtime]++;
        }
        if(avoidDuplicate_column != column || avoidDuplicate_row  != row){
            s.append("{\n\tMAC: ").append(QString::fromUtf8(mac.c_str())).append("\n\tDATA:").append(time.toString()).append("\n}\n");
            avoidDuplicate_column = column;
            avoidDuplicate_row = row;
        }
        emit doUpdateGridSignal(row,column,s);
        //emit doUpdateChartSeries(unixtime);
        qDebug() << QString::fromStdString(p.toString());
    }
    ui->deviceslabel->setText(QString::number(distinctdevices.count()));
    UpdateChartLayout(ui->dateTimeFrom->dateTime(),ui->dateTimeTo->dateTime(),map);
}

void MainWindow::UpdateChartLayout(QDateTime from, QDateTime to, QMap<uint,int> map)
{
    QDateTime t;
    int old = 0, _new = 0;

    series->append(from.toMSecsSinceEpoch(),0);
    for(uint i : map.keys())
    {
        _new = map[i];
        t.setTime_t(i);
        if(old != _new)
        {
            series->append(t.toMSecsSinceEpoch(),old);
        }
        series->append(t.toMSecsSinceEpoch(),_new);
        old = _new;
    }
    series->append(to.toMSecsSinceEpoch(),_new);
    series->append(to.toMSecsSinceEpoch(),0);
}

