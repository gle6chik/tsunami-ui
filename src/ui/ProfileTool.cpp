#include "ProfileTool.h"
#include "model/GridDataset.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <algorithm>
#include <cmath>

ProfileTool::ProfileTool(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* controls = new QHBoxLayout;
    controls->addWidget(new QLabel(tr("Mode:")));
    modeCombo_ = new QComboBox;
    modeCombo_->addItems({tr("Row"), tr("Column")});
    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) { mode_ = static_cast<Mode>(idx); extractProfile(); });
    controls->addWidget(modeCombo_);

    controls->addWidget(new QLabel(tr("Index:")));
    indexSpin_ = new QSpinBox;
    indexSpin_->setRange(0, 10000);
    connect(indexSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) { selectedIndex_ = v; extractProfile(); });
    controls->addWidget(indexSpin_);

    controls->addWidget(new QLabel(tr("Variable:")));
    varCombo_ = new QComboBox;
    varCombo_->addItems({"D (bathymetry)", "eta (frame)"});
    connect(varCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { extractProfile(); });
    controls->addWidget(varCombo_);

    controls->addStretch();
    layout->addLayout(controls);

    infoLabel_ = new QLabel;
    layout->addWidget(infoLabel_);

    setMinimumHeight(200);
}

void ProfileTool::setGridDataset(GridDataset* grid)
{
    grid_ = grid;
    if (grid_) {
        indexSpin_->setMaximum(std::max(grid_->rows(), grid_->cols()) - 1);
    }
    extractProfile();
}

void ProfileTool::setFrameData(const std::vector<double>& data, int rows, int cols)
{
    frameData_ = data;
    frameRows_ = rows;
    frameCols_ = cols;
    extractProfile();
}

void ProfileTool::selectRow(int row)
{
    mode_ = Row;
    modeCombo_->setCurrentIndex(0);
    indexSpin_->setValue(row);
}

void ProfileTool::selectColumn(int col)
{
    mode_ = Column;
    modeCombo_->setCurrentIndex(1);
    indexSpin_->setValue(col);
}

void ProfileTool::extractProfile()
{
    profile_.clear();
    bool useFrame = (varCombo_->currentIndex() == 1) && !frameData_.empty();

    if (mode_ == Row) {
        int row = selectedIndex_;
        int cols = useFrame ? frameCols_ : (grid_ ? grid_->cols() : 0);
        if (useFrame && row < frameRows_) {
            for (int c = 0; c < cols; ++c)
                profile_.push_back(frameData_[row * cols + c]);
        } else if (grid_ && row < grid_->rows()) {
            for (int c = 0; c < grid_->cols(); ++c)
                profile_.push_back(grid_->value(row, c));
        }
    } else {
        int col = selectedIndex_;
        int rows = useFrame ? frameRows_ : (grid_ ? grid_->rows() : 0);
        if (useFrame && col < frameCols_) {
            for (int r = 0; r < rows; ++r)
                profile_.push_back(frameData_[r * frameCols_ + col]);
        } else if (grid_ && col < grid_->cols()) {
            for (int r = 0; r < grid_->rows(); ++r)
                profile_.push_back(grid_->value(r, col));
        }
    }

    if (!profile_.empty()) {
        profileMin_ = *std::min_element(profile_.begin(), profile_.end());
        profileMax_ = *std::max_element(profile_.begin(), profile_.end());
    } else {
        profileMin_ = 0;
        profileMax_ = 0;
    }

    infoLabel_->setText(tr("%1 %2: %3 points, range [%4, %5]")
                            .arg(mode_ == Row ? tr("Row") : tr("Col"))
                            .arg(selectedIndex_)
                            .arg(profile_.size())
                            .arg(profileMin_, 0, 'f', 2)
                            .arg(profileMax_, 0, 'f', 2));
    update();
}

void ProfileTool::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::white);

    int margin = 40;
    QRect chartRect = rect().adjusted(margin, margin + 30, -margin, -margin);
    if (chartRect.width() < 20 || chartRect.height() < 20 || profile_.empty())
        return;

    double range = profileMax_ - profileMin_;
    if (range < 1e-12) range = 1.0;

    // Axes
    p.setPen(Qt::black);
    p.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
    p.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

    // Y labels
    p.drawText(chartRect.topLeft() + QPoint(-35, 12), QString::number(profileMax_, 'f', 1));
    p.drawText(chartRect.bottomLeft() + QPoint(-35, 0), QString::number(profileMin_, 'f', 1));

    // Profile line
    p.setPen(QPen(QColor(0, 80, 180), 1.5));
    double dx = static_cast<double>(chartRect.width()) / (profile_.size() - 1);

    QPainterPath path;
    for (size_t i = 0; i < profile_.size(); ++i) {
        double x = chartRect.left() + i * dx;
        double y = chartRect.bottom() - (profile_[i] - profileMin_) / range * chartRect.height();
        if (i == 0)
            path.moveTo(x, y);
        else
            path.lineTo(x, y);
    }
    p.drawPath(path);
}
