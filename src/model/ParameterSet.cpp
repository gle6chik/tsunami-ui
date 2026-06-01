#include "ParameterSet.h"

#include "DefaultParametersParser.h"
#include "EllipsoidalSource.h"

#include <QFile>
#include <cmath>
#include <fstream>
#include <sstream>

ParameterSet::ParameterSet(QObject* parent)
    : QObject(parent)
{
}

bool ParameterSet::loadFromFile(const QString& path)
{
    try {
        DefaultParametersParser parser;
        parser.Read(path.toStdString());

        deltaT_ = parser.GetDeltaT();
        timesteps_ = parser.GetTimesteps();
        numThreads_ = parser.GetNumThreads();
        numSubgrids_ = parser.GetNumSubgrids();
        boundaryExchangeType_ = parser.GetBoundaryExchangeType();
        hatBoundaryExchangeType_ = parser.GetHatBoundaryExchangeType();
        deltaXType_ = std::stoi(parser.GetDeltaXType());
        deltaXStep_ = parser.GetDeltaXStep();
        deltaYType_ = std::stoi(parser.GetDeltaYType());
        deltaYStep_ = parser.GetDeltaYStep();
        generationType_ = QString::fromStdString(parser.GetGenerationType());
        inputBathGenType_ = static_cast<int>(parser.GetInputBathymetryGenerationType());
        inputBathType_ = static_cast<int>(parser.GetInputBathymetryType());
        minDepth_ = parser.GetMinDepthToCalc();
        coordSystem_ = parser.GetCoordSystem();

        sources_.clear();
        int srcIdx = 0;
        for (const auto& src : parser.GetSources()) {
            SourceParams sp;
            sp.name = QString("Source_%1").arg(++srcIdx);
            // Map SourceType enum to .par file convention:
            // .par: "0"=Ellipsoidal, "1"=Point, "2"=Cylindrical, "3"=Rectangular
            // enum: POINT=0, ELLIPSOIDAL=1, CYLINDRICAL=2, RECTANGULAR=3
            switch (src->GetSourceType()) {
                case ELLIPSOIDAL:  sp.type = 0; break;
                case POINT:        sp.type = 1; break;
                case CYLINDRICAL:  sp.type = 2; break;
                case RECTANGULAR:  sp.type = 3; break;
                default:           sp.type = 0; break;
            }
            if (src->GetSourceType() == ELLIPSOIDAL) {
                auto* ell = dynamic_cast<EllipsoidalSource*>(src.get());
                if (ell) {
                    sp.centerX = ell->GetCenterX();
                    sp.centerY = ell->GetCenterY();
                    sp.radius1 = ell->GetRadius1();
                    sp.radius2 = ell->GetRadius2();
                    sp.angle = ell->GetAngle();
                }
            }
            sources_.push_back(sp);
        }

        emit parameterChanged();
        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

bool ParameterSet::saveToFile(const QString& path) const
{
    try {
        std::ofstream out(path.toStdString());
        if (!out.is_open()) return false;

        // Format version
        out << "120\n";

        // Header line (6 fields for v1.2.0)
        out << deltaT_ << " "
            << timesteps_ << " "
            << numThreads_ << " "
            << numSubgrids_ << " "
            << boundaryExchangeType_ << " "
            << hatBoundaryExchangeType_ << "\n";

        // Grid line
        out << deltaXType_ << " "
            << deltaXStep_ << " "
            << deltaYType_ << " "
            << deltaYStep_ << " "
            << generationType_.toStdString() << " "
            << sources_.size() << "\n";

        // Bathymetry line
        out << inputBathGenType_ << " "
            << inputBathType_ << " "
            << minDepth_ << "\n";

        // Coord system
        out << CoordSystemToString(coordSystem_) << "\n";

        // Sources
        for (const auto& s : sources_) {
            out << s.name.toStdString() << " "
                << s.type << " "
                << s.centerX << " "
                << s.centerY << " "
                << s.radius1 << " "
                << s.radius2 << " "
                << s.angle << "\n";
        }

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}

double ParameterSet::computeCFL(double maxDepth) const
{
    constexpr double g = 9.81;
    double c = std::sqrt(g * maxDepth);
    double dx = (deltaXStep_ > 0) ? deltaXStep_ : 1.0;
    double dy = (deltaYStep_ > 0) ? deltaYStep_ : 1.0;
    return deltaT_ * c * (1.0 / dx + 1.0 / dy);
}

bool ParameterSet::isCFLValid(double maxDepth) const
{
    return computeCFL(maxDepth) < 1.0;
}

// Setters with signal emission
void ParameterSet::setDeltaT(double v) { deltaT_ = v; emit parameterChanged(); }
void ParameterSet::setTimesteps(int v) { timesteps_ = v; emit parameterChanged(); }
void ParameterSet::setNumThreads(int v) { numThreads_ = v; emit parameterChanged(); }
void ParameterSet::setNumSubgrids(int v) { numSubgrids_ = v; emit parameterChanged(); }
void ParameterSet::setBoundaryExchangeType(int v) { boundaryExchangeType_ = v; emit parameterChanged(); }
void ParameterSet::setHatBoundaryExchangeType(int v) { hatBoundaryExchangeType_ = v; emit parameterChanged(); }
void ParameterSet::setDeltaXType(int v) { deltaXType_ = v; emit parameterChanged(); }
void ParameterSet::setDeltaXStep(double v) { deltaXStep_ = v; emit parameterChanged(); }
void ParameterSet::setDeltaYType(int v) { deltaYType_ = v; emit parameterChanged(); }
void ParameterSet::setDeltaYStep(double v) { deltaYStep_ = v; emit parameterChanged(); }
void ParameterSet::setInputBathymetryGenerationType(int v) { inputBathGenType_ = v; emit parameterChanged(); }
void ParameterSet::setInputBathymetryType(int v) { inputBathType_ = v; emit parameterChanged(); }
void ParameterSet::setMinimumDepthToCalculate(double v) { minDepth_ = v; emit parameterChanged(); }
void ParameterSet::setCoordSystem(CoordSystem v) { coordSystem_ = v; emit parameterChanged(); }
void ParameterSet::setGenerationType(const QString& v) { generationType_ = v; emit parameterChanged(); }

void ParameterSet::setSources(const std::vector<SourceParams>& sources)
{
    sources_ = sources;
    emit parameterChanged();
}

void ParameterSet::addSource(const SourceParams& s)
{
    sources_.push_back(s);
    emit parameterChanged();
}

void ParameterSet::removeSource(int index)
{
    if (index >= 0 && index < static_cast<int>(sources_.size())) {
        sources_.erase(sources_.begin() + index);
        emit parameterChanged();
    }
}
