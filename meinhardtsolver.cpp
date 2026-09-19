#include "meinhardtsolver.h"

#ifdef _OPENMP
#include <omp.h>
#endif

MeinhardtSolver::MeinhardtSolver(QObject* parent) : QObject(parent)
{

}

void MeinhardtSolver::Run()
{
    try
    {
        Solver();

        emit timingReady(GetSequentialProgonkaMs(), GetSequentialProgonkaCalls(), GetBlockProgonkaMs(), GetBlockProgonkaCalls());

        emit solverFinished();
    }
    catch (const std::exception& e)
    {
        emit timingReady(GetSequentialProgonkaMs(), GetSequentialProgonkaCalls(), GetBlockProgonkaMs(), GetBlockProgonkaCalls());

        emit solverError(QString::fromUtf8(e.what()));
        emit solverFinished();
    }
    catch(...)
    {
        emit timingReady(GetSequentialProgonkaMs(), GetSequentialProgonkaCalls(), GetBlockProgonkaMs(), GetBlockProgonkaCalls());

        emit solverError("Unknown solver error");
        emit solverFinished();
    }
}

void MeinhardtSolver::SetParams(double _L, double _T,
                           int _n, int _m, int _j_shag,
                           double _Da, double _Db, double _Dc,
                           double _ro_a, double _ro_b, double _ro_c,
                           double _Ka, double _Kb,
                           double _sigma_a, double _sigma_b,
                           int _scheme, int _method, int _p)
{
    this->L = _L;
    this->T = _T;
    this->n = _n;
    this->m = _m;

    this->h = _L / _n;
    this->tau = _T / _m;

    this->j_shag = _j_shag;

    this->Da = _Da;
    this->Db = _Db;
    this->Dc = _Dc;

    this->ro_a = _ro_a;
    this->ro_b = _ro_b;
    this->ro_c = _ro_c;

    this->Ka = _Ka;
    this->Kb = _Kb;

    this->sigma_a = _sigma_a;
    this->sigma_b = _sigma_b;

    this->scheme = _scheme;

    a.resize(n + 1);
    b.resize(n + 1);
    c.resize(n + 1);

    this->method = _method;
    this->p = _p;
}

void MeinhardtSolver::DefaultValues()
{
    QFile file("initial_values.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Не удалось открыть initial_values.txt");

    QTextStream in(&file);

    // coefficients for a
    in >> A0 >> A1 >> A2 >> A3 >> k1 >> k2 >> k3;

    // coefficients for b
    in >> B0 >> B1 >> B2 >> B3 >> k1 >> k2 >> k3;

    // coefficients for c
    in >> C0 >> C1 >> C2 >> C3 >> k1 >> k2 >> k3;

    file.close();

    a.resize(n + 1);
    b.resize(n + 1);
    c.resize(n + 1);

    for (int i = 0; i <= n; ++i)
    {
        double x = i * h;

        a[i] = A0
               + A1 * std::cos(k1 * M_PI * x)
               + A2 * std::cos(k2 * M_PI * x)
               + A3 * std::cos(k3 * M_PI * x);

        b[i] = B0
               + B1 * std::cos(k1 * M_PI * x)
               + B2 * std::cos(k2 * M_PI * x)
               + B3 * std::cos(k3 * M_PI * x);

        c[i] = C0
               + C1 * std::cos(k1 * M_PI * x)
               + C2 * std::cos(k2 * M_PI * x)
               + C3 * std::cos(k3 * M_PI * x);
    }
}

void MeinhardtSolver::FillSemiLinearScheme()
{
    // for a
    a_A.resize(n + 1);
    a_B.resize(n + 1);
    a_C.resize(n + 1);
    a_fi.resize(n + 1);

    a_A[0] = 0.;
    a_C[0] = 1.;
    a_B[0] = 1.;
    a_fi[0] = 0.;
    for (int i = 1; i < n; ++i)
    {
        a_A[i] = Da * tau / (h * h);
        a_B[i] = a_A[i];
        a_C[i] = 1. + 2. * Da * tau / (h * h);

        a_fi[i] = tau * ro_a * (c[i] / (1. + Ka * b[i] * b[i]) - a[i]) + tau * sigma_a + a[i];
    }
    a_A[n] = 1.;
    a_C[n] = 1.;
    a_B[n] = 0.;
    a_fi[n] = 0.;


    // for b
    b_A.resize(n + 1);
    b_B.resize(n + 1);
    b_C.resize(n + 1);
    b_fi.resize(n + 1);

    b_A[0] = 0.;
    b_C[0] = 1.;
    b_B[0] = 1.;
    b_fi[0] = 0;
    for (int i = 1; i < n; ++i)
    {
        b_A[i] = Db * tau / (h * h);
        b_B[i] = b_A[i];
        b_C[i] = 1. + 2. * Db * tau / (h * h);

        b_fi[i] = tau * ro_b * (1. / (1. + Kb * a[i] * a[i] * c[i]) - b[i]) + tau * sigma_b + b[i];
    }
    b_A[n] = 1.;
    b_C[n] = 1.;
    b_B[n] = 0.;
    b_fi[n] = 0;


    //// for c
    c_A.resize(n + 1);
    c_B.resize(n + 1);
    c_C.resize(n + 1);
    c_fi.resize(n + 1);

    c_A[0] = 0.;
    c_C[0] = 1.;
    c_B[0] = 1.;
    c_fi[0] = 0.;
    for (int i = 1; i < n; ++i)
    {
        c_A[i] = Dc * tau / (h * h);
        c_B[i] = c_A[i];
        c_C[i] = 1. + 2. * Dc * tau / (h * h);

        c_fi[i] = tau * ro_c * (b[i] - a[i] * c[i]) + c[i];
    }
    c_A[n] = 1.;
    c_C[n] = 1.;
    c_B[n] = 0.;
    c_fi[n] = 0.;
}

void MeinhardtSolver::FillExplicitScheme()
{
    QVector<double> aNew(n + 1);
    QVector<double> bNew(n + 1);
    QVector<double> cNew(n + 1);


    for (int i = 1; i < n; ++i)
    {
        aNew[i] = a[i] + tau * Da * (a[i-1] - 2 * a[i] + a[i+1]) / (h * h) + tau * ro_a * (c[i] / (1 + Ka * b[i] * b[i]) - a[i]) + tau * sigma_a;
        bNew[i] = b[i] + tau * Db * (b[i-1] - 2 * b[i] + b[i+1]) / (h * h) + tau * ro_b * (1. / (1 + Ka * a[i] * a[i] * c[i]) - b[i]) + tau * sigma_b;
        cNew[i] = c[i] + tau * Dc * (c[i-1] - 2 * c[i] + c[i+1]) / (h * h) + tau * ro_c * (b[i] - a[i] * c[i]);


    }
    aNew[0] = aNew[1];
    aNew[n] = aNew[n - 1];

    bNew[0] = bNew[1];
    bNew[n] = bNew[n - 1];

    cNew[0] = cNew[1];
    cNew[n] = cNew[n - 1];

    a = aNew;
    b = bNew;
    c = cNew;
}

void MeinhardtSolver::Solver()
{
    ResetTiming();

    qDebug() << "scheme =" << scheme;
    qDebug() << "m =" << m;
    qDebug() << "j_shag =" << j_shag;

    DefaultValues();

    savedLayers.clear();
    SaveLayerToMemory(0);

    if (scheme == 1)
    {
        for(int j = 1; j <= m; ++j)
        {
            this->FillSemiLinearScheme();
            this->Progonka();

            if (j % j_shag == 0)
            {
                SaveLayerToCsv(j);
                SaveLayerToMemory(j);
            }
        }
        if ((m - 1) % j_shag != 0)
        {
            SaveLayerToCsv(m - 1);
            SaveLayerToMemory(m - 1);
        }
    } else if (scheme == 0)
    {
        for(int j = 1; j <= m; ++j)
        {
            this->FillExplicitScheme();

            if (j % j_shag == 0)
            {
                SaveLayerToCsv(j);
                SaveLayerToMemory(j);
            }
        }
        if ((m - 1) % j_shag != 0)
        {
            SaveLayerToCsv(m - 1);
            SaveLayerToMemory(m - 1);
        }
    }
}

void MeinhardtSolver::Progonka()
{
    if (method == 0)
    {
        QVector<double> aNew = SolveTridiagonal(a_A, a_B, a_C, a_fi);
        QVector<double> bNew = SolveTridiagonal(b_A, b_B, b_C, b_fi);
        QVector<double> cNew = SolveTridiagonal(c_A, c_B, c_C, c_fi);

        a = aNew;
        b = bNew;
        c = cNew;
    }
    else if (method == 1)
    {
        auto solveBlock = [this](const QVector<double>& left,
                                 const QVector<double>& right,
                                 const QVector<double>& diag,
                                 const QVector<double>& rhs)
        {
            int size = diag.size();

            QVector<double> lower(size);
            QVector<double> mainDiag(size);
            QVector<double> upper(size);
            QVector<double> f = rhs;

            for (int i = 0; i < size; ++i)
            {
                lower[i] = -left[i];
                mainDiag[i] = diag[i];
                upper[i] = -right[i];
            }

            return BlockProgonka(lower, mainDiag, upper, f, p);
        };

        QVector<double> aNew = solveBlock(a_A, a_B, a_C, a_fi);
        QVector<double> bNew = solveBlock(b_A, b_B, b_C, b_fi);
        QVector<double> cNew = solveBlock(c_A, c_B, c_C, c_fi);

        a = aNew;
        b = bNew;
        c = cNew;
    }

}

QVector<double> MeinhardtSolver::SolveTridiagonal(const QVector<double>& A, const QVector<double>& B, const QVector<double>& C, const QVector<double>& Fi)
{    
    QVector<double> alpha(n + 1);
    QVector<double> beta(n + 1);
    QVector<double> y(n + 1);

    QElapsedTimer timer;
    timer.start();

    alpha[1] = 1.;
    beta[1] = 0.;

    //Прямой ход
    for (int i = 1; i < n; i++)
    {
        double znam = (C[i] - A[i] * alpha[i]);
        alpha[i + 1] = B[i] / znam;
        beta[i + 1] = (Fi[i] + A[i] * beta[i]) / znam;
    }

    //Обратный ход
    y[n] = (-1. * beta[n]) / (alpha[n] - 1.);

    for (int i = n - 1; i >= 0; i--)
    {
        y[i] = alpha[i + 1] * y[i + 1] + beta[i + 1];
    }

    sequentialProgonkaNs += timer.nsecsElapsed();
    sequentialProgonkaCalls++;

    return y;
}

void MeinhardtSolver::SaveLayerToCsv(int layerIndex)
{
    // Папка results рядом с программой / рабочей директорией
    QString dirPath = "results";
    QDir dir;
    if (!dir.mkpath(dirPath))
        throw std::runtime_error("Cannot create results directory");

    // Имя файла: layer_0000.csv, layer_0010.csv, ...
    QString filePath = QString("%1/layer_%2.csv")
                           .arg(dirPath)
                           .arg(layerIndex, 4, 10, QChar('0'));

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw std::runtime_error(("Cannot open file: " + filePath).toStdString());

    QTextStream out(&file);
    out.setRealNumberNotation(QTextStream::SmartNotation);
    out.setRealNumberPrecision(16);

    // Можно записать служебную информацию сверху
    out << "layer;" << layerIndex << "\n";
    out << "time;" << layerIndex * tau << "\n";
    out << "h;" << h << "\n";
    out << "tau;" << tau << "\n";
    out << "i;x;a;b;c\n";

    for (int i = 0; i <= n; ++i)
    {
        double x = i * h;
        out << i << ";" << x << ";" << a[i] << ";" << b[i] << ";" << c[i] << "\n";
    }

    if (!file.commit())
        throw std::runtime_error(("Cannot commit file: " + filePath).toStdString());
}

const QVector<LayerSnapshot>& MeinhardtSolver::GetSavedLayers() const
{
    return savedLayers;
}

void MeinhardtSolver::SaveLayerToMemory(int layerIndex)
{
    LayerSnapshot snap;
    snap.layerIndex = layerIndex;
    snap.time = layerIndex * tau;

    snap.a = a;
    snap.b = b;
    snap.c = c;

    savedLayers.push_back(snap);

    emit layerReady(snap);
}

QVector<double> MeinhardtSolver::SolveTridiagonalClassic(
    const QVector<double>& A,
    const QVector<double>& B,
    const QVector<double>& C,
    const QVector<double>& F
    )
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

QVector<double> MeinhardtSolver::BlockProgonka(QVector<double>& A, QVector<double>& B, QVector<double>& C, QVector<double>& F, int p_)
{
    QElapsedTimer timer;
    timer.start();

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

    blockProgonkaNs += timer.nsecsElapsed();
    blockProgonkaCalls++;

    return U;
}

void MeinhardtSolver::ResetTiming()
{
    sequentialProgonkaNs = 0;
    blockProgonkaNs = 0;

    sequentialProgonkaCalls = 0;
    blockProgonkaCalls = 0;
}

double MeinhardtSolver::GetSequentialProgonkaMs() const
{
    return sequentialProgonkaNs / 1'000'000.0;
}

double MeinhardtSolver::GetBlockProgonkaMs() const
{
    return blockProgonkaNs / 1'000'000.0;
}

int MeinhardtSolver::GetSequentialProgonkaCalls() const
{
    return sequentialProgonkaCalls;
}

int MeinhardtSolver::GetBlockProgonkaCalls() const
{
    return blockProgonkaCalls;
}
