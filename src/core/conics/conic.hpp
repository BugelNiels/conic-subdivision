#pragma once

#include <QMatrix4x4>
#include <QVector2D>
#include <QVector>

class Settings;

class Conic {
public:
    explicit Conic(const Settings &settings);

    Conic(const QVector<QVector2D> &coords, const QVector<QVector2D> &normals,
          const Settings &settings);

    bool sample(const QVector2D &origin, const QVector2D &direction,
                QVector2D &point, QVector2D &normal) const;

    bool intersects(const QVector2D &ro, const QVector2D &rd, double &t) const;

    inline bool isValid() const { return hasSolution_; }

    inline QMatrix4x4 getCoefficients() const { return Q_; }

    Conic average(const Conic &other) const;

    Conic operator+(const Conic &other) const;

    void operator+=(const Conic &other);

    float getStability() const;

private:
    float stability_ = 0;
    QMatrix4x4 Q_;
    bool hasSolution_;
    const Settings &settings_;

    bool fitConic(const QVector<QVector2D> &coords,
                  const QVector<QVector2D> &normals, const Settings &settings);

    QVector2D conicNormal(const QVector2D &p, const QVector2D &rd) const;

};
