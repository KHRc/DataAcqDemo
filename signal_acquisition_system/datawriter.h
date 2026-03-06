#ifndef DATAWRITER_H
#define DATAWRITER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <QDateTime>
#include <QMutex>
#include <QThread>

class DataWriter : public QObject
{
    Q_OBJECT
public:
    explicit DataWriter(int channels = 32, QObject* parent = nullptr);
    ~DataWriter();

    void writeData(const QVector<double>& data, qint64 timestamp);
    void pause();
    void resume();
    void setFileInterval(int minutes);

public slots:
    void closeCurrentFile();

private:
    void openNewFile();
    void checkFileRotation();

private:
    QFile m_file;
    QTextStream m_stream;
    int m_channels;
    QDateTime m_fileStartTime;
    int m_fileInterval;  // 文件分段间隔(分钟)
    bool m_isPaused;
    QMutex m_mutex;
    qint64 m_dataCount;
};

#endif // DATAWRITER_H
