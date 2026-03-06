#include "datasource.h"
#include <QRandomGenerator>
#include <cmath>

DataSource::DataSource(int channels, double sampleRate, QObject *parent)
    : QObject(parent), m_channels(channels), m_sampleRate(sampleRate), m_sampleCount(0)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DataSource::generateData);
    
    // 计算定时器间隔 (采样率51.2K，每批生成100个样本)
    int samplesPerBatch = 100;
    double intervalMs = (samplesPerBatch / m_sampleRate) * 1000.0;
    m_timer->setInterval(static_cast<int>(intervalMs));
}

void DataSource::start()
{
    m_sampleCount = 0;
    m_timer->start();
}

void DataSource::stop()
{
    m_timer->stop();
}

void DataSource::setSampleRate(double rate)
{
    m_sampleRate = rate;
    int samplesPerBatch = 100;
    double intervalMs = (samplesPerBatch / m_sampleRate) * 1000.0;
    m_timer->setInterval(static_cast<int>(intervalMs));
}

void DataSource::setChannels(int channels)
{
    m_channels = channels;
}

void DataSource::generateData()
{
	QVector<double> data(m_channels);

	//模拟32通道信号数据
	double time = 0.0;
	double signal = 0.0;
	double noise = 0.0;
	double finalValue = 0.0;
	for (int i = 0; i < m_channels; ++i) {
		// 生成模拟信号：正弦波 + 噪声
		time = m_sampleCount / m_sampleRate;
		signal = 1.0 * sin(2.0 * M_PI * 10.0 * time + i * (M_PI / 2.0)); // 10Hz正弦波，通道间相位差π/2
		noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.5;  // 噪声

		// 偶尔添加峰值
		if (QRandomGenerator::global()->generateDouble() < 0.001) {
			signal += 2.0;  // 突发峰值
		}
		finalValue = signal + noise;
		data[i] = finalValue;
	}

	m_sampleCount++;
	qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

	emit dataReady(data, timestamp);
}
