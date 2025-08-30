#include "MainMenu.h"
#include "getCurlData.h"
#include <qtimer.h>
#include <qlineseries.h>
#include <QDate>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <qdebug.h>

QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // chart в frame1
    chartViewFrom = new QChartView();
    chartViewFrom->setRenderHint(QPainter::Antialiasing);
    
    auto layout1 = new QVBoxLayout(ui.frame);
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->addWidget(chartViewFrom);
    
    // chart в frame2
    chartViewTo = new QChartView();
    chartViewTo->setRenderHint(QPainter::Antialiasing);

    auto layout2 = new QVBoxLayout(ui.frame_2);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(chartViewTo);
    

    // сигналы
    connect(ui.comboFrom, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &QtWidgetsApplication1::updateFromChart);
    connect(ui.comboTo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &QtWidgetsApplication1::updateToChart);
    connect(ui.lineEditMoney, &QLineEdit::returnPressed,
        this, &QtWidgetsApplication1::showMoney);

    auto* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &QtWidgetsApplication1::updateCurrencyRates);
    timer->start(5 * 60 * 1000);

    updateCurrencyRates();   // заполнит комбо + вызовет showChart (мы его вырежем / адаптируем)
    updateFromChart();
    updateToChart();
}




QtWidgetsApplication1::~QtWidgetsApplication1() {
}

static QChart* buildChartForCurrency(const QString& baseCur,
    const QDate& start,
    const QDate& end,
    CurrencyFetcher& fetcher,
    const QString& quoteCur /*= "RUB"*/)
{
    std::string url = "https://api.frankfurter.app/"
        + start.toString("yyyy-MM-dd").toStdString()
        + ".." + end.toString("yyyy-MM-dd").toStdString()
        + "?from=" + baseCur.toStdString()
        + "&to=" + quoteCur.toStdString();

    std::vector<std::pair<QDate, double>> hist = fetcher.fetchHistoricalRates(url);
    qDebug() << "[buildChart]" << QString::fromStdString(url)
        << "hist.size=" << hist.size();

    QLineSeries* series = new QLineSeries();
    double minV = std::numeric_limits<double>::max();
    double maxV = std::numeric_limits<double>::lowest();

    for (auto& [d, r] : hist) {
        if (!d.isValid()) continue;
        if (r < minV) minV = r;
        if (r > maxV) maxV = r;
        series->append(QDateTime(d, QTime(0, 0)).toMSecsSinceEpoch(), r);
    }

    series->setPointsVisible(true);
    series->setPen(QPen(Qt::red, 2));

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QString("Курс %1 -> %2").arg(baseCur, quoteCur));
    chart->legend()->hide();

    QDateTimeAxis* axisX = new QDateTimeAxis;
    axisX->setFormat("dd.MM");
    axisX->setTitleText("Дата");
    axisX->setRange(QDateTime(start, QTime(0, 0)), QDateTime(end, QTime(0, 0)));
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis;
    axisY->setTitleText("Курс");
    axisY->setLabelFormat("%.4f");
    if (minV < maxV) {
        double pad = (maxV - minV) * 0.05;
        axisY->setRange(minV - pad, maxV + pad);
    }
    else if (minV != std::numeric_limits<double>::max()) {
        axisY->setRange(minV * 0.95, minV * 1.05);
    }
    else {
        axisY->setRange(0.0, 1.0);
    }
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    return chart;
}




void QtWidgetsApplication1::updateCurrencyRates() {
    CurrencyFetcher mapTable;
    ratesMap = mapTable.fetchRates("https://open.er-api.com/v6/latest/RUB");

    ui.comboFrom->clear();
    ui.comboTo->clear();

    for (const auto& pair : ratesMap) {
        const std::string& code = pair.first;
        QString itemText = QString::fromStdString(code);
        ui.comboFrom->addItem(itemText);
        ui.comboTo->addItem(itemText);
    }

    //Установим валюты по умолчанию
    int idxFrom = ui.comboFrom->findText("USD");
    if (idxFrom >= 0) ui.comboFrom->setCurrentIndex(idxFrom);

    int idxTo = ui.comboTo->findText("EUR");
    if (idxTo >= 0) ui.comboTo->setCurrentIndex(idxTo);

    
}



void QtWidgetsApplication1::showMoney() {
    QString text = ui.lineEditMoney->text();
    bool ok;
    double value = text.toDouble(&ok);

    if (ok) {
        QString fromCurrency = ui.comboFrom->currentText();
        QString toCurrency = ui.comboTo->currentText();

        std::string keyFrom = fromCurrency.toStdString();
        std::string keyTo = toCurrency.toStdString();

        auto itFrom = ratesMap.find(keyFrom);
        auto itTo = ratesMap.find(keyTo);

        if (itFrom != ratesMap.end() && itTo != ratesMap.end()) {
            double rateFrom = itFrom->second;
            double rateTo = itTo->second;

            double result = value * (rateTo / rateFrom);
            ui.labelMoney->setText(QString::number(result, 'f', 2));
        }
        else {
            ui.labelMoney->setText("Currency not found");
        }
    }
    else {
        ui.labelMoney->setText("Error: not a number");
    }
}

void QtWidgetsApplication1::updateFromChart() {
    QString cur = ui.comboFrom->currentText();
    if (cur.isEmpty() || !chartViewFrom)
        return;

    QDate end = QDate::currentDate();
    QDate start = end.addDays(-30);

    CurrencyFetcher fetcher;
    QChart* chart = buildChartForCurrency(cur, start, end, fetcher, "EUR");

    bool havePoints = false;
    if (chart) {
        const auto seriesList = chart->series();
        for (QAbstractSeries* abs : seriesList) {
            if (auto xy = qobject_cast<QXYSeries*>(abs)) {
                if (xy->count() > 0) {
                    havePoints = true;
                    break;
                }
            }
        }
    }

    if (havePoints) {
        chartViewFrom->setChart(chart);          // chartViewFrom возьмёт владение
    }
    else {
        delete chart;                             // не нужен -> удаляем
        QChart* empty = new QChart();
        empty->setTitle(QString("Нет данных для %1").arg(cur));
        chartViewFrom->setChart(empty);
    }
}


void QtWidgetsApplication1::updateToChart() {
    QString cur = ui.comboTo->currentText();
    if (cur.isEmpty() || !chartViewTo)
        return;

    QDate end = QDate::currentDate();
    QDate start = end.addDays(-30);

    CurrencyFetcher fetcher;
    QChart* chart = buildChartForCurrency(cur, start, end, fetcher, "EUR");

    bool havePoints = false;
    if (chart) {
        const auto seriesList = chart->series();
        for (QAbstractSeries* abs : seriesList) {
            if (auto xy = qobject_cast<QXYSeries*>(abs)) {
                if (xy->count() > 0) {
                    havePoints = true;
                    break;
                }
            }
        }
    }

    if (havePoints) {
        chartViewTo->setChart(chart);
    }
    else {
        delete chart;
        QChart* empty = new QChart();
        empty->setTitle(QString("Нет данных для %1").arg(cur));
        chartViewTo->setChart(empty);
    }
}

void QtWidgetsApplication1::showChart() {
    updateFromChart();
    updateToChart();
}




