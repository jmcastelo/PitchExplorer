#include "hurst.h"
#include <math.h>

Hurst::Hurst(QObject *parent) : QThread(parent)
{
    H = 0;
}

Hurst::~Hurst()
{
    quit();
    requestInterruption();
    wait();
}

void Hurst::initData(QVector<int> receivedData)
{
    data = receivedData;
}

void Hurst::performHurst()
{
    start();
}

void Hurst::run()
{
    computeSeries();

    cumulativeSeries = computeCumulativeSeries(series);

    getMinMaxSeries();

    rescaledRanges.clear();
    lengths.clear();

    int N = series.size();

    if (N / 16 < 10)
    {
        emit(notEnoughData());
        return;
    }

    int d = 2;
    int numElements = N / d;

    while (numElements >= 10)
    {
        double rescaledRange = 0;

        for (int i = 0; i < d; i++)
        {
            QVector<double> slice(numElements);

            for (int j = 0; j < numElements; j++)
            {
                slice[j] = series[i * numElements + j];
            }

            rescaledRange += computeRescaledRange(slice);
        }

        rescaledRanges.push_back(log10(rescaledRange / d));
        lengths.push_back(log10(numElements));

        int numElementsLeft = N - d * numElements;

        if (numElementsLeft > 0)
        {
            rescaledRange = 0;

            for (int i = 0; i < d; i++)
            {
                QVector<double> slice(numElements);

                for (int j = 0; j < numElements; j++)
                {
                    slice[j] = series[N - 1 - i * numElements - j];
                }

                rescaledRange += computeRescaledRange(slice);
            }

            rescaledRanges.push_back(log10(rescaledRange / d));
            lengths.push_back(log10(numElements));
        }

        d *= 2;
        numElements = N / d;
    }

    getMinMaxRescaledRange();

    computeHurstExponent();

    emit(hurstPerformed());
}

void Hurst::computeSeries()
{
    series.clear();

    int length = 1;
    int index = 0;
    bool iterate = true;

    while (iterate)
    {
        for (int i = index + 1; i < data.size(); i++)
        {
            if (data[i] == data[index])
            {
                length++;
            }
            else
            {
                series.push_back(length);

                length = 1;

                index = i;

                if (i == data.size() - 1)
                {
                    iterate = false;
                }

                break;
            }

            if (i == data.size() - 1)
            {
                series.push_back(length);
                iterate = false;
            }
        }
    }
}

QVector<double> Hurst::computeCumulativeSeries(QVector<double> timeSeries)
{
    int N = timeSeries.size();

    double mean = 0;

    for (double element : timeSeries)
    {
        mean += element;
    }

    mean /= N;

    QVector<double> normalizedTimeSeries(N);

    for (int i = 0; i < N; i++)
    {
        normalizedTimeSeries[i] = timeSeries[i] - mean;
    }

    QVector<double> cumulativeTimeSeries(N, 0);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < i + 1; j++)
        {
            cumulativeTimeSeries[i] += normalizedTimeSeries[j];
        }
    }

    return cumulativeTimeSeries;
}

double Hurst::computeRescaledRange(QVector<double> timeSeries)
{
    int N = timeSeries.size();

    double mean = 0;

    for (double element : timeSeries)
    {
        mean += element;
    }

    mean /= N;

    double stdDev = 0;

    for (double element : timeSeries)
    {
        double diff = element - mean;
        stdDev += diff * diff;
    }

    stdDev = sqrt(stdDev / N);

    QVector<double> cumulativeTimeSeries = computeCumulativeSeries(timeSeries);

    double max = cumulativeTimeSeries[0];
    double min = cumulativeTimeSeries[0];

    for (double cum : cumulativeTimeSeries)
    {
        if (cum < min)
        {
            min = cum;
        }
        if (cum > max)
        {
            max = cum;
        }
    }

    if (stdDev == 0.0)
    {
        return 0;
    }

    return (max - min) / stdDev;
}

void Hurst::computeHurstExponent()
{
    int N = rescaledRanges.size();

    QVector<double> y = rescaledRanges;
    QVector<double> x = lengths;

    double yMean = 0;

    for (double value: y)
    {
        yMean += value;
    }

    yMean /= N;

    double xMean = 0;

    for (double value: x)
    {
        xMean += value;
    }

    xMean /= N;

    double covariance = 0;
    double variance = 0;

    for (int i = 0; i < N; i++)
    {
        double xCentered = x[i] - xMean;
        double yCentered = y[i] - yMean;

        covariance += xCentered * yCentered;
        variance += xCentered * xCentered;
    }

    H = covariance / variance;
    c = yMean - H * xMean;
}

void Hurst::getMinMaxRescaledRange()
{
    minLength = lengths[0];
    maxLength = lengths[0];

    for (double length : lengths)
    {
        if (length < minLength)
        {
            minLength = length;
        }
        if (length > maxLength)
        {
            maxLength = length;
        }
    }

    minRescaledRange = rescaledRanges[0];
    maxRescaledRange = rescaledRanges[0];

    for (double range : rescaledRanges)
    {
        if (range < minRescaledRange)
        {
            minRescaledRange = range;
        }
        if (range > maxRescaledRange)
        {
            maxRescaledRange = range;
        }
    }
}

void Hurst::getMinMaxSeries()
{
    minSeries = series[0];
    maxSeries = series[0];

    for (double element : series)
    {
        if (element < minSeries)
        {
            minSeries = element;
        }
        if (element > maxSeries)
        {
            maxSeries = element;
        }
    }

    minCumulativeSeries = cumulativeSeries[0];
    maxCumulativeSeries = cumulativeSeries[0];

    for (double element : cumulativeSeries)
    {
        if (element < minCumulativeSeries)
        {
            minCumulativeSeries = element;
        }
        if (element > maxCumulativeSeries)
        {
            maxCumulativeSeries = element;
        }
    }
}
