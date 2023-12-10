#include "pca.h"
#include <math.h>

PCA::PCA(QObject *parent) : QThread(parent)
{
    componentNumber = 5;
    abort = false;
}

PCA::~PCA()
{
    abort = true;
    quit();
    requestInterruption();
    wait();
}

void PCA::initData(QVector<QVector<double>> receivedData)
{
    data = receivedData;
}

void PCA::centerColumns()
{
    int nRows = data.size();
    int nCols = data[0].size();

    // Obtain means

    QVector<double> mean(nCols, 0);

    for (int row = 0; row < nRows; row++)
    {
        for (int col = 0; col < nCols; col++)
        {
            mean[col] += data[row][col];
        }
    }

    for (int col = 0; col < nCols; col++)
    {
        mean[col] /= nRows;
    }

    // Center columns

    for (int row = 0; row < nRows; row++)
    {
        for (int col = 0; col < nCols; col++)
        {
            data[row][col] -= mean[col];
        }
    }
}

void PCA::performPCA()
{
    abort = false;
    start();
}

void PCA::run()
{
    centerColumns();

    int nRows = data.size();
    int nCols = data[0].size();

    double tolerance = 1.0e-7;

    QVector<QVector<double>> rowScore(componentNumber, QVector<double>(nRows, 0));
    QVector<QVector<double>> colScore(componentNumber, QVector<double>(nCols, 0));

    QVector<double> eigenvalue(componentNumber, 0);

    int step = 0;
    emit(pcaStep(step));

    for (int k = 0; k < componentNumber; k++)
    {
        // Init object scores

        for (int row = 0; row < nRows; row++)
        {
            rowScore[k][row] = row + 1;
        }

        bool iterate = true;

        int iterationStep = 0;
        emit(pcaIterationStep(iterationStep));

        while (iterate)
        {
            if (abort)
            {
                emit(pcaAborted());
                return;
            }

            // Compute new variable loadings

            for (int col = 0; col < nCols; col++)
            {
                colScore[k][col] = 0;
                for (int row = 0; row < nRows; row++)
                {
                    colScore[k][col] += data[row][col] * rowScore[k][row];
                }
            }

            // Compute new object scores

            for (int row = 0; row < nRows; row++)
            {
                rowScore[k][row] = 0;
                for (int col = 0; col < nCols; col++)
                {
                    rowScore[k][row] += data[row][col] * colScore[k][col];
                }
            }

            if (k > 0)
            {
                // Gram-Schmidt orthogonalization: make object scores uncorrelated with all previous axes

                for (int prevK = 0; prevK < k; prevK++)
                {
                    double norm2 = 0;
                    double product = 0;
                    for (int row = 0; row < nRows; row++)
                    {
                        norm2 += rowScore[prevK][row] * rowScore[prevK][row];
                        product += rowScore[k][row] * rowScore[prevK][row];
                    }
                    for (int row = 0; row < nRows; row++)
                    {
                        rowScore[k][row] -= product * rowScore[prevK][row] / norm2;
                    }
                }
            }

            // Obtain norm of object scores

            double rowScoreNorm = 0;
            for (int row = 0; row < nRows; row++)
            {
                rowScoreNorm += rowScore[k][row] * rowScore[k][row];
            }
            rowScoreNorm = sqrt(rowScoreNorm);

            // Normalize object scores

            for (int row = 0; row < nRows; row++)
            {
                rowScore[k][row] /= rowScoreNorm;
            }

            // Estimate of eigenvalue

            double eigenvalueEstimate = rowScoreNorm / (nRows - 1);

            // Check for convergence

            iterationStep++;
            emit(pcaIterationStep(iterationStep));

            if (fabs(eigenvalue[k] - eigenvalueEstimate) < tolerance)
            {
                eigenvalue[k] = eigenvalueEstimate;
                iterate = false;
            }

            eigenvalue[k] = eigenvalueEstimate;
        }

        // Obtain norm of eigenvector (variable loadings)

        double colScoreNorm = 0;
        for (int col = 0; col < nCols; col++)
        {
            colScoreNorm += colScore[k][col] * colScore[k][col];
        }
        colScoreNorm = sqrt(colScoreNorm);

        // Normalize eigenvector

        for (int col = 0; col < nCols; col++)
        {
            colScore[k][col] /= colScoreNorm;
        }

        // Rescale principal component (object scores)

        double scale = sqrt((nRows - 1) * eigenvalue[k]);
        for (int row = 0; row < nRows; row++)
        {
            rowScore[k][row] *= scale;
        }

        step++;
        emit(pcaStep(step));
    }

    eigenvalues = eigenvalue;

    // Format principal components for use in K-Means

    principalComponents.clear();
    principalComponents.reserve(nRows);

    QVector<double> components(componentNumber);

    for (int row = 0; row < nRows; row++)
    {
        for (int k = 0; k < componentNumber; k++)
        {
            components[k] = rowScore[k][row];
        }

        principalComponents.push_back(components);
    }

    // PC1, PC2 and PC3 for plotting

    pc1.clear();
    pc1.reserve(nRows);

    pc2.clear();
    pc2.reserve(nRows);

    pc3.clear();
    pc3.reserve(nRows);

    pc1Min = rowScore[0][0];
    pc1Max = rowScore[0][0];

    pc2Min = rowScore[1][0];
    pc2Max = rowScore[1][0];

    pc3Min = rowScore[2][0];
    pc3Max = rowScore[2][0];

    for (int row = 0; row < nRows; row++)
    {
        pc1.push_back(rowScore[0][row]);
        pc2.push_back(rowScore[1][row]);
        pc3.push_back(rowScore[2][row]);

        if (rowScore[0][row] < pc1Min)
        {
            pc1Min = rowScore[0][row];
        }
        if (rowScore[0][row] > pc1Max)
        {
            pc1Max = rowScore[0][row];
        }

        if (rowScore[1][row] < pc2Min)
        {
            pc2Min = rowScore[1][row];
        }
        if (rowScore[1][row] > pc2Max)
        {
            pc2Max = rowScore[1][row];
        }

        if (rowScore[2][row] < pc3Min)
        {
            pc3Min = rowScore[2][row];
        }
        if (rowScore[2][row] > pc3Max)
        {
            pc3Max = rowScore[2][row];
        }
    }

    data.clear();
    data.shrink_to_fit();

    emit(pcaPerformed());
}

void PCA::clearPCAData()
{
    pc1.clear();
    pc1.shrink_to_fit();
    pc2.clear();
    pc2.shrink_to_fit();
    pc3.clear();
    pc3.shrink_to_fit();
}
