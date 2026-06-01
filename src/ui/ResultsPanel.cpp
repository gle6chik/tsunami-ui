#include "ResultsPanel.h"
#include "AnimationPlayer.h"
#include "ProfileTool.h"
#include "CoastHistogramTool.h"
#include "GridViewerWidget.h"
#include "model/ResultDataset.h"
#include "model/GridDataset.h"

#include <QVBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>

ResultsPanel::ResultsPanel(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ResultsPanel::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Load results button
    auto* loadBtn = new QPushButton(tr("Load Result Directory..."));
    connect(loadBtn, &QPushButton::clicked, this, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Result Directory"));
        if (!dir.isEmpty() && results_) {
            results_->scanDirectory(dir);
            animPlayer_->setResultDataset(results_);
        }
    });
    layout->addWidget(loadBtn);

    // Animation player (always visible)
    animPlayer_ = new AnimationPlayer(this);
    connect(animPlayer_, &AnimationPlayer::frameChanged,
            this, &ResultsPanel::onFrameChanged);
    layout->addWidget(animPlayer_);

    // Tabbed tools
    tabs_ = new QTabWidget(this);

    profileTool_ = new ProfileTool(this);
    tabs_->addTab(profileTool_, tr("Profile"));

    coastTool_ = new CoastHistogramTool(this);
    tabs_->addTab(coastTool_, tr("Coast Histogram"));

    layout->addWidget(tabs_, 1);
}

void ResultsPanel::setResultDataset(ResultDataset* results)
{
    results_ = results;
    animPlayer_->setResultDataset(results);
}

void ResultsPanel::setGridDataset(GridDataset* grid)
{
    grid_ = grid;
    profileTool_->setGridDataset(grid);
    coastTool_->setGridDataset(grid);
}

void ResultsPanel::setGridViewer(GridViewerWidget* viewer)
{
    viewer_ = viewer;
}

void ResultsPanel::onFrameChanged(int timestep)
{
    if (!results_) return;

    auto frame = results_->frame(timestep);
    if (!frame) return;

    // Update grid viewer overlay
    if (viewer_) {
        viewer_->setOverlayData(frame->values, frame->rows, frame->cols,
                                frame->minVal, frame->maxVal);
    }

    // Update profile tool
    profileTool_->setFrameData(frame->values, frame->rows, frame->cols);
}
