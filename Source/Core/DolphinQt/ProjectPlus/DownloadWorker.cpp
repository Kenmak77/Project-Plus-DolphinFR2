#include "DownloadWorker.h"
#include "Common/HttpRequest.h"
#include <QFile>
#include <QDebug>
#include <QProcess>

DownloadWorker::DownloadWorker(const QString& url, const QString& filename)
    : url(url), filename(filename)
{
}

void DownloadWorker::startDownload()
{
    bool success = false;

    // 1️⃣ aria2c (multithreaded, rapide)
    int ariaResult = QProcess::execute(
        QStringLiteral("aria2c"),
        { QStringLiteral("-x4"), QStringLiteral("-s4"),
          QStringLiteral("-o"), filename, url });

    if (ariaResult == 0)
    {
        qDebug().noquote() << "✅ Download succeeded with aria2c";
        success = true;
    }

    // 2️⃣ fallback rclone
    if (!success)
    {
        int rcloneResult = QProcess::execute(
            QStringLiteral("rclone"),
            { QStringLiteral("copyurl"), url, filename, QStringLiteral("--no-check-certificate") });
        if (rcloneResult == 0)
        {
            qDebug().noquote() << "✅ Download succeeded with rclone";
            success = true;
        }
    }

    // 3️⃣ fallback HTTP interne
    if (!success)
    {
        Common::HttpRequest request;
        request.FollowRedirects();

        auto response = request.Get(url.toStdString());
        if (response)
        {
            QFile f(filename);
            if (f.open(QIODevice::WriteOnly))
            {
                f.write(reinterpret_cast<const char*>(response->data()), response->size());
                f.close();
                qDebug().noquote() << "✅ Download succeeded with HTTP fallback";
                success = true;
            }
        }
    }

    if (success)
        emit finished();
    else
        emit errorOccurred(QStringLiteral("All download methods failed (aria2c, rclone, HTTP)."));
}

void DownloadWorker::updateProgress(qint64 dlnow, qint64 dltotal)
{
    emit progressUpdated(dlnow, dltotal);
}
