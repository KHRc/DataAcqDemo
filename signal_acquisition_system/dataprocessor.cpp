#include "dataprocessor.h"

DataProcessor::DataProcessor(int channels, int windowSize, double threshold, QObject* parent)
    : QObject(parent), m_channels(channels), m_windowSize(windowSize), m_threshold(threshold)
{
    m_dataBuffers.resize(channels);
}

void DataProcessor::processData(const QVector<double>& data, qint64 timestamp)
{
    QMutexLocker locker(&m_mutex);

    ProcessedData processed;
    processed.filteredData.resize(m_channels);
    processed.peakDetected.resize(m_channels);
    processed.timestamp = timestamp;

	double sum = 0.0;
	double avg = 0.0;
	bool isPeak = false;
    for (int i = 0; i < m_channels; ++i) {
        QQueue<double>& buffer = m_dataBuffers[i];

        // 添加新数据到缓冲区
        buffer.enqueue(data[i]);
        if (buffer.size() > m_windowSize) {
            buffer.dequeue();
        }

        // 计算移动平均值
        sum = 0.0;
        for (double val : buffer) {
            sum += val;
        }
        avg = sum / buffer.size();
        processed.filteredData[i] = avg;

        // 峰值检测
        isPeak = data[i] > m_threshold;
        processed.peakDetected[i] = isPeak;

        if (isPeak) {
            emit alertTriggered(i, data[i]);
        }
    }

    emit processedReady(processed);
}

void DataProcessor::setWindowSize(int size)
{
    QMutexLocker locker(&m_mutex);
    m_windowSize = size;
}

void DataProcessor::setThreshold(double threshold)
{
    QMutexLocker locker(&m_mutex);
    m_threshold = threshold;
}
