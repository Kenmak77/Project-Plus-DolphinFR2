#pragma once

#include <QObject>
#include <QString>

class DownloadWorker : public QObject
{
    Q_OBJECT

public:
    explicit DownloadWorker(const QString& url, const QString& filename);

public slots:
    void startDownload();                            // Lance le téléchargement
    void updateProgress(qint64 dlnow, qint64 dltotal);

signals:
    void progressUpdated(qint64 dlnow, qint64 dltotal);   // 🔹 Pour la barre de progression
    void statusTextUpdated(const QString& text);          // 🔹 Pour afficher "xx% (xx MB/s)" dans le label
    void finished();                                      // 🔹 Émis quand tout est téléchargé
    void errorOccurred(const QString& message);           // 🔹 Émis en cas d’échec

private:
    QString url;
    QString filename;
};
