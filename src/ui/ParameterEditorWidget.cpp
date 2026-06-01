#include "ParameterEditorWidget.h"
#include "model/ParameterSet.h"

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>

ParameterEditorWidget::ParameterEditorWidget(QWidget* parent)
    : QWidget(parent),
      params_(new ParameterSet(this))
{
    setupUI();
    syncUIFromModel();

    connect(params_, &ParameterSet::parameterChanged,
            this, &ParameterEditorWidget::updateCFLIndicator);
}

void ParameterEditorWidget::setupUI()
{
    auto* mainLayout = new QHBoxLayout(this);

    // Left column: simulation + grid + bathymetry
    auto* leftColumn = new QVBoxLayout;
    setupSimulationGroup();
    setupGridGroup();
    setupBathymetryGroup();
    setupCFLIndicator();

    auto* simGroup = new QGroupBox(tr("Simulation"));
    auto* simForm = new QFormLayout(simGroup);
    simForm->addRow(tr("delta_t (s):"), deltaTSpin_);
    simForm->addRow(tr("Timesteps:"), timestepsSpin_);
    simForm->addRow(tr("Threads:"), numThreadsSpin_);
    simForm->addRow(tr("Subgrids:"), numSubgridsSpin_);
    simForm->addRow(tr("Boundary exchange:"), boundaryExchangeCombo_);
    simForm->addRow(tr("Hat boundary exchange:"), hatBoundaryExchangeCombo_);
    leftColumn->addWidget(simGroup);

    auto* gridGroup = new QGroupBox(tr("Grid"));
    auto* gridForm = new QFormLayout(gridGroup);
    gridForm->addRow(tr("delta_x type:"), deltaXTypeCombo_);
    gridForm->addRow(tr("delta_x step (m):"), deltaXStepSpin_);
    gridForm->addRow(tr("delta_y type:"), deltaYTypeCombo_);
    gridForm->addRow(tr("delta_y step (m):"), deltaYStepSpin_);
    gridForm->addRow(tr("Coordinate system:"), coordSystemCombo_);
    leftColumn->addWidget(gridGroup);

    auto* bathGroup = new QGroupBox(tr("Bathymetry"));
    auto* bathForm = new QFormLayout(bathGroup);
    bathForm->addRow(tr("Generation type:"), bathGenTypeCombo_);
    bathForm->addRow(tr("Input type:"), bathTypeCombo_);
    bathForm->addRow(tr("Min depth (m):"), minDepthSpin_);
    leftColumn->addWidget(bathGroup);

    // CFL indicator row
    auto* cflGroup = new QGroupBox(tr("CFL Check"));
    auto* cflLayout = new QFormLayout(cflGroup);
    cflLayout->addRow(tr("Max depth for CFL (m):"), maxDepthSpin_);
    cflLayout->addRow(tr("CFL number:"), cflLabel_);
    leftColumn->addWidget(cflGroup);

    leftColumn->addStretch();
    mainLayout->addLayout(leftColumn, 1);

    // Right column: sources table
    setupSourcesGroup();
    auto* srcGroup = new QGroupBox(tr("Sources"));
    auto* srcLayout = new QVBoxLayout(srcGroup);
    srcLayout->addWidget(sourceTable_);
    auto* btnRow = new QHBoxLayout;
    btnRow->addWidget(addSourceBtn_);
    btnRow->addWidget(removeSourceBtn_);
    btnRow->addStretch();
    srcLayout->addLayout(btnRow);
    mainLayout->addWidget(srcGroup, 1);
}

void ParameterEditorWidget::setupSimulationGroup()
{
    deltaTSpin_ = new QDoubleSpinBox;
    deltaTSpin_->setRange(0.001, 1000.0);
    deltaTSpin_->setDecimals(4);
    deltaTSpin_->setSuffix(" s");
    connect(deltaTSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { params_->setDeltaT(v); });

    timestepsSpin_ = new QSpinBox;
    timestepsSpin_->setRange(1, 1000000);
    connect(timestepsSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) { params_->setTimesteps(v); });

    numThreadsSpin_ = new QSpinBox;
    numThreadsSpin_->setRange(1, 256);
    connect(numThreadsSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) { params_->setNumThreads(v); });

    numSubgridsSpin_ = new QSpinBox;
    numSubgridsSpin_->setRange(1, 256);
    connect(numSubgridsSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) { params_->setNumSubgrids(v); });

    boundaryExchangeCombo_ = new QComboBox;
    boundaryExchangeCombo_->addItem(tr("0 - Free wave exit"), 0);
    boundaryExchangeCombo_->addItem(tr("1 - Reflection"), 1);
    boundaryExchangeCombo_->addItem(tr("2 - Periodic"), 2);
    boundaryExchangeCombo_->addItem(tr("3 - Sponge layer"), 3);
    connect(boundaryExchangeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setBoundaryExchangeType(
                    boundaryExchangeCombo_->itemData(idx).toInt());
            });

    hatBoundaryExchangeCombo_ = new QComboBox;
    hatBoundaryExchangeCombo_->addItem(tr("0 - Free wave exit"), 0);
    hatBoundaryExchangeCombo_->addItem(tr("1 - Reflection"), 1);
    hatBoundaryExchangeCombo_->addItem(tr("2 - Periodic"), 2);
    hatBoundaryExchangeCombo_->addItem(tr("3 - Sponge layer"), 3);
    connect(hatBoundaryExchangeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setHatBoundaryExchangeType(
                    hatBoundaryExchangeCombo_->itemData(idx).toInt());
            });
}

void ParameterEditorWidget::setupGridGroup()
{
    deltaXTypeCombo_ = new QComboBox;
    deltaXTypeCombo_->addItem(tr("0 - Uniform"), 0);
    deltaXTypeCombo_->addItem(tr("1 - Variable"), 1);
    connect(deltaXTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setDeltaXType(deltaXTypeCombo_->itemData(idx).toInt());
            });

    deltaXStepSpin_ = new QDoubleSpinBox;
    deltaXStepSpin_->setRange(0.01, 1e7);
    deltaXStepSpin_->setDecimals(4);
    deltaXStepSpin_->setSuffix(" m");
    connect(deltaXStepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { params_->setDeltaXStep(v); });

    deltaYTypeCombo_ = new QComboBox;
    deltaYTypeCombo_->addItem(tr("0 - Uniform"), 0);
    deltaYTypeCombo_->addItem(tr("1 - Variable"), 1);
    connect(deltaYTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setDeltaYType(deltaYTypeCombo_->itemData(idx).toInt());
            });

    deltaYStepSpin_ = new QDoubleSpinBox;
    deltaYStepSpin_->setRange(0.01, 1e7);
    deltaYStepSpin_->setDecimals(4);
    deltaYStepSpin_->setSuffix(" m");
    connect(deltaYStepSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { params_->setDeltaYStep(v); });

    coordSystemCombo_ = new QComboBox;
    coordSystemCombo_->addItem(tr("Cartesian"), static_cast<int>(CoordSystem::Cartesian));
    coordSystemCombo_->addItem(tr("Spherical"), static_cast<int>(CoordSystem::Spherical));
    connect(coordSystemCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setCoordSystem(
                    static_cast<CoordSystem>(coordSystemCombo_->itemData(idx).toInt()));
            });
}

void ParameterEditorWidget::setupBathymetryGroup()
{
    bathGenTypeCombo_ = new QComboBox;
    bathGenTypeCombo_->addItem(tr("0 - Flat bottom"), 0);
    bathGenTypeCombo_->addItem(tr("1 - From file"), 1);
    connect(bathGenTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setInputBathymetryGenerationType(
                    bathGenTypeCombo_->itemData(idx).toInt());
            });

    bathTypeCombo_ = new QComboBox;
    bathTypeCombo_->addItem(tr("0 - Z format"), 0);
    bathTypeCombo_->addItem(tr("1 - ASC format"), 1);
    connect(bathTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int idx) {
                params_->setInputBathymetryType(
                    bathTypeCombo_->itemData(idx).toInt());
            });

    minDepthSpin_ = new QDoubleSpinBox;
    minDepthSpin_->setRange(0.0, 10000.0);
    minDepthSpin_->setDecimals(2);
    minDepthSpin_->setSuffix(" m");
    connect(minDepthSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v) { params_->setMinimumDepthToCalculate(v); });
}

void ParameterEditorWidget::setupSourcesGroup()
{
    sourceTable_ = new QTableWidget(0, 7);
    sourceTable_->setHorizontalHeaderLabels(
        {tr("Name"), tr("Type"), tr("Center X"), tr("Center Y"),
         tr("Radius 1"), tr("Radius 2"), tr("Angle")});
    sourceTable_->horizontalHeader()->setStretchLastSection(true);
    sourceTable_->setSelectionBehavior(QTableWidget::SelectRows);
    sourceTable_->setSelectionMode(QTableWidget::SingleSelection);

    // Sync inline edits back to model
    connect(sourceTable_, &QTableWidget::cellChanged,
            this, &ParameterEditorWidget::onSourceCellChanged);

    addSourceBtn_ = new QPushButton(tr("Add Source"));
    connect(addSourceBtn_, &QPushButton::clicked, this, &ParameterEditorWidget::addSource);

    removeSourceBtn_ = new QPushButton(tr("Remove Source"));
    connect(removeSourceBtn_, &QPushButton::clicked, this, &ParameterEditorWidget::removeSelectedSource);
}

void ParameterEditorWidget::setupCFLIndicator()
{
    maxDepthSpin_ = new QDoubleSpinBox;
    maxDepthSpin_->setRange(1.0, 20000.0);
    maxDepthSpin_->setDecimals(1);
    maxDepthSpin_->setValue(4000.0);
    maxDepthSpin_->setSuffix(" m");
    connect(maxDepthSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { updateCFLIndicator(); });

    cflLabel_ = new QLabel;
    cflLabel_->setMinimumWidth(200);
    updateCFLIndicator();
}

void ParameterEditorWidget::syncUIFromModel()
{
    // Block signals while syncing to avoid feedback loops
    const QSignalBlocker b1(deltaTSpin_);
    const QSignalBlocker b2(timestepsSpin_);
    const QSignalBlocker b3(numThreadsSpin_);
    const QSignalBlocker b4(numSubgridsSpin_);
    const QSignalBlocker b5(boundaryExchangeCombo_);
    const QSignalBlocker b6(hatBoundaryExchangeCombo_);
    const QSignalBlocker b7(deltaXTypeCombo_);
    const QSignalBlocker b8(deltaXStepSpin_);
    const QSignalBlocker b9(deltaYTypeCombo_);
    const QSignalBlocker b10(deltaYStepSpin_);
    const QSignalBlocker b11(coordSystemCombo_);
    const QSignalBlocker b12(bathGenTypeCombo_);
    const QSignalBlocker b13(bathTypeCombo_);
    const QSignalBlocker b14(minDepthSpin_);

    deltaTSpin_->setValue(params_->deltaT());
    timestepsSpin_->setValue(params_->timesteps());
    numThreadsSpin_->setValue(params_->numThreads());
    numSubgridsSpin_->setValue(params_->numSubgrids());

    int beIdx = boundaryExchangeCombo_->findData(params_->boundaryExchangeType());
    boundaryExchangeCombo_->setCurrentIndex(beIdx >= 0 ? beIdx : 0);

    int hbeIdx = hatBoundaryExchangeCombo_->findData(params_->hatBoundaryExchangeType());
    hatBoundaryExchangeCombo_->setCurrentIndex(hbeIdx >= 0 ? hbeIdx : 0);

    int dxIdx = deltaXTypeCombo_->findData(params_->deltaXType());
    deltaXTypeCombo_->setCurrentIndex(dxIdx >= 0 ? dxIdx : 0);
    deltaXStepSpin_->setValue(params_->deltaXStep());

    int dyIdx = deltaYTypeCombo_->findData(params_->deltaYType());
    deltaYTypeCombo_->setCurrentIndex(dyIdx >= 0 ? dyIdx : 0);
    deltaYStepSpin_->setValue(params_->deltaYStep());

    int csIdx = coordSystemCombo_->findData(static_cast<int>(params_->coordSystem()));
    coordSystemCombo_->setCurrentIndex(csIdx >= 0 ? csIdx : 0);

    int bgIdx = bathGenTypeCombo_->findData(params_->inputBathymetryGenerationType());
    bathGenTypeCombo_->setCurrentIndex(bgIdx >= 0 ? bgIdx : 0);

    int btIdx = bathTypeCombo_->findData(params_->inputBathymetryType());
    bathTypeCombo_->setCurrentIndex(btIdx >= 0 ? btIdx : 0);

    minDepthSpin_->setValue(params_->minimumDepthToCalculate());

    updateSourceTable();
    updateCFLIndicator();
}

void ParameterEditorWidget::updateCFLIndicator()
{
    double maxDepth = maxDepthSpin_->value();
    double cfl = params_->computeCFL(maxDepth);
    bool valid = cfl < 1.0;

    QString text = QString::number(cfl, 'f', 4);
    if (valid) {
        cflLabel_->setText(text + tr("  [OK]"));
        cflLabel_->setStyleSheet("color: green; font-weight: bold;");
    } else {
        cflLabel_->setText(text + tr("  [CFL VIOLATED]"));
        cflLabel_->setStyleSheet("color: red; font-weight: bold;");
    }
}

void ParameterEditorWidget::updateSourceTable()
{
    const QSignalBlocker blocker(sourceTable_);
    const auto& sources = params_->sources();
    sourceTable_->setRowCount(static_cast<int>(sources.size()));

    for (int i = 0; i < static_cast<int>(sources.size()); ++i) {
        const auto& s = sources[i];
        sourceTable_->setItem(i, 0, new QTableWidgetItem(s.name));
        sourceTable_->setItem(i, 1, new QTableWidgetItem(QString::number(s.type)));
        sourceTable_->setItem(i, 2, new QTableWidgetItem(QString::number(s.centerX)));
        sourceTable_->setItem(i, 3, new QTableWidgetItem(QString::number(s.centerY)));
        sourceTable_->setItem(i, 4, new QTableWidgetItem(QString::number(s.radius1)));
        sourceTable_->setItem(i, 5, new QTableWidgetItem(QString::number(s.radius2)));
        sourceTable_->setItem(i, 6, new QTableWidgetItem(QString::number(s.angle)));
    }
}

void ParameterEditorWidget::onSourceCellChanged(int row, int column)
{
    if (row < 0 || row >= static_cast<int>(params_->sources().size()))
        return;

    auto sources = params_->sources();
    auto& s = sources[row];
    QString text = sourceTable_->item(row, column)->text();

    switch (column) {
        case 0: s.name = text; break;
        case 1: s.type = text.toInt(); break;
        case 2: s.centerX = text.toDouble(); break;
        case 3: s.centerY = text.toDouble(); break;
        case 4: s.radius1 = text.toDouble(); break;
        case 5: s.radius2 = text.toDouble(); break;
        case 6: s.angle = text.toDouble(); break;
    }

    // Update model without triggering full table rebuild
    const QSignalBlocker blocker(sourceTable_);
    params_->setSources(sources);
}

void ParameterEditorWidget::addSource()
{
    SourceParams s;
    s.name = tr("Source_%1").arg(params_->sources().size() + 1);
    s.type = 0;
    params_->addSource(s);
    updateSourceTable();
}

void ParameterEditorWidget::removeSelectedSource()
{
    int row = sourceTable_->currentRow();
    if (row >= 0) {
        params_->removeSource(row);
        updateSourceTable();
    }
}

void ParameterEditorWidget::loadFromFile(const QString& path)
{
    if (params_->loadFromFile(path)) {
        syncUIFromModel();
    } else {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to load parameter file: %1").arg(path));
    }
}

void ParameterEditorWidget::saveToFile(const QString& path)
{
    if (!params_->saveToFile(path)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to save parameter file: %1").arg(path));
    }
}
