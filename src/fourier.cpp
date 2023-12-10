#include "fourier.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "fftw3.h"
#include <QFile>
#include <QTextStream>

Fourier::Fourier(QObject *parent) : QThread(parent)
{
    sampleRate = 0;
    sampleNumber = 0;
    milliseconds = 250;
    frequencyBinSize = 1;
    duration = 0;
}

Fourier::~Fourier()
{
    quit();
    requestInterruption();
    wait();
}

void Fourier::readAudioFile(const QString filePath)
{
    ma_decoder_config audioConfig = ma_decoder_config_init(ma_format_f32, 1, 0);

    ma_uint64 frameCount;
    void *pAudioData;

    int result = ma_decode_file(filePath.toStdString().c_str(), &audioConfig, &frameCount, &pAudioData);

    if (result != MA_SUCCESS) {
        ma_free(pAudioData);
        emit(fileDecodingFailed());
        return;
    }

    sampleRate = static_cast<int>(audioConfig.sampleRate);
    sampleNumber = frameCount;
    duration = static_cast<int>(1000 * (sampleNumber / static_cast<unsigned long>(sampleRate)));

    waveForm.clear();
    waveForm.reserve(static_cast<int>(frameCount));

    times.clear();
    times.reserve(static_cast<int>(frameCount));

    float *array = static_cast<float*>(pAudioData);

    float min = array[0];
    float max = array[0];

    for (unsigned int i = 0; i < frameCount; i++)
    {
        waveForm.push_back(static_cast<double>(array[i]));
        times.push_back(static_cast<double>(i) / sampleRate);
        if (array[i] < min)
        {
            min = array[i];
        }
        if (array[i] > max)
        {
            max = array[i];
        }
    }

    minWaveForm = static_cast<double>(min);
    maxWaveForm = static_cast<double>(max);

    minTime = times.first();
    maxTime = times.last();

    ma_free(pAudioData);

    emit(fileRead());
}

void Fourier::readDataFile(const QString filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream in(&file);

    sampleRate = in.readLine().toInt();

    waveForm.clear();
    times.clear();

    unsigned long i = 0;
    double firstDatum = in.readLine().toDouble();
    waveForm.push_back(firstDatum);
    times.push_back(static_cast<double>(i) / sampleRate);
    i++;

    minWaveForm = firstDatum;
    maxWaveForm = firstDatum;

    while (!in.atEnd())
    {
        double datum = in.readLine().toDouble();
        waveForm.push_back(datum);
        times.push_back(static_cast<double>(i) / sampleRate);
        i++;

        if (datum < minWaveForm)
        {
            minWaveForm = datum;
        }
        if (datum > maxWaveForm)
        {
            maxWaveForm = datum;
        }
    }

    minTime = times.first();
    maxTime = times.last();

    sampleNumber = i;
    duration = static_cast<int>(1000 * (sampleNumber / static_cast<unsigned long>(sampleRate)));

    emit(fileRead());
}

void Fourier::performFFTAnalysis()
{
    start();
}

void Fourier::run()
{
    step = 0;
    emit(fftAnalysisStep(step));

    spectrum.clear();

    int nSamples = static_cast<int>(sampleRate) * milliseconds / 1000;
    nFrequencies = nSamples / 2 + 1;

    double *in = fftw_alloc_real(static_cast<unsigned long>(nSamples));
    fftw_complex *out = fftw_alloc_complex(static_cast<unsigned long>(nFrequencies));

    emit(sendMessage("Planning FFT..."));

    fftw_plan plan = fftw_plan_dft_r2c_1d(static_cast<int>(nSamples), in, out, FFTW_PATIENT | FFTW_DESTROY_INPUT);

    emit(sendMessage("Computing FFTs..."));

    QVector<std::complex<double>> power(nFrequencies, std::complex<double>(0, 0));

    for (int i = 0; i < waveForm.size(); i += nSamples)
    {
        if (i + nSamples < waveForm.size())
        {
            for (int j = 0; j < nSamples; j++)
            {
                in[j] = waveForm[i + j];
            }

            fftw_execute(plan);

            for (int j = 0; j < nFrequencies; j++)
            {
                power[j] = std::complex<double>(out[j][0], out[j][1]);
            }

            spectrum.push_back(power);

            step++;
            emit(fftAnalysisStep(step));
        }
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    obtainSpectra();
}

void Fourier::obtainSpectra()
{
    int nFrequencyBins = static_cast<int>(ceil(static_cast<double>(nFrequencies - 1) / frequencyBinSize));

    double deltaF = 1000.0 / milliseconds;

    frequencies.clear();
    frequencies.reserve(nFrequencyBins);

    for (int i = 0; i < nFrequencyBins; i++)
    {
        frequencies.push_back(((i + 1.0) * frequencyBinSize - (frequencyBinSize >> 1))* deltaF);
    }

    spectra.clear();
    spectra.reserve(static_cast<int>(spectrum.size()));

    QVector<double> components;
    components.reserve(nFrequencyBins);

    for (QVector<std::complex<double>> oneSpectrum : spectrum)
    {
        components.clear();

        int j = 1; // Skip first (DC) component (frequency index = 0)

        bool iterate = true;

        while (iterate)
        {
            double sum = 0;

            for (int k = j; k < j + frequencyBinSize; k++)
            {
                sum += std::abs(oneSpectrum[k]);
            }

            sum /= frequencyBinSize;

            components.push_back(sum);

            if (sum > supPower)
            {
                supPower= sum;
            }

            if (j + 2 * frequencyBinSize < nFrequencies)
            {
                j += frequencyBinSize;
            }
            else
            {
                iterate = false;
            }
        }

        if (j < nFrequencies - 1)
        {
            int delta = nFrequencies - 1 - j;

            double sum = 0;

            for (int k = j; k < nFrequencies; k++)
            {
                sum += std::abs(oneSpectrum[k]);
            }

            sum /= delta;

            components.push_back(sum);

            if (sum > supPower)
            {
                supPower = sum;
            }
        }

        spectra.push_back(components);

        step++;
        emit(fftAnalysisStep(step));
    }

    spectrum.clear();
    spectrum.shrink_to_fit();

    emit(fftAnalysisPerformed());
}


void Fourier::clearFFTData()
{
    frequencies.clear();
    frequencies.shrink_to_fit();
    spectra.clear();
    spectra.shrink_to_fit();
}

void Fourier::clearWaveFormData()
{
    times.clear();
    times.shrink_to_fit();
}
