
#pragma execution_character_set("utf-8")

#include "signal_acquisition.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QDateTime>



signal_acquisition::signal_acquisition(QWidget* parent)
    : QWidget(parent), m_displayChannel(0), m_maxPoints(500), m_isRunning(false)
{
    ui.setupUi(this);
    setupUI();
    setupCharts();
    connectSignals();

    // 初始化数据处理模块
    m_dataSource = new DataSource(32, 51200.0, this);
    m_dataProcessor = new DataProcessor(32, 512, 2.5, this);
    m_dataWriter = new DataWriter(32, this);

    // 连接数据流
    connect(m_dataSource, &DataSource::dataReady, this, &signal_acquisition::onDataReceived);
    connect(m_dataSource, &DataSource::dataReady, m_dataProcessor, &DataProcessor::processData);
    connect(m_dataProcessor, &DataProcessor::processedReady, this, &signal_acquisition::onProcessedReceived);
    connect(m_dataProcessor, &DataProcessor::alertTriggered, this, &signal_acquisition::onAlertTriggered);

    // 连接数据写入器
    connect(m_dataSource, &DataSource::dataReady, m_dataWriter, &DataWriter::writeData);

    m_statusLabel->setText("状态: 已停止");
}

signal_acquisition::~signal_acquisition()
{
    if (m_isRunning) {
        m_dataSource->stop();
    }
}

void signal_acquisition::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    /*setCentralWidget(centralWidget);*/

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 控制面板
    QGroupBox* controlGroup = new QGroupBox("控制面板", this);
    QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);

    m_startBtn = new QPushButton("开始采集", this);
    m_stopBtn = new QPushButton("停止采集", this);
    m_stopBtn->setEnabled(false);

    controlLayout->addWidget(m_startBtn);
    controlLayout->addWidget(m_stopBtn);

    // 参数设置
    QGroupBox* paramGroup = new QGroupBox("参数设置", this);
    QGridLayout* paramLayout = new QGridLayout(paramGroup);

    paramLayout->addWidget(new QLabel("显示通道:", this), 0, 0);
    m_channelCombo = new QComboBox(this);
    for (int i = 0; i < 32; ++i) {
        m_channelCombo->addItem(QString("通道 %1").arg(i));
    }
    paramLayout->addWidget(m_channelCombo, 0, 1);

    paramLayout->addWidget(new QLabel("采样率 (Hz):", this), 1, 0);
    m_sampleRateSpin = new QDoubleSpinBox(this);
    m_sampleRateSpin->setRange(1000.0, 100000.0);
    m_sampleRateSpin->setValue(51200.0);
    m_sampleRateSpin->setSingleStep(100.0);
    paramLayout->addWidget(m_sampleRateSpin, 1, 1);

    paramLayout->addWidget(new QLabel("告警阈值:", this), 2, 0);
    m_thresholdSpin = new QDoubleSpinBox(this);
    m_thresholdSpin->setRange(0.0, 10.0);
    m_thresholdSpin->setValue(2.5);
    m_thresholdSpin->setSingleStep(0.1);
    paramLayout->addWidget(m_thresholdSpin, 2, 1);

    // 数据写入控制
    QGroupBox* writeGroup = new QGroupBox("数据写入", this);
    QHBoxLayout* writeLayout = new QHBoxLayout(writeGroup);

    m_pauseWriteBtn = new QPushButton("暂停写入", this);
    m_resumeWriteBtn = new QPushButton("恢复写入", this);
    m_resumeWriteBtn->setEnabled(false);

    writeLayout->addWidget(m_pauseWriteBtn);
    writeLayout->addWidget(m_resumeWriteBtn);

    // 状态显示
    m_statusLabel = new QLabel("状态: 已停止", this);
    m_alertLabel = new QLabel("告警: 无", this);
    m_alertLabel->setStyleSheet("color: green;");

    // 布局
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addWidget(controlGroup);
    topLayout->addWidget(paramGroup);
    topLayout->addWidget(writeGroup);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_alertLabel);

    // 图表视图
    m_chartView = new QChartView(this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->resize(1000, 400);
    mainLayout->addWidget(m_chartView);

    // 连接信号
    connect(m_startBtn, &QPushButton::clicked, this, &signal_acquisition::onStartClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &signal_acquisition::onStopClicked);
    connect(m_channelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &signal_acquisition::onChannelChanged);
    connect(m_sampleRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &signal_acquisition::onSampleRateChanged);
    connect(m_thresholdSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        this, &signal_acquisition::onThresholdChanged);
    connect(m_pauseWriteBtn, &QPushButton::clicked, this, &signal_acquisition::onWritePauseClicked);
    connect(m_resumeWriteBtn, &QPushButton::clicked, this, &signal_acquisition::onWriteResumeClicked);
}

void signal_acquisition::setupCharts()
{
    m_chart = new QChart();
    m_chart->setTitle(QString("实时信号波形 - 通道 %1").arg(m_displayChannel));

    m_rawSeries = new QLineSeries();
    m_rawSeries->setName("原始信号");
    m_chart->addSeries(m_rawSeries);

    m_filteredSeries = new QLineSeries();
    m_filteredSeries->setName("滤波信号");
    m_chart->addSeries(m_filteredSeries);

    m_axisX = new QValueAxis();
    m_axisX->setTitleText("时间");
    m_axisX->setRange(0, m_maxPoints);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("幅值");
    m_axisY->setRange(-3, 3);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_rawSeries->attachAxis(m_axisX);
    m_rawSeries->attachAxis(m_axisY);
    m_filteredSeries->attachAxis(m_axisX);
    m_filteredSeries->attachAxis(m_axisY);

    m_chartView->setChart(m_chart);
}

void signal_acquisition::connectSignals()
{
    // 定时器用于更新图表(50ms = 20FPS)
    QTimer* chartTimer = new QTimer(this);
    connect(chartTimer, &QTimer::timeout, this, &signal_acquisition::updateCharts);
    chartTimer->start(33);
}

void signal_acquisition::onStartClicked()
{
    m_isRunning = true;
    m_dataSource->start();

    m_startBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    m_statusLabel->setText("状态: 采集中");

    // 清空缓冲区
    m_rawDataBuffer.clear();
    m_filteredDataBuffer.clear();
    m_timeBuffer.clear();
}

void signal_acquisition::onStopClicked()
{
    m_isRunning = false;
    m_dataSource->stop();

    m_startBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    m_statusLabel->setText("状态: 已停止");
}

void signal_acquisition::onDataReceived(const QVector<double>& data, qint64 timestamp)
{
    if (m_displayChannel < data.size()) {
        m_rawDataBuffer.append(data[m_displayChannel]);
        m_timeBuffer.enqueue(timestamp);

        // 保持缓冲区大小
        if (m_rawDataBuffer.size() > m_maxPoints) {
            m_rawDataBuffer.removeFirst();
            m_timeBuffer.dequeue();
        }
    }
}

void signal_acquisition::onProcessedReceived(const ProcessedData& processed)
{
    if (m_displayChannel < processed.filteredData.size()) {
        m_filteredDataBuffer.append(processed.filteredData[m_displayChannel]);

        // 保持缓冲区大小
        if (m_filteredDataBuffer.size() > m_maxPoints) {
            m_filteredDataBuffer.removeFirst();
        }
    }
}

void signal_acquisition::onAlertTriggered(int channel, double value)
{
    QString alertText = QString("告警: 通道 %1 触发阈值! 值: %2").arg(channel).arg(value, 0, 'f', 3);
    m_alertLabel->setText(alertText);
    m_alertLabel->setStyleSheet("color: red;");

    // 3秒后恢复绿色
    QTimer::singleShot(3000, this, [this]() {
        m_alertLabel->setText("告警: 无");
        m_alertLabel->setStyleSheet("color: green;");
        });
}

void signal_acquisition::updateCharts()
{
    if (!m_isRunning) {
        return;
    }

    // 更新原始信号曲线
    m_rawSeries->clear();
    for (int i = 0; i < m_rawDataBuffer.size(); ++i) {
        m_rawSeries->append(i, m_rawDataBuffer[i]);
    }

    // 更新滤波信号曲线
    m_filteredSeries->clear();
    for (int i = 0; i < m_filteredDataBuffer.size(); ++i) {
        m_filteredSeries->append(i, m_filteredDataBuffer[i]);
    }
}

void signal_acquisition::onChannelChanged(int index)
{
    m_displayChannel = index;
    //m_chart->setTitle(QString("实时信号波形 - 通道 %1").arg(m_displayChannel));

    // 清空缓冲区
    m_rawDataBuffer.clear();
    m_filteredDataBuffer.clear();
    m_timeBuffer.clear();
}

void signal_acquisition::onSampleRateChanged(double value)
{
    m_dataSource->setSampleRate(value);
}

void signal_acquisition::onThresholdChanged(double value)
{
    m_dataProcessor->setThreshold(value);
}

void signal_acquisition::onWritePauseClicked()
{
    m_dataWriter->pause();
    m_pauseWriteBtn->setEnabled(false);
    m_resumeWriteBtn->setEnabled(true);
}

void signal_acquisition::onWriteResumeClicked()
{
    m_dataWriter->resume();
    m_pauseWriteBtn->setEnabled(true);
    m_resumeWriteBtn->setEnabled(false);
}
