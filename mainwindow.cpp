#include <QMessageBox>
#include <QMetaType>
#include <QDir>
#include <QFileInfoList>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qcustomplot.h"

#ifdef _OPENMP
#include <omp.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Численное решение нелинейных параболических задач");

    qRegisterMetaType<LayerSnapshot>("LayerSnapshot");
    qRegisterMetaType<LayerSnapshot2>("LayerSnapshot2");

    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(0);
    ui->horizontalSlider->setValue(0);

    ui->widget->addGraph();
    ui->widget->xAxis->setLabel("x");
    ui->widget->yAxis->setLabel("y");
    ui->widget->xAxis->setRange(0, 1);
    ui->widget->rescaleAxes();
    ui->widget->replot();

    ui->widget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);


    ui->horizontalSlider_2->setMinimum(0);
    ui->horizontalSlider_2->setMaximum(0);
    ui->horizontalSlider_2->setValue(0);

    ui->widget_2->addGraph();
    ui->widget_2->xAxis->setLabel("x");
    ui->widget_2->yAxis->setLabel("y");
    ui->widget_2->xAxis->setRange(0, 1);
    ui->widget_2->rescaleAxes();
    ui->widget_2->replot();

    ui->widget_2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

#ifdef _OPENMP
    int maxThreads = omp_get_max_threads();
#else
    int maxThreads = 1;
#endif

    ui->spinBox->setMinimum(1);
    ui->spinBox->setMaximum(maxThreads);
    ui->spinBox->setValue(1);

    ui->spinBox_2->setMinimum(1);
    ui->spinBox_2->setMaximum(maxThreads);
    ui->spinBox_2->setValue(1);

}

MainWindow::~MainWindow()
{
    if (meinhardtThread)
    {
        meinhardtThread->quit();
        meinhardtThread->wait();
    }

    if (nonlinearThread)
    {
        nonlinearThread->quit();
        nonlinearThread->wait();
    }

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(meinhardtThread)
    {
        qDebug() << "Meinhardt solver is already running";
        return;
    }

    ClearCsvResults("results");

    meinhardtLayers.clear();

    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(0);
    ui->horizontalSlider->setValue(0);

    ui->pushButton->setEnabled(false);

    MeinhardtSolver* solver = new MeinhardtSolver();

    solver->SetParams(ui->lineEdit_14->text().toDouble(), ui->lineEdit_15->text().toDouble(),
ui->lineEdit_16->text().toInt(), ui->lineEdit_17->text().toInt(),
ui->lineEdit_18->text().toInt(),
ui->lineEdit->text().toDouble(), ui->lineEdit_2->text().toDouble(), ui->lineEdit_3->text().toDouble(),
ui->lineEdit_4->text().toDouble(), ui->lineEdit_5->text().toDouble(), ui->lineEdit_6->text().toDouble(),
ui->lineEdit_7->text().toDouble(), ui->lineEdit_8->text().toDouble(),
ui->lineEdit_9->text().toDouble(), ui->lineEdit_10->text().toDouble(), ui->comboBox->currentIndex(), ui->comboBox_2->currentIndex(), ui->spinBox->value());

    meinhardtThread = new QThread(this);

    solver->moveToThread(meinhardtThread);

    connect(meinhardtThread, &QThread::started, solver, &MeinhardtSolver::Run);

    connect(solver, &MeinhardtSolver::layerReady, this, &MainWindow::onMeinhardtLayerReady);

    connect(solver, &MeinhardtSolver::solverError, this, &MainWindow::onMeinhardtError);

    connect(solver, &MeinhardtSolver::solverFinished, meinhardtThread, &QThread::quit);

    connect(solver, &MeinhardtSolver::solverFinished, solver, &QObject::deleteLater);

    connect(meinhardtThread, &QThread::finished, meinhardtThread, &QObject::deleteLater);

    connect(meinhardtThread, &QThread::finished, this, &MainWindow::onMeinhardtFinished);

    connect(solver, &MeinhardtSolver::timingReady, this, &MainWindow::onMeinhardtTimingReady);

    meinhardtThread->start();
}

void MainWindow::onMeinhardtLayerReady(const LayerSnapshot& snap)
{
    meinhardtLayers.push_back(snap);

    int index = static_cast<int>(meinhardtLayers.size()) - 1;

    ui->horizontalSlider->blockSignals(true);
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(index);
    ui->horizontalSlider->setValue(index);
    ui->horizontalSlider->blockSignals(false);

    DrawLayer(snap);
}

void MainWindow::onMeinhardtFinished()
{
    meinhardtThread = nullptr;
    ui->pushButton->setEnabled(true);

    qDebug() << "Meinhardt solver finished. Layers = " << meinhardtLayers.size();
}

void MainWindow::onMeinhardtError(const QString& message)
{
    QMessageBox::critical(this, "Ошибка рассчета", message);
}

void MainWindow::onMeinhardtTimingReady(double sequentialMs, int sequentialCalls, double blockMs, int blockCalls)
{
    qDebug() << "Sequential progonka:"
             << sequentialMs << "ms,"
             << "calls:" << sequentialCalls;

    qDebug() << "Block progonka:"
             << blockMs << "ms,"
             << "calls:" << blockCalls;

    double avgSeq = 0.0;
    double avgBlock = 0.0;

    if (sequentialCalls > 0)
        avgSeq = sequentialMs / sequentialCalls;

    if (blockCalls > 0)
        avgBlock = blockMs / blockCalls;

    qDebug() << "Avg sequential:" << avgSeq << "ms";
    qDebug() << "Avg block:" << avgBlock << "ms";

    ui->label_31->setText(
        QString("Обычная прогонка: %1 мс, вызовов: %2, среднее: %3 мс\n"
                "Блочная прогонка: %4 мс, вызовов: %5, среднее: %6 мс")
            .arg(sequentialMs, 0, 'f', 3)
            .arg(sequentialCalls)
            .arg(avgSeq, 0, 'f', 6)
            .arg(blockMs, 0, 'f', 3)
            .arg(blockCalls)
            .arg(avgBlock, 0, 'f', 6)
        );
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if (value < 0 || value >= static_cast<int>(meinhardtLayers.size()))
        return;

    DrawLayer(meinhardtLayers[value]);
}







void MainWindow::DrawLayer(const LayerSnapshot& snap)
{
    int N = static_cast<int>(snap.a.size());

    QVector<double> x(N), yA(N), yB(N), yC(N);

    double xLeft = 0.0;
    double h = 1.0 / (N - 1);   // если у тебя отрезок [0,1]

    for (int i = 0; i < N; ++i)
    {
        x[i] = xLeft + i * h;
        yA[i] = snap.a[i];
        yB[i] = snap.b[i];
        yC[i] = snap.c[i];
    }

    ui->widget->clearGraphs();

    ui->widget->addGraph();
    ui->widget->graph(0)->setName("a");
    ui->widget->graph(0)->setData(x, yA);
    ui->widget->graph(0)->setPen(QPen(QColor(220, 50, 47), 2));   // красный

    ui->widget->addGraph();
    ui->widget->graph(1)->setName("b");
    ui->widget->graph(1)->setData(x, yB);
    ui->widget->graph(1)->setPen(QPen(QColor(38, 139, 210), 2));  // синий

    ui->widget->addGraph();
    ui->widget->graph(2)->setName("c");
    ui->widget->graph(2)->setData(x, yC);
    ui->widget->graph(2)->setPen(QPen(QColor(133, 153, 0), 2));   // зелёный

    ui->widget->xAxis->setLabel("x");
    ui->widget->yAxis->setLabel("y");

    ui->widget->legend->setVisible(true);
    //ui->widget->rescaleAxes();

    ui->widget->xAxis->setRange(0, 1);
    ui->widget->yAxis->setRange(0, 2);

    ui->widget->replot();
}

void MainWindow::on_horizontalSlider_2_valueChanged(int value)
{
    if (value < 0 || value >= static_cast<int>(nonlinearLayers.size()))
        return;

    DrawNonLinearLayersUpTo(value);
}


void MainWindow::on_pushButton_2_clicked()
{
    if (nonlinearThread)
    {
        qDebug() << "Nonlinear heat solver is already running";
        return;
    }

    ClearCsvResults("resultsNonLinear");

    nonlinearLayers.clear();

    ui->widget_2->clearGraphs();
    ui->widget_2->legend->setVisible(true);
    ui->widget_2->replot();

    ui->horizontalSlider_2->setMinimum(0);
    ui->horizontalSlider_2->setMaximum(0);
    ui->horizontalSlider_2->setValue(0);

    ui->pushButton_2->setEnabled(false);

    NonLinearHeat* solver = new NonLinearHeat();

    solver->SetParams(
        ui->lineEdit_19->text().toDouble(),
        ui->lineEdit_20->text().toDouble(),
        ui->lineEdit_26->text().toInt(),
        ui->lineEdit_25->text().toInt(),
        ui->lineEdit_27->text().toInt(),
        ui->lineEdit_22->text().toDouble(),
        ui->lineEdit_23->text().toDouble(),
        ui->lineEdit_30->text().toDouble(),
        ui->lineEdit_21->text().toDouble(),
        ui->lineEdit_24->text().toDouble(),
        ui->lineEdit_29->text().toDouble(),
        ui->lineEdit_28->text().toDouble(),
        ui->comboBox_3->currentIndex(), ui->comboBox_4->currentIndex(), ui->spinBox_2->value());

    nonlinearThread = new QThread(this);

    solver->moveToThread(nonlinearThread);

    connect(nonlinearThread, &QThread::started,
            solver, &NonLinearHeat::Run);

    connect(solver, &NonLinearHeat::layerReady,
            this, &MainWindow::onNonLinearLayerReady);

    connect(solver, &NonLinearHeat::solverError,
            this, &MainWindow::onNonLinearError);

    connect(solver, &NonLinearHeat::timingReady,
            this, &MainWindow::onNonLinearTimingReady);

    connect(solver, &NonLinearHeat::solverFinished,
            nonlinearThread, &QThread::quit);

    connect(solver, &NonLinearHeat::solverFinished,
            solver, &QObject::deleteLater);

    connect(nonlinearThread, &QThread::finished,
            nonlinearThread, &QObject::deleteLater);

    connect(nonlinearThread, &QThread::finished,
            this, &MainWindow::onNonLinearFinished);

    nonlinearThread->start();
}

void MainWindow::DrawNonLinearLayersUpTo(int lastIndex)
{
    if (lastIndex < 0 || lastIndex >= static_cast<int>(nonlinearLayers.size()))
        return;

    ui->widget_2->clearGraphs();

    double L = nonlinearLayers[lastIndex].L;
    if (L <= 0.0)
        L = 1.0;

    for (int layerNumber = 0; layerNumber <= lastIndex; ++layerNumber)
    {
        const LayerSnapshot2& snap = nonlinearLayers[layerNumber];

        int N = static_cast<int>(snap.v.size());

        if (N < 2)
            continue;

        QVector<double> x(N), yV(N);

        double h = snap.L / (N - 1);

        if (snap.L <= 0.0)
            h = L / (N - 1);

        for (int i = 0; i < N; ++i)
        {
            x[i] = i * h;
            yV[i] = snap.v[i];
        }

        ui->widget_2->addGraph();

        int graphIndex = ui->widget_2->graphCount() - 1;

        ui->widget_2->graph(graphIndex)->setData(x, yV);

        if (layerNumber == 0)
        {
            ui->widget_2->graph(graphIndex)->setName("Начальный слой");
            ui->widget_2->graph(graphIndex)->setPen(QPen(QColor(38, 139, 210), 3)); // синий
        }
        else if (layerNumber == lastIndex)
        {
            ui->widget_2->graph(graphIndex)->setName(
                QString("Текущий слой, t = %1").arg(snap.time, 0, 'f', 4)
                );
            ui->widget_2->graph(graphIndex)->setPen(QPen(QColor(220, 50, 47), 2)); // красный
        }
        else
        {
            ui->widget_2->graph(graphIndex)->setName("");
            ui->widget_2->graph(graphIndex)->setPen(QPen(QColor(120, 120, 120, 80), 1)); // серый полупрозрачный
        }
    }

    ui->widget_2->xAxis->setLabel("x");
    ui->widget_2->yAxis->setLabel("v");

    ui->widget_2->legend->setVisible(false);

    ui->widget_2->xAxis->setRange(0, L);

    double yMax = 0.0;

    for (int layerNumber = 0; layerNumber <= lastIndex; ++layerNumber)
    {
        const LayerSnapshot2& snap = nonlinearLayers[layerNumber];

        for (double value : snap.v)
        {
            if (value > yMax)
                yMax = value;
        }
    }

    double yTop = 1.0;

    if (yMax > 0.0)
        yTop = yMax * 1.15;

    ui->widget_2->yAxis->setRange(0, yTop);

    ui->widget_2->replot();
}

void MainWindow::ClearCsvResults(const QString& dirPath)
{
    QDir dir(dirPath);

    if (!dir.exists())
        return;

    QStringList filters;
    filters << "layer_*.csv";

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : files)
    {
        QFile::remove(fileInfo.absoluteFilePath());
    }
}

void MainWindow::onNonLinearLayerReady(const LayerSnapshot2& snap)
{
    nonlinearLayers.push_back(snap);

    int index = static_cast<int>(nonlinearLayers.size()) - 1;

    ui->horizontalSlider_2->blockSignals(true);
    ui->horizontalSlider_2->setMinimum(0);
    ui->horizontalSlider_2->setMaximum(index);
    ui->horizontalSlider_2->setValue(index);
    ui->horizontalSlider_2->blockSignals(false);

    DrawNonLinearLayersUpTo(index);
}

void MainWindow::onNonLinearFinished()
{
    nonlinearThread = nullptr;
    ui->pushButton_2->setEnabled(true);

    qDebug() << "Nonlinear heat solver finished. Layers ="
             << nonlinearLayers.size();
}

void MainWindow::onNonLinearError(const QString& message)
{
    QMessageBox::critical(this, "Ошибка расчёта нелинейной задачи", message);
}

void MainWindow::onNonLinearTimingReady(double tridiagonalMs,
                                        int tridiagonalCalls)
{
    double avg = 0.0;

    if (tridiagonalCalls > 0)
        avg = tridiagonalMs / tridiagonalCalls;

    qDebug() << "Nonlinear heat tridiagonal:"
             << tridiagonalMs << "ms,"
             << "calls:" << tridiagonalCalls
             << "avg:" << avg << "ms";

    ui->label_32->setText(
        QString("Прогонка: %1 мс, вызовов: %2, среднее: %3 мс")
            .arg(tridiagonalMs, 0, 'f', 3)
            .arg(tridiagonalCalls)
            .arg(avg, 0, 'f', 6)
    );

}

void MainWindow::on_pushButton_3_clicked()
{
    // Размер сетки
    int n = 100;

    // Если хочешь брать n из поля, можно так:
    // int n = ui->lineEdit_nHeatTest->text().toInt();
    // if (n < 2) n = 100;

    const double L = 1.0;
    const double h = L / n;

    const double muLeft = 10.0;
    const double muRight = 100.0;

    // Сетка и решения
    QVector<double> x(n + 1);
    QVector<double> numerical(n + 1, 0.0);
    QVector<double> exact(n + 1, 0.0);

    numerical[0] = muLeft;
    numerical[n] = muRight;

    // Внутренних неизвестных: v1, v2, ..., v_{n-1}
    int innerCount = n - 1;

    QVector<double> A(innerCount, 0.0); // поддиагональ
    QVector<double> B(innerCount, 0.0); // главная диагональ
    QVector<double> C(innerCount, 0.0); // наддиагональ
    QVector<double> F(innerCount, 0.0); // правая часть

    /*
        Исходная схема:

        12 * (v_{i-1} - 2v_i + v_{i+1}) / h^2 - 5v_i
        =
        2110 - 450x_i^2

        Умножаем на -1, чтобы главная диагональ была положительной:

        -12/h^2 * v_{i-1}
        + (24/h^2 + 5) * v_i
        -12/h^2 * v_{i+1}
        =
        -2110 + 450x_i^2
    */

    for (int i = 1; i <= n - 1; ++i)
    {
        int k = i - 1;
        double xi = i * h;

        A[k] = -12.0 / (h * h);
        B[k] =  24.0 / (h * h) + 5.0;
        C[k] = -12.0 / (h * h);

        F[k] = -2110.0 + 450.0 * xi * xi;
    }

    // Учитываем граничное условие v0 = 10
    F[0] -= A[0] * muLeft;
    A[0] = 0.0;

    // Учитываем граничное условие vn = 100
    F[innerCount - 1] -= C[innerCount - 1] * muRight;
    C[innerCount - 1] = 0.0;

    // Метод прогонки для системы:
    // A[i] * y[i - 1] + B[i] * y[i] + C[i] * y[i + 1] = F[i]

    QVector<double> alpha(innerCount, 0.0);
    QVector<double> beta(innerCount, 0.0);
    QVector<double> y(innerCount, 0.0);

    double denom = B[0];

    alpha[0] = -C[0] / denom;
    beta[0] = F[0] / denom;

    for (int i = 1; i < innerCount; ++i)
    {
        denom = B[i] + A[i] * alpha[i - 1];

        if (i < innerCount - 1)
            alpha[i] = -C[i] / denom;

        beta[i] = (F[i] - A[i] * beta[i - 1]) / denom;
    }

    y[innerCount - 1] = beta[innerCount - 1];

    for (int i = innerCount - 2; i >= 0; --i)
    {
        y[i] = alpha[i] * y[i + 1] + beta[i];
    }

    // Переносим внутренние значения в общий вектор numerical
    for (int i = 1; i <= n - 1; ++i)
    {
        numerical[i] = y[i - 1];
    }

    // Точное решение и ошибка
    double maxError = 0.0;

    for (int i = 0; i <= n; ++i)
    {
        x[i] = i * h;
        exact[i] = 10.0 + 90.0 * x[i] * x[i];

        double error = std::abs(numerical[i] - exact[i]);

        if (error > maxError)
            maxError = error;
    }

    qDebug() << "Max error =" << maxError;

    // Рисуем графики
    ui->widget_3->clearGraphs();

    ui->widget_3->addGraph();
    ui->widget_3->graph(0)->setName("Численное решение");
    ui->widget_3->graph(0)->setData(x, numerical);
    ui->widget_3->graph(0)->setPen(QPen(Qt::red, 2));

    ui->widget_3->addGraph();
    ui->widget_3->graph(1)->setName("Точное решение");
    ui->widget_3->graph(1)->setData(x, exact);
    ui->widget_3->graph(1)->setPen(QPen(Qt::blue, 2));

    ui->widget_3->xAxis->setLabel("x");
    ui->widget_3->yAxis->setLabel("u(x)");

    ui->widget_3->legend->setVisible(true);

    ui->widget_3->rescaleAxes();
    ui->widget_3->replot();

    // Если есть QLabel, можно вывести ошибку:
    // ui->labelHeatTestError->setText(
    //     QString("Максимальная ошибка: %1").arg(maxError, 0, 'e', 3)
    // );
}
