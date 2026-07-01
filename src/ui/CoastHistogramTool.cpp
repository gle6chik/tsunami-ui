#include "CoastHistogramTool.h"
#include "model/GridDataset.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

CoastHistogramTool::CoastHistogramTool(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    showCoastlineCheckBox_ = new QCheckBox(tr("Show coastline"), this);
    showCoastlineCheckBox_->setChecked(true);
    layout->addWidget(showCoastlineCheckBox_);
    connect(showCoastlineCheckBox_, &QCheckBox::toggled, this, &CoastHistogramTool::onShowCoastlineToggled);

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
    hasRegion_ = true;
    recompute();
}

void CoastHistogramTool::clearRegion() {
    hasRegion_ = false;
    recompute();
}

bool CoastHistogramTool::hasRegion() const {
    return hasRegion_;
}

void CoastHistogramTool::setEtaMaxData(const std::vector<double>& etaMax, int rows, int cols)
{
    etaMaxData_ = etaMax;
    etaRows_ = rows;
    etaCols_ = cols;
}

void CoastHistogramTool::updateEtaMaxData() {
    if (coastNodes_.empty() || etaMaxData_.empty()) {
        return;
    }

    for (auto& node : coastNodes_) {
        if (node.isSeparator) {
            continue;
        }

        if (node.row < 0 || node.row >= etaRows_ ||
            node.col < 0 || node.col >= etaCols_) {
            continue;
        }

        int idx = node.row * etaCols_ + node.col;
        if (idx >= 0 && idx < static_cast<int>(etaMaxData_.size())) {
            node.etaMax = etaMaxData_[idx];
        }
    }

    update();
}

void CoastHistogramTool::recompute()
{
    coastNodes_ = findCoastNodes();

    QVector<QPointF> points;
    for (const auto& node : coastNodes_) {
        if (!node.isSeparator) {
            points.append(QPointF(node.col, node.row));
        }
    }
    emit coastlineCellsCalculated(points);

    QMap<int, QPointF> labels;
    int currentId = -1;
    for (const auto& node : coastNodes_) {
        if (node.componentId != currentId && node.componentId > 0) {
            labels[node.componentId] = QPointF(node.col, node.row);
            currentId = node.componentId;
        }
    }
    emit coastlineLabelsReady(labels);

    infoLabel_->setText(tr("Coast nodes found: %1").arg(coastNodes_.size()));
    update();
}

/*
 * orderCoastNodes orders coastline nodes along the coastline.
 *
 * Complex cases:
 * 1. Islands / Closed loops.
 * Full perimeter traversal via traverseComponent().
 * Detection: 4-connection fill from outside - if component encloses
 * unreachable cells, it is ring.
 *
 * 2. Breaks (multiple components).
 * The algorithm orders all found components separately.
 * This allows for outputting data for all found components, adding separators.
 *
 * 3. Coves.
 * Traversal from the farthest endpoint via greedy walk with backtracking.
 */
std::vector<CoastHistogramTool::CoastNode> CoastHistogramTool::orderCoastNodes(const std::vector<CoastNode>& nodes)
{
    if (nodes.size() < 2) {
        return nodes;
    }

    std::vector<std::vector<int>> adj(nodes.size());

    std::unordered_map<int, std::unordered_map<int, int>> spatialIndex;
    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        spatialIndex[nodes[i].row][nodes[i].col] = i;
    }

    const int dr8[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int dc8[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < static_cast<int>(nodes.size()); ++i) {
        int r = nodes[i].row;
        int c = nodes[i].col;

        for (int d = 0; d < 8; ++d) {
            int nr = r + dr8[d];
            int nc = c + dc8[d];

            auto rowIt = spatialIndex.find(nr);
            if (rowIt != spatialIndex.end()) {
                auto colIt = rowIt->second.find(nc);

                if (colIt != rowIt->second.end()) {
                    int j = colIt->second;

                    if (j > i) {
                        adj[i].push_back(j);
                        adj[j].push_back(i);
                    }
                }
            }
        }
    }


    // Finding all connected components
    std::vector<bool> visited(nodes.size(), false);
    std::vector<std::vector<int>> components;

    for (int i = 0; i < nodes.size(); ++i) {
        if (!visited[i]) {
            std::vector<int> comp;
            std::queue<int> q;
            q.push(i);
            visited[i] = true;
            while (!q.empty()) {
                int v = q.front(); q.pop();
                comp.push_back(v);
                for (int nb : adj[v]) {
                    if (!visited[nb]) {
                        visited[nb] = true;
                        q.push(nb);
                    }
                }
            }
            components.push_back(comp);
        }
    }

    std::vector<CoastNode> finalOrderedNodes;

    // Finding the farthest node
    auto findFarthestNode = [&](int startIdx, const std::unordered_set<int> &compSet, std::vector<int> &parent) -> int {
        std::vector<bool> was_visited(nodes.size(), false);
        std::queue<std::pair<int, int>> q; // Node - Distance from the start

        // Start of traversal
        was_visited[startIdx] = true;
        parent[startIdx] = -1;
        q.push({startIdx, 0});

        // Traversal
        int farthestNode = startIdx;
        int maxDist = 0;
        while (!q.empty()) {
            auto [node, dist] = q.front();
            q.pop();

            if (dist > maxDist) {
                maxDist = dist;
                farthestNode = node;
            }

            for (int neighbour : adj[node]) {
                if (was_visited[neighbour]) {
                    continue;
                }

                if (!compSet.count(neighbour)) {
                    continue;
                }

                was_visited[neighbour] = true;
                parent[neighbour] = node;
                q.push({neighbour, dist + 1});
            }
        }

        return farthestNode;
    };

    // Checking for closed loop
    auto isRing = [&](const std::vector<int>& comp) -> bool {
        // Not island
        if (comp.size() < 8) {
            return false;
        }

        // Find bounding box of the component
        int minR = nodes[comp[0]].row, maxR = nodes[comp[0]].row;
        int minC = nodes[comp[0]].col, maxC = nodes[comp[0]].col;
        for (int idx : comp) {
            minR = std::min(minR, nodes[idx].row);
            maxR = std::max(maxR, nodes[idx].row);
            minC = std::min(minC, nodes[idx].col);
            maxC = std::max(maxC, nodes[idx].col);
        }

        int height = maxR - minR + 3; // +2 for padding, +1 for 0-indexing
        int width = maxC - minC + 3;

        // Create a binary grid with padding on all sides
        std::vector<std::vector<bool>> grid(height, std::vector<bool>(width, false));
        for (int idx : comp) {
            int r = nodes[idx].row - minR + 1;
            int c = nodes[idx].col - minC + 1;
            grid[r][c] = true;
        }

        // Fill from outside corner (0,0) using 4-connectivity
        std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));
        std::queue<std::pair<int,int>> q;

        q.push({0, 0});
        visited[0][0] = true;

        int dr4[] = {-1, 0, 1, 0};
        int dc4[] = {0, 1, 0, -1};

        while (!q.empty()) {
            auto [r, c] = q.front(); q.pop();

            for (int d = 0; d < 4; d++) {
                int nr = r + dr4[d];
                int nc = c + dc4[d];

                if (nr < 0 || nr >= height || nc < 0 || nc >= width) {
                    continue;
                }

                if (visited[nr][nc] || grid[nr][nc]) {
                    continue;
                }

                visited[nr][nc] = true;
                q.push({nr, nc});
            }
        }

        // Count cells that are not component and not reachable from outside
        int insideCells = 0;
        int totalCells = 0;
        for (int r = 1; r < height - 1; r++) {
            for (int c = 1; c < width - 1; c++) {
                totalCells++;
                if (!grid[r][c] && !visited[r][c]) {
                    insideCells++;
                }
            }
        }

        double insideRatio = (double)insideCells / totalCells;

        // Protection against 8-connectivity artifacts (micro-loops)
        const double MINIMUM_INSIDE_RATIO_FOR_RING = 0.1;
        return insideRatio > MINIMUM_INSIDE_RATIO_FOR_RING;
    };

    // Building final path
    auto traverseComponent = [&](const std::vector<int>& comp, int startNode) -> std::vector<int> {
        std::vector<int> path;
        if (comp.empty()) return path;

        std::unordered_set<int> compSet(comp.begin(), comp.end());
        std::vector<bool> used(nodes.size(), false);

        int current = startNode;
        path.push_back(current);
        used[current] = true;

        while (path.size() < comp.size()) {
            bool found = false;

            for (int nb : adj[current]) {
                if (compSet.count(nb) && !used[nb]) {
                    path.push_back(nb);
                    used[nb] = true;
                    current = nb;
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
                    for (int nb : adj[path[i]]) {
                        if (compSet.count(nb) && !used[nb]) {
                            current = path[i];
                            found = true;
                            break;
                        }
                    }
                    if (found) break;
                }
                if (!found) break;
            }
        }

        return path;
    };

    // Ordering of nodes in each component
    int componentCounter = 0;
    for (const auto& comp : components) {
        // If the component is too small
        const int MINIMUM_COMPONENT_SIZE = 5;
        if (comp.size() < MINIMUM_COMPONENT_SIZE) {
            continue;
        }

        componentCounter++;
        std::vector<int> path;

        if (isRing(comp)) {
            path = traverseComponent(comp, comp[0]);
        } else {
            std::unordered_set<int> compSet(comp.begin(), comp.end());
            std::vector<int> parent(nodes.size(), -1);
            int startNode = findFarthestNode(comp[0], compSet, parent);
            path = traverseComponent(comp, startNode);
        }

        // Add separator
        if (!finalOrderedNodes.empty()) {
            CoastNode separator;
            separator.row = -1;
            separator.col = -1;
            separator.etaMax = -1.0;
            separator.isSeparator = true;
            finalOrderedNodes.push_back(separator);
        }

        for (int idx : path) {
            CoastNode node = nodes[idx];
            node.componentId = componentCounter;
            finalOrderedNodes.push_back(node);
        }
    }

    return finalOrderedNodes;
}

std::vector<CoastHistogramTool::CoastNode> CoastHistogramTool::findCoastNodes()
{
    std::vector<CoastNode> nodes;
    if (!hasRegion_ || !grid_ || !grid_->isLoaded() || etaMaxData_.empty())
        return nodes;

    int rows = grid_->rows();
    int cols = grid_->cols();

    // Clamp region
    int rMin = std::max(0, regionRowMin_);
    int rMax = std::min(rows - 1, regionRowMax_);
    int cMin = std::max(0, regionColMin_);
    int cMax = std::min(cols - 1, regionColMax_);

    static const int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static const int dc[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int r = rMin; r <= rMax; ++r) {
        for (int c = cMin; c <= cMax; ++c) {
            double depth = grid_->value(r, c);
            // Coastal water node: depth >= minDepth (water) adjacent to land
            if (depth < minDepth_) continue;

            bool adjacentToLand = false;
            for (int d = 0; d < 8; ++d) {
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

    return orderCoastNodes(nodes);
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
    int topOffset = 50;
    QRect chartArea = rect().adjusted(0, topOffset, 0, 0);

    int margin = 30;
    QRect chartRect = chartArea.adjusted(margin, margin, -margin, -margin);
    if (chartRect.width() < 10 || chartRect.height() < 10) return;

    double maxEta = 0;
    for (const auto& n : coastNodes_) {
        if (!n.isSeparator && n.componentId > 0) {
            maxEta = std::max(maxEta, std::abs(n.etaMax));
        }
    }
    if (maxEta < 1e-9) maxEta = 1.0;

    int barCount = static_cast<int>(coastNodes_.size());
    double barWidth = static_cast<double>(chartRect.width()) / barCount;

    p.setPen(Qt::black);
    p.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
    p.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

    int currentComponentId = -1;
    for (int i = 0; i < barCount; ++i) {
        const auto& node = coastNodes_[i];
        double x = chartRect.left() + i * barWidth;

        // Draw separator
        if (node.isSeparator) {
            p.save();
            QPen separatorPen(Qt::red, 1, Qt::DashLine);
            p.setPen(separatorPen);
            double lineX = x + barWidth / 2.0;
            p.drawLine(QPointF(lineX, chartRect.top()), QPointF(lineX, chartRect.bottom()));
            p.restore();
            currentComponentId = -1;
            continue;
        }

        if (node.componentId != currentComponentId && node.componentId > 0) {
            currentComponentId = node.componentId;

            QFont font = p.font();
            font.setPointSize(10);
            font.setBold(true);
            p.setFont(font);
            p.setPen(Qt::black);
            p.drawText(QPointF(x + barWidth / 2 - 5, chartRect.top() - 5), QString::number(currentComponentId));
        }

        double h = (coastNodes_[i].etaMax / maxEta) * chartRect.height();
        QRectF bar(x, chartRect.bottom() - h,
                   barWidth * 0.8, h);
        p.fillRect(bar, QColor(0, 100, 200));
        p.drawRect(bar);
    }

    // Y-axis label
    p.drawText(chartRect.topLeft() + QPoint(-25, 10),
               QString::number(maxEta, 'f', 1) + " m");
    p.drawText(chartRect.bottomLeft() + QPoint(-10, 15), "0");
}

void CoastHistogramTool::onShowCoastlineToggled(bool state) {
    emit showCoastlineChanged(state);
}
