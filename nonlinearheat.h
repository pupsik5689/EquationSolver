#ifndef NONLINEARHEAT_H
#define NONLINEARHEAT_H

#include <QObject>
#include <QVector>
#include <QDir>
#include <QSaveFile>
#include <QTextStream>
#include <QString>
#include <stdexcept>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <cmath>

#include "layersnapshot.h"

class NonLinearHeat : public QObject
{
    Q_OBJECT

public:
    explicit NonLinearHeat(QObject* parent = nullptr);

    void DefaultValues();

    void SetParams(double _L, double _T,
                   int _n, int _m, int _j_shag,
                   double _k0, double _q0, double _alpha, double _beta,
                   double _A, double _x_c, double _r,
                   int _scheme, int method, int _p);

    void FillSemiLinearScheme();
    void FillExplicitScheme();
    void Solver();

    QVector<double> SolveTridiagonal(const QVector<double>& A,
                                     const QVector<double>& B,
                                     const QVector<double>& C,
                                     const QVector<double>& Fi);

    QVector<double> SolveTridiagonalClassic(const QVector<double>& A, const QVector<double>& B, const QVector<double>& C, const QVector<double>& F);
    QVector<double> BlockProgonka(QVector<double>& A, QVector<double>& B, QVector<double>& C, QVector<double>& F, int p_);

    void Progonka();

    void SaveLayerToCsv(int layerIndex);

    const QVector<LayerSnapshot2>& GetSavedLayers() const;

    double GetL() const { return L; }

    void ResetTiming();
    double GetTridiagonalMs() const;
    int GetTridiagonalCalls() const;

public slots:
    void Run();

signals:
    void layerReady(const LayerSnapshot2& snap);
    void solverFinished();
    void solverError(const QString& message);

    void timingReady(double tridiagonalMs,
                     int tridiagonalCalls);

private:
    QVector<double> v;

    double A = 5;
    double r = 3;
    double x_c = 25;

    double L = 0;
    double T = 0;

    int n = 0;
    int m = 0;

    double h = 0.0;
    double tau = 0.0;

    int j_shag = 0;

    double k0 = 0;
    double q0 = 0;
    double alpha = 0;
    double beta = 0;

    int scheme = 0;

    int method, p;

    void SaveLayerToMemory(int layerIndex);

    QVector<LayerSnapshot2> savedLayers;

    QVector<double> v_A;
    QVector<double> v_B;
    QVector<double> v_C;
    QVector<double> v_fi;

    qint64 tridiagonalNs = 0;
    int tridiagonalCalls = 0;
};

#endif // NONLINEARHEAT_H
