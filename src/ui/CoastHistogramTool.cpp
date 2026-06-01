#include "CoastHistogramTool.h"
#include "model/GridDataset.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <algorithm>
#include <cmath>

CoastHistogramTool::CoastHistogramTool(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    infoLabel_ = new QLabel(tr("Select a region on the grid to see coastline wave heights."));
    layout->addWidget(infoLabel_);
    layout->addStretch();
    setMinimumHeight(200);
}

void CoastHistogramTool::setGridDataset(GridDataset* grid)
{
    grid_ = grid;
}

void CoastHistogramTool::setRegion(int rowMin, int rowMax, int colMin, int colMax)
{
    regionRowMin_ = rowMin;
    regionRowMax_ = rowMax;
    regionColMin_ = colMin;
    regionColMax_ = colMax;
    recompute();
}

void CoastHistogramTool::setEtaMaxData(const std::vector<double>& etaMax, int rows, int cols)
{
    etaMaxData_ = etaMax;
    etaRows_ = rows;
    etaCols_ = cols;
}

void CoastHistogramTool::recompute()
{
    coastNodes_ = findCoastNodes();
    infoLabel_->setText(tr("Coast nodes found: %1").arg(coastNodes_.size()));
    update();
}

std::vector<CoastHistogramTool::CoastNode> CoastHistogramTool::findCoastNodes()
{
    std::vector<CoastNode> nodes;
    if (!grid_ || !grid_->isLoaded() || etaMaxData_.empty())
        return nodes;

    int rows = grid_->rows();
    int cols = grid_->cols();

    // Clamp region
    int rMin = std::max(0, regionRowMin_);
    int rMax = std::min(rows - 1, regionRowMax_);
    int cMin = std::max(0, regionColMin_);
    int cMax = std::min(cols - 1, regionColMax_);

    static const int dr[] = {-1, 1, 0, 0};
    static const int dc[] = {0, 0, -1, 1};

    for (int r = rMin; r <= rMax; ++r) {
        for (int c = cMin; c <= cMax; ++c) {
            double depth = grid_->value(r, c);
            // Coastal water node: depth >= minDepth (water) adjacent to land
            if (depth < minDepth_) continue;

            bool adjacentToLand = false;
            for (int d = 0; d < 4; ++d) {
                int nr = r + dr[d];
                int nc = c + dc[d];
                if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) {
                    adjacentToLand = true; // edge of grid = land
                    break;
                }
                if (grid_->value(nr, nc) < minDepth_) {
                    adjacentToLand = true;
                    break;
                }
            }

            if (adjacentToLand && r < etaRows_ && c < etaCols_) {
                double eta = etaMaxData_[r * etaCols_ + c];
                nodes.push_back({r, c, eta});
            }
        }
    }

    return nodes;
}

void CoastHistogramTool::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    if (coastNodes_.empty()) {
        p.drawText(rect(), Qt::AlignCenter, tr("No coast data"));
        return;
    }

    // Draw bar chart
    int margin = 30;
    QRect chartRect = rect().adjusted(margin, margin, -margin, -margin);
    if (chartRect.width() < 10 || chartRect.height() < 10) return;

    double maxEta = 0;
    for (const auto& n : coastNodes_)
        maxEta = std::max(maxEta, std::abs(n.etaMax));
    if (maxEta < 1e-9) maxEta = 1.0;

    int barCount = static_cast<int>(coastNodes_.size());
    double barWidth = static_cast<double>(chartRect.width()) / barCount;

    p.setPen(Qt::black);
    p.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
    p.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

    for (int i = 0; i < barCount; ++i) {
        double h = (coastNodes_[i].etaMax / maxEta) * chartRect.height();
        QRectF bar(chartRect.left() + i * barWidth, chartRect.bottom() - h,
                   barWidth * 0.8, h);
        p.fillRect(bar, QColor(0, 100, 200));
        p.drawRect(bar);
    }

    // Y-axis label
    p.drawText(chartRect.topLeft() + QPoint(-25, 10),
               QString::number(maxEta, 'f', 1) + " m");
    p.drawText(chartRect.bottomLeft() + QPoint(-10, 15), "0");
}
