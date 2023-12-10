#ifndef HURST_H
#define HURST_H

#include <QThread>

class Hurst : public QThread
{
    Q_OBJECT

public:
    Hurst(QObject *parent = nullptr);
    ~Hurst() override;

    double H, c;
    QVector<double> rescaledRanges;
    QVector<double> lengths;
    double minLength, maxLength;
    double minRescaledRange, maxRescaledRange;
    QVector<double> series;
    QVector<double> cumulativeSeries;
    double minSeries, maxSeries;
    double minCumulativeSeries, maxCumulativeSeries;

    void initData(QVector<int> receivedData);
    void performHurst();

signals:
    void notEnoughData();
    void hurstPerformed();

protected:
    void run() override;

private:
    QVector<int> data;

    void computeSeries();
    QVector<double> computeCumulativeSeries(QVector<double> timeSeries);
    double computeRescaledRange(QVector<double> timeSeries);
    void computeHurstExponent();
    void getMinMaxRescaledRange();
    void getMinMaxSeries();
};

#endif
