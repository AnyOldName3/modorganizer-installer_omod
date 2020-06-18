#pragma once

#include <optional>

#include <QVector>
#include <QWidget>

std::optional<QVector<int>> DialogSelect(QWidget* parent, const QString& title, const QVector<QString>& items,
                                         const QVector<QString>& descriptions, const QVector<QString>& pixmaps,
                                         bool multiSelect);
