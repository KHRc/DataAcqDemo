#pragma once

#include <QtWidgets/QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>

#include <QChartView>     
#include <QChart>         
#include <QLineSeries>    
#include <QValueAxis>     


#include "datasource.h"
#include "dataprocessor.h"
#include "datawriter.h"

#include "ui_signal_acquisition.h"

using namespace QtCharts;

class signal_acquisition : public QWidget
{
    Q_OBJECT

private slots:
    void onStartClicked();
    void onStopClicked();
    void onDataReceived(const QVector<double>& data, qint64 timestamp);
    void onProcessedReceived(const ProcessedData& processed);
    void onAlertTriggered(int channel, double value);
    void updateCharts();
    void onChannelChanged(int index);
    void onSampleRateChanged(double value);
    void onThresholdChanged(double value);
    void onWritePauseClicked();
    void onWriteResumeClicked();

private:
    void setupUI();
    void setupCharts();
    void connectSignals();

private:
    // UI控件
    QPushButton* m_startBtn;
    QPushButton* m_stopBtn;
    QPushButton* m_pauseWriteBtn;
    QPushButton* m_resumeWriteBtn;
    QLabel* m_statusLabel;
    QLabel* m_alertLabel;
    QComboBox* m_channelCombo;
    QDoubleSpinBox* m_sampleRateSpin;
    QDoubleSpinBox* m_thresholdSpin;

    // 图表
    QChartView* m_chartView;
    QChart* m_chart;
    QLineSeries* m_rawSeries;
    QLineSeries* m_filteredSeries;
    QValueAxis* m_axisX;
    QValueAxis* m_axisY;

    // 数据处理模块
    DataSource* m_dataSource;
    DataProcessor* m_dataProcessor;
    DataWriter* m_dataWriter;

    // 数据缓存
    QVector<double> m_rawDataBuffer;
    QVector<double> m_filteredDataBuffer;
    QQueue<qint64> m_timeBuffer;
    int m_displayChannel;
    int m_maxPoints;
    bool m_isRunning;
public:
    signal_acquisition(QWidget *parent = nullptr);
    ~signal_acquisition();

private:
    Ui::signal_acquisitionClass ui;
};

