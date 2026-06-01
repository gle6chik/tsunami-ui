#include "RunSession.h"

#include <QFile>
#include <QJsonDocument>

RunSession::RunSession(QObject* parent)
    : QObject(parent)
{
}

void RunSession::setState(State s)
{
    if (state_ != s) {
        state_ = s;
        emit stateChanged(s);
    }
}

void RunSession::setCurrentTimestep(int t)
{
    currentTimestep_ = t;
    emit progressChanged(currentTimestep_, totalTimesteps_);
}

void RunSession::appendLog(const QString& text)
{
    log_ += text;
    emit logAppended(text);
}

QJsonObject RunSession::toManifest() const
{
    QJsonObject obj;
    obj["parFile"] = parPath_;
    obj["outputDir"] = outputDir_;
    obj["startTime"] = startTime_.toString(Qt::ISODate);
    obj["endTime"] = endTime_.toString(Qt::ISODate);
    obj["exitCode"] = exitCode_;
    obj["totalTimesteps"] = totalTimesteps_;
    obj["lastTimestep"] = currentTimestep_;

    QString stateStr;
    switch (state_) {
        case State::Idle:      stateStr = "idle"; break;
        case State::Running:   stateStr = "running"; break;
        case State::Finished:  stateStr = "finished"; break;
        case State::Failed:    stateStr = "failed"; break;
        case State::Cancelled: stateStr = "cancelled"; break;
    }
    obj["state"] = stateStr;

    return obj;
}

bool RunSession::saveManifest(const QString& path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    file.write(QJsonDocument(toManifest()).toJson());
    return true;
}
