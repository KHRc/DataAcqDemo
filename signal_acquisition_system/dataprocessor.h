#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QVector>
#include <QQueue>
#include <QMutex>

struct ProcessedData {
    QVector<double> filteredData;
    QVector<bool> peakDetected;
    qint64 timestamp;
};

class DataProcessor : public QObject
{
    Q_OBJECT
public:
    explicit DataProcessor(int channels = 32, int windowSize = 512, double threshold = 2.5, QObject* parent = nullptr);

    void processData(const QVector<double>& data, qint64 timestamp);
    void setWindowSize(int size);
    void setThreshold(double threshold);

signals:
    void processedReady(const ProcessedData& processed);
    void alertTriggered(int channel, double value);

private:
    QVector<QQueue<double>> m_dataBuffers;
    int m_channels;
    int m_windowSize;
    double m_threshold;
    QMutex m_mutex;
};

#endif // DATAPROCESSOR_H
