#include "MainWindow.h"
#include "DrawingWidget.h"

#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QGroupBox>
#include <QFont>
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    Ui::MainWindow ui;
    ui.setupUi(this);

    m_btnCreateSecond   = findChild<QPushButton*>("btnCreateSecond");
    m_btnFinalizeSecond = findChild<QPushButton*>("btnFinalizeSecond");
    m_btnCompute        = findChild<QPushButton*>("btnCompute");
    m_btnClear = findChild<QPushButton*>("btnClear");
    if (!m_btnClear) m_btnClear = findChild<QPushButton*>("pushButtonClear");
    if (!m_btnClear) m_btnClear = findChild<QPushButton*>("pushButton_4");

    m_rbIntersect  = findChild<QRadioButton*>("rbIntersect");
    m_rbUnion      = findChild<QRadioButton*>("rbUnion");
    m_rbDifference = findChild<QRadioButton*>("rbDifference");

    QWidget* placeholder = findChild<QWidget*>("canvasPlaceholder");
    m_canvas = new CanvasWidget(this);

    if (placeholder) {
        auto* parentLay = qobject_cast<QBoxLayout*>(placeholder->parentWidget()->layout());
        if (parentLay) {
            int idx = parentLay->indexOf(placeholder);
            parentLay->insertWidget(idx, m_canvas, 1);
            placeholder->hide();
            placeholder->deleteLater();
        } else {
            QWidget* cw = findChild<QWidget*>("centralwidget");
            if (!cw) cw = new QWidget(this), setCentralWidget(cw);
            auto* lay = new QHBoxLayout(cw);
            lay->setContentsMargins(10,10,10,10);
            lay->addWidget(m_canvas, 1);
        }
    } else {
        setCentralWidget(m_canvas);
    }

    // === УЛУЧШЕННЫЕ СТИЛИ КНОПОК ===
    auto styleButton = [](QPushButton* btn, const QString& color) {
        if (!btn) return;
        QString style = QString(R"(
            QPushButton {
                background-color: %1;
                color: white;
                border: 2px solid %2;
                padding: 8px 16px;
                border-radius: 6px;
                font-weight: bold;
                font-size: 12px;
                min-width: 140px;
            }
            QPushButton:hover {
                background-color: %3;
                border: 2px solid white;
            }
            QPushButton:pressed {
                background-color: %4;
                padding: 7px 15px;
            }
            QPushButton:disabled {
                background-color: #cccccc;
                border: 2px solid #aaaaaa;
                color: #888888;
            }
        )").arg(color)
                            .arg(QColor(color).darker(130).name())
                            .arg(QColor(color).lighter(120).name())
                            .arg(QColor(color).darker(120).name());
        btn->setStyleSheet(style);
    };

    if (m_btnCreateSecond) styleButton(m_btnCreateSecond, "#1E90FF");    // Синий
    if (m_btnFinalizeSecond) styleButton(m_btnFinalizeSecond, "#32CD32"); // Зеленый
    if (m_btnCompute) styleButton(m_btnCompute, "#FF4500");               // Оранжевый
    if (m_btnClear) styleButton(m_btnClear, "#708090");                   // Серый

    // === СТИЛИ РАДИОКНОПОК ===
    QString radioStyle = R"(
        QRadioButton {
            spacing: 8px;
            font-size: 12px;
            font-weight: bold;
            padding: 4px;
        }
        QRadioButton::indicator {
            width: 14px;
            height: 14px;
            border-radius: 7px;
            border: 2px solid #555;
        }
        QRadioButton::indicator:checked {
            background-color: #FF4500;
            border: 2px solid #FF4500;
        }
        QRadioButton::indicator:checked:hover {
            background-color: #FF6347;
            border: 2px solid #FF6347;
        }
    )";

    if (m_rbIntersect) {
        m_rbIntersect->setStyleSheet(radioStyle + " QRadioButton { color: #FF4500; }");
        m_rbIntersect->setToolTip("Пересечение полигонов A и B");
    }
    if (m_rbUnion) {
        m_rbUnion->setStyleSheet(radioStyle + " QRadioButton { color: #32CD32; }");
        m_rbUnion->setToolTip("Объединение полигонов A и B");
    }
    if (m_rbDifference) {
        m_rbDifference->setStyleSheet(radioStyle + " QRadioButton { color: #1E90FF; }");
        m_rbDifference->setToolTip("Разность полигонов (A минус B)");
    }

    // === СТАТУС БАР ===
    statusBar()->setStyleSheet(R"(
        QStatusBar {
            background-color: #F8F8F8;
            color: #333333;
            font-size: 11px;
            border-top: 1px solid #DDDDDD;
        }
    )");

    statusBar()->showMessage("Готов к работе. Добавляйте точки полигона A кликами мыши", 3000);

    connect(m_btnCreateSecond,   &QPushButton::clicked, this, &MainWindow::onCreateSecond);
    connect(m_btnFinalizeSecond, &QPushButton::clicked, this, &MainWindow::onFinalizeSecond);
    connect(m_btnCompute,        &QPushButton::clicked, this, &MainWindow::onCompute);
    connect(m_btnClear,          &QPushButton::clicked, this, &MainWindow::onClearAll);
}


void MainWindow::onCreateSecond() {
    if (m_canvas) {
        m_canvas->finalizeFirst();
        statusBar()->showMessage("Теперь добавляйте точки полигона B. Двойной клик или кнопка 'Завершить второй' для завершения.", 3000);
    }
}

void MainWindow::onFinalizeSecond() {
    if (m_canvas) {
        m_canvas->finalizeSecond();
        statusBar()->showMessage("Оба полигона готовы! Выберите операцию и нажмите 'Выполнить операцию'.", 3000);
    }
}

void MainWindow::onCompute() {
    if (!m_canvas) return;

    QString opName;
    if (m_rbIntersect && m_rbIntersect->isChecked()) {
        m_canvas->setOp(CanvasWidget::Op::Intersect);
        opName = "Пересечение";
    } else if (m_rbUnion && m_rbUnion->isChecked()) {
        m_canvas->setOp(CanvasWidget::Op::Union);
        opName = "Объединение";
    } else if (m_rbDifference && m_rbDifference->isChecked()) {
        m_canvas->setOp(CanvasWidget::Op::Difference);
        opName = "Разность (A \\ B)";
    }

    statusBar()->showMessage("Операция '" + opName + "' выполнена. Результат показан на рисунке.", 3000);
}

void MainWindow::onClearAll() {
    if (!m_canvas) return;
    m_canvas->clearAll();
    statusBar()->showMessage("Сцена очищена. Начните добавлять точки полигона A.", 2000);
}
