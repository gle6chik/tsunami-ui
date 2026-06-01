#pragma once

#include <QWidget>

class QSlider;
class QPushButton;
class QLabel;
class QTimer;
class QDoubleSpinBox;
class ResultDataset;

class AnimationPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationPlayer(QWidget* parent = nullptr);

    void setResultDataset(ResultDataset* results);

    int currentTimestep() const;

signals:
    void frameChanged(int timestep);

public slots:
    void play();
    void pause();
    void stop();
    void stepForward();
    void stepBack();
    void seekToFrame(int index);

private slots:
    void onTimerTick();

private:
    void setupUI();
    void updateLabels();

    ResultDataset* results_ = nullptr;
    QList<int> timesteps_;
    int currentIndex_ = 0;

    QPushButton* playBtn_ = nullptr;
    QPushButton* pauseBtn_ = nullptr;
    QPushButton* stopBtn_ = nullptr;
    QPushButton* stepFwdBtn_ = nullptr;
    QPushButton* stepBackBtn_ = nullptr;
    QSlider* scrubBar_ = nullptr;
    QLabel* frameLabel_ = nullptr;
    QDoubleSpinBox* speedSpin_ = nullptr;
    QTimer* timer_ = nullptr;
    bool playing_ = false;
};
