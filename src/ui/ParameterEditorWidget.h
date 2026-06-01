#pragma once

#include <QWidget>

class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QLabel;
class QTableWidget;
class QPushButton;
class ParameterSet;

class ParameterEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterEditorWidget(QWidget* parent = nullptr);

    void loadFromFile(const QString& path);
    void saveToFile(const QString& path);

    ParameterSet* parameterSet() const { return params_; }

private:
    void setupUI();
    void setupSimulationGroup();
    void setupGridGroup();
    void setupBathymetryGroup();
    void setupSourcesGroup();
    void setupCFLIndicator();

    void syncUIFromModel();
    void updateCFLIndicator();
    void updateSourceTable();

    // Source table actions
    void addSource();
    void removeSelectedSource();
    void onSourceCellChanged(int row, int column);

    ParameterSet* params_ = nullptr;

    // Simulation group
    QDoubleSpinBox* deltaTSpin_ = nullptr;
    QSpinBox* timestepsSpin_ = nullptr;
    QSpinBox* numThreadsSpin_ = nullptr;
    QSpinBox* numSubgridsSpin_ = nullptr;
    QComboBox* boundaryExchangeCombo_ = nullptr;
    QComboBox* hatBoundaryExchangeCombo_ = nullptr;

    // Grid group
    QComboBox* deltaXTypeCombo_ = nullptr;
    QDoubleSpinBox* deltaXStepSpin_ = nullptr;
    QComboBox* deltaYTypeCombo_ = nullptr;
    QDoubleSpinBox* deltaYStepSpin_ = nullptr;
    QComboBox* coordSystemCombo_ = nullptr;

    // Bathymetry group
    QComboBox* bathGenTypeCombo_ = nullptr;
    QComboBox* bathTypeCombo_ = nullptr;
    QDoubleSpinBox* minDepthSpin_ = nullptr;

    // Sources
    QTableWidget* sourceTable_ = nullptr;
    QPushButton* addSourceBtn_ = nullptr;
    QPushButton* removeSourceBtn_ = nullptr;

    // CFL indicator
    QLabel* cflLabel_ = nullptr;
    QDoubleSpinBox* maxDepthSpin_ = nullptr;
};
