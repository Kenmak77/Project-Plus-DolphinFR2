#pragma once

#include <QDialog>
#include <QString>
#include <QLabel>
#include <QProgressBar>
#include <memory>
#include <functional>

class InstallUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InstallUpdateDialog(QWidget* parent,
                                 QString installationDirectory,
                                 QString temporaryDirectory,
                                 QString filename,
                                 QString downloadUrl,
                                 QString sdUrl);
    ~InstallUpdateDialog();

protected:
    void timerEvent(QTimerEvent* e) override;
    void closeEvent(QCloseEvent* event) override;

private:
    // === Étapes principales ===
    void ensureDependenciesThen(std::function<void()> cont);
    void checkIfAllDownloadsFinished(bool sdFinished, bool sdSuccess);
    void download();                // ✅ manquait
    void startSDDownload();         // ✅ manquait
    void install();
    bool m_isClosing = false;
    bool unzipFile(const std::string& zipFilePath,
                   const std::string& destDir,
                   std::function<void(int, int)> progressCallback);

    // === Fallbacks ===
    void startRcloneFallback(const QString& sdUrl,
                             const QString& sdPath,
                             std::shared_ptr<bool> sdFinished,
                             std::shared_ptr<bool> sdSuccess,
                             std::function<void(int, const QString&)> uiProgress,
                             std::function<void(bool, const QString&)> uiDone);

    void startHttpFallback(const QString& sdUrl,
                           const QString& sdPath,
                           std::shared_ptr<bool> sdFinished,
                           std::shared_ptr<bool> sdSuccess,
                           std::function<void(int, const QString&)> uiProgress,
                           std::function<void(bool, const QString&)> uiDone);

private:
    // === Variables ===
    QString installationDirectory;
    QString temporaryDirectory;
    QString filename;
    QString downloadUrl;
    QString m_sdUrl;

    QThread* m_hashThread = nullptr;

    QLabel* label;
    QLabel* stepLabel;

    QProgressBar* progressBar;
    QProgressBar* stepProgressBar;
};
