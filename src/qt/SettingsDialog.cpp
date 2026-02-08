#include "SettingsDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QStringConverter>
#include <QHBoxLayout>

namespace aether {

static QString settingsFilePath() {
    return QApplication::applicationDirPath() + QLatin1String("/settings.ini");
}

static QString readIniValue(const QString& path, const QString& key) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0) continue;
        if (line.left(eq).trimmed() == key)
            return line.mid(eq + 1).trimmed();
    }
    return QString();
}

static void writeIniValues(const QString& path, const QMap<QString, QString>& map) {
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        for (auto it = map.begin(); it != map.end(); ++it)
            out << it.key() << "=" << it.value() << "\n";
        f.close();
    }
}

static QMap<QString, QString> readIniAll(const QString& path) {
    QMap<QString, QString> map;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return map;
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        QString line = in.readLine();
        int eq = line.indexOf(QLatin1Char('='));
        if (eq > 0) {
            QString k = line.left(eq).trimmed();
            QString v = line.mid(eq + 1).trimmed();
            map.insert(k, v);
        }
    }
    f.close();
    return map;
}

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Settings"));
    setMinimumWidth(420);

    QFormLayout* form = new QFormLayout(this);

    m_ffmpegPath = new QLineEdit(this);
    m_ffmpegPath->setPlaceholderText(tr("e.g. C:/ffmpeg"));
    QPushButton* browseBtn = new QPushButton(tr("Browse..."), this);
    connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::browseFfmpeg);
    QHBoxLayout* ffmpegRow = new QHBoxLayout;
    ffmpegRow->addWidget(m_ffmpegPath, 1);
    ffmpegRow->addWidget(browseBtn);
    form->addRow(tr("FFmpeg folder:"), ffmpegRow);

    m_skipFfmpegCheck = new QCheckBox(tr("Skip FFmpeg check at startup (not recommended)"), this);
    form->addRow(QString(), m_skipFfmpegCheck);

    m_proxyDir = new QLineEdit(this);
    m_proxyDir->setPlaceholderText(tr("e.g. ./cache/proxies"));
    QPushButton* proxyBrowseBtn = new QPushButton(tr("Browse..."), this);
    connect(proxyBrowseBtn, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select proxy cache folder"), m_proxyDir->text());
        if (!dir.isEmpty()) m_proxyDir->setText(QDir::toNativeSeparators(dir));
    });
    QHBoxLayout* proxyRow = new QHBoxLayout;
    proxyRow->addWidget(m_proxyDir, 1);
    proxyRow->addWidget(proxyBrowseBtn);
    form->addRow(tr("Proxy cache folder:"), proxyRow);

    m_useProxyForPlayback = new QCheckBox(tr("Use proxy for playback when available"), this);
    form->addRow(QString(), m_useProxyForPlayback);

    m_playbackResolution = new QComboBox(this);
    m_playbackResolution->addItem(tr("Full"), QStringLiteral("full"));
    m_playbackResolution->addItem(tr("Half (1/2)"), QStringLiteral("half"));
    m_playbackResolution->addItem(tr("Quarter (1/4)"), QStringLiteral("quarter"));
    form->addRow(tr("Playback resolution (proxy):"), m_playbackResolution);

    m_performanceProfile = new QComboBox(this);
    m_performanceProfile->addItem(tr("Normal"), QStringLiteral("normal"));
    m_performanceProfile->addItem(tr("Low RAM (8GB)"), QStringLiteral("lowram"));
    m_performanceProfile->setToolTip(tr("Low RAM enables Use proxy by default and Half playback resolution for 8GB systems."));
    connect(m_performanceProfile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        if (m_performanceProfile->currentData().toString() == QLatin1String("lowram")) {
            m_useProxyForPlayback->setChecked(true);
            int halfIdx = m_playbackResolution->findData(QStringLiteral("half"));
            if (halfIdx >= 0) m_playbackResolution->setCurrentIndex(halfIdx);
        }
    });
    form->addRow(tr("Performance profile:"), m_performanceProfile);

    QPushButton* saveBtn = new QPushButton(tr("Save"), this);
    QPushButton* cancelBtn = new QPushButton(tr("Cancel"), this);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addStretch();
    buttons->addWidget(saveBtn);
    buttons->addWidget(cancelBtn);
    form->addRow(buttons);

    loadSettings();
}

void SettingsDialog::loadSettings() {
    QString path = settingsFilePath();
    m_ffmpegPath->setText(readIniValue(path, QStringLiteral("FFmpegDir")));
    if (m_ffmpegPath->text().isEmpty())
        m_ffmpegPath->setText(QStringLiteral("C:/ffmpeg"));
    m_skipFfmpegCheck->setChecked(readIniValue(path, QStringLiteral("SkipFFmpegCheck")) == QStringLiteral("1"));
    m_proxyDir->setText(readIniValue(path, QStringLiteral("ProxyDir")));
    if (m_proxyDir->text().isEmpty())
        m_proxyDir->setText(QStringLiteral("./cache/proxies"));
    m_useProxyForPlayback->setChecked(readIniValue(path, QStringLiteral("UseProxyForPlayback")) == QStringLiteral("1"));
    QString res = readIniValue(path, QStringLiteral("ProxyResolution"));
    int idx = m_playbackResolution->findData(res.isEmpty() ? QStringLiteral("full") : res);
    m_playbackResolution->setCurrentIndex(idx >= 0 ? idx : 0);
    QString profile = readIniValue(path, QStringLiteral("PerformanceProfile"));
    if (profile == QLatin1String("lowram")) {
        int pi = m_performanceProfile->findData(QStringLiteral("lowram"));
        if (pi >= 0) m_performanceProfile->setCurrentIndex(pi);
        m_useProxyForPlayback->setChecked(true);
        int halfIdx = m_playbackResolution->findData(QStringLiteral("half"));
        if (halfIdx >= 0) m_playbackResolution->setCurrentIndex(halfIdx);
    }
}

void SettingsDialog::saveSettings() {
    QString dir = m_ffmpegPath->text().trimmed();
    if (!dir.isEmpty()) {
        QDir d(dir);
        if (!d.exists()) {
            QMessageBox::warning(this, tr("Settings"),
                tr("FFmpeg folder does not exist. It will be used after you install FFmpeg there or choose another folder."));
        }
    }
    QString path = settingsFilePath();
    QMap<QString, QString> map = readIniAll(path);
    map.insert(QStringLiteral("FFmpegDir"), dir.isEmpty() ? QStringLiteral("C:/ffmpeg") : dir);
    map.insert(QStringLiteral("SkipFFmpegCheck"), m_skipFfmpegCheck->isChecked() ? QStringLiteral("1") : QStringLiteral("0"));
    QString proxyDir = m_proxyDir->text().trimmed();
    map.insert(QStringLiteral("ProxyDir"), proxyDir.isEmpty() ? QStringLiteral("./cache/proxies") : proxyDir);
    map.insert(QStringLiteral("UseProxyForPlayback"), m_useProxyForPlayback->isChecked() ? QStringLiteral("1") : QStringLiteral("0"));
    map.insert(QStringLiteral("ProxyResolution"), m_playbackResolution->currentData().toString());
    map.insert(QStringLiteral("PerformanceProfile"), m_performanceProfile->currentData().toString());
    writeIniValues(path, map);
    QMessageBox::information(this, tr("Settings"), tr("Settings saved. Restart the application for FFmpeg path to take effect."));
    accept();
}

void SettingsDialog::browseFfmpeg() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select FFmpeg folder"),
        m_ffmpegPath->text().isEmpty() ? QStringLiteral("C:/") : m_ffmpegPath->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
        m_ffmpegPath->setText(QDir::toNativeSeparators(dir));
}

} // namespace aether
