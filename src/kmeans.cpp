#include "kmeans.h"
#include <random>
#include <math.h>

KMeans::KMeans(QObject *parent) : QThread(parent)
{
    clusterNumber = 10;
}

KMeans::~KMeans()
{
    quit();
    requestInterruption();
    wait();
}

void KMeans::initData(QVector<QVector<double>> receivedData)
{
    data = receivedData;
}

void KMeans::performKMeans()
{
    start();
}

void KMeans::run()
{
    int dataSize = data.size();
    int dim = data[0].size();

    QVector<QVector<double>> centroids;
    centroids.reserve(clusterNumber);

    int chunk = (dataSize - 1) / clusterNumber;

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, chunk);

    for (int i = 0; i < clusterNumber; i++)
    {
        int j = i * chunk + distribution(generator);
        centroids.push_back(data[j]);
    }

    clusterIndexes.clear();
    clusterIndexes.reserve(dataSize);

    for (int i = 0; i < dataSize; i++)
    {
        clusterIndexes.push_back(0);
    }

    QVector<int> clusterCount(clusterNumber, 0);

    bool iterate = true;

    int step = 0;
    emit(kMeansIterationStep(step));

    while (iterate)
    {
        for (int i = 0; i < clusterNumber; i++)
        {
            clusterCount[i] = 0;
        }

        int numClusterChanges = 0;

        for (int i = 0; i < dataSize; i++)
        {
            double minDistance = distance(data[i], centroids[0]);
            int c = 0;

            for (int j = 1; j < clusterNumber; j++)
            {
                double dist = distanceCheck(data[i], centroids[j], minDistance);
                if (dist < minDistance)
                {
                    minDistance = dist;
                    c = j;
                }
            }

            if (clusterIndexes[i] != c)
            {
                numClusterChanges++;
                clusterIndexes[i] = c;
            }

            clusterCount[c]++;
        }

        step++;
        emit(kMeansIterationStep(step));

        if (numClusterChanges == 0)
        {
            iterate = false;
        }

        for (int i = 0; i < clusterNumber; i++)
        {
            for (int j = 0; j < dim; j++)
            {
                centroids[i][j] = 0;
            }
        }

        for (int i = 0; i < dataSize; i++)
        {
            for (int j = 0; j < dim; j++)
            {
                centroids[clusterIndexes[i]][j] += data[i][j];
            }
        }

        for (int i = 0; i < clusterNumber; i++)
        {
            for (int j = 0; j < dim; j++)
            {
                centroids[i][j] /= clusterCount[i];
            }
        }
    }

    computeClusterHistogram(clusterCount);
    reassignClusterIndexes();
    computeClusterLengthHistogram();

    data.clear();
    data.shrink_to_fit();

    emit(kMeansPerformed());
}

void KMeans::computeClusterHistogram(QVector<int> clusterCount)
{
    clusters.clear();
    clusters.reserve(clusterNumber);

    segmentsPerCluster.clear();
    segmentsPerCluster.reserve(clusterNumber);

    for (int c = 0; c < clusterNumber; c++)
    {
        clusters.push_back(c);
        segmentsPerCluster.push_back(clusterCount[c]);
    }

    for (int i = 0; i < clusterNumber; i++)
    {
        for (int j = i + 1; j < clusterNumber; j++)
        {
            if (segmentsPerCluster[j] > segmentsPerCluster[i])
            {
                segmentsPerCluster.swapItemsAt(j, i);
                clusters.swapItemsAt(j, i);
            }
        }
    }

    segmentsPerClusterMax = segmentsPerCluster[0];

    for (int i = 1; i < clusterNumber; i++)
    {
        if (segmentsPerCluster[i] > segmentsPerClusterMax)
        {
            segmentsPerClusterMax = segmentsPerCluster[i];
        }
    }
}

void KMeans::reassignClusterIndexes()
{
    for (int i = 0; i < clusterIndexes.size(); i++)
    {
        for (int c = 0; c < clusters.size(); c++)
        {
            if (clusterIndexes[i] == static_cast<int>(clusters[c]))
            {
                clusterIndexes[i] = c;
                break;
            }
        }
    }
}

void KMeans::computeClusterLengthHistogram()
{
    lengths.clear();
    lengths.reserve(clusterIndexes.size());

    clusterLengthHistogram.clear();
    clusterLengthHistogram.reserve(clusterIndexes.size());

    for (int i = 0; i < clusterIndexes.size(); i++)
    {
        lengths.push_back(i + 1);
        clusterLengthHistogram.push_back(0);
    }

    int length = 0;
    int index = 0;
    bool iterate = true;

    while (iterate)
    {
        for (int i = index + 1; i < clusterIndexes.size(); i++)
        {
            if (clusterIndexes[i] == clusterIndexes[index])
            {
                length++;
            }
            else
            {
                clusterLengthHistogram[length]++;

                length = 0;

                index = i;

                if (i == data.size() - 1)
                {
                    iterate = false;
                }

                break;
            }

            if (i ==clusterIndexes.size() - 1)
            {
                clusterLengthHistogram[length]++;
                iterate = false;
            }
        }
    }

    for (int i = 0; i < clusterLengthHistogram.size(); i++)
    {
        for (int j = i + 1; j < clusterLengthHistogram.size(); j++)
        {
            if (clusterLengthHistogram[j] > clusterLengthHistogram[i])
            {
                clusterLengthHistogram.swapItemsAt(i, j);
                lengths.swapItemsAt(i, j);
            }
        }
    }

    for (int i = clusterLengthHistogram.size() - 1; i >= 0; i--)
    {
        if (clusterLengthHistogram[i] == 0.0)
        {
            clusterLengthHistogram.pop_back();
            lengths.pop_back();
        }
    }

    index = 0;

    while (index < clusterLengthHistogram.size())
    {
        int kmax = clusterLengthHistogram.size();

        for (int j = index + 1; j < clusterLengthHistogram.size(); j++)
        {
            if (static_cast<int>(clusterLengthHistogram[j]) != static_cast<int>(clusterLengthHistogram[index]))
            {
                kmax = j;
                break;
            }
        }

        for (int k = index; k < kmax; k++)
        {
            for (int j = k + 1; j < kmax; j++)
            {
                if (lengths[j] < lengths[k])
                {
                    clusterLengthHistogram.swapItemsAt(k, j);
                    lengths.swapItemsAt(k, j);
                }
            }
        }

        index = kmax;
    }

    clusterLengthHistogramMax = 0;

    for (int i = 0; i < clusterLengthHistogram.size(); i++)
    {
        if (clusterLengthHistogram[i] > clusterLengthHistogramMax)
        {
            clusterLengthHistogramMax = clusterLengthHistogram[i];
        }
    }
}

double KMeans::distance(const QVector<double> &vector1, const QVector<double> &vector2)
{
    double dist = 0;

    for (int i = 0; i < vector1.size(); i++)
    {
        double diff = vector1[i] - vector2[i];
        dist += diff * diff;
    }

    return dist;
}

double KMeans::distanceCheck(const QVector<double> &vector1, const QVector<double> &vector2, const double &minDistance)
{
    double dist = 0;

    for (int i = 0; i < vector1.size(); i++)
    {
        double diff = vector1[i] - vector2[i];
        dist += diff * diff;

        if (dist >= minDistance)
        {
            return minDistance;
        }
    }

    return dist;
}

void KMeans::clearKMeansData()
{
    clusterIndexes.clear();
    clusters.clear();
    segmentsPerCluster.clear();
}
