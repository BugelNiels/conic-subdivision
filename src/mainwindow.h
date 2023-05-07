#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class represents the main window.
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

 private slots:
  void on_numPresetPointsSpinBox_valueChanged(int arg1);

 private slots:
  void on_netPresets_currentIndexChanged(int index);
  void on_subdivSteps_valueChanged(int numSteps);
  void on_pointWeightSpinBox_valueChanged(double value);
  void on_normalWeightSpinBox_valueChanged(double value);
  void on_normalizedSolveCheckBox_toggled(bool checked);

  void on_circularNormalsCheckBox_toggled(bool checked);

  void on_recalculateNormsCheckBox_toggled(bool checked);

  void on_visualizeNormalscheckBox_toggled(bool checked);

  void on_outwardNormalsCheckBox_toggled(bool checked);

  void on_useEdgeNormalsCheckBox_toggled(bool checked);

  void on_closedCheckBox_toggled(bool checked);

  void on_controlPointsCheckBox_toggled(bool checked);

  void on_controlCurveCheckBox_toggled(bool checked);

  void on_middlePointWeightSpinBox_valueChanged(double arg1);

  void on_outerPointWeightSpinBox_valueChanged(double arg1);

  void on_middleNormalWeightSpinBox_valueChanged(double arg1);

  void on_outerNormalWeightSpinBox_valueChanged(double arg1);

 private:
  Ui::MainWindow *ui;
};

#endif  // MAINWINDOW_H
