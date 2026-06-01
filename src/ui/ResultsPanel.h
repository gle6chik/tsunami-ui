#pragma once

#include <QWidget>

class QTabWidget;
class AnimationPlayer;
class ProfileTool;
class CoastHistogramTool;
class GridViewerWidget;
class ResultDataset;
class GridDataset;

class ResultsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ResultsPanel(QWidget* parent = nullptr);

    void setResultDataset(ResultDataset* results);
    void setGridDataset(GridDataset* grid);
    void setGridViewer(GridViewerWidget* viewer);

    AnimationPlayer* animationPlayer() const { return animPlayer_; }
    ProfileTool* profileTool() const { return profileTool_; }
    CoastHistogramTool* coastTool() const { return coastTool_; }

private slots:
    void onFrameChanged(int timestep);

private:
    void setupUI();

    QTabWidget* tabs_ = nullptr;
    AnimationPlayer* animPlayer_ = nullptr;
    ProfileTool* profileTool_ = nullptr;
    CoastHistogramTool* coastTool_ = nullptr;

    GridViewerWidget* viewer_ = nullptr;
    ResultDataset* results_ = nullptr;
    GridDataset* grid_ = nullptr;
};
