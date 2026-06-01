#include "GradientEditor.h"

#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QColorDialog>
#include <QMenu>
#include <QSizePolicy>
#include <algorithm>

GradientEditor::GradientEditor(QWidget* parent)
    : QWidget(parent)
{
    // Vertical bar: enough width for labels, stretch vertically
    setMinimumWidth(50);
    setMaximumWidth(70);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setupPresets();
    loadPreset("ocean");
}

void GradientEditor::setupPresets()
{
    presets_["ocean"] = {
        {0.0,  QColor(0, 0, 80)},
        {0.3,  QColor(0, 0, 200)},
        {0.5,  QColor(0, 128, 255)},
        {0.7,  QColor(0, 200, 200)},
        {1.0,  QColor(0, 255, 128)}
    };

    presets_["terrain"] = {
        {0.0,  QColor(0, 100, 0)},
        {0.3,  QColor(34, 139, 34)},
        {0.5,  QColor(160, 140, 80)},
        {0.7,  QColor(139, 90, 43)},
        {1.0,  QColor(255, 255, 255)}
    };

    presets_["diverging"] = {
        {0.0,  QColor(0, 0, 200)},
        {0.5,  QColor(255, 255, 255)},
        {1.0,  QColor(200, 0, 0)}
    };

    presets_["hot"] = {
        {0.0,  QColor(0, 0, 0)},
        {0.33, QColor(200, 0, 0)},
        {0.66, QColor(255, 200, 0)},
        {1.0,  QColor(255, 255, 255)}
    };
}

void GradientEditor::loadPreset(const QString& name)
{
    auto it = presets_.find(name);
    if (it != presets_.end()) {
        stops_ = it.value();
        update();
        emit gradientChanged();
    }
}

void GradientEditor::setStops(const std::vector<GradientStop>& stops)
{
    stops_ = stops;
    std::sort(stops_.begin(), stops_.end(),
              [](const GradientStop& a, const GradientStop& b) {
                  return a.position < b.position;
              });
    update();
    emit gradientChanged();
}

void GradientEditor::setRange(double min, double max)
{
    rangeMin_ = min;
    rangeMax_ = max;
}

QColor GradientEditor::colorAt(double t) const
{
    if (stops_.empty()) return Qt::black;
    if (stops_.size() == 1) return stops_[0].color;

    t = std::clamp(t, 0.0, 1.0);

    // Find bounding stops
    if (t <= stops_.front().position) return stops_.front().color;
    if (t >= stops_.back().position) return stops_.back().color;

    for (size_t i = 0; i + 1 < stops_.size(); ++i) {
        if (t >= stops_[i].position && t <= stops_[i + 1].position) {
            double frac = (t - stops_[i].position) /
                          (stops_[i + 1].position - stops_[i].position);
            const QColor& c1 = stops_[i].color;
            const QColor& c2 = stops_[i + 1].color;
            return QColor(
                static_cast<int>(c1.red()   + frac * (c2.red()   - c1.red())),
                static_cast<int>(c1.green() + frac * (c2.green() - c1.green())),
                static_cast<int>(c1.blue()  + frac * (c2.blue()  - c1.blue())));
        }
    }
    return stops_.back().color;
}

QColor GradientEditor::mapValue(double value, double minVal, double maxVal) const
{
    if (maxVal <= minVal) return colorAt(0.5);
    double t = (value - minVal) / (maxVal - minVal);
    return colorAt(t);
}

QLinearGradient GradientEditor::toLinearGradient(double x1, double y1, double x2, double y2) const
{
    QLinearGradient grad(x1, y1, x2, y2);
    for (const auto& s : stops_)
        grad.setColorAt(s.position, s.color);
    return grad;
}

void GradientEditor::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QRect r = rect().adjusted(2, 2, -2, -2);

    // Draw vertical gradient bar (top = t=1.0 (max), bottom = t=0.0 (min))
    for (int y = r.top(); y <= r.bottom(); ++y) {
        double t = 1.0 - static_cast<double>(y - r.top()) / r.height();
        p.setPen(colorAt(t));
        p.drawLine(r.left(), y, r.right(), y);
    }

    // Draw stop markers on the right edge
    for (size_t i = 0; i < stops_.size(); ++i) {
        int y = r.bottom() - static_cast<int>(stops_[i].position * r.height());
        bool selected = (static_cast<int>(i) == selectedStop_);
        p.setPen(QPen(selected ? Qt::white : Qt::black, selected ? 2 : 1));
        p.setBrush(stops_[i].color);
        int sz = selected ? 6 : 4;
        QPolygon tri;
        tri << QPoint(r.right() + 2, y - sz)
            << QPoint(r.right() + 2, y + sz)
            << QPoint(r.right() - sz, y);
        p.drawPolygon(tri);
    }

    // Draw tick marks and value labels along the bar
    p.setPen(Qt::white);
    QFont f = font();
    f.setPointSize(7);
    p.setFont(f);

    const int numTicks = 5; // top, bottom, and 3 intermediate
    for (int i = 0; i <= numTicks; ++i) {
        double frac = static_cast<double>(i) / numTicks;  // 0=top(max) .. 1=bottom(min)
        int y = r.top() + static_cast<int>(frac * r.height());
        double value = rangeMax_ - frac * (rangeMax_ - rangeMin_);

        // Tick line
        p.setPen(QPen(Qt::white, 1));
        p.drawLine(r.left(), y, r.left() + 4, y);

        // Value label with dark outline for readability
        QString label = QString::number(value, 'g', 4);
        QRect textRect(r.left(), y - 6, r.width() - 2, 12);
        p.setPen(Qt::black);
        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy)
                p.drawText(textRect.translated(dx, dy), Qt::AlignCenter, label);
        p.setPen(Qt::white);
        p.drawText(textRect, Qt::AlignCenter, label);
    }
}

void GradientEditor::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) return;

    QRect r = rect().adjusted(2, 2, -2, -2);
    double t = 1.0 - static_cast<double>(event->pos().y() - r.top()) / r.height();
    t = std::clamp(t, 0.0, 1.0);

    // Find closest stop
    selectedStop_ = -1;
    double minDist = 0.03;
    for (size_t i = 0; i < stops_.size(); ++i) {
        double dist = std::abs(stops_[i].position - t);
        if (dist < minDist) {
            minDist = dist;
            selectedStop_ = static_cast<int>(i);
        }
    }

    if (selectedStop_ >= 0)
        dragging_ = true;

    update();
}

void GradientEditor::mouseMoveEvent(QMouseEvent* event)
{
    if (!dragging_ || selectedStop_ < 0) return;

    QRect r = rect().adjusted(2, 2, -2, -2);
    double t = 1.0 - static_cast<double>(event->pos().y() - r.top()) / r.height();
    t = std::clamp(t, 0.0, 1.0);

    stops_[selectedStop_].position = t;

    // Keep sorted
    std::sort(stops_.begin(), stops_.end(),
              [](const GradientStop& a, const GradientStop& b) {
                  return a.position < b.position;
              });

    // Find where our stop ended up after sorting
    for (size_t i = 0; i < stops_.size(); ++i) {
        if (std::abs(stops_[i].position - t) < 1e-9) {
            selectedStop_ = static_cast<int>(i);
            break;
        }
    }

    update();
    emit gradientChanged();
}

void GradientEditor::mouseReleaseEvent(QMouseEvent*)
{
    dragging_ = false;
}

void GradientEditor::mouseDoubleClickEvent(QMouseEvent* event)
{
    QRect r = rect().adjusted(2, 2, -2, -2);
    double t = 1.0 - static_cast<double>(event->pos().y() - r.top()) / r.height();
    t = std::clamp(t, 0.0, 1.0);

    if (selectedStop_ >= 0) {
        QColor c = QColorDialog::getColor(stops_[selectedStop_].color, this);
        if (c.isValid()) {
            stops_[selectedStop_].color = c;
            update();
            emit gradientChanged();
        }
    } else {
        QColor c = QColorDialog::getColor(colorAt(t), this);
        if (c.isValid()) {
            stops_.push_back({t, c});
            std::sort(stops_.begin(), stops_.end(),
                      [](const GradientStop& a, const GradientStop& b) {
                          return a.position < b.position;
                      });
            update();
            emit gradientChanged();
        }
    }
}

void GradientEditor::contextMenuEvent(QContextMenuEvent* event)
{
    QRect r = rect().adjusted(2, 2, -2, -2);
    double t = 1.0 - static_cast<double>(event->pos().y() - r.top()) / r.height();
    t = std::clamp(t, 0.0, 1.0);

    // Find closest stop
    int idx = -1;
    double minDist = 0.03;
    for (size_t i = 0; i < stops_.size(); ++i) {
        double dist = std::abs(stops_[i].position - t);
        if (dist < minDist) { minDist = dist; idx = static_cast<int>(i); }
    }

    QMenu menu(this);

    if (idx >= 0 && stops_.size() > 2) {
        menu.addAction(tr("Remove stop"), this, [this, idx]() {
            stops_.erase(stops_.begin() + idx);
            selectedStop_ = -1;
            update();
            emit gradientChanged();
        });
    }

    menu.addAction(tr("Add stop here"), this, [this, t]() {
        QColor c = QColorDialog::getColor(colorAt(t), this);
        if (c.isValid()) {
            stops_.push_back({t, c});
            std::sort(stops_.begin(), stops_.end(),
                      [](const GradientStop& a, const GradientStop& b) {
                          return a.position < b.position;
                      });
            update();
            emit gradientChanged();
        }
    });

    menu.exec(event->globalPos());
}
