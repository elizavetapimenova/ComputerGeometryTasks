#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <vector>
#include "algorithms/convex_hull.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onBuildHullClicked();
    void onClearClicked();
    void onToggleOnlineMode(bool checked);

private:
    Ui::MainWindow *ui;

    std::vector<Point> points;
    std::vector<Point> convexHull;

    bool dragging = false;
    int draggedPointIndex = -1;
    QPointF dragStartPos;

    bool onlineMode = false;
    bool showHull = false;

    int findPointNear(const QPointF& pos, double threshold = 10.0);
    void updateHull();
    void drawPoint(QPainter& painter, const QPointF& point, bool isHullPoint = false);
    void drawHull(QPainter& painter);
};

#endif
