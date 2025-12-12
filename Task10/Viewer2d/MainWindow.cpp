#include "MainWindow.h"
#include "CanvasWidget.h"

#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QStyle>
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

    // Стилизация кнопок
    if (m_btnCreateSecond) {
        m_btnCreateSecond->setStyleSheet(
            "QPushButton {"
            "   background-color: #2196F3;"  // Синий
            "   color: white;"
            "   font-weight: bold;"
            "   padding: 8px;"
            "   border-radius: 5px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #1976D2;"
            "}"
            );
    }

    if (m_btnFinalizeSecond) {
        m_btnFinalizeSecond->setStyleSheet(
            "QPushButton {"
            "   background-color: #4CAF50;"  // Зеленый
            "   color: white;"
            "   font-weight: bold;"
            "   padding: 8px;"
            "   border-radius: 5px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #388E3C;"
            "}"
            );
    }

    if (m_btnCompute) {
        m_btnCompute->setStyleSheet(
            "QPushButton {"
            "   background-color: #F44336;"  // Красный
            "   color: white;"
            "   font-weight: bold;"
            "   padding: 8px;"
            "   border-radius: 5px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #D32F2F;"
            "}"
            );
    }

    if (m_btnClear) {
        m_btnClear->setStyleSheet(
            "QPushButton {"
            "   background-color: #9E9E9E;"  // Серый
            "   color: white;"
            "   font-weight: bold;"
            "   padding: 8px;"
            "   border-radius: 5px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #757575;"
            "}"
            );
    }

    // Стилизация радиокнопок
    if (m_rbIntersect) {
        m_rbIntersect->setStyleSheet(
            "QRadioButton {"
            "   color: #F44336;"  // Красный текст
            "   font-weight: bold;"
            "   spacing: 8px;"
            "}"
            "QRadioButton::indicator {"
            "   width: 16px;"
            "   height: 16px;"
            "}"
            );
    }

    if (m_rbUnion) {
        m_rbUnion->setStyleSheet(
            "QRadioButton {"
            "   color: #4CAF50;"  // Зеленый текст
            "   font-weight: bold;"
            "   spacing: 8px;"
            "}"
            "QRadioButton::indicator {"
            "   width: 16px;"
            "   height: 16px;"
            "}"
            );
    }

    if (m_rbDifference) {
        m_rbDifference->setStyleSheet(
            "QRadioButton {"
            "   color: #03A9F4;"  // Голубой текст
            "   font-weight: bold;"
            "   spacing: 8px;"
            "}"
            "QRadioButton::indicator {"
            "   width: 16px;"
            "   height: 16px;"
            "}"
            );
    }

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

    connect(m_btnCreateSecond,   &QPushButton::clicked, this, &MainWindow::onCreateSecond);
    connect(m_btnFinalizeSecond, &QPushButton::clicked, this, &MainWindow::onFinalizeSecond);
    connect(m_btnCompute,        &QPushButton::clicked, this, &MainWindow::onCompute);
    connect(m_btnClear,          &QPushButton::clicked, this, &MainWindow::onClearAll);

    // Соединяем радиокнопки с изменением операции
    if (m_rbIntersect) {
        connect(m_rbIntersect, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked && m_canvas) {
                m_canvas->setOp(CanvasWidget::Op::Intersect);
                statusBar()->showMessage("Операция: Пересечение");
            }
        });
    }

    if (m_rbUnion) {
        connect(m_rbUnion, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked && m_canvas) {
                m_canvas->setOp(CanvasWidget::Op::Union);
                statusBar()->showMessage("Операция: Объединение");
            }
        });
    }

    if (m_rbDifference) {
        connect(m_rbDifference, &QRadioButton::toggled, this, [this](bool checked) {
            if (checked && m_canvas) {
                m_canvas->setOp(CanvasWidget::Op::Difference);
                statusBar()->showMessage("Операция: Разность (A \\ B)");
            }
        });
    }

    statusBar()->showMessage("ЛКМ — добавление/перетаскивание точек. "
                             "'Создать второй' — завершить A; "
                             "'Завершить второй' — завершить B.");
}


void MainWindow::onCreateSecond() {
    if (m_canvas) {
        m_canvas->finalizeFirst();
        statusBar()->showMessage("Добавляйте точки полигона B. 'Завершить второй' — завершить B.");
    }
}

void MainWindow::onFinalizeSecond() {
    if (m_canvas) {
        m_canvas->finalizeSecond();
        statusBar()->showMessage("Оба полигона зафиксированы. Выберите операцию или очистите сцену.");
    }
}

void MainWindow::onCompute() {
    if (!m_canvas) return;
    // Операция уже меняется через радиокнопки, но оставляем для совместимости
    if (m_rbIntersect && m_rbIntersect->isChecked()) {
        m_canvas->setOp(CanvasWidget::Op::Intersect);
        statusBar()->showMessage("Операция: Пересечение");
    } else if (m_rbUnion && m_rbUnion->isChecked()) {
        m_canvas->setOp(CanvasWidget::Op::Union);
        statusBar()->showMessage("Операция: Объединение");
    } else if (m_rbDifference && m_rbDifference->isChecked()) {
        m_canvas->setOp(CanvasWidget::Op::Difference);
        statusBar()->showMessage("Операция: Разность (A \\ B)");
    }
}


void MainWindow::onClearAll() {
    if (!m_canvas) return;
    m_canvas->clearAll();
    statusBar()->showMessage("Сцена очищена. Начните с полигона A.");

    // Сброс радиокнопок к пересечению
    if (m_rbIntersect) {
        m_rbIntersect->setChecked(true);
    }
}
