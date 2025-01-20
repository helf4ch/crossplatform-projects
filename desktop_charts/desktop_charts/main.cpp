#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineSeries>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class WeatherApp : public QMainWindow {
  Q_OBJECT

public:
  WeatherApp(QWidget *parent = nullptr) : QMainWindow(parent) {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *controlsLayout = new QHBoxLayout();

    {
      QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      controlsLayout->addItem(spacer);
    }

    QLabel *hoursLabel = new QLabel("Период данных (в часах):", this);
    controlsLayout->addWidget(hoursLabel);

    hoursCombo = new QComboBox(this);
    hoursCombo->addItems({"1", "3", "6", "12", "24"});
    controlsLayout->addWidget(hoursCombo);


    QLabel *paramLabel = new QLabel("Параметр:", this);
    controlsLayout->addWidget(paramLabel);

    paramCombo = new QComboBox(this);
    paramCombo->addItems({"Температура", "Давление", "Влажность",
                          "Скорость ветра", "По ощущению"});

    controlsLayout->addWidget(paramCombo);

    QPushButton *updateButton = new QPushButton("Обновить данные", this);
    controlsLayout->addWidget(updateButton);

    {
      QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      controlsLayout->addItem(spacer);
    }

    mainLayout->addLayout(controlsLayout);

    chart = new QChart();
    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(chartView);

    setCentralWidget(centralWidget);

    networkManager = new QNetworkAccessManager(this);

    connect(updateButton, &QPushButton::clicked, this, &WeatherApp::fetchData);
    connect(networkManager, &QNetworkAccessManager::finished, this,
            &WeatherApp::handleReply);

    fetchData();
  }

private slots:
  void fetchData() {
    QString hours = hoursCombo->currentText();
    QString url = QString("http://127.0.0.1:8080/get?hours=%1").arg(hours);

    QNetworkRequest request(QUrl{url});
    networkManager->get(request);
  }

  void handleReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
      qWarning() << "Network error. " << reply->errorString();
      reply->deleteLater();
      return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (!jsonDoc.isArray()) {
      qWarning() << "Data error.";
      reply->deleteLater();
      return;
    }

    QJsonArray dataArray = jsonDoc.array();

    QString param = paramCombo->currentText();
    QString field = name_to_field[param];

    if (dataArray.isEmpty()) {
      qWarning() << "Empty list.";
      reply->deleteLater();
      return;
    }

    QVector<QPointF> dataPoints;
    for (const QJsonValue &value : dataArray) {
      QJsonObject obj = value.toObject();
      double unixTime = obj["unixtime"].toDouble();
      double paramValue = obj[field].toDouble();

      dataPoints.append(QPointF(unixTime * 1000, paramValue));
    }

    updateChart(dataPoints, param);
    reply->deleteLater();
  }

  void updateChart(const QVector<QPointF> &dataPoints, const QString &param) {
    chart->removeAllSeries();

    for (auto axis : chart->axes(Qt::Horizontal)) {
      chart->removeAxis(axis);
    }
    for (auto axis : chart->axes(Qt::Vertical)) {
      chart->removeAxis(axis);
    }

    QLineSeries *series = new QLineSeries();
    series->append(dataPoints);
    chart->addSeries(series);

    QDateTimeAxis *xAxis = new QDateTimeAxis();
    xAxis->setFormat("hh:mm dd.MM");
    xAxis->setTitleText("Время");
    chart->addAxis(xAxis, Qt::AlignBottom);
    series->attachAxis(xAxis);

    QValueAxis *yAxis = new QValueAxis();
    yAxis->setTitleText(param);
    chart->addAxis(yAxis, Qt::AlignLeft);
    series->attachAxis(yAxis);

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    for (const QPointF &point : dataPoints) {
      minY = std::min(minY, point.y());
      maxY = std::max(maxY, point.y());
    }

    yAxis->setRange(minY - 1, maxY + 1);

    chart->setTitle(QString("График: %1").arg(param));
  }

private:
  QMap<QString, QString> name_to_field = {{"Время", "unixtime"},
                                          {"Температура", "temperature"},
                                          {"Давление", "pressure"},
                                          {"Влажность", "humidity"},
                                          {"Скорость ветра", "wind_speed"},
                                          {"По ощущению", "feels_like"}};

  QComboBox *hoursCombo;
  QComboBox *paramCombo;
  QChart *chart;
  QChartView *chartView;
  QNetworkAccessManager *networkManager;
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  WeatherApp window;
  window.resize(800, 600);
  window.show();
  return app.exec();
}

#include "main.moc"
