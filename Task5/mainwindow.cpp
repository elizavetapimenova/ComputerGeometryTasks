#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *controlLayout = new QHBoxLayout();

    QPushButton *buildButton = new QPushButton("Построить оболочку", this);
    QPushButton *clearButton = new QPushButton("Очистить", this);
    QCheckBox *onlineCheck = new QCheckBox("Онлайн-режим", this);
    QLabel *infoLabel = new QLabel("Клик: добавить точку. Перетащите точку для перемещения.", this);

    controlLayout->addWidget(buildButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(onlineCheck);
    controlLayout->addWidget(infoLabel);
    controlLayout->addStretch();

    mainLayout->addLayout(controlLayout);

    connect(buildButton, &QPushButton::clicked, this, &MainWindow::onBuildHullClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(onlineCheck, &QCheckBox::toggled, this, &MainWindow::onToggleOnlineMode);

    setWindowTitle("Задача 1: Выпуклая оболочка");
    setMinimumSize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto& point : points) {
        drawPoint(painter, point.toQPointF());
    }

    if (showHull && convexHull.size() >= 2) {
        drawHull(painter);

        for (const auto& point : convexHull) {
            drawPoint(painter, point.toQPointF(), true);
        }
    }
}

void MainWindow::drawPoint(QPainter& painter, const QPointF& point, bool isHullPoint)
{
    painter.save();

    if (isHullPoint) {
        painter.setBrush(Qt::red);
        painter.setPen(QPen(Qt::darkRed, 2));
        painter.drawEllipse(point, 8, 8);
    } else {
        painter.setBrush(Qt::blue);
        painter.setPen(QPen(Qt::darkBlue, 1));
        painter.drawEllipse(point, 6, 6);
    }

    painter.restore();
}

void MainWindow::drawHull(QPainter& painter)
{
    if (convexHull.size() < 2) return;

    painter.save();
    painter.setPen(QPen(Qt::green, 3));

    QPolygonF polygon;
    for (const auto& point : convexHull) {
        polygon << point.toQPointF();
    }
    if (!convexHull.empty()) {
        polygon << convexHull[0].toQPointF();
    }

    painter.drawPolyline(polygon);
    painter.restore();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QPointF pos = event->pos();

    if (event->button() == Qt::LeftButton) {
        int pointIndex = findPointNear(pos);

        if (pointIndex != -1) {
            dragging = true;
            draggedPointIndex = pointIndex;
            dragStartPos = points[pointIndex].toQPointF();
        } else {
            points.push_back(Point(pos));

            if (onlineMode && showHull) {
                updateHull();
            }

            update();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!dragging || draggedPointIndex < 0 || draggedPointIndex >= (int)points.size()) {
        return;
    }

    points[draggedPointIndex] = Point(event->pos());

    if (onlineMode && showHull) {
        updateHull();
    }

    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && dragging) {
        dragging = false;

        if (!onlineMode && showHull) {
            updateHull();
        }

        update();
    }
}

int MainWindow::findPointNear(const QPointF& pos, double threshold)
{
    for (size_t i = 0; i < points.size(); ++i) {
        QPointF pointPos = points[i].toQPointF();
        if (qAbs(pointPos.x() - pos.x()) < threshold && qAbs(pointPos.y() - pos.y()) < threshold) {
            return (int)i;
        }
    }
    return -1;
}

void MainWindow::updateHull()
{
    convexHull = ConvexHull::compute(points);
}

void MainWindow::onBuildHullClicked()
{
    if (points.size() < 3) {
        return;
    }

    showHull = true;
    updateHull();
    update();
}

void MainWindow::onClearClicked()
{
    points.clear();
    convexHull.clear();
    showHull = false;
    update();
}

void MainWindow::onToggleOnlineMode(bool checked)
{
    onlineMode = checked;

    if (onlineMode && showHull) {
        updateHull();
        update();
    }
}
