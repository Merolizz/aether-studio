#pragma once

#include <QWidget>
#include <QLabel>

namespace aether {

class PlaceholderPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlaceholderPageWidget(const QString& pageName, QWidget* parent = nullptr);
};

} // namespace aether
