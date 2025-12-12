#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QPushButton; class QRadioButton; class QWidget;
QT_END_NAMESPACE
class CanvasWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onCreateSecond();
    void onFinalizeSecond();
    void onCompute();
    void onClearAll();
private:
    QPushButton*   m_btnClear{nullptr};
    CanvasWidget*  m_canvas{nullptr};
    QPushButton*   m_btnCreateSecond{nullptr};
    QPushButton*   m_btnFinalizeSecond{nullptr};
    QPushButton*   m_btnCompute{nullptr};
    QRadioButton*  m_rbIntersect{nullptr};
    QRadioButton*  m_rbUnion{nullptr};
    QRadioButton*  m_rbDifference{nullptr};
};
