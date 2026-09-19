#ifndef MEINHARDTSOLVER_H
#define MEINHARDTSOLVER_H

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
#include <cmath>
#include <QElapsedTimer>
#include "layersnapshot.h"

class MeinhardtSolver : public QObject
{
    Q_OBJECT

public:

    explicit MeinhardtSolver(QObject* parent = nullptr);

    void DefaultValues();
    void SetParams(double _L, double _T,
                   int _n, int _m, int _j_shag,
                   double _Da, double _Db, double _Dc,
                   double _ro_a, double _ro_b, double _ro_c,
                   double _Ka, double _Kb,
                   double _sigma_a, double _sigma_b,
                   int _scheme, int _method, int _p);

    void FillSemiLinearScheme();
    void FillExplicitScheme();
    void Solver();

    QVector<double> SolveTridiagonal(const QVector<double>& A, const QVector<double>& B, const QVector<double>& C, const QVector<double>& Fi);
    QVector<double> SolveTridiagonalClassic(const QVector<double>& A, const QVector<double>& B, const QVector<double>& C, const QVector<double>& F);
    QVector<double> BlockProgonka(QVector<double>& A, QVector<double>& B, QVector<double>& C, QVector<double>& F, int p_);

    void Progonka();
    void SaveLayerToCsv(int layerIndex);

    const QVector<LayerSnapshot>& GetSavedLayers() const;

    void ResetTiming();

    double GetSequentialProgonkaMs() const;
    double GetBlockProgonkaMs() const;

    int GetSequentialProgonkaCalls() const;
    int GetBlockProgonkaCalls() const;


public slots:
    void Run();

signals:
    void layerReady(const LayerSnapshot& snap);
    void solverFinished();
    void solverError(const QString& message);
    void timingReady(double sequentialMs, int sequentialCalls, double blockMs, int blockCalls);

public:
    QVector<double> get_a;
    QVector<double> get_b;
    QVector<double> get_c;

private:
    QVector<double> a;
    QVector<double> b;
    QVector<double> c;

    QVector<double> a_A;
    QVector<double> a_B;
    QVector<double> a_C;
    QVector<double> a_fi;

    QVector<double> b_A;
    QVector<double> b_B;
    QVector<double> b_C;
    QVector<double> b_fi;

    QVector<double> c_A;
    QVector<double> c_B;
    QVector<double> c_C;
    QVector<double> c_fi;

    double L = 0, T = 0;
    int n = 0, m = 0;
    double h = 0., tau = 0.;

    int j_shag = 0;

    double Da = 0, Db = 0, Dc = 0,
        ro_a = 0, ro_b = 0, ro_c = 0,
        Ka = 0, Kb = 0,
        sigma_a = 0, sigma_b = 0;

    int scheme = 0;


    double A0, A1, A2, A3;
    double B0, B1, B2, B3;
    double C0, C1, C2, C3;

    int k1, k2, k3;

    void SaveLayerToMemory(int layerIndex);
    QVector<LayerSnapshot> savedLayers;

    int method, p;

    qint64 sequentialProgonkaNs = 0;
    qint64 blockProgonkaNs = 0;

    int sequentialProgonkaCalls = 0;
    int blockProgonkaCalls = 0;
};

#endif // MEINHARDTSOLVER_H
