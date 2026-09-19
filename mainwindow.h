#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "meinhardtsolver.h"
#include "nonlinearheat.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_2_valueChanged(int value);

    void on_pushButton_2_clicked();

    void onMeinhardtLayerReady(const LayerSnapshot& snap);
    void onMeinhardtFinished();
    void onMeinhardtError(const QString& message);
    void onMeinhardtTimingReady(double sequentialMs, int sequentialCalls, double blockMs, int blockCalls);

    void onNonLinearLayerReady(const LayerSnapshot2& snap);
    void onNonLinearFinished();
    void onNonLinearError(const QString& message);
    void onNonLinearTimingReady(double tridiagonalMs, int tridiagonalCalls);

    void on_pushButton_3_clicked();

private:
    void DrawLayer(const LayerSnapshot& snap);
    void DrawNonLinearLayersUpTo(int lastIndex);

private:
    Ui::MainWindow *ui;
    MeinhardtSolver meinhardt;
    NonLinearHeat nonlinear;

    QThread* meinhardtThread = nullptr;
    QVector<LayerSnapshot> meinhardtLayers;

    QThread* nonlinearThread = nullptr;
    QVector<LayerSnapshot2> nonlinearLayers;

    void ClearCsvResults();
    void ClearCsvResults(const QString& dirPath);

};
#endif // MAINWINDOW_H
