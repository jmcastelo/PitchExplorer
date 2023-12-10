#ifndef FOURIER_H
#define FOURIER_H

#include <QThread>
#include <complex>

class Fourier : public QThread
{
    Q_OBJECT

public:
    Fourier(QObject *parent = nullptr);
    ~Fourier() override;

    int sampleRate;
    unsigned long sampleNumber;
    int milliseconds;
    int frequencyBinSize;
    int duration;
    QVector<double> waveForm;
    QVector<double> times;
    double minWaveForm;
    double maxWaveForm;
    double minTime;
    double maxTime;
    QVector<QVector<double>> spectra;
    QVector<double> frequencies;
    double supPower;

    void clearFFTData();
    void clearWaveFormData();

signals:
    void fileRead();
    void fileDecodingFailed();
    void sendMessage(QString message);
    void fftAnalysisStep(int step);
    void fftAnalysisPerformed();

public slots:
    void readAudioFile(const QString filePath);
    void readDataFile(const QString filePath);
    void performFFTAnalysis();

protected:
    void run() override;

private:
    QVector<QVector<std::complex<double>>> spectrum;
    int nFrequencies;
    int step;

    void obtainSpectra();
};

#endif
