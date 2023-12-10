#ifndef PCA_H
#define PCA_H

#include <QThread>

class PCA : public QThread
{
    Q_OBJECT

public:
    PCA(QObject *parent = nullptr);
    ~PCA() override;

    int componentNumber;
    QVector<double> eigenvalues;
    QVector<QVector<double>> principalComponents;
    QVector<double> pc1, pc2, pc3;
    double pc1Min, pc1Max;
    double pc2Min, pc2Max;
    double pc3Min, pc3Max;

    bool abort;

    void initData(QVector<QVector<double>> receivedData);
    void performPCA();
    void clearPCAData();

signals:
    void pcaIterationStep(int iterationStep);
    void pcaStep(int step);
    void pcaPerformed();
    void pcaAborted();

protected:
    void run() override;

private:
    QVector<QVector<double>> data;

    void centerColumns();
};

#endif
