#include "mainwindow.h"

#include "ui_mainwindow.h"

/**
 * @brief MainWindow::MainWindow Creates a new Main Window UI.
 * @param parent Qt parent widget.
 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // Set some default UI values
  Settings& settings = ui->mainView->settings;

  ui->pointWeightSpinBox->setValue(settings.pointWeight);
  ui->normalWeightSpinBox->setValue(settings.normalWeight);
  ui->middlePointWeightSpinBox->setValue(settings.middlePointWeight);
  ui->middleNormalWeightSpinBox->setValue(settings.middleNormalWeight);
  ui->outerPointWeightSpinBox->setValue(settings.outerPointWeight);
  ui->outerNormalWeightSpinBox->setValue(settings.outerNormalWeight);

  ui->normalizedSolveCheckBox->setChecked(settings.normalizedSolve);

  ui->circularNormalsCheckBox->setChecked(settings.circleNormals);
  ui->recalculateNormsCheckBox->setChecked(settings.recalculateNormals);
  ui->visualizeNormalscheckBox->setChecked(settings.visualizeNormals);
  ui->useEdgeNormalsCheckBox->setChecked(settings.edgeTangentSample);
  ui->closedCheckBox->setChecked(settings.closed);

  ui->controlPointsCheckBox->setChecked(settings.showControlPoints);
  ui->controlCurveCheckBox->setChecked(settings.showControlCurve);
}

/**
 * @brief MainWindow::~MainWindow Deconstructs the main window.
 */
MainWindow::~MainWindow() {
  qDebug() << "✗✗ MainWindow destructor";
  delete ui;
}

void MainWindow::on_netPresets_currentIndexChanged(int index) {
  if (ui->mainView->isValid()) {
    ui->subdivSteps->setValue(0);
    ui->mainView->subCurve.presetNet(index);
  }
  ui->mainView->updateBuffers();
}

void MainWindow::on_subdivSteps_valueChanged(int numSteps) {
  ui->mainView->subCurve.subdivide(numSteps);
  ui->mainView->updateBuffers();
}

void MainWindow::on_pointWeightSpinBox_valueChanged(double value) {
  ui->mainView->settings.pointWeight = value;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_normalWeightSpinBox_valueChanged(double value) {
  ui->mainView->settings.normalWeight = value;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_normalizedSolveCheckBox_toggled(bool checked) {
  ui->mainView->settings.normalizedSolve = checked;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_circularNormalsCheckBox_toggled(bool checked) {
  ui->mainView->settings.circleNormals = checked;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_recalculateNormsCheckBox_toggled(bool checked) {
  ui->mainView->settings.recalculateNormals = checked;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_visualizeNormalscheckBox_toggled(bool checked) {
  ui->mainView->settings.visualizeNormals = checked;
  ui->mainView->update();
}

void MainWindow::on_outwardNormalsCheckBox_toggled(bool checked) {
  ui->mainView->settings.outwardNormals = checked;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_useEdgeNormalsCheckBox_toggled(bool checked) {
  ui->mainView->settings.edgeTangentSample = checked;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_closedCheckBox_toggled(bool checked) {
  ui->mainView->settings.closed = checked;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_controlPointsCheckBox_toggled(bool checked) {
  ui->mainView->settings.showControlPoints = checked;
  ui->mainView->updateBuffers();
}

void MainWindow::on_controlCurveCheckBox_toggled(bool checked) {
  ui->mainView->settings.showControlCurve = checked;
  ui->mainView->updateBuffers();
}

void MainWindow::on_middlePointWeightSpinBox_valueChanged(double value) {
  ui->mainView->settings.middlePointWeight = value;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_outerPointWeightSpinBox_valueChanged(double value) {
  ui->mainView->settings.outerPointWeight = value;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_middleNormalWeightSpinBox_valueChanged(double value) {
  ui->mainView->settings.middleNormalWeight = value;
  ui->mainView->recalculateCurve();
}

void MainWindow::on_outerNormalWeightSpinBox_valueChanged(double value) {
  ui->mainView->settings.outerNormalWeight = value;
  ui->mainView->recalculateCurve();
}
