#ifndef LAYERSNAPSHOT_H
#define LAYERSNAPSHOT_H

#include <QVector>
#include <QMetaType>

struct LayerSnapshot
{
    int layerIndex = 0;
    double time = 0.0;

    QVector<double> a;
    QVector<double> b;
    QVector<double> c;
};

struct LayerSnapshot2
{
    int layerIndex = 0;
    double time = 0.0;

    double L = 0.0;
    double h = 0.0;
    double tau = 0.0;

    QVector<double> v;
};

Q_DECLARE_METATYPE(LayerSnapshot)
Q_DECLARE_METATYPE(LayerSnapshot2)

#endif // LAYERSNAPSHOT_H
