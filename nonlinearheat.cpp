
// #include "nonlinearheat.h"

// NonLinearHeat::NonLinearHeat()
// {

// }

// void NonLinearHeat::SetParams(double _L, double _T,
//                               int _n, int _m, int _j_shag,
//                               double _k0, double _q0, double _alpha, double _beta,
//                               double _A, double _x_c, double _r,
//                               int _scheme)
// {
//     this->L = _L;
//     this->T = _T;
//     this->n = _n;
//     this->m = _m;

//     this->h = _L / _n;
//     this->tau = _T / _m;

//     this->j_shag = _j_shag;

//     this->k0 = _k0;
//     this->q0 = _q0;
//     this->alpha = _alpha;
//     this->beta = _beta;

//     this->scheme = _scheme;

//     v.resize(n + 1);
// }

// void NonLinearHeat::DefaultValues()
// {
//     // for (int i = 0; i <= n; ++i)
//     // {
//     //     double x = i * h;

//     //     if (abs(x - x_c) <= r)
//     //     {
//     //         v[i] = A * (1 - ((x - x_c)/r) * ((x - x_c)/r));
//     //     }
//     // }


//     for (int i = 0; i <= n; ++i)
//     {
//         double x = i * h;


//         v[i] = sin(M_PI * x);

//     }
//     v[0] = 0.0;
//     v[n] = 0.0;
// }

// void NonLinearHeat::FillSemiLinearScheme()
// {
//     // for a
//     v_A.resize(n + 1);
//     v_B.resize(n + 1);
//     v_C.resize(n + 1);
//     v_fi.resize(n + 1);

//     v_A[0] = 0.;
//     v_C[0] = 1.;
//     v_B[0] = 0.;
//     v_fi[0] = 0.;
//     for (int i = 1; i < n; ++i)
//     {
//         v_A[i] = k0 * tau / (2 * h * h) * (pow(v[i], alpha) + pow(v[i - 1], alpha));
//         v_B[i] = k0 * tau / (2 * h * h) * (pow(v[i], alpha) + pow(v[i + 1], alpha));
//         v_C[i] = 1. + k0 * tau / (2 * h * h) * (pow(v[i + 1], alpha) + 2. * pow(v[i], alpha) + pow(v[i - 1], alpha));

//         v_fi[i] = v[i] + q0 * tau * pow(v[i], beta);
//     }
//     v_A[n] = 0.;
//     v_C[n] = 1.;
//     v_B[n] = 0.;
//     v_fi[n] = 0.;
// }

// void NonLinearHeat::FillExplicitScheme()
// {
//     QVector<double> vNew(n + 1, 0.0);

//     // Границы Дирихле: v(0,t)=0, v(L,t)=0
//     vNew[0] = 0.0;
//     vNew[n] = 0.0;

//     for (int i = 1; i < n; ++i)
//     {
//         double kPlus  = 0.5 * (std::pow(v[i], alpha) + std::pow(v[i + 1], alpha));
//         double kMinus = 0.5 * (std::pow(v[i], alpha) + std::pow(v[i - 1], alpha));

//         double diffusion =
//             k0 / (h * h) *
//             ( kPlus  * (v[i + 1] - v[i])
//              - kMinus * (v[i] - v[i - 1]) );

//         double reaction = q0 * std::pow(v[i], beta);

//         vNew[i] = v[i] + tau * (diffusion + reaction);
//     }

//     v = vNew;
// }

// void NonLinearHeat::Solver()
// {
//     qDebug() << "scheme =" << scheme;
//     qDebug() << "m =" << m;
//     qDebug() << "j_shag =" << j_shag;

//     DefaultValues();

//     savedLayers.clear();
//     SaveLayerToMemory(0);

//     if (scheme == 1)
//     {
//         for(int j = 1; j < m; ++j)
//         {
//             this->FillSemiLinearScheme();
//             this->Progonka();

//             if (j % j_shag == 0)
//             {
//                 SaveLayerToCsv(j);
//                 SaveLayerToMemory(j);
//             }
//         }
//         if ((m - 1) % j_shag != 0)
//         {
//             SaveLayerToCsv(m - 1);
//             SaveLayerToMemory(m - 1);
//         }
//     } else if (scheme == 0)
//     {
//         for(int j = 1; j < m; ++j)
//         {
//             this->FillExplicitScheme();

//             if (j % j_shag == 0)
//             {
//                 SaveLayerToCsv(j);
//                 SaveLayerToMemory(j);
//             }
//         }
//         if ((m - 1) % j_shag != 0)
//         {
//             SaveLayerToCsv(m - 1);
//             SaveLayerToMemory(m - 1);
//         }
//     }
// }

// void NonLinearHeat::Progonka()
// {
//     QVector<double> vNew = SolveTridiagonal(v_A, v_B, v_C, v_fi);

//     v = vNew;
// }

// QVector<double> NonLinearHeat::SolveTridiagonal(const QVector<double>& A, const QVector<double>& B, const QVector<double>& C, const QVector<double>& Fi)
// {
//     qDebug() << "began";
//     //QVector<double> alpha(n + 1);
//     //QVector<double> beta(n + 1);
//     //QVector<double> y(n + 1);

//     //alpha[1] = 0.;
//     //beta[1] = 0.;

//     //Прямой ход
//     // for (int i = 1; i < n; i++)
//     // {
//     //     double znam = (C[i] - A[i] * alpha[i]);
//     //     alpha[i + 1] = B[i] / znam;
//     //     beta[i + 1] = (Fi[i] + A[i] * beta[i]) / znam;
//     // }

//     // //Обратный ход
//     // y[n] = (A[n] * beta[n] - Fi[n]) / (-A[n] * alpha[n] - B[n]);

//     // for (int i = n - 1; i >= 0; i--)
//     // {
//     //     y[i] = alpha[i + 1] * y[i + 1] + beta[i + 1];
//     // }

//     // qDebug() << "end";
//     // return y;

//     QVector<double> alphaCoef(n + 1, 0.0);
//     QVector<double> betaCoef(n + 1, 0.0);
//     QVector<double> y(n + 1, 0.0);

//     // Первая строка: C[0] * y[0] - B[0] * y[1] = Fi[0]
//     alphaCoef[0] = B[0] / C[0];
//     betaCoef[0]  = Fi[0] / C[0];

//     for (int i = 1; i <= n; ++i)
//     {
//         double denom = C[i] - A[i] * alphaCoef[i - 1];

//         if (std::abs(denom) < 1e-14)
//             throw std::runtime_error("Zero denominator in tridiagonal solver");

//         alphaCoef[i] = (i < n ? B[i] / denom : 0.0);
//         betaCoef[i]  = (Fi[i] + A[i] * betaCoef[i - 1]) / denom;
//     }

//     y[n] = betaCoef[n];

//     for (int i = n - 1; i >= 0; --i)
//         y[i] = alphaCoef[i] * y[i + 1] + betaCoef[i];

//     return y;
// }

// void NonLinearHeat::SaveLayerToCsv(int layerIndex)
// {
//     // Папка results рядом с программой / рабочей директорией
//     QString dirPath = "resultsNonLinear";
//     QDir dir;
//     if (!dir.mkpath(dirPath))
//         throw std::runtime_error("Cannot create results directory");

//     // Имя файла: layer_0000.csv, layer_0010.csv, ...
//     QString filePath = QString("%1/layer_%2.csv")
//                            .arg(dirPath)
//                            .arg(layerIndex, 4, 10, QChar('0'));

//     QSaveFile file(filePath);
//     if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
//         throw std::runtime_error(("Cannot open file: " + filePath).toStdString());

//     QTextStream out(&file);
//     out.setRealNumberNotation(QTextStream::SmartNotation);
//     out.setRealNumberPrecision(16);

//     // Можно записать служебную информацию сверху
//     out << "layer;" << layerIndex << "\n";
//     out << "time;" << layerIndex * tau << "\n";
//     out << "h;" << h << "\n";
//     out << "tau;" << tau << "\n";
//     out << "i;x;v\n";

//     for (int i = 0; i <= n; ++i)
//     {
//         double x = i * h;
//         out << i << ";" << x << ";" << v[i] << "\n";
//     }

//     if (!file.commit())
//         throw std::runtime_error(("Cannot commit file: " + filePath).toStdString());
// }

// const QVector<LayerSnapshot2>& NonLinearHeat::GetSavedLayers() const
// {
//     return savedLayers;
// }

// void NonLinearHeat::SaveLayerToMemory(int layerIndex)
// {
//     LayerSnapshot2 snap;
//     snap.layerIndex = layerIndex;
//     snap.time = layerIndex * tau;

//     snap.v = v;

//     savedLayers.push_back(std::move(snap));
// }


#include "nonlinearheat.h"

#include <algorithm>
#include <cmath>
#include <limits>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace
{
double safePow(double value, double power)
{
    // Для задачи теплопроводности с горением обычно хотим работать с неотрицательными u.
    // Это также защищает от NaN, если alpha/beta нецелые, а из-за округления где-то вылез
    // маленький отрицательный хвост.
    return std::pow(std::max(0.0, value), power);
}
}

NonLinearHeat::NonLinearHeat(QObject* parent)
    : QObject(parent)
{

}

void NonLinearHeat::ResetTiming()
{
    tridiagonalNs = 0;
    tridiagonalCalls = 0;
}

double NonLinearHeat::GetTridiagonalMs() const
{
    return tridiagonalNs / 1'000'000.0;
}

int NonLinearHeat::GetTridiagonalCalls() const
{
    return tridiagonalCalls;
}

void NonLinearHeat::Run()
{
    try
    {
        Solver();

        emit timingReady(GetTridiagonalMs(),
                         GetTridiagonalCalls());

        emit solverFinished();
    }
    catch (const std::exception& e)
    {
        emit timingReady(GetTridiagonalMs(),
                         GetTridiagonalCalls());

        emit solverError(QString::fromUtf8(e.what()));
        emit solverFinished();
    }
    catch (...)
    {
        emit timingReady(GetTridiagonalMs(),
                         GetTridiagonalCalls());

        emit solverError("Unknown nonlinear heat solver error");
        emit solverFinished();
    }
}

void NonLinearHeat::SetParams(double _L, double _T,
                              int _n, int _m, int _j_shag,
                              double _k0, double _q0, double _alpha, double _beta,
                              double _A, double _x_c, double _r,
                              int _scheme, int _method, int _p)
{
    L = _L;
    T = _T;
    n = _n;
    m = _m;

    h = L / n;
    tau = T / m;

    j_shag = _j_shag;

    k0 = _k0;
    q0 = _q0;
    alpha = _alpha;
    beta = _beta;

    A = _A;
    x_c = _x_c;
    r = _r;

    scheme = _scheme;

    v.resize(n + 1);
    v_A.resize(n + 1);
    v_B.resize(n + 1);
    v_C.resize(n + 1);
    v_fi.resize(n + 1);

    this->method = _method;
    this->p = _p;
}

void NonLinearHeat::DefaultValues()
{
    v.fill(0.0, n + 1);

    // Локальный очаг.
    // Если r <= 0, оставляем всё нулями.
    if (r > 0.0)
    {
        for (int i = 0; i <= n; ++i)
        {
            double x = i * h;

            if (std::abs(x - x_c) <= r)
            {
                double s = (x - x_c) / r;
                v[i] = A * (1.0 - s * s);
            }
        }
    }

    // Нулевые граничные условия
    v[0] = 0.0;
    v[n] = 0.0;


    // // тест с точным решением
    // for (int i = 0; i <= n; ++i)
    // {
    //     double x = i * h;
    //     v[i] = std::sin(M_PI * x);
    // }
    // v[0] = 0.0;
    // v[n] = 0.0;

}

void NonLinearHeat::FillSemiLinearScheme()
{
    v_A.fill(0.0, n + 1);
    v_B.fill(0.0, n + 1);
    v_C.fill(0.0, n + 1);
    v_fi.fill(0.0, n + 1);

    // Границы: u(0,t)=0, u(L,t)=0
    v_B[0] = 1.0;
    v_fi[0] = 0.0;

    v_B[n] = 1.0;
    v_fi[n] = 0.0;

    for (int i = 1; i < n; ++i)
    {
        const double leftCoeff =
            0.5 * k0 * (safePow(v[i], alpha) + safePow(v[i - 1], alpha));

        const double rightCoeff =
            0.5 * k0 * (safePow(v[i + 1], alpha) + safePow(v[i], alpha));

        v_A[i] = -tau * leftCoeff / (h * h);
        v_B[i] = 1.0 + tau * (leftCoeff + rightCoeff) / (h * h);
        v_C[i] = -tau * rightCoeff / (h * h);

        v_fi[i] = v[i] + tau * q0 * safePow(v[i], beta);
    }
}

void NonLinearHeat::FillExplicitScheme()
{
    QVector<double> vNew(n + 1, 0.0);

    for (int i = 1; i < n; ++i)
    {
        const double leftCoeff =
            0.5 * k0 * (safePow(v[i], alpha) + safePow(v[i - 1], alpha));

        const double rightCoeff =
            0.5 * k0 * (safePow(v[i + 1], alpha) + safePow(v[i], alpha));

        const double diffusion =
            (rightCoeff * (v[i + 1] - v[i]) -
             leftCoeff  * (v[i] - v[i - 1])) / (h * h);

        const double source = q0 * safePow(v[i], beta);

        vNew[i] = v[i] + tau * diffusion + tau * source;
    }

    vNew[0] = 0.0;
    vNew[n] = 0.0;

    v = vNew;
}

void NonLinearHeat::Solver()
{
    ResetTiming();

    if (n < 2)
        throw std::runtime_error("n must be >= 2");

    if (m <= 0)
        throw std::runtime_error("m must be >= 1");

    const int saveStep = std::max(1, j_shag);

    DefaultValues();

    savedLayers.clear();
    SaveLayerToMemory(0);

    for (int j = 1; j <= m; ++j)
    {
        if (scheme == 1)
        {
            FillSemiLinearScheme();
            Progonka();
        }
        else
        {
            FillExplicitScheme();
        }

        double vmax = 0.0;

        for (double val : v)
        {
            if (!std::isfinite(val))
                throw std::runtime_error("Non-finite value detected in nonlinear heat solution");

            vmax = std::max(vmax, val);
        }

        if (vmax > 1e6)
        {
            SaveLayerToCsv(j);
            SaveLayerToMemory(j);
            throw std::runtime_error("Possible blow-up detected in nonlinear heat solution");
        }

        if (j % saveStep == 0 || j == m)
        {
            SaveLayerToCsv(j);
            SaveLayerToMemory(j);
        }
    }
}

void NonLinearHeat::Progonka()
{
    QVector<double> vNew = SolveTridiagonal(v_A, v_B, v_C, v_fi);
    v = vNew;
}

QVector<double> NonLinearHeat::SolveTridiagonal(const QVector<double>& A,
                                                const QVector<double>& B,
                                                const QVector<double>& C,
                                                const QVector<double>& Fi)
{
    QVector<double> y(n + 1, 0.0);

    // Решаем только по внутренним узлам 1..n-1
    if (n < 2)
        return y;

    QVector<double> alphaSweep(n + 1, 0.0);
    QVector<double> betaSweep(n + 1, 0.0);

    QElapsedTimer timer;
    timer.start();

    double denom = B[1];
    if (std::abs(denom) < 1e-14)
        throw std::runtime_error("Zero denominator in tridiagonal solver at i=1");

    alphaSweep[1] = -C[1] / denom;
    betaSweep[1] = Fi[1] / denom;

    for (int i = 2; i <= n - 1; ++i)
    {
        denom = B[i] + A[i] * alphaSweep[i - 1];

        if (std::abs(denom) < 1e-14)
            throw std::runtime_error("Zero denominator in tridiagonal solver");

        alphaSweep[i] = -C[i] / denom;
        betaSweep[i] = (Fi[i] - A[i] * betaSweep[i - 1]) / denom;
    }

    // Правая граница y[n] = 0
    y[n] = 0.0;
    y[n - 1] = alphaSweep[n - 1] * y[n] + betaSweep[n - 1];

    for (int i = n - 2; i >= 1; --i)
        y[i] = alphaSweep[i] * y[i + 1] + betaSweep[i];

    // Левая граница y[0] = 0
    y[0] = 0.0;

    tridiagonalNs += timer.nsecsElapsed();
    tridiagonalCalls++;

    return y;
}

void NonLinearHeat::SaveLayerToCsv(int layerIndex)
{
    QString dirPath = "resultsNonLinear";
    QDir dir;
    if (!dir.mkpath(dirPath))
        throw std::runtime_error("Cannot create results directory");

    QString filePath = QString("%1/layer_%2.csv")
                           .arg(dirPath)
                           .arg(layerIndex, 4, 10, QChar('0'));

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw std::runtime_error(("Cannot open file: " + filePath).toStdString());

    QTextStream out(&file);
    out.setRealNumberNotation(QTextStream::SmartNotation);
    out.setRealNumberPrecision(16);

    out << "layer;" << layerIndex << "\n";
    out << "time;" << layerIndex * tau << "\n";
    out << "h;" << h << "\n";
    out << "tau;" << tau << "\n";
    out << "i;x;v\n";

    for (int i = 0; i <= n; ++i)
    {
        double x = i * h;
        out << i << ";" << x << ";" << v[i] << "\n";
    }

    if (!file.commit())
        throw std::runtime_error(("Cannot commit file: " + filePath).toStdString());
}

const QVector<LayerSnapshot2>& NonLinearHeat::GetSavedLayers() const
{
    return savedLayers;
}

void NonLinearHeat::SaveLayerToMemory(int layerIndex)
{
    LayerSnapshot2 snap;
    snap.layerIndex = layerIndex;
    snap.time = layerIndex * tau;

    snap.L = L;
    snap.h = h;
    snap.tau = tau;

    snap.v = v;

    emit layerReady(snap);
}

QVector<double> NonLinearHeat::SolveTridiagonalClassic(const QVector<double>& A, const QVector<double>& B, const QVector<double>& C, const QVector<double>& F)
{
    int m = B.size();

    QVector<double> alpha(m, 0.0);
    QVector<double> beta(m, 0.0);
    QVector<double> y(m, 0.0);

    if (m == 0)
        return y;

    double denom = B[0];

    alpha[0] = -C[0] / denom;
    beta[0] = F[0] / denom;

    for (int i = 1; i < m; i++)
    {
        denom = B[i] + A[i] * alpha[i - 1];

        if (i < m - 1)
            alpha[i] = -C[i] / denom;

        beta[i] = (F[i] - A[i] * beta[i - 1]) / denom;
    }

    y[m - 1] = beta[m - 1];

    for (int i = m - 2; i >= 0; i--)
    {
        y[i] = alpha[i] * y[i + 1] + beta[i];
    }

    return y;
}

QVector<double> NonLinearHeat::BlockProgonka(QVector<double>& A, QVector<double>& B, QVector<double>& C, QVector<double>& F, int p_)
{

    int n = A.size();
    int p = p_;
    int R = n / p;
    int extraR = n % p; // Последний блок

    QVector<double> U(n);

    double* a = A.data();
    double* b = B.data();
    double* c = C.data();
    double* f = F.data();

#ifdef _OPENMP
    int threadCount = std::min(p, omp_get_max_threads());
#endif

#pragma omp parallel for schedule(static) num_threads(threadCount)
    for(int i = 0; i < p; i++)
    {
        int modifier = 0;
        if (i == p - 1)
        {
            modifier = extraR;
        }

        int blockL = i * R;
        int blockR = R * (i + 1) + modifier - 1;

        //Зануление под-диагонали
        for(int l = blockL; l < blockR; l++)
        {
            double z = a[l + 1] / b[l];

            b[l + 1] -= c[l] * z;
            f[l + 1] -= f[l] * z;

            a[l + 1] = -a[l] * z; //тут храним эти хвосты
        }


        //Зануление над-диагонали
        for(int r = blockR - 1; r > blockL; r--)
        {
            double z = c[r - 1] / b[r];

            f[r - 1] -= f[r] * z;

            c[r - 1] = -c[r] * z;
            a[r - 1] -= a[r] * z;
        }


    }

    // Последовательно занулим оставшиеся элементы вектора C
    for(int i = 0; i < p - 1; i++)
    {
        int ind = R * (i + 1);
        double z = c[ind - 1] / b[ind];
        b[ind - 1] -= a[ind] * z;
        f[ind - 1] -= f[ind] * z;

        c[ind - 1] = -c[ind] * z;
    }


    //Собираем трехдиагональную матрицу, ее размер равен числу вычислительных узлов p
    QVector<double> Anew(p);
    QVector<double> Bnew(p);
    QVector<double> Cnew(p);
    QVector<double> Fnew(p);

    for(int i = 0; i < p; i++)
    {
        int modifier = 0;

        if (i == p - 1)
        {
            modifier = extraR;
        }

        int ind = R * (i + 1) - 1 + modifier;
        Anew[i] = a[ind];
        Bnew[i] = b[ind];
        Cnew[i] = c[ind];
        Fnew[i] = f[ind];
    }
    Anew[0] = 0.0; Cnew[p - 1] = 0.0;

    QVector<double> Unew = SolveTridiagonalClassic(Anew, Bnew, Cnew, Fnew);

    double* u = U.data();
    const double* unew = Unew.constData();

//Второй параллельный участок
#pragma omp parallel for schedule(static) num_threads(threadCount)
    for(int i = 0; i < p; i++)
    {
        int modifier = 0;
        double UnewRight = Unew[i];
        double UnewLeft = 0.0;

        if (i == p - 1)
        {
            modifier = extraR;
        }

        if (i > 0)
        {
            UnewLeft = Unew[i - 1];
        }

        int blockL = i * R;
        int blockR = R * (i + 1) + modifier - 1;

        for (int l = blockL; l < blockR; l++)
        {
            u[l] = (f[l] - a[l] * UnewLeft - c[l] * UnewRight) / b[l];

        }

        u[blockR] = UnewRight;

    }



    return U;
}
