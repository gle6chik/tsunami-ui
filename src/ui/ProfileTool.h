#pragma once

#include <QWidget>
#include <vector>

class QComboBox;
class QSpinBox;
class QLabel;
class GridDataset;
class ResultDataset;

// Displays 1D cross-section profile along a row or column
class ProfileTool : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileTool(QWidget* parent = nullptr);

    void setGridDataset(GridDataset* grid);

    // Set current frame data for dynamic profile
    void setFrameData(const std::vector<double>& data, int rows, int cols);

    // Select row or column
    void selectRow(int row);
    void selectColumn(int col);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void extractProfile();

    enum Mode { Row, Column };

    GridDataset* grid_ = nullptr;
    std::vector<double> frameData_;
    int frameRows_ = 0;
    int frameCols_ = 0;

    Mode mode_ = Row;
    int selectedIndex_ = 0;
    std::vector<double> profile_;
    double profileMin_ = 0.0;
    double profileMax_ = 0.0;

    QComboBox* modeCombo_ = nullptr;
    QSpinBox* indexSpin_ = nullptr;
    QComboBox* varCombo_ = nullptr;
    QLabel* infoLabel_ = nullptr;
};
