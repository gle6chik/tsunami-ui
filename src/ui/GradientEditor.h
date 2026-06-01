#pragma once

#include <QWidget>
#include <QColor>
#include <QLinearGradient>
#include <vector>

class QDoubleSpinBox;
class QComboBox;

struct GradientStop {
    double position; // 0.0 .. 1.0
    QColor color;
};

class GradientEditor : public QWidget
{
    Q_OBJECT

public:
    explicit GradientEditor(QWidget* parent = nullptr);

    // Stops
    const std::vector<GradientStop>& stops() const { return stops_; }
    void setStops(const std::vector<GradientStop>& stops);

    // Preset loading
    void loadPreset(const QString& name);

    // Map a normalized value [0,1] to a color
    QColor colorAt(double t) const;

    // Map a raw value to color given data range
    QColor mapValue(double value, double minVal, double maxVal) const;

    // Build QLinearGradient
    QLinearGradient toLinearGradient(double x1, double y1, double x2, double y2) const;

    // Data range
    double rangeMin() const { return rangeMin_; }
    double rangeMax() const { return rangeMax_; }
    void setRange(double min, double max);

signals:
    void gradientChanged();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void setupPresets();

    std::vector<GradientStop> stops_;
    double rangeMin_ = 0.0;
    double rangeMax_ = 1.0;
    int selectedStop_ = -1;
    bool dragging_ = false;

    QMap<QString, std::vector<GradientStop>> presets_;
};
