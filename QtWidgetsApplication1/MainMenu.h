#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_MainMenuNEW.h"
#include <qchartview.h>

class QtWidgetsApplication1 : public QMainWindow {
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget* parent = nullptr);
    ~QtWidgetsApplication1();

private slots:
    void showMoney();
    void updateCurrencyRates();
    void showChart();
    void updateFromChart();
    void updateToChart();


private:
    Ui::QtWidgetsApplication1Class ui;
    std::map<std::string, double> ratesMap;

    QChartView* chartViewFrom = nullptr;
    QChartView* chartViewTo = nullptr;

    QChartView* chartView = nullptr;
};
