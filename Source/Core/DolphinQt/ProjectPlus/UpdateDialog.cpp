/*
*  Project+ Dolphin Self-Updater
*  Credit to the Mario Party Netplay team for the base code of this updater
*  Copyright (C) 2025 Tabitha Hanegan <tabithahanegan.com>
*/

#include "UpdateDialog.h"
#include "InstallUpdateDialog.h"

#include <QFileInfo>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QDesktopServices>
#include <QTemporaryDir>
#include <QFile>
#include <QSysInfo>
#include <QProcess>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "../../Common/Logging/Log.h"
#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QJsonDocument>

using namespace UserInterface::Dialog;

UpdateDialog::UpdateDialog(QWidget *parent, QJsonObject jsonObject, bool forced) 
    : QDialog(parent)
{
    this->jsonObject = jsonObject;

    // Create UI components
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create and set up the label
    label = new QLabel(this);
    QString tagName = jsonObject.value(QStringLiteral("tag_name")).toString();
    label->setText(QStringLiteral("%1 Available").arg(tagName));
    mainLayout->addWidget(label);

    // Create and set up the text edit
    textEdit = new QTextEdit(this);
    #if defined(__APPLE__)
    textEdit->setText(jsonObject.value(QStringLiteral("body")).toString());
    #elif defined(_WIN32)
    textEdit->setText(jsonObject.value(QStringLiteral("body")).toString());
    #else
    textEdit->setText(QStringLiteral("Auto Updater is not supported on your platform."));
    #endif
    textEdit->setReadOnly(true);
    mainLayout->addWidget(textEdit);

    // Create and set up the button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton* updateButton = buttonBox->button(QDialogButtonBox::Ok);
    updateButton->setText(QStringLiteral("Update"));
    mainLayout->addWidget(buttonBox);

    // Connect signals
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UpdateDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &UpdateDialog::reject);

    // Set the layout
    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("Update Available"));
    resize(400, 300);
}

UpdateDialog::~UpdateDialog()
{
}

void UpdateDialog::accept()
{
    QString urlToDownload;
    QString filenameToDownload;

    QString version = jsonObject.value(QStringLiteral("version")).toString();
    QString changelog = jsonObject.value(QStringLiteral("changelog")).toString();

    qDebug() << "JSON keys available:" << jsonObject.keys();

    // --- Étape 1 : lecture des URLs personnalisées ---
#ifdef _WIN32
    urlToDownload = jsonObject.value(QStringLiteral("download-page-windows")).toString();
#elif defined(__APPLE__)
    urlToDownload = jsonObject.value(QStringLiteral("download-page-mac")).toString();
#else
    urlToDownload = jsonObject.value(QStringLiteral("download-linux-appimage")).toString();
#endif

    // --- Étape 2 : fallback vers les assets GitHub ---
if (urlToDownload.isEmpty())
{
    QJsonArray assets = jsonObject.value(QStringLiteral("assets")).toArray();
    for (const QJsonValue& assetVal : assets)
    {
        QJsonObject asset = assetVal.toObject();
        QString name = asset.value(QStringLiteral("name")).toString().toLower();
        QString assetUrl = asset.value(QStringLiteral("browser_download_url")).toString();

#ifdef _WIN32
        if (name.contains(QStringLiteral("windows")))
            urlToDownload = assetUrl;
#elif defined(__APPLE__)
        if (name.contains(QStringLiteral("macos")) || name.contains(QStringLiteral("osx")))
            urlToDownload = assetUrl;
#else
        if (name.contains(QStringLiteral("linux")) || name.contains(QStringLiteral("appimage")))
            urlToDownload = assetUrl;
#endif
    } // ← ✅ fermeture du for ici
} // ← ✅ fermeture du if ici

    // --- Étape 3 : affichage debug ---
    QString platformName;
#ifdef _WIN32
    platformName = QStringLiteral("Windows");
#elif defined(__APPLE__)
    platformName = QStringLiteral("macOS");
#else
    platformName = QStringLiteral("Linux");
#endif

    qDebug().noquote() << QStringLiteral("Platform detected: %1 → Selected URL: %2")
                          .arg(platformName, urlToDownload);

    // --- Étape 4 : validation ---
    if (urlToDownload.isEmpty())
    {
        QMessageBox::warning(
            this,
            QStringLiteral("Update Error"),
            QStringLiteral("No valid download URL found for this platform.\n(Unsupported update file format)")
        );
        return;
    }

    // --- Étape 5 : téléchargement ---
    filenameToDownload = QFileInfo(urlToDownload).fileName();
    this->url = urlToDownload;
    this->filename = filenameToDownload;

    QDialog::accept();

    QString installationDirectory = QCoreApplication::applicationDirPath();
    QString temporaryDirectory = QDir::tempPath();

   // --- Étape 6 : récupération du lien SD ---
QString sdUrl;
QJsonArray assets = jsonObject.value(QStringLiteral("assets")).toArray();
if (!assets.isEmpty())
{
    QJsonObject firstAsset = assets.first().toObject();
    sdUrl = firstAsset.value(QStringLiteral("download-sd")).toString();
}

InstallUpdateDialog installDialog(
    this,
    installationDirectory,
    temporaryDirectory,
    filenameToDownload,
    urlToDownload,
    sdUrl // ✅ URL SD correctement extraite du JSON
);
installDialog.exec();

}


