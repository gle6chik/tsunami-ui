#include "AnimationPlayer.h"
#include "model/ResultDataset.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include <QDoubleSpinBox>

AnimationPlayer::AnimationPlayer(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void AnimationPlayer::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Transport controls
    auto* transport = new QHBoxLayout;

    stepBackBtn_ = new QPushButton(tr("|<"));
    stepBackBtn_->setFixedWidth(32);
    connect(stepBackBtn_, &QPushButton::clicked, this, &AnimationPlayer::stepBack);
    transport->addWidget(stepBackBtn_);

    playBtn_ = new QPushButton(tr("Play"));
    connect(playBtn_, &QPushButton::clicked, this, &AnimationPlayer::play);
    transport->addWidget(playBtn_);

    pauseBtn_ = new QPushButton(tr("Pause"));
    connect(pauseBtn_, &QPushButton::clicked, this, &AnimationPlayer::pause);
    transport->addWidget(pauseBtn_);

    stopBtn_ = new QPushButton(tr("Stop"));
    connect(stopBtn_, &QPushButton::clicked, this, &AnimationPlayer::stop);
    transport->addWidget(stopBtn_);

    stepFwdBtn_ = new QPushButton(tr(">|"));
    stepFwdBtn_->setFixedWidth(32);
    connect(stepFwdBtn_, &QPushButton::clicked, this, &AnimationPlayer::stepForward);
    transport->addWidget(stepFwdBtn_);

    transport->addWidget(new QLabel(tr("Speed:")));
    speedSpin_ = new QDoubleSpinBox;
    speedSpin_->setRange(0.5, 10.0);
    speedSpin_->setValue(1.0);
    speedSpin_->setSingleStep(0.5);
    speedSpin_->setSuffix("x");
    transport->addWidget(speedSpin_);

    transport->addStretch();
    layout->addLayout(transport);

    // Scrub bar
    auto* scrubLayout = new QHBoxLayout;
    scrubBar_ = new QSlider(Qt::Horizontal);
    scrubBar_->setMinimum(0);
    connect(scrubBar_, &QSlider::valueChanged, this, &AnimationPlayer::seekToFrame);
    scrubLayout->addWidget(scrubBar_);

    frameLabel_ = new QLabel(tr("Frame: 0/0"));
    frameLabel_->setMinimumWidth(120);
    scrubLayout->addWidget(frameLabel_);
    layout->addLayout(scrubLayout);

    // Timer
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &AnimationPlayer::onTimerTick);
}

int AnimationPlayer::currentTimestep() const
{
    if (currentIndex_ >= 0 && currentIndex_ < timesteps_.size())
        return timesteps_[currentIndex_];
    return 0;
}

void AnimationPlayer::setResultDataset(ResultDataset* results)
{
    results_ = results;
    if (!results_ || !results_->isLoaded()) {
        timesteps_.clear();
        scrubBar_->setMaximum(0);
        updateLabels();
        return;
    }

    timesteps_ = results_->timesteps();
    scrubBar_->setMaximum(timesteps_.isEmpty() ? 0 : static_cast<int>(timesteps_.size()) - 1);
    currentIndex_ = 0;
    updateLabels();
}

void AnimationPlayer::play()
{
    if (timesteps_.isEmpty()) return;
    playing_ = true;
    int intervalMs = static_cast<int>(100.0 / speedSpin_->value());
    timer_->start(intervalMs);
}

void AnimationPlayer::pause()
{
    playing_ = false;
    timer_->stop();
}

void AnimationPlayer::stop()
{
    playing_ = false;
    timer_->stop();
    currentIndex_ = 0;
    scrubBar_->setValue(0);
    updateLabels();
    if (!timesteps_.isEmpty())
        emit frameChanged(timesteps_[0]);
}

void AnimationPlayer::stepForward()
{
    if (timesteps_.isEmpty()) return;
    if (currentIndex_ < static_cast<int>(timesteps_.size()) - 1) {
        currentIndex_++;
        scrubBar_->setValue(currentIndex_);
    }
}

void AnimationPlayer::stepBack()
{
    if (timesteps_.isEmpty()) return;
    if (currentIndex_ > 0) {
        currentIndex_--;
        scrubBar_->setValue(currentIndex_);
    }
}

void AnimationPlayer::seekToFrame(int index)
{
    if (index < 0 || index >= static_cast<int>(timesteps_.size())) return;
    currentIndex_ = index;
    updateLabels();
    emit frameChanged(timesteps_[currentIndex_]);
}

void AnimationPlayer::onTimerTick()
{
    if (timesteps_.isEmpty()) { pause(); return; }

    currentIndex_++;
    if (currentIndex_ >= static_cast<int>(timesteps_.size())) {
        currentIndex_ = 0; // loop
    }

    const QSignalBlocker blocker(scrubBar_);
    scrubBar_->setValue(currentIndex_);
    updateLabels();
    emit frameChanged(timesteps_[currentIndex_]);
}

void AnimationPlayer::updateLabels()
{
    if (timesteps_.isEmpty()) {
        frameLabel_->setText(tr("Frame: 0/0"));
        return;
    }
    frameLabel_->setText(tr("Frame: %1/%2  (t=%3)")
                             .arg(currentIndex_ + 1)
                             .arg(timesteps_.size())
                             .arg(timesteps_[currentIndex_]));
}
