#include "datawriter.h"
#include <QDir>
#include <QCoreApplication>

DataWriter::DataWriter(int channels, QObject* parent)
    : QObject(parent), m_channels(channels), m_fileInterval(5), m_isPaused(false), m_dataCount(0)
{
    // 创建数据目录
    QDir dir("data");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

DataWriter::~DataWriter()
{
    closeCurrentFile();
}

void DataWriter::writeData(const QVector<double>& data, qint64 timestamp)
{
    if (m_isPaused) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    // 检查是否需要轮换文件
    if (!m_file.isOpen()) {
        openNewFile();
    }
    else {
        checkFileRotation();
    }
    QDateTime dateTime;
    if (m_file.isOpen()) {
        // 写入时间戳
        m_stream << dateTime.currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
        for (int i = 0; i < m_channels; ++i) {
            m_stream << "," << QString::number(data[i], 'f', 6);
        }
        m_stream << "\n";
        m_stream.flush();

        m_dataCount++;
    }
}

void DataWriter::pause()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = true;
}

void DataWriter::resume()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = false;
}

void DataWriter::setFileInterval(int minutes)
{
    QMutexLocker locker(&m_mutex);
    m_fileInterval = minutes;
}

void DataWriter::openNewFile()
{
    if (m_file.isOpen()) {
        m_file.close();
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("data/signal_%1.csv").arg(timestamp);

    m_file.setFileName(filename);
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_stream.setDevice(&m_file);
        m_stream.setCodec("UTF-8");

        // 写入CSV头部
        m_stream << "timestamp";
        for (int i = 0; i < m_channels; ++i) {
            m_stream << ",channel" << i;
        }
        m_stream << "\n";
        m_stream.flush();

        m_fileStartTime = QDateTime::currentDateTime();
        m_dataCount = 0;
    }
}

void DataWriter::checkFileRotation()
{
    QDateTime now = QDateTime::currentDateTime();
    qint64 elapsed = m_fileStartTime.secsTo(now);

    // 每5分钟创建新文件
    if (elapsed >= m_fileInterval * 60) {
        closeCurrentFile();
        openNewFile();
    }
}

void DataWriter::closeCurrentFile()
{
    QMutexLocker locker(&m_mutex);
    if (m_file.isOpen()) {
        m_stream.flush();
        m_file.close();
    }
}
