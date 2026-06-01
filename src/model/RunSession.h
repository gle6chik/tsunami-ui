#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>

class RunSession : public QObject
{
    Q_OBJECT

public:
    enum class State { Idle, Running, Finished, Failed, Cancelled };
    Q_ENUM(State)

    explicit RunSession(QObject* parent = nullptr);

    State state() const { return state_; }
    void setState(State s);

    int currentTimestep() const { return currentTimestep_; }
    void setCurrentTimestep(int t);

    int totalTimesteps() const { return totalTimesteps_; }
    void setTotalTimesteps(int t) { totalTimesteps_ = t; }

    int exitCode() const { return exitCode_; }
    void setExitCode(int c) { exitCode_ = c; }

    QString parFilePath() const { return parPath_; }
    void setParFilePath(const QString& p) { parPath_ = p; }

    QString outputDirectory() const { return outputDir_; }
    void setOutputDirectory(const QString& p) { outputDir_ = p; }

    QString log() const { return log_; }
    void appendLog(const QString& text);

    QDateTime startTime() const { return startTime_; }
    void setStartTime(const QDateTime& t) { startTime_ = t; }

    QDateTime endTime() const { return endTime_; }
    void setEndTime(const QDateTime& t) { endTime_ = t; }

    // Save run manifest as JSON
    QJsonObject toManifest() const;
    bool saveManifest(const QString& path) const;

signals:
    void stateChanged(RunSession::State state);
    void progressChanged(int current, int total);
    void logAppended(const QString& text);

private:
    State state_ = State::Idle;
    int currentTimestep_ = 0;
    int totalTimesteps_ = 0;
    int exitCode_ = -1;
    QString parPath_;
    QString outputDir_;
    QString log_;
    QDateTime startTime_;
    QDateTime endTime_;
};
