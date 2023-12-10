#include "mainWindow.h"
#include <QGroupBox>
#include <QScrollArea>
#include <QTime>
#include <QMessageBox>
#include <chrono>
#include <thread>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent): QWidget(parent)
{
    fourier = new Fourier;
    pca = new PCA;
    kmeans = new KMeans;
    hurst = new Hurst;

    currentClusterButton = nullptr;

    milliseconds = fourier->milliseconds;

    // Load audio file button

    loadAudioFileButton = new QPushButton("Load audio file");
    loadAudioFileButton->setToolTip("WAV, MP3, FLAC");

    // Load data file button

    loadDataFileButton = new QPushButton("Load data file");

    // About button

    aboutButton = new QPushButton("About");

    // Help button

    helpButton = new QPushButton("Help");

    // Player controls

    QGroupBox *playerGroupBox = new QGroupBox("Player");

    QVBoxLayout *playerLayout = new QVBoxLayout;

    positionLabel = new QLabel(this);
    positionLabel->setText("00:00:00 / 00:00:00");

    playPauseButton = new QPushButton("Play");
    playPauseButton->setCheckable(true);
    playPauseButton->setEnabled(false);

    playerLayout->addWidget(positionLabel);
    playerLayout->addWidget(playPauseButton);

    playerLayout->setAlignment(Qt::AlignCenter);

    playerGroupBox->setLayout(playerLayout);

    // Action buttons layout 0

    QVBoxLayout *actionButtonsLayout0 = new QVBoxLayout;

    actionButtonsLayout0->addWidget(loadAudioFileButton);
    actionButtonsLayout0->addWidget(loadDataFileButton);

    // Action buttons layout 1

    QVBoxLayout *actionButtonsLayout1 = new QVBoxLayout;

    actionButtonsLayout1->addWidget(aboutButton);
    actionButtonsLayout1->addWidget(helpButton);

    // Main buttons layout

    QHBoxLayout *mainButtonsLayout = new QHBoxLayout;

    mainButtonsLayout->addLayout(actionButtonsLayout0);
    mainButtonsLayout->addLayout(actionButtonsLayout1);
    mainButtonsLayout->addWidget(playerGroupBox);

    // FFT analysis

    sampleRateLabel = new QLabel(this);
    sampleRateLabel->setText(QString("Sample rate: %1Hz").arg(fourier->sampleRate));

    samplesPerSegmentLabel = new QLabel(this);
    samplesPerSegmentLabel->setText("Samples/segment: 0");

    segmentsLabel = new QLabel(this);
    segmentsLabel->setText("Segments: 0");

    frequenciesLabel = new QLabel(this);
    frequenciesLabel->setText("Frequencies: 0");

    frequencyBinsLabel = new QLabel(this);
    frequencyBinsLabel->setText("Frequency bins: 0");

    QLabel *segmentDurationLabel = new QLabel("Segment duration (ms):");
    segmentDurationSpinBox = new QSpinBox;
    segmentDurationSpinBox->setRange(1, 10000);
    segmentDurationSpinBox->setSingleStep(1);
    segmentDurationSpinBox->setValue(fourier->milliseconds);
    segmentDurationSpinBox->setEnabled(false);
    segmentDurationSpinBox->setMaximumWidth(100);

    QLabel *frequencyBinSizeLabel = new QLabel("Frequency bin size:");
    frequencyBinSizeSpinBox = new QSpinBox;
    frequencyBinSizeSpinBox->setRange(1, 1000);
    frequencyBinSizeSpinBox->setSingleStep(1);
    frequencyBinSizeSpinBox->setValue(fourier->frequencyBinSize);
    frequencyBinSizeSpinBox->setEnabled(false);
    frequencyBinSizeSpinBox->setMaximumWidth(100);

    startFFTAnalysisButton = new QPushButton("Start FFT analysis");
    startFFTAnalysisButton->setEnabled(false);

    fftProgressBar = new QProgressBar;
    fftProgressBar->setRange(0, 10);
    fftProgressBar->setValue(0);
    fftProgressBar->setTextVisible(true);
    fftProgressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // Spectrum graph axes scale

    xAxisScaleCheckBox = new QCheckBox("X-Log", this);
    xAxisScaleCheckBox->setChecked(true);

    yAxisScaleCheckBox = new QCheckBox("Y-Log", this);
    yAxisScaleCheckBox->setChecked(false);

    QVBoxLayout *axesScaleLayout = new QVBoxLayout;
    axesScaleLayout->addWidget(xAxisScaleCheckBox);
    axesScaleLayout->addWidget(yAxisScaleCheckBox);

    QGroupBox *axesScaleGroupBox = new QGroupBox("Spectrum graph");
    axesScaleGroupBox->setLayout(axesScaleLayout);

    // FFT widget

    QVBoxLayout *fftV0Layout = new QVBoxLayout;

    fftV0Layout->setAlignment(Qt::AlignTop);

    fftV0Layout->addWidget(sampleRateLabel);
    fftV0Layout->addWidget(samplesPerSegmentLabel);
    fftV0Layout->addWidget(frequenciesLabel);
    fftV0Layout->addWidget(frequencyBinsLabel);
    fftV0Layout->addWidget(segmentsLabel);
    fftV0Layout->addWidget(axesScaleGroupBox);

    QVBoxLayout *fftV1Layout = new QVBoxLayout;

    fftV1Layout->setAlignment(Qt::AlignTop);

    fftV1Layout->addWidget(segmentDurationLabel);
    fftV1Layout->addWidget(segmentDurationSpinBox);
    fftV1Layout->addWidget(frequencyBinSizeLabel);
    fftV1Layout->addWidget(frequencyBinSizeSpinBox);
    fftV1Layout->addWidget(startFFTAnalysisButton);
    fftV1Layout->addWidget(fftProgressBar);

    QHBoxLayout *fftHLayout = new QHBoxLayout;

    fftHLayout->setAlignment(Qt::AlignCenter);

    fftHLayout->addLayout(fftV0Layout);
    fftHLayout->addLayout(fftV1Layout);

    QWidget *fftWidget = new QWidget;
    fftWidget->setLayout(fftHLayout);

    // PCA

    QLabel *componentNumberLabel = new QLabel("Components:");

    componentNumberSpinBox = new QSpinBox;
    componentNumberSpinBox->setRange(3, 100);
    componentNumberSpinBox->setSingleStep(1);
    componentNumberSpinBox->setValue(pca->componentNumber);
    componentNumberSpinBox->setEnabled(false);
    componentNumberSpinBox->setMaximumWidth(100);

    startPCAButton = new QPushButton("Start PCA");
    startPCAButton->setEnabled(false);

    abortPCAButton = new QPushButton("Abort");
    abortPCAButton->setEnabled(false);

    pcaIterationLabel = new QLabel("Iteration: 0");

    pcaProgressBar = new QProgressBar;
    pcaProgressBar->setRange(0, pca->componentNumber);
    pcaProgressBar->setValue(0);
    pcaProgressBar->setTextVisible(true);
    pcaProgressBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // PCA graphs' convex hulls toggle

    QGroupBox *hullsToggleGroupBox = new QGroupBox("PCA graphs");

    QVBoxLayout *hullsToggleLayout = new QVBoxLayout;

    hullsToggleCheckBox = new QCheckBox("Convex hulls", this);
    hullsToggleCheckBox->setChecked(true);

    hullsToggleLayout->addWidget(hullsToggleCheckBox);

    hullsToggleGroupBox->setLayout(hullsToggleLayout);

    // PCA widget

    QVBoxLayout *pcaV0Layout = new QVBoxLayout;

    pcaV0Layout->setAlignment(Qt::AlignTop);
    pcaV0Layout->setMargin(10);

    pcaV0Layout->addWidget(componentNumberLabel);
    pcaV0Layout->addWidget(componentNumberSpinBox);
    pcaV0Layout->addWidget(startPCAButton);
    pcaV0Layout->addWidget(abortPCAButton);
    pcaV0Layout->addWidget(pcaIterationLabel);
    pcaV0Layout->addWidget(pcaProgressBar);

    QVBoxLayout *pcaV1Layout = new QVBoxLayout;

    pcaV1Layout->setAlignment(Qt::AlignTop);
    pcaV1Layout->setMargin(10);

    pcaV1Layout->addWidget(hullsToggleGroupBox);

    QHBoxLayout *pcaHLayout = new QHBoxLayout;

    pcaHLayout->setAlignment(Qt::AlignCenter);

    pcaHLayout->addLayout(pcaV0Layout);
    pcaHLayout->addLayout(pcaV1Layout);

    QWidget *pcaWidget = new QWidget;
    pcaWidget->setLayout(pcaHLayout);

    // K-Means

    QLabel *clusterNumberLabel = new QLabel("Clusters:");

    clusterNumberSpinBox = new QSpinBox;
    clusterNumberSpinBox->setRange(1, 500);
    clusterNumberSpinBox->setSingleStep(1);
    clusterNumberSpinBox->setValue(kmeans->clusterNumber);
    clusterNumberSpinBox->setEnabled(false);
    clusterNumberSpinBox->setMaximumWidth(100);

    onFFTData = new QRadioButton("On FFT data", this);
    onFFTData->setChecked(true);

    onPCAData = new QRadioButton("On PCA data", this);
    onPCAData->setChecked(false);
    onPCAData->setEnabled(false);

    startKMeansButton = new QPushButton("Start K-Means");
    startKMeansButton->setEnabled(false);

    iterationLabel = new QLabel(this);
    iterationLabel->setText("Iteration: 0");

    QVBoxLayout *kmeansLayout = new QVBoxLayout;

    kmeansLayout->setAlignment(Qt::AlignTop);
    kmeansLayout->setMargin(10);

    kmeansLayout->addWidget(clusterNumberLabel);
    kmeansLayout->addWidget(clusterNumberSpinBox);
    kmeansLayout->addWidget(onFFTData);
    kmeansLayout->addWidget(onPCAData);
    kmeansLayout->addWidget(startKMeansButton);
    kmeansLayout->addWidget(iterationLabel);

    // Hurst

    startHurstButton = new QPushButton("Compute H");
    startHurstButton->setEnabled(false);

    hurstExponentLabel = new QLabel("H = 0");

    QVBoxLayout *hurstLayout = new QVBoxLayout;

    hurstLayout->setAlignment(Qt::AlignTop);

    hurstLayout->addWidget(hurstExponentLabel);
    hurstLayout->addWidget(startHurstButton);

    QGroupBox *hurstGroupBox = new QGroupBox("Hurst exponent");
    hurstGroupBox->setLayout(hurstLayout);

    QVBoxLayout *hurstVLayout = new QVBoxLayout;
    hurstVLayout->setAlignment(Qt::AlignTop);
    hurstVLayout->setMargin(10);
    hurstVLayout->addWidget(hurstGroupBox);

    // K-Means + Hurst widget

    QHBoxLayout *kmeansHurstHLayout = new QHBoxLayout;

    kmeansHurstHLayout->setAlignment(Qt::AlignCenter);

    kmeansHurstHLayout->addLayout(kmeansLayout);
    kmeansHurstHLayout->addLayout(hurstVLayout);

    QWidget *kmeansHurstWidget = new QWidget;
    kmeansHurstWidget->setLayout(kmeansHurstHLayout);

    // Controls tab widget

    QTabWidget *controlsTabWidget = new QTabWidget(this);

    controlsTabWidget->setTabPosition(QTabWidget::North);
    controlsTabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    controlsTabWidget->addTab(fftWidget, "FFT Analysis");
    controlsTabWidget->addTab(pcaWidget, "PCA");
    controlsTabWidget->addTab(kmeansHurstWidget, "K-Means");

    // Clusters buttons layout

    clusterButtonsLayout = new FlowLayout(this, 0, 0, 0);

    QWidget *scrollAreaContent = new QWidget;
    scrollAreaContent->setLayout(clusterButtonsLayout);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollAreaContent);

    // Controls widget

    QVBoxLayout *controlsVLayout = new QVBoxLayout;

    controlsVLayout->setAlignment(Qt::AlignTop);

    controlsVLayout->addLayout(mainButtonsLayout);
    controlsVLayout->addWidget(controlsTabWidget);

    QWidget *controlsWidget = new QWidget;
    controlsWidget->setLayout(controlsVLayout);

    // Up splitter

    QSplitter *upSplitter = new QSplitter;

    upSplitter->setOrientation(Qt::Horizontal);

    upSplitter->addWidget(controlsWidget);
    upSplitter->addWidget(scrollArea);

    upSplitter->setStretchFactor(0, 0);
    upSplitter->setStretchFactor(1, 1);

    upSplitter->setCollapsible(0, true);
    upSplitter->setCollapsible(1, false);

    // Spectrum graph

    spectrumGraph = new QCustomPlot(this);

    spectrumGraph->xAxis->setLabel("Frequency (Hz)");
    spectrumGraph->yAxis->setLabel("Power");

    spectrumGraph->xAxis->setScaleType(QCPAxis::stLogarithmic);
    spectrumGraph->xAxis->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));

    spectrumGraph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    spectrumGraph->axisRect()->setupFullAxesBox(true);
    spectrumGraph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    spectrumGraph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    spectrumGraph->addGraph();
    spectrumGraph->graph(0)->setName("Spectrum");

    spectrumGraph->legend->setVisible(true);
    spectrumGraph->legend->setBrush(QColor(255, 255, 255, 150));

    itemTracer = new QCPItemTracer(spectrumGraph);
    itemTracer->setInterpolating(false);
    itemTracer->setStyle(QCPItemTracer::tsCircle);
    itemTracer->setPen(QPen(Qt::red));
    itemTracer->setBrush(Qt::red);
    itemTracer->setSize(5);

    // Waveform graph

    waveFormGraph = new QCustomPlot(this);

    waveFormGraph->xAxis->setLabel("Time");
    waveFormGraph->yAxis->setLabel("Amplitude");

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("mm:ss.zzz");
    waveFormGraph->xAxis->setTicker(dateTicker);

    waveFormGraph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    waveFormGraph->axisRect()->setupFullAxesBox(true);
    waveFormGraph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    waveFormGraph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    waveFormGraph->addGraph();
    waveFormGraph->graph(0)->setName("Waveform");

    waveFormGraph->legend->setVisible(true);
    waveFormGraph->legend->setBrush(QColor(255, 255, 255, 150));

    // Waveform full graph

    waveFormFullGraph = new QCustomPlot(this);

    waveFormFullGraph->xAxis->setLabel("Time");
    waveFormFullGraph->yAxis->setLabel("Amplitude");

    waveFormFullGraph->xAxis->setTicker(dateTicker);

    waveFormFullGraph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    waveFormFullGraph->axisRect()->setupFullAxesBox(true);
    waveFormFullGraph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    waveFormFullGraph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    waveFormFullGraph->addGraph();
    waveFormFullGraph->graph(0)->setName("Waveform");

    waveFormFullGraph->legend->setVisible(true);
    waveFormFullGraph->legend->setBrush(QColor(255, 255, 255, 150));

    // Waveform full cursor

    waveFormFullCursor = new QCPItemLine(waveFormFullGraph);

    waveFormFullCursor->setPen(QPen(Qt::red));

    waveFormFullCursor->start->setCoords(0, -1.0e4);
    waveFormFullCursor->end->setCoords(0, 1.0e4);

    // Spectrogram

    spectrogramGraph = new QCustomPlot(this);

    spectrogramGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    spectrogramGraph->axisRect()->setupFullAxesBox(true);

    spectrogramGraph->xAxis->setLabel("Time");
    spectrogramGraph->yAxis->setLabel("Frequency (Hz)");

    spectrogramGraph->xAxis->setTicker(dateTicker);

    spectrogram = new QCPColorMap(spectrogramGraph->xAxis, spectrogramGraph->yAxis);

    QCPColorScale *powerColorScale = new QCPColorScale(spectrogramGraph);
    spectrogramGraph->plotLayout()->addElement(0, 1, powerColorScale);
    powerColorScale->setType(QCPAxis::atRight);
    powerColorScale->setDataScaleType(QCPAxis::stLogarithmic);
    powerColorScale->axis()->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));
    spectrogram->setColorScale(powerColorScale);
    powerColorScale->axis()->setLabel("Power");

    spectrogram->setGradient(QCPColorGradient::gpThermal);

    spectrogram->setInterpolate(false);

    spectrogram->setDataScaleType(QCPAxis::stLogarithmic);

    QCPMarginGroup *marginGroup = new QCPMarginGroup(spectrogramGraph);
    spectrogramGraph->axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
    powerColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

    // Spectrogram cursor

    spectrogramCursor = new QCPItemLine(spectrogramGraph);

    spectrogramCursor->setPen(QPen(Qt::white));

    spectrogramCursor->start->setCoords(0, 0);
    spectrogramCursor->end->setCoords(0, 1.0e6);

    // PC1 vs PC2 graph

    pc1pc2Graph = new QCustomPlot(this);

    pc1pc2Graph->xAxis->setLabel("PC1");
    pc1pc2Graph->yAxis->setLabel("PC2");

    pc1pc2Graph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    pc1pc2Graph->axisRect()->setupFullAxesBox(true);
    pc1pc2Graph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    pc1pc2Graph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    pc1pc2Graph->setBackground(QBrush(Qt::black));
    QList<QCPAxis*> pc1pc2Axes = pc1pc2Graph->axisRect()->axes();
    for (QCPAxis *axis : pc1pc2Axes)
    {
        axis->setBasePen(QPen(Qt::white));
        axis->setTickPen(QPen(Qt::white));
        axis->setSubTickPen(QPen(Qt::white));
        axis->setTickLabelColor(Qt::white);
        axis->setLabelColor(Qt::white);
    }

    // PC1 vs PC3 graph

    pc1pc3Graph = new QCustomPlot(this);

    pc1pc3Graph->xAxis->setLabel("PC1");
    pc1pc3Graph->yAxis->setLabel("PC3");

    pc1pc3Graph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    pc1pc3Graph->axisRect()->setupFullAxesBox(true);
    pc1pc3Graph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    pc1pc3Graph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    pc1pc3Graph->setBackground(QBrush(Qt::black));
    QList<QCPAxis*> pc1pc3Axes = pc1pc3Graph->axisRect()->axes();
    for (QCPAxis *axis : pc1pc3Axes)
    {
        axis->setBasePen(QPen(Qt::white));
        axis->setTickPen(QPen(Qt::white));
        axis->setSubTickPen(QPen(Qt::white));
        axis->setTickLabelColor(Qt::white);
        axis->setLabelColor(Qt::white);
    }

    // PC2 vs PC3 graph

    pc2pc3Graph = new QCustomPlot(this);

    pc2pc3Graph->xAxis->setLabel("PC2");
    pc2pc3Graph->yAxis->setLabel("PC3");

    pc2pc3Graph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    pc2pc3Graph->axisRect()->setupFullAxesBox(true);
    pc2pc3Graph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    pc2pc3Graph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    pc2pc3Graph->setBackground(QBrush(Qt::black));
    QList<QCPAxis*> pc2pc3Axes = pc2pc3Graph->axisRect()->axes();
    for (QCPAxis *axis : pc2pc3Axes)
    {
        axis->setBasePen(QPen(Qt::white));
        axis->setTickPen(QPen(Qt::white));
        axis->setSubTickPen(QPen(Qt::white));
        axis->setTickLabelColor(Qt::white);
        axis->setLabelColor(Qt::white);
    }

    // Cluster histogram

    clusterHistogramGraph = new QCustomPlot(this);

    clusterHistogramGraph->axisRect()->setupFullAxesBox(true);

    clusterHistogramGraph->xAxis->setLabel("Cluster");
    clusterHistogramGraph->yAxis->setLabel("Segments");

    clusterHistogram = new QCPBars(clusterHistogramGraph->xAxis, clusterHistogramGraph->yAxis);

    // Cluster length histogram

    clusterLengthHistogramGraph = new QCustomPlot(this);

    clusterLengthHistogramGraph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    clusterLengthHistogramGraph->axisRect()->setupFullAxesBox();
    clusterLengthHistogramGraph->axisRect()->setRangeZoom(Qt::Vertical);
    clusterLengthHistogramGraph->axisRect()->setRangeDrag(Qt::Vertical);

    clusterLengthHistogramGraph->xAxis->setLabel("Interval length");
    clusterLengthHistogramGraph->yAxis->setLabel("Intervals");

    clusterLengthHistogram = new QCPBars(clusterLengthHistogramGraph->xAxis, clusterLengthHistogramGraph->yAxis);

    // Hurst's rescaled range graph

    rescaledRangeGraph = new QCustomPlot(this);

    rescaledRangeGraph->axisRect()->setupFullAxesBox(true);

    rescaledRangeGraph->xAxis->setLabel("Log Length");
    rescaledRangeGraph->yAxis->setLabel("Log R/S");

    rescaledRangeGraph->addGraph();

    // H = 0.5, random line

    randomLine = new QCPItemLine(rescaledRangeGraph);

    randomLine->setPen(QPen(Qt::red));

    randomLine->start->setCoords(0, 0);
    randomLine->end->setCoords(10, 5);

    // Linear regression line

    linearRegressionLine = new QCPItemLine(rescaledRangeGraph);

    linearRegressionLine->setVisible(false);

    // Interval graph

    intervalGraph = new QCustomPlot(this);

    intervalGraph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    intervalGraph->axisRect()->setupFullAxesBox(true);
    intervalGraph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    intervalGraph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    intervalGraph->xAxis->setLabel("Index");
    intervalGraph->yAxis->setLabel("Interval length");

    intervalGraph->addGraph();

    // Cumulative interval graph

    cumulativeIntervalGraph = new QCustomPlot(this);

    cumulativeIntervalGraph->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    cumulativeIntervalGraph->axisRect()->setupFullAxesBox(true);
    cumulativeIntervalGraph->axisRect()->setRangeZoom(Qt::Vertical | Qt::Horizontal);
    cumulativeIntervalGraph->axisRect()->setRangeDrag(Qt::Vertical | Qt::Horizontal);

    cumulativeIntervalGraph->xAxis->setLabel("Index");
    cumulativeIntervalGraph->yAxis->setLabel("Cumulative interval length");

    cumulativeIntervalGraph->addGraph();

    // Waveform graphs splitter

    QSplitter *waveFormSplitter = new QSplitter;

    waveFormSplitter->setOrientation(Qt::Horizontal);

    waveFormSplitter->addWidget(waveFormGraph);
    waveFormSplitter->addWidget(waveFormFullGraph);

    waveFormSplitter->setCollapsible(0, false);
    waveFormSplitter->setCollapsible(1, false);

    // Spectrum + Spectrogram splitter

    QSplitter *spectrumSplitter = new QSplitter;

    spectrumSplitter->setOrientation(Qt::Horizontal);

    spectrumSplitter->addWidget(spectrumGraph);
    spectrumSplitter->addWidget(spectrogramGraph);

    spectrumSplitter->setCollapsible(0, false);
    spectrumSplitter->setCollapsible(1, false);

    // Time-frequency graphs splitter

    QSplitter *timeFrequencySplitter = new QSplitter;

    timeFrequencySplitter->setOrientation(Qt::Vertical);

    timeFrequencySplitter->addWidget(waveFormSplitter);
    timeFrequencySplitter->addWidget(spectrumSplitter);

    // PCA graphs splitter

    QSplitter *pcaGraphsSplitter = new QSplitter;

    pcaGraphsSplitter->setOrientation(Qt::Horizontal);

    pcaGraphsSplitter->addWidget(pc1pc2Graph);
    pcaGraphsSplitter->addWidget(pc1pc3Graph);
    pcaGraphsSplitter->addWidget(pc2pc3Graph);

    // Cluster graphs splitter

    QSplitter *clusterGraphsSplitter = new QSplitter;

    clusterGraphsSplitter->setOrientation(Qt::Vertical);

    clusterGraphsSplitter->addWidget(clusterHistogramGraph);
    clusterGraphsSplitter->addWidget(clusterLengthHistogramGraph);

    // Interval graphs splitter

    QSplitter *intervalGraphsSplitter = new QSplitter;

    intervalGraphsSplitter->setOrientation(Qt::Vertical);

    intervalGraphsSplitter->addWidget(cumulativeIntervalGraph);
    intervalGraphsSplitter->addWidget(intervalGraph);

    // Statistics splitter

    QSplitter *statisticsSplitter = new QSplitter;

    statisticsSplitter->setOrientation(Qt::Horizontal);

    statisticsSplitter->addWidget(clusterGraphsSplitter);
    statisticsSplitter->addWidget(intervalGraphsSplitter);
    statisticsSplitter->addWidget(rescaledRangeGraph);

    // Graphs tab widget

    graphsTabWidget = new QTabWidget(this);

    graphsTabWidget->setTabPosition(QTabWidget::South);

    graphsTabWidget->addTab(timeFrequencySplitter, "Time-frequency");
    graphsTabWidget->addTab(pcaGraphsSplitter, "PCA");
    graphsTabWidget->addTab(statisticsSplitter, "Statistics");

    // Splitter

    QSplitter *splitter = new QSplitter(this);

    splitter->setOrientation(Qt::Vertical);

    splitter->addWidget(upSplitter);
    splitter->addWidget(graphsTabWidget);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);

    // Vertical layout

    QVBoxLayout *verticalLayout = new QVBoxLayout;

    verticalLayout->addWidget(splitter);

    setLayout(verticalLayout);

    setWindowTitle("Pitch Explorer");

    QPixmap pixmap;
    pixmap.load(":/images/logo-small.png");

    setWindowIcon(QIcon(pixmap));

    resize(1200, 800);

    // Audio file dialog

    loadAudioFileDialog = new QFileDialog(this);

    loadAudioFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    loadAudioFileDialog->setFileMode(QFileDialog::ExistingFile);
    loadAudioFileDialog->setNameFilter(tr("Audio files (*.wav *.mp3 *.flac)"));

    // Data file dialog

    loadDataFileDialog = new QFileDialog(this);

    loadDataFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    loadDataFileDialog->setFileMode(QFileDialog::ExistingFile);
    loadDataFileDialog->setNameFilter(tr("Data files (*.dat *.txt)"));

    // Player

    player = new QMediaPlayer;

    // Clear FFT graphs data

    clearFFTGraphs();

    // Signals / Slots

    connect(aboutButton, &QPushButton::clicked, this, &MainWindow::about);
    connect(loadAudioFileButton, &QPushButton::clicked, [this](){ loadAudioFileDialog->open(); });
    connect(loadAudioFileDialog, &QFileDialog::fileSelected, fourier, &Fourier::readAudioFile);
    connect(loadAudioFileDialog, &QFileDialog::fileSelected, this, &MainWindow::loadAudio);
    connect(loadDataFileButton, &QPushButton::clicked, [this](){ loadDataFileDialog->open(); });
    connect(loadDataFileDialog, &QFileDialog::fileSelected, fourier, &Fourier::readDataFile);
    connect(loadDataFileDialog, &QFileDialog::fileSelected, this, &MainWindow::onDataFileSelected);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::enableFFTActions);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::disablePCAActions);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::disableKMeansActions);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::disableHurstActions);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::deleteClusterButtons);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::clearFFTGraphs);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::setWaveFormGraph);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::setWaveFormFullGraph);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::clearPCAGraphs);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::clearClusterHistogram);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::clearRescaledRangeGraph);
    connect(fourier, &Fourier::fileRead, this, &MainWindow::clearIntervalGraphs);
    connect(fourier, &Fourier::fileDecodingFailed, this, &MainWindow::showFileDecodingFailedDialog);
    connect(startFFTAnalysisButton, &QPushButton::clicked, this, &MainWindow::updateFFTProgressBarMaximum);
    connect(startFFTAnalysisButton, &QPushButton::clicked, fourier, &Fourier::performFFTAnalysis);
    connect(fourier, &Fourier::sendMessage, [this](QString message){ startFFTAnalysisButton->setText(message); });
    connect(fourier, &Fourier::fftAnalysisStep, fftProgressBar, &QProgressBar::setValue);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::onFFTPerformed);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::deleteClusterButtons);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::setSpectrogram);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::clearPCAGraphs);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::clearClusterHistogram);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::clearRescaledRangeGraph);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::clearIntervalGraphs);
    connect(fourier, &Fourier::fftAnalysisPerformed, this, &MainWindow::disableHurstActions);
    connect(startPCAButton, &QPushButton::clicked, this, &MainWindow::onPCAStarted);
    connect(startPCAButton, &QPushButton::clicked, this, &MainWindow::performPCA);
    connect(pca, &PCA::pcaIterationStep, [this](int step){ pcaIterationLabel->setText(QString("Iteration: %1").arg(step)); });
    connect(pca, &PCA::pcaStep, pcaProgressBar, &QProgressBar::setValue);
    connect(pca, &PCA::pcaPerformed, this, &MainWindow::onPCAPerformed);
    connect(pca, &PCA::pcaPerformed, this, &MainWindow::setPCAGraphs);
    connect(pca, &PCA::pcaAborted, this, &MainWindow::onPCAAborted);
    connect(abortPCAButton, &QPushButton::clicked, [this](){ pca->abort = true; });
    connect(componentNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateComponentNumber);
    connect(startKMeansButton, &QPushButton::clicked, this, &MainWindow::onKMeansStarted);
    connect(startKMeansButton, &QPushButton::clicked, this, &MainWindow::performKMeans);
    connect(kmeans, &KMeans::kMeansIterationStep, [this](int step){ iterationLabel->setText(QString("Iteration: %1").arg(step)); });
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::onKMeansPerformed);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::createClusterButtons);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::setPCAClusteredGraphs);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::setClusterHistogram);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::setClusterLengthHistogram);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::enableHurstActions);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::clearRescaledRangeGraph);
    connect(kmeans, &KMeans::kMeansPerformed, this, &MainWindow::clearIntervalGraphs);
    connect(startHurstButton, &QPushButton::clicked, this, &MainWindow::onHurstStarted);
    connect(startHurstButton, &QPushButton::clicked, this, &MainWindow::performHurst);
    connect(hurst, &Hurst::hurstPerformed, this, &MainWindow::onHurstPerformed);
    connect(hurst, &Hurst::hurstPerformed, this, &MainWindow::setRescaledRangeGraph);
    connect(hurst, &Hurst::hurstPerformed, this, &MainWindow::setIntervalGraph);
    connect(hurst, &Hurst::hurstPerformed, this, &MainWindow::setCumulativeIntervalGraph);
    connect(hurst, &Hurst::notEnoughData, this, &MainWindow::onHurstNotEnoughData);
    connect(hurst, &Hurst::notEnoughData, this, &MainWindow::setIntervalGraph);
    connect(hurst, &Hurst::notEnoughData, this, &MainWindow::setCumulativeIntervalGraph);
    connect(segmentDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateSegmentDuration);
    connect(frequencyBinSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateFrequencyBinSize);
    connect(clusterNumberSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::updateClusterNumber);
    connect(playPauseButton, &QPushButton::clicked, this, &MainWindow::togglePlayback);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::updatePositionLabel);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::selectCurrentSegment);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::shiftWaveFormFullCursor);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::shiftSpectrogramCursor);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::replotSpectrumGraph);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::shiftWaveFormGraph);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::selectCurrentPCAPoint);
    connect(player, &QMediaPlayer::stateChanged, this, &MainWindow::onPlayerStateChanged);
    connect(spectrumGraph, &QCustomPlot::mouseMove, this, &MainWindow::showSpectrumPointValue);
    connect(xAxisScaleCheckBox, &QCheckBox::stateChanged, this, &MainWindow::toggleXAxisScale);
    connect(yAxisScaleCheckBox, &QCheckBox::stateChanged, this, &MainWindow::toggleYAxisScale);
    connect(hullsToggleCheckBox, &QCheckBox::stateChanged, [this](int state){ Q_UNUSED(state) setPCAClusteredGraphs(); });
    connect(waveFormGraph->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (waveFormGraph->graph(0)->dataCount() > 0) waveFormGraph->xAxis->setRange(newRange.bounded(fourier->minTime, fourier->maxTime)); });
    connect(waveFormGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (waveFormGraph->graph(0)->dataCount() > 0) waveFormGraph->yAxis->setRange(newRange.bounded(fourier->minWaveForm, fourier->maxWaveForm)); });
    connect(waveFormFullGraph->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (waveFormFullGraph->graph(0)->dataCount() > 0) waveFormFullGraph->xAxis->setRange(newRange.bounded(fourier->minTime, fourier->maxTime)); });
    connect(waveFormFullGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (waveFormFullGraph->graph(0)->dataCount() > 0) waveFormFullGraph->yAxis->setRange(newRange.bounded(fourier->minWaveForm, fourier->maxWaveForm)); });
    connect(spectrumGraph->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!fourier->frequencies.empty()) spectrumGraph->xAxis->setRange(newRange.bounded(fourier->frequencies.first(), fourier->frequencies.last())); });
    connect(spectrumGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!fourier->spectra.empty()) spectrumGraph->yAxis->setRange(newRange.bounded(0, fourier->supPower)); });
    connect(spectrogramGraph->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!fourier->frequencies.empty()) spectrogramGraph->xAxis->setRange(newRange.bounded(fourier->minTime, fourier->maxTime)); });
    connect(spectrogramGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!fourier->frequencies.empty()) spectrogramGraph->yAxis->setRange(newRange.bounded(fourier->frequencies.first(), fourier->frequencies.last())); });
    connect(clusterLengthHistogramGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!kmeans->clusterLengthHistogram.empty()) clusterLengthHistogramGraph->yAxis->setRange(newRange.bounded(0, kmeans->clusterLengthHistogramMax * 1.1)); });
    connect(intervalGraph->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!hurst->series.empty()) intervalGraph->xAxis->setRange(newRange.bounded(0, hurst->series.size() - 1)); });
    connect(intervalGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!hurst->series.empty()) intervalGraph->yAxis->setRange(newRange.bounded(0, hurst->maxSeries)); });
    connect(cumulativeIntervalGraph->xAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!hurst->cumulativeSeries.empty()) cumulativeIntervalGraph->xAxis->setRange(newRange.bounded(0, hurst->cumulativeSeries.size() - 1)); });
    connect(cumulativeIntervalGraph->yAxis, QOverload<const QCPRange&>::of(&QCPAxis::rangeChanged), [this](const QCPRange &newRange){ if (!hurst->cumulativeSeries.empty()) cumulativeIntervalGraph->yAxis->setRange(newRange.bounded(hurst->minCumulativeSeries, hurst->maxCumulativeSeries)); });
}

MainWindow::~MainWindow()
{
    delete fourier;
    delete pca;
    delete kmeans;
    delete hurst;
}

void MainWindow::about()
{
    QMessageBox *aboutBox = new QMessageBox(this);

    aboutBox->setWindowTitle("About");

    aboutBox->setIconPixmap(QPixmap(":/images/logo-big.png"));

    QStringList lines;
    lines.append("Pitch Explorer 0.1\n");
    lines.append("Analyze and visualize a song's or time series' pitch characteristics.\n");
    lines.append("Required dependencies:");
    lines.append(" Qt >= 5.14.0");
    lines.append(" FFTW >= 3.3.5");
    lines.append(" miniaudio >= 0.9.8");
    lines.append(" QCustomPlot >= 2.0.1\n");
    lines.append("Especial thanks to José Ángel Oteo.");

    QString text = lines.join("\n");

    aboutBox->setText(text);

    aboutBox->setInformativeText("Copyright 2020 José María Castelo Ares\njose.maria.castelo@gmail.com\nLicense: GPLv3");

    aboutBox->exec();
}

void MainWindow::showFileDecodingFailedDialog()
{
    QMessageBox *errorBox = new QMessageBox(this);

    errorBox->setWindowTitle("Error");

    errorBox->setText("Failed to decode audio file.");

    errorBox->exec();
}

void MainWindow::disableKMeansActions()
{
    startKMeansButton->setEnabled(false);
    clusterNumberSpinBox->setEnabled(false);
    onPCAData->setChecked(false);
    onPCAData->setEnabled(false);
    onFFTData->setChecked(true);
    iterationLabel->setText("Iteration: 0");
}

void MainWindow::disablePCAActions()
{
    startPCAButton->setEnabled(false);
    componentNumberSpinBox->setEnabled(false);
    pcaProgressBar->setValue(0);
    pcaIterationLabel->setText("Iteration: 0");
}

void MainWindow::disableHurstActions()
{
    startHurstButton->setEnabled(false);
    hurstExponentLabel->setText("H = 0");
}

void MainWindow::enableHurstActions()
{
    startHurstButton->setEnabled(true);
    hurstExponentLabel->setText("H = 0");
}

void MainWindow::enableFFTActions()
{
    startFFTAnalysisButton->setEnabled(true);
    segmentDurationSpinBox->setEnabled(true);
    frequencyBinSizeSpinBox->setEnabled(true);

    sampleRateLabel->setText(QString("Sample rate: %1Hz").arg(fourier->sampleRate));

    int minSegmentDuration = 1 + 1000 * 10 / fourier->sampleRate;
    segmentDurationSpinBox->setMinimum(minSegmentDuration);

    int nSamplesPerSegment = fourier->sampleRate * fourier->milliseconds / 1000;
    if (nSamplesPerSegment < 10)
    {
        fourier->milliseconds = minSegmentDuration;
        segmentDurationSpinBox->setValue(minSegmentDuration);
    }

    int nSegments = computeSegmentNumber();
    int nFrequencies = nSamplesPerSegment / 2 + 1;
    int nFrequencyBins = static_cast<int>(ceil(static_cast<double>(nFrequencies - 1) / fourier->frequencyBinSize));

    samplesPerSegmentLabel->setText(QString("Samples/segment: %1").arg(nSamplesPerSegment));
    segmentsLabel->setText(QString("Segments: %1").arg(nSegments));
    frequenciesLabel->setText(QString("Frequencies: %1").arg(nFrequencies));
    frequencyBinsLabel->setText(QString("Frequency bins: %1").arg(nFrequencyBins));

    frequencyBinSizeSpinBox->setMaximum(nFrequencyBins);

    fftProgressBar->setValue(0);
}

void MainWindow::onFFTPerformed()
{
    startKMeansButton->setEnabled(true);
    clusterNumberSpinBox->setEnabled(true);
    onPCAData->setChecked(false);
    onPCAData->setEnabled(false);
    onFFTData->setChecked(true);

    startPCAButton->setEnabled(true);
    componentNumberSpinBox->setEnabled(true);
    componentNumberSpinBox->setMaximum(fourier->frequencies.size());
    pcaProgressBar->setValue(0);

    startFFTAnalysisButton->setText("Start FFT analysis");

    milliseconds = fourier->milliseconds;

    player->setNotifyInterval(fourier->milliseconds);

    spectrumGraph->xAxis->setRange(fourier->frequencies.first(), fourier->frequencies.last());
    spectrumGraph->yAxis->setRange(0, fourier->supPower);

    replotSpectrumGraph(0);
}

void MainWindow::performPCA()
{
    startPCAButton->setEnabled(false);
    componentNumberSpinBox->setEnabled(false);

    pca->initData(fourier->spectra);
    pca->performPCA();
}

void MainWindow::onPCAStarted()
{
    startPCAButton->setText("Computing...");
    startPCAButton->setEnabled(false);

    abortPCAButton->setEnabled(true);

    loadAudioFileButton->setEnabled(false);
    loadDataFileButton->setEnabled(false);
    startFFTAnalysisButton->setEnabled(false);
    componentNumberSpinBox->setEnabled(false);

    pcaProgressBar->setMaximum(pca->componentNumber);
    pcaProgressBar->setValue(0);
    pcaIterationLabel->setText("Iteration: 0");
}

void MainWindow::onPCAPerformed()
{
    startPCAButton->setText("Start PCA");
    startPCAButton->setEnabled(true);

    abortPCAButton->setEnabled(false);

    onPCAData->setEnabled(true);

    loadAudioFileButton->setEnabled(true);
    loadDataFileButton->setEnabled(true);
    startFFTAnalysisButton->setEnabled(true);
    componentNumberSpinBox->setEnabled(true);
}

void MainWindow::onPCAAborted()
{
    startPCAButton->setText("Start PCA");
    startPCAButton->setEnabled(true);

    abortPCAButton->setEnabled(false);

    loadAudioFileButton->setEnabled(true);
    loadDataFileButton->setEnabled(true);
    startFFTAnalysisButton->setEnabled(true);
    componentNumberSpinBox->setEnabled(true);
}

void MainWindow::performKMeans()
{
    if (onPCAData->isChecked())
    {
        kmeans->initData(pca->principalComponents);
    }
    else
    {
        kmeans->initData(fourier->spectra);
    }

    kmeans->performKMeans();
}

void MainWindow::onKMeansStarted()
{
    startKMeansButton->setText("Computing...");
    startKMeansButton->setEnabled(false);

    loadAudioFileButton->setEnabled(false);
    loadDataFileButton->setEnabled(false);
    startFFTAnalysisButton->setEnabled(false);
    clusterNumberSpinBox->setEnabled(false);
}

void MainWindow::onKMeansPerformed()
{
    startKMeansButton->setText("Start K-Means");
    startKMeansButton->setEnabled(true);

    loadAudioFileButton->setEnabled(true);
    loadDataFileButton->setEnabled(true);
    startFFTAnalysisButton->setEnabled(true);
    clusterNumberSpinBox->setEnabled(true);
}

void MainWindow::performHurst()
{
    hurst->initData(kmeans->clusterIndexes);
    hurst->performHurst();
}

void MainWindow::onHurstStarted()
{
    startHurstButton->setText("Computing...");
    startHurstButton->setEnabled(false);
}

void MainWindow::onHurstPerformed()
{
    startHurstButton->setText("Compute H");
    startHurstButton->setEnabled(true);

    hurstExponentLabel->setText(QString("H = %1").arg(hurst->H));
}

void MainWindow::onHurstNotEnoughData()
{
    startHurstButton->setText("Compute H");
    startHurstButton->setEnabled(true);

    QMessageBox *errorBox = new QMessageBox(this);

    errorBox->setWindowTitle("Error");

    errorBox->setText("Not enough data to perform linear fit.\nAt least 4 points needed to compute Hurst exponent.");

    errorBox->exec();
}

void MainWindow::updateComponentNumber(int value)
{
    pca->componentNumber = value;
}

void MainWindow::updateSegmentDuration(int value)
{
    fourier->milliseconds = value;

    int nSamplesPerSegment = fourier->sampleRate * fourier->milliseconds / 1000;
    int nSegments = computeSegmentNumber();
    int nFrequencies = nSamplesPerSegment / 2 + 1;
    int nFrequencyBins = static_cast<int>(ceil(static_cast<double>(nFrequencies - 1) / fourier->frequencyBinSize));

    samplesPerSegmentLabel->setText(QString("Samples/segment: %1").arg(nSamplesPerSegment));
    segmentsLabel->setText(QString("Segments: %1").arg(nSegments));
    frequenciesLabel->setText(QString("Frequencies: %1").arg(nFrequencies));
    frequencyBinsLabel->setText(QString("Frequency bins: %1").arg(nFrequencyBins));

    frequencyBinSizeSpinBox->setMaximum(nFrequencyBins);
}

void MainWindow::updateFrequencyBinSize(int value)
{
    fourier->frequencyBinSize = value;

    int nSamplesPerSegment = fourier->sampleRate * fourier->milliseconds / 1000;
    int nFrequencies = nSamplesPerSegment / 2 + 1;
    int nFrequencyBins = static_cast<int>(ceil(static_cast<double>(nFrequencies - 1) / fourier->frequencyBinSize));

    frequencyBinsLabel->setText(QString("Frequency bins: %1").arg(nFrequencyBins));
    frequencyBinSizeSpinBox->setMaximum(nFrequencyBins);
}

void MainWindow::updateFFTProgressBarMaximum()
{
    int nSegments = computeSegmentNumber();

    fftProgressBar->setMaximum(2 * nSegments);
    fftProgressBar->setValue(0);
}

int MainWindow::computeSegmentNumber()
{
    unsigned int nSamplesPerSegment = static_cast<unsigned int>(fourier->sampleRate * fourier->milliseconds / 1000);

    int nSegments = 0;

    for (unsigned int i = 0; i < fourier->sampleNumber; i += nSamplesPerSegment)
    {
        if (i + nSamplesPerSegment < fourier->sampleNumber)
        {
            nSegments++;
        }
    }

    return nSegments;
}

void MainWindow::updateClusterNumber(int value)
{
    kmeans->clusterNumber = value;
}

void MainWindow::deleteClusterButtons()
{
    for (int i = 0; i < clusterButtons.size(); i++)
    {
        clusterButtons[i]->disconnect();
    }

    clusterButtons.clear();

    while(!clusterButtonsLayout->isEmpty())
    {
        QWidget *w = clusterButtonsLayout->takeAt(0)->widget();
        delete w;
    }
}

void MainWindow::createClusterButtons()
{
    deleteClusterButtons();

    clusterButtons.reserve(kmeans->clusterIndexes.size());

    clusterNumber = kmeans->clusterNumber;

    for (int i = 0; i < kmeans->clusterIndexes.size(); i++)
    {
        QPushButton *button = new QPushButton(QString("%1").arg(kmeans->clusterIndexes[i] + 1));
        button->setFixedSize(14, 14);
        button->setFont(QFont("Helvetica", 7));
        button->setAutoFillBackground(true);
        button->setStyleSheet(QString("background-color: hsl(%1, 255, 180); border: none; padding-top: 2px;").arg(320 * kmeans->clusterIndexes[i] / clusterNumber));

        clusterButtonsLayout->addWidget(button);

        clusterButtons.push_back(button);

        connect(button, &QPushButton::clicked, [=] { changePlaybackPosition(i); });
    }

    currentClusterButton = nullptr;
}

void MainWindow::loadAudio(const QString path)
{
    currentClusterButton = nullptr;

    player->stop();
    player->setMedia(QUrl::fromLocalFile(path));

    player->setNotifyInterval(fourier->milliseconds);

    playPauseButton->setEnabled(true);

    positionLabel->setText(QString("00:00:00 / %1").arg(msToTime(fourier->duration)));

    setWindowTitle(QString("Pitch Explorer - %1").arg(path));
}

void MainWindow::togglePlayback(bool checked)
{
    if (checked)
    {
        player->play();
        playPauseButton->setText("Pause");
    }
    else
    {
        player->pause();
        playPauseButton->setText("Play");
    }
}

void MainWindow::updatePositionLabel(qint64 position)
{
    int pos = static_cast<int>(position);
    positionLabel->setText(QString("%1 / %2").arg(msToTime(pos)).arg(msToTime(fourier->duration)));
}

void MainWindow::selectCurrentSegment(qint64 position)
{
    if (currentClusterButton != nullptr)
    {
        currentClusterButton->setStyleSheet(QString("background-color: hsl(%1, 255, 180); border: none; padding-top: 2px;").arg(320 * kmeans->clusterIndexes[currentClusterButtonIndex] / clusterNumber));
    }

    if (!clusterButtons.empty())
    {
        int i = static_cast<int>(position / milliseconds);

        if (i < clusterButtons.size())
        {
            clusterButtons[i]->setStyleSheet(QString("background-color: hsl(%1, 255, 180); border: 1px solid black; padding-top: 2px;").arg(320 * kmeans->clusterIndexes[i] / clusterNumber));
            currentClusterButton = clusterButtons[i];
            currentClusterButtonIndex = i;
        }
    }
}

void MainWindow::changePlaybackPosition(int index)
{
    qint64 position = static_cast<qint64>(index * milliseconds);
    player->setPosition(position);

    if (player->state() == QMediaPlayer::StoppedState || player->state() == QMediaPlayer::PausedState)
    {
        player->blockSignals(true);
        player->play();
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        player->pause();
        player->blockSignals(false);
    }
}

void MainWindow::replotSpectrumGraph(qint64 position)
{
    if (!fourier->spectra.empty())
    {
        int index = static_cast<int>(position / milliseconds);

        if (index < fourier->spectra.size())
        {
            spectrumGraph->graph(0)->setData(fourier->frequencies, fourier->spectra[index], true);
            spectrumGraph->replot();
        }
    }
}

void MainWindow::onPlayerStateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState)
    {
        playPauseButton->setChecked(false);
        playPauseButton->setText("Play");
    }
}

QString MainWindow::msToTime(int ms)
{
    QTime zero(0, 0, 0);
    QTime time;
    time = zero.addMSecs(ms);
    return time.toString();
}

void MainWindow::onDataFileSelected(const QString path)
{
    currentClusterButton = nullptr;

    player->stop();

    playPauseButton->setEnabled(false);

    positionLabel->setText(QString("00:00:00 / %1").arg(msToTime(fourier->duration)));

    setWindowTitle(QString("Pitch Explorer - %1").arg(path));
}

void MainWindow::showSpectrumPointValue(QMouseEvent *event)
{
    if (!spectrumGraph->graph(0)->data()->isEmpty())
    {
        itemTracer->setGraph(spectrumGraph->graph(0));
        itemTracer->setGraphKey(spectrumGraph->xAxis->pixelToCoord(event->pos().x()));
        itemTracer->setVisible(true);
        spectrumGraph->replot();

        QPointF point = itemTracer->position->coords();

        QToolTip::showText(event->globalPos(), QString("Frequency: %1Hz\nPower: %2").arg(point.x()).arg(point.y()), spectrumGraph, spectrumGraph->rect());

    }
}

void MainWindow::toggleXAxisScale(int state)
{
    if (state == Qt::Checked)
    {
        spectrumGraph->xAxis->setScaleType(QCPAxis::stLogarithmic);
        spectrumGraph->xAxis->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));

        spectrumGraph->replot();
    }
    else
    {
        spectrumGraph->xAxis->setScaleType(QCPAxis::stLinear);
        spectrumGraph->xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));

        spectrumGraph->replot();
    }
}

void MainWindow::toggleYAxisScale(int state)
{
    if (state == Qt::Checked)
    {
        spectrumGraph->yAxis->setScaleType(QCPAxis::stLogarithmic);
        spectrumGraph->yAxis->setTicker(QSharedPointer<QCPAxisTickerLog>(new QCPAxisTickerLog));

        spectrumGraph->replot();
    }
    else
    {
        spectrumGraph->yAxis->setScaleType(QCPAxis::stLinear);
        spectrumGraph->yAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker));

        spectrumGraph->replot();
    }
}

void MainWindow::setWaveFormGraph()
{
    waveFormGraph->graph(0)->setData(fourier->times, fourier->waveForm, true);
    waveFormGraph->xAxis->setRange(fourier->times.first(), milliseconds / 1000.0);
    waveFormGraph->yAxis->setRange(fourier->minWaveForm, fourier->maxWaveForm);
    waveFormGraph->replot();
}

void MainWindow::shiftWaveFormGraph(qint64 position)
{
    int index = static_cast<int>(position / milliseconds);
    double conversion = milliseconds / 1000.0;
    waveFormGraph->xAxis->setRange(index * conversion, (index + 1) * conversion);
    waveFormGraph->replot();
}

void MainWindow::setWaveFormFullGraph()
{
    waveFormFullGraph->graph(0)->setData(fourier->times, fourier->waveForm, true);
    waveFormFullGraph->xAxis->setRange(fourier->times.first(), fourier->times.last());
    waveFormFullGraph->yAxis->setRange(fourier->minWaveForm, fourier->maxWaveForm);
    waveFormFullGraph->replot();

    fourier->clearWaveFormData();
}

void MainWindow::shiftWaveFormFullCursor(qint64 position)
{
    waveFormFullCursor->start->setCoords(position / 1000.0, -1.0e4);
    waveFormFullCursor->end->setCoords(position / 1000.0, 1.0e4);
    waveFormFullGraph->replot();
}

void MainWindow::shiftSpectrogramCursor(qint64 position)
{
    if (!fourier->spectra.empty())
    {
        spectrogramCursor->start->setCoords(position / 1000.0, 0);
        spectrogramCursor->end->setCoords(position / 1000.0, 1.0e6);
        spectrogramGraph->replot();
    }
}

void MainWindow::setSpectrogram()
{
    int nx = fourier->spectra.size();
    int ny = fourier->frequencies.size();

    spectrogram->data()->setSize(nx, ny);
    spectrogram->data()->setRange(QCPRange(fourier->minTime, fourier->maxTime), QCPRange(fourier->frequencies.first(), fourier->frequencies.last()));

    for (int xIndex = 0; xIndex < nx; xIndex++)
    {
        for (int yIndex = 0; yIndex < ny; yIndex++)
        {
            double z = fourier->spectra[xIndex][yIndex];
            spectrogram->data()->setCell(xIndex, yIndex, z);
        }
    }

    spectrogram->rescaleDataRange();
    spectrogramGraph->rescaleAxes();
    spectrogramGraph->replot();

    shiftSpectrogramCursor(0);
}

void MainWindow::clearFFTGraphs()
{
    fourier->clearFFTData();

    itemTracer->setGraph(nullptr);
    itemTracer->setVisible(false);

    spectrumGraph->graph(0)->data()->clear();
    spectrumGraph->replot();

    spectrogram->data()->clear();
    spectrogram->rescaleDataRange();
    spectrogramGraph->rescaleAxes();
    spectrogramGraph->replot();
}

void MainWindow::setPCAGraphs()
{
    pc1pc2Graph->xAxis->setRange(pca->pc1Min * 1.05, pca->pc1Max * 1.05);
    pc1pc2Graph->yAxis->setRange(pca->pc2Min * 1.05, pca->pc2Max * 1.05);
    //pc1pc2Graph->yAxis->setScaleRatio(pc1pc2Graph->xAxis, 1.0);

    pc1pc3Graph->xAxis->setRange(pca->pc1Min * 1.05, pca->pc1Max * 1.05);
    pc1pc3Graph->yAxis->setRange(pca->pc3Min * 1.05, pca->pc3Max * 1.05);
    //pc1pc3Graph->yAxis->setScaleRatio(pc1pc3Graph->xAxis, 1.0);

    pc2pc3Graph->xAxis->setRange(pca->pc2Min * 1.05, pca->pc2Max * 1.05);
    pc2pc3Graph->yAxis->setRange(pca->pc3Min * 1.05, pca->pc3Max * 1.05);
    //pc2pc3Graph->yAxis->setScaleRatio(pc2pc3Graph->xAxis, 1.0);

    if (!kmeans->clusterIndexes.empty())
    {
        setPCAClusteredGraphs();
    }
    else
    {
        pc1pc2Graph->addGraph();
        pc1pc2Graph->graph(0)->setLineStyle(QCPGraph::lsNone);
        pc1pc2Graph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
        pc1pc2Graph->graph(0)->setPen(QPen(Qt::white));
        pc1pc2Graph->graph(0)->setData(pca->pc1, pca->pc2);
        pc1pc2Graph->replot();

        pc1pc3Graph->addGraph();
        pc1pc3Graph->graph(0)->setLineStyle(QCPGraph::lsNone);
        pc1pc3Graph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
        pc1pc3Graph->graph(0)->setPen(QPen(Qt::white));
        pc1pc3Graph->graph(0)->setData(pca->pc1, pca->pc3);
        pc1pc3Graph->replot();

        pc2pc3Graph->addGraph();
        pc2pc3Graph->graph(0)->setLineStyle(QCPGraph::lsNone);
        pc2pc3Graph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
        pc2pc3Graph->graph(0)->setPen(QPen(Qt::white));
        pc2pc3Graph->graph(0)->setData(pca->pc2, pca->pc3);
        pc2pc3Graph->replot();
    }
}

void MainWindow::setPCAClusteredGraphs()
{
    if (!pca->pc1.empty())
    {
        QVector<QVector<double>> pc1(clusterNumber);
        QVector<QVector<double>> pc2(clusterNumber);
        QVector<QVector<double>> pc3(clusterNumber);

        QVector<QVector<std::array<double, 2>>> pc1pc2(clusterNumber);
        QVector<QVector<std::array<double, 2>>> pc1pc3(clusterNumber);
        QVector<QVector<std::array<double, 2>>> pc2pc3(clusterNumber);

        for (int i = 0; i < pca->pc1.size(); i++)
        {
            pc1[kmeans->clusterIndexes[i]].push_back(pca->pc1[i]);
            pc2[kmeans->clusterIndexes[i]].push_back(pca->pc2[i]);
            pc3[kmeans->clusterIndexes[i]].push_back(pca->pc3[i]);

            std::array<double, 2> temp12 = { pca->pc1[i], pca->pc2[i] };
            std::array<double, 2> temp13 = { pca->pc1[i], pca->pc3[i] };
            std::array<double, 2> temp23 = { pca->pc2[i], pca->pc3[i] };

            pc1pc2[kmeans->clusterIndexes[i]].push_back(temp12);
            pc1pc3[kmeans->clusterIndexes[i]].push_back(temp13);
            pc2pc3[kmeans->clusterIndexes[i]].push_back(temp23);
        }

        pc1pc2Graph->clearGraphs();
        pc1pc3Graph->clearGraphs();
        pc2pc3Graph->clearGraphs();

        pc1pc2Graph->clearPlottables();
        pc1pc3Graph->clearPlottables();
        pc2pc3Graph->clearPlottables();

        pc1pc2Graph->clearItems();
        pc1pc3Graph->clearItems();
        pc2pc3Graph->clearItems();

        currentPoint12Graph = nullptr;
        currentPoint13Graph = nullptr;
        currentPoint23Graph = nullptr;

        for (int i = 0; i < pc1.size(); i++)
        {
            QColor color;
            color.setHsl(320 * i / clusterNumber, 255, 127);

            pc1pc2Graph->addGraph();
            pc1pc2Graph->graph(i)->setData(pc1[i], pc2[i]);
            pc1pc2Graph->graph(i)->setLineStyle(QCPGraph::lsNone);
            pc1pc2Graph->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
            pc1pc2Graph->graph(i)->setPen(QPen(color));

            pc1pc3Graph->addGraph();
            pc1pc3Graph->graph(i)->setData(pc1[i], pc3[i]);
            pc1pc3Graph->graph(i)->setLineStyle(QCPGraph::lsNone);
            pc1pc3Graph->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
            pc1pc3Graph->graph(i)->setPen(QPen(color));

            pc2pc3Graph->addGraph();
            pc2pc3Graph->graph(i)->setData(pc2[i], pc3[i]);
            pc2pc3Graph->graph(i)->setLineStyle(QCPGraph::lsNone);
            pc2pc3Graph->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
            pc2pc3Graph->graph(i)->setPen(QPen(color));
        }

        if (hullsToggleCheckBox->checkState() == Qt::Checked)
        {
            setConvexHulls(pc1pc2, pc1pc2Graph);
            setConvexHulls(pc1pc3, pc1pc3Graph);
            setConvexHulls(pc2pc3, pc2pc3Graph);
        }

        pc1pc2Graph->replot();
        pc1pc3Graph->replot();
        pc2pc3Graph->replot();
    }
}

void MainWindow::setConvexHulls(const QVector<QVector<std::array<double, 2> > > &points, QCustomPlot *graph)
{
    for (int i = 0; i < points.size(); i++)
    {
        QVector<std::array<double, 2>> convexHull = computeConvexHulls(points[i]);

        QVector<double> t, x, y;

        for (int j = 0; j < convexHull.size(); j++)
        {
            t.push_back(j);
            x.push_back(convexHull[j][0]);
            y.push_back(convexHull[j][1]);
        }

        QCPCurve *hull = new QCPCurve(graph->xAxis, graph->yAxis);
        hull->setData(t, x, y);

        QColor color;
        color.setHsl(320 * i / clusterNumber, 255, 127);

        hull->setPen(QPen(color));

        QCPItemText *clusterLabel = new QCPItemText(graph);
        clusterLabel->position->setType(QCPItemPosition::ptPlotCoords);
        clusterLabel->setColor(color);
        clusterLabel->setText(QString::number(i + 1));

        std::array<double, 2> centroid = computeCentroid(convexHull);
        clusterLabel->position->setCoords(centroid[0], centroid[1]);
    }
}

bool sortFunction(std::array<double, 2> a, std::array<double, 2> b)
{
    return (a[0] > b[0] || a[0] < b[0]) ? a[0] < b[0] : a[1] < b[1];
}

QVector<std::array<double, 2>> MainWindow::computeConvexHulls(QVector<std::array<double, 2>> points)
{
    std::sort(points.begin(), points.end(), sortFunction);

    int n = points.size();
    QVector<std::array<double, 2>> hull;

    for (int i = 0; i < 2 * n; i++)
    {
        int j = i < n ? i : 2 * n - 1 - i;
        while (hull.size() >= 2 && removeMiddle(hull[hull.size() - 2], hull[hull.size() - 1], points[j]))
        {
            hull.pop_back();
        }
        hull.push_back(points[j]);
    }

    return hull;
}

bool MainWindow::removeMiddle(std::array<double, 2> a, std::array<double, 2> b, std::array<double, 2> c)
{
    double cross = (a[0] - b[0]) * (c[1] - b[1]) - (a[1] - b[1]) * (c[0] - b[0]);
    double dot = (a[0] - b[0]) * (c[0] - b[0]) + (a[1] - b[1]) * (c[1] - b[1]);
    return cross < 0 || (cross == 0.0 && dot <= 0);
}

std::array<double, 2> MainWindow::computeCentroid(QVector<std::array<double, 2> > vertices)
{
    std::array<double, 2> centroid {0, 0};

    double det = 0;

    int nVertices = vertices.size() - 1;

    for (int i = 0; i < nVertices; i++)
    {
        int j = i + 1;

        double tempDet = vertices[i][0] * vertices[j][1] - vertices[j][0] * vertices[i][1];
        det += tempDet;

        centroid[0] += (vertices[i][0] + vertices[j][0]) * tempDet;
        centroid[1] += (vertices[i][1] + vertices[j][1]) * tempDet;
    }

    centroid[0] /= 3 * det;
    centroid[1] /= 3 * det;

    return centroid;
}

void MainWindow::clearPCAGraphs()
{
    pca->clearPCAData();

    pc1pc2Graph->clearGraphs();
    pc1pc3Graph->clearGraphs();
    pc2pc3Graph->clearGraphs();

    pc1pc2Graph->clearPlottables();
    pc1pc3Graph->clearPlottables();
    pc2pc3Graph->clearPlottables();

    pc1pc2Graph->clearItems();
    pc1pc3Graph->clearItems();
    pc2pc3Graph->clearItems();

    pc1pc2Graph->replot();
    pc1pc3Graph->replot();
    pc2pc3Graph->replot();
}

void MainWindow::selectCurrentPCAPoint(qint64 position)
{
    if (!pca->pc1.empty())
    {
        int index = static_cast<int>(position / milliseconds);

        if (index < pca->pc1.size())
        {
            QVector<double> pc1Sel(1, pca->pc1[index]);
            QVector<double> pc2Sel(1, pca->pc2[index]);
            QVector<double> pc3Sel(1, pca->pc3[index]);

            if (pc1pc2Graph->hasPlottable(currentPoint12Graph))
            {
                pc1pc2Graph->removeGraph(currentPoint12Graph);
            }

            currentPoint12Graph = pc1pc2Graph->addGraph();
            currentPoint12Graph->setData(pc1Sel, pc2Sel);
            currentPoint12Graph->setLineStyle(QCPGraph::lsNone);
            currentPoint12Graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 9));
            currentPoint12Graph->setPen(QPen(Qt::white));

            pc1pc2Graph->replot();

            if (pc1pc3Graph->hasPlottable(currentPoint13Graph))
            {
                pc1pc3Graph->removeGraph(currentPoint13Graph);
            }

            currentPoint13Graph = pc1pc3Graph->addGraph();
            currentPoint13Graph->setData(pc1Sel, pc3Sel);
            currentPoint13Graph->setLineStyle(QCPGraph::lsNone);
            currentPoint13Graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 9));
            currentPoint13Graph->setPen(QPen(Qt::white));

            pc1pc3Graph->replot();

            if (pc2pc3Graph->hasPlottable(currentPoint23Graph))
            {
                pc2pc3Graph->removeGraph(currentPoint23Graph);
            }

            currentPoint23Graph = pc2pc3Graph->addGraph();
            currentPoint23Graph->setData(pc2Sel, pc3Sel);
            currentPoint23Graph->setLineStyle(QCPGraph::lsNone);
            currentPoint23Graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 9));
            currentPoint23Graph->setPen(QPen(Qt::white));

            pc2pc3Graph->replot();
        }
    }
}

void MainWindow::setClusterHistogram()
{
    QVector<double> x;

    for (int i = 0; i < kmeans->clusterNumber; i++)
    {
        x.push_back(i + 1.0);
    }

    clusterHistogram->setData(x, kmeans->segmentsPerCluster);
    clusterHistogramGraph->xAxis->setRange(0, kmeans->clusterNumber + 1);
    clusterHistogramGraph->yAxis->setRange(0, kmeans->segmentsPerClusterMax * 1.1);

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    clusterHistogramGraph->xAxis->setTicker(textTicker);

    for (int i = 0; i < x.size(); i++)
    {
        textTicker->addTick(i + 1, QString::number(x[i], 'f', 0));
    }

    clusterHistogramGraph->replot();
}

void MainWindow::clearClusterHistogram()
{
    kmeans->clearKMeansData();

    clusterHistogram->data()->clear();
    clusterHistogramGraph->replot();

    clusterLengthHistogram->data()->clear();
    clusterLengthHistogramGraph->replot();
}

void MainWindow::setClusterLengthHistogram()
{
    QVector<double> x;

    for (int i = 0; i < kmeans->clusterLengthHistogram.size(); i++)
    {
        x.push_back(i + 1.0);
    }

    clusterLengthHistogram->setData(x, kmeans->clusterLengthHistogram);
    clusterLengthHistogramGraph->xAxis->setRange(0, x.size() + 1);
    clusterLengthHistogramGraph->yAxis->setRange(0, kmeans->clusterLengthHistogramMax * 1.1);

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    clusterLengthHistogramGraph->xAxis->setTicker(textTicker);

    for (int i = 0; i < kmeans->lengths.size(); i++)
    {
        textTicker->addTick(i + 1, QString::number(kmeans->lengths[i], 'f', 0));
    }

    clusterLengthHistogramGraph->replot();
}

void MainWindow::setRescaledRangeGraph()
{
    rescaledRangeGraph->graph(0)->setData(hurst->lengths, hurst->rescaledRanges);

    rescaledRangeGraph->xAxis->setRange(hurst->minLength * 0.9, hurst->maxLength * 1.1);
    rescaledRangeGraph->yAxis->setRange(hurst->minRescaledRange * 0.9, hurst->maxRescaledRange * 1.1);

    rescaledRangeGraph->graph(0)->setLineStyle(QCPGraph::lsNone);
    rescaledRangeGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));

    randomLine->start->setCoords(hurst->minLength * 0.9, 0.5 * hurst->minLength * 0.9);
    randomLine->end->setCoords(hurst->maxLength * 1.1, 0.5 * hurst->maxLength * 1.1);

    linearRegressionLine->start->setCoords(hurst->minLength, hurst->c + hurst->H * hurst->minLength);
    linearRegressionLine->end->setCoords(hurst->maxLength, hurst->c + hurst->H * hurst->maxLength);

    if (!linearRegressionLine->visible())
    {
        linearRegressionLine->setVisible(true);
    }

    rescaledRangeGraph->replot();
}

void MainWindow::clearRescaledRangeGraph()
{
    rescaledRangeGraph->graph(0)->data()->clear();
    if (linearRegressionLine->visible())
    {
        linearRegressionLine->setVisible(false);
    }
    rescaledRangeGraph->replot();
}

void MainWindow::setIntervalGraph()
{
    QVector<double> indexes(hurst->series.size());

    for (int i = 0; i < hurst->series.size(); i++)
    {
        indexes[i] = i;
    }

    intervalGraph->graph(0)->setData(indexes, hurst->series);

    intervalGraph->xAxis->setRange(indexes.first(), indexes.last());
    intervalGraph->yAxis->setRange(0, hurst->maxSeries);

    intervalGraph->replot();
}

void MainWindow::setCumulativeIntervalGraph()
{
    QVector<double> indexes(hurst->cumulativeSeries.size());

    for (int i = 0; i < hurst->cumulativeSeries.size(); i++)
    {
        indexes[i] = i;
    }

    cumulativeIntervalGraph->graph(0)->setData(indexes, hurst->cumulativeSeries);

    cumulativeIntervalGraph->xAxis->setRange(indexes.first(), indexes.last());
    cumulativeIntervalGraph->yAxis->setRange(hurst->minCumulativeSeries, hurst->maxCumulativeSeries);

    cumulativeIntervalGraph->replot();
}

void MainWindow::clearIntervalGraphs()
{
    intervalGraph->graph(0)->data()->clear();
    intervalGraph->replot();

    cumulativeIntervalGraph->graph(0)->data()->clear();
    cumulativeIntervalGraph->replot();
}
