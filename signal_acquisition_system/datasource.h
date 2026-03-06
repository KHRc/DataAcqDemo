#ifndef DATASOURCE_H
#define DATASOURCE_H


#include <QObject>
#include <QTimer>
#include <QVector>
#include <QDateTime>

class DataSource : public QObject
{
    Q_OBJECT
public:
    explicit DataSource(int channels = 32, double sampleRate = 51200.0, QObject *parent = nullptr);
    
    void start();
    void stop();
    void setSampleRate(double rate);
    void setChannels(int channels);

signals:
    void dataReady(const QVector<double>& data, qint64 timestamp);

private slots:
    void generateData();

private:
    QTimer* m_timer;
    int m_channels;
    double m_sampleRate;
    int m_sampleCount;
    double M_PI = 3.14;
};

#endif // DATASOURCE_H
