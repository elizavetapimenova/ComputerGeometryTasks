#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include "Canvas.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Delaunay Triangulation");

    QVBoxLayout* mainLayout = new QVBoxLayout(&window);
    DrawingWidget* drawing = new DrawingWidget();
    mainLayout->addWidget(drawing);

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    QPushButton* buildButton = new QPushButton("Build Triangulation");
    buttonLayout->addWidget(buildButton);

    QPushButton* clearButton = new QPushButton("Clear Points");
    buttonLayout->addWidget(clearButton);

    QCheckBox* autoCheck = new QCheckBox("Auto-update");
    buttonLayout->addWidget(autoCheck);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    QObject::connect(buildButton, &QPushButton::clicked, drawing, &DrawingWidget::rebuildTriangulation);

    QObject::connect(clearButton, &QPushButton::clicked, drawing, &DrawingWidget::clearPoints);

    QObject::connect(autoCheck, &QCheckBox::toggled, [=](bool checked) {
        drawing->autoUpdate = checked;
    });

    window.resize(800, 600);
    window.show();

    return app.exec();
}
