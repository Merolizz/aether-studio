#include "HomeWidget.h"
#include <QVBoxLayout>
#include <QShowEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFileDialog>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QSettings>

namespace aether {

static QString recentListPath() {
    return QApplication::applicationDirPath() + QLatin1String("/recent_projects.txt");
}

static const int kMaxRecent = 10;

void HomeWidget::refreshRecentList() {
    m_recentList->clear();
    QFile f(recentListPath());
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    while (!f.atEnd() && m_recentList->count() < kMaxRecent) {
        QString line = QString::fromUtf8(f.readLine().trimmed());
        if (line.isEmpty()) continue;
        QListWidgetItem* item = new QListWidgetItem(QDir::toNativeSeparators(line), m_recentList);
        item->setData(Qt::UserRole, line);
    }
}

HomeWidget::HomeWidget(QWidget* parent) : QWidget(parent) {
    setStyleSheet("HomeWidget { background: #1e1e1e; }");
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(24);
    mainLayout->setContentsMargins(48, 48, 48, 48);

    QLabel* title = new QLabel(tr("Aether Studio"), this);
    title->setStyleSheet("font-size: 28px; font-weight: bold; color: #e0e0e0;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);
    mainLayout->addSpacing(16);

    QLabel* sub = new QLabel(tr("Professional video editing and creative suite"), this);
    sub->setStyleSheet("font-size: 14px; color: #888;");
    sub->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(sub);
    mainLayout->addSpacing(32);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    m_newProjectBtn = new QPushButton(tr("New Project"), this);
    m_newProjectBtn->setMinimumSize(160, 40);
    m_newProjectBtn->setStyleSheet(
        "QPushButton { background: #0e639c; color: white; border: none; border-radius: 4px; font-weight: 500; }"
        "QPushButton:hover { background: #1177bb; }"
        "QPushButton:pressed { background: #0d5a8a; }"
    );
    connect(m_newProjectBtn, &QPushButton::clicked, this, &HomeWidget::onNewProject);

    m_openProjectBtn = new QPushButton(tr("Open Project..."), this);
    m_openProjectBtn->setMinimumSize(160, 40);
    m_openProjectBtn->setStyleSheet(
        "QPushButton { background: #2d2d32; color: #e0e0e0; border: 1px solid #3c3c3c; border-radius: 4px; }"
        "QPushButton:hover { background: #3c3c3c; }"
    );
    connect(m_openProjectBtn, &QPushButton::clicked, this, &HomeWidget::onOpenProject);

    btnLayout->addStretch();
    btnLayout->addWidget(m_newProjectBtn);
    btnLayout->addWidget(m_openProjectBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    mainLayout->addSpacing(24);

    QLabel* recentLabel = new QLabel(tr("Recent"), this);
    recentLabel->setStyleSheet("font-size: 12px; color: #888;");
    mainLayout->addWidget(recentLabel);
    m_recentList = new QListWidget(this);
    m_recentList->setMaximumHeight(120);
    m_recentList->setStyleSheet(
        "QListWidget { background: #252526; border: 1px solid #3f3f46; border-radius: 4px; }"
        "QListWidget::item { padding: 6px 8px; color: #ccc; }"
        "QListWidget::item:hover { background: #2d2d32; }"
    );
    connect(m_recentList, &QListWidget::itemDoubleClicked, this, &HomeWidget::onRecentClicked);
    mainLayout->addWidget(m_recentList, 0, Qt::AlignLeft);
    mainLayout->addStretch();
    refreshRecentList();
}

void HomeWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    refreshRecentList();
}

void HomeWidget::onNewProject() {
    emit newProjectRequested();
}

void HomeWidget::onOpenProject() {
    QString path = QFileDialog::getOpenFileName(this, tr("Open Project"), QString(),
        tr("Aether Project (*.aether);;All Files (*)"));
    if (path.isEmpty()) return;
    emit openRecentRequested(path);
}

void HomeWidget::onRecentClicked() {
    QListWidgetItem* item = m_recentList->currentItem();
    if (!item) return;
    QString path = item->data(Qt::UserRole).toString();
    if (path.isEmpty()) return;
    emit openRecentRequested(path);
}

} // namespace aether
