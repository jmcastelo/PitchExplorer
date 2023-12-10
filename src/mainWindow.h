#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fourier.h"
#include "pca.h"
#include "kmeans.h"
#include "hurst.h"
#include "flowLayout.h"
#include "qcustomplot.h"
#include <array>
#include <QWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QProgressBar>
#include <QMediaPlayer>
#include <QCheckBox>
#include <QRadioButton>
#include <QTabWidget>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void about();
    void showFileDecodingFailedDialog();
    void disablePCAActions();
    void disableKMeansActions();
    void disableHurstActions();
    void enableFFTActions();
    void enableHurstActions();
    void onFFTPerformed();
    void performPCA();
    void onPCAStarted();
    void onPCAPerformed();
    void onPCAAborted();
    void performKMeans();
    void onKMeansStarted();
    void onKMeansPerformed();
    void performHurst();
    void onHurstStarted();
    void onHurstPerformed();
    void onHurstNotEnoughData();
    void updateComponentNumber(int value);
    void updateSegmentDuration(int value);
    void updateFrequencyBinSize(int value);
    void updateClusterNumber(int value);
    void updateFFTProgressBarMaximum();
    void onDataFileSelected(const QString path);
    void loadAudio(const QString path);
    void togglePlayback(bool checked);
    void selectCurrentSegment(qint64 position);
    void changePlaybackPosition(int index);
    void onPlayerStateChanged(QMediaPlayer::State state);
    void updatePositionLabel(qint64 position);
    void deleteClusterButtons();
    void clearFFTGraphs();
    void clearPCAGraphs();
    void clearClusterHistogram();
    void clearRescaledRangeGraph();
    void clearIntervalGraphs();
    void toggleXAxisScale(int state);
    void toggleYAxisScale(int state);
    void showSpectrumPointValue(QMouseEvent *event);
    void setWaveFormGraph();
    void setWaveFormFullGraph();
    void shiftWaveFormFullCursor(qint64 position);
    void setSpectrogram();
    void shiftSpectrogramCursor(qint64 position);
    void replotSpectrumGraph(qint64 position);
    void shiftWaveFormGraph(qint64 position);
    void setPCAGraphs();
    void setPCAClusteredGraphs();
    void selectCurrentPCAPoint(qint64 position);
    void setClusterHistogram();
    void setClusterLengthHistogram();
    void setRescaledRangeGraph();
    void setIntervalGraph();
    void setCumulativeIntervalGraph();

private:
    Fourier *fourier;
    PCA *pca;
    KMeans *kmeans;
    Hurst *hurst;

    QPushButton *loadAudioFileButton;
    QPushButton *loadDataFileButton;
    QPushButton *aboutButton;
    QPushButton *helpButton;
    QPushButton *startFFTAnalysisButton;
    QPushButton *startPCAButton;
    QPushButton *abortPCAButton;
    QPushButton *startKMeansButton;
    QPushButton *startHurstButton;

    QFileDialog *loadAudioFileDialog;
    QFileDialog *loadDataFileDialog;

    QLabel *sampleRateLabel;
    QLabel *samplesPerSegmentLabel;
    QLabel *segmentsLabel;
    QLabel *frequenciesLabel;
    QLabel *frequencyBinsLabel;
    QLabel *iterationLabel;
    QLabel *pcaIterationLabel;
    QLabel *hurstExponentLabel;

    QSpinBox *segmentDurationSpinBox;
    QSpinBox *frequencyBinSizeSpinBox;
    QSpinBox *componentNumberSpinBox;
    QSpinBox *clusterNumberSpinBox;

    QProgressBar *fftProgressBar;
    QProgressBar *pcaProgressBar;

    QRadioButton *onPCAData;
    QRadioButton *onFFTData;

    FlowLayout *clusterButtonsLayout;
    QVector<QPushButton*> clusterButtons;
    QPushButton *currentClusterButton;
    int currentClusterButtonIndex;
    int clusterNumber;
    int milliseconds;

    QMediaPlayer *player;
    QLabel *positionLabel;
    QPushButton *playPauseButton;

    QCustomPlot *spectrumGraph;
    QCPItemTracer *itemTracer;
    QCheckBox *xAxisScaleCheckBox;
    QCheckBox *yAxisScaleCheckBox;

    QCustomPlot *waveFormGraph;

    QCustomPlot *waveFormFullGraph;
    QCPItemLine *waveFormFullCursor;

    QCustomPlot *spectrogramGraph;
    QCPColorMap *spectrogram;
    QCPItemLine *spectrogramCursor;

    QCustomPlot *pc1pc2Graph;
    QCustomPlot *pc1pc3Graph;
    QCustomPlot *pc2pc3Graph;
    QCPGraph *currentPoint12Graph;
    QCPGraph *currentPoint13Graph;
    QCPGraph *currentPoint23Graph;

    QCheckBox *hullsToggleCheckBox;

    QCustomPlot *clusterHistogramGraph;
    QCPBars *clusterHistogram;

    QCustomPlot *clusterLengthHistogramGraph;
    QCPBars *clusterLengthHistogram;

    QCustomPlot *rescaledRangeGraph;
    QCPItemLine *randomLine;
    QCPItemLine *linearRegressionLine;

    QCustomPlot *intervalGraph;
    QCustomPlot *cumulativeIntervalGraph;

    QTabWidget *graphsTabWidget;

    int computeSegmentNumber();
    void createClusterButtons();
    void setConvexHulls(const QVector<QVector<std::array<double, 2>>> &points, QCustomPlot *graph);
    QVector<std::array<double, 2>> computeConvexHulls(QVector<std::array<double, 2>> points);
    bool removeMiddle(std::array<double, 2> a, std::array<double, 2> b, std::array<double, 2> c);
    std::array<double, 2> computeCentroid(QVector<std::array<double, 2>> vertices);
    QString msToTime(int ms);
};

#endif
