#ifndef KMEANS_H
#define KMEANS_H

#include <QThread>

class KMeans : public QThread
{
    Q_OBJECT

public:
    KMeans(QObject *parent = nullptr);
    ~KMeans() override;

    int clusterNumber;
    QVector<int> clusterIndexes;
    QVector<double> clusters;
    QVector<double> segmentsPerCluster;
    double segmentsPerClusterMax;
    QVector<double> lengths;
    QVector<double> clusterLengthHistogram;
    double clusterLengthHistogramMax;

    void initData(QVector<QVector<double>> receivedData);
    void performKMeans();
    void clearKMeansData();

signals:
    void kMeansIterationStep(int step);
    void kMeansPerformed();

protected:
    void run() override;

private:
    QVector<QVector<double>> data;

    void computeClusterHistogram(QVector<int> clusterCount);
    void reassignClusterIndexes();
    void computeClusterLengthHistogram();
    double distance(const QVector<double> &vector1, const QVector<double> &vector2);
    double distanceCheck(const QVector<double> &vector1, const QVector<double> &vector2, const double &minDistance);
};

#endif
