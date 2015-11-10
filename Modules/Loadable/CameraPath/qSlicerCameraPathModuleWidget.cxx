/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QTimer>
#include <QInputDialog>
#include <QtGlobal>
#include <QProgressDialog>

// CTK includes
#include "ctkMessageBox.h"
#include "ctkFileDialog.h"

// SlicerQt includes
#include "qSlicerCameraPathModuleWidget.h"
#include "ui_qSlicerCameraPathModuleWidget.h"

// CameraPath includes
#include "vtkSlicerCameraPathLogic.h"
#include "vtkMRMLCameraPathNode.h"

// VTK includes
#include "vtkNew.h"
#include "vtkCamera.h"
#include "vtkMRMLScene.h"

// Test Export
#include "vtkMRMLViewNode.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLLayoutViewFactory.h"
#include "qMRMLThreeDWidget.h"
#include "qMRMLThreeDView.h"
#include "vtkRenderWindow.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkFFMPEGWriter.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCameraPathModuleWidgetPrivate: public Ui_qSlicerCameraPathModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerCameraPathModuleWidget);
protected:
  qSlicerCameraPathModuleWidget* const q_ptr;

public:
  qSlicerCameraPathModuleWidgetPrivate(qSlicerCameraPathModuleWidget& object);
  ~qSlicerCameraPathModuleWidgetPrivate();
  vtkSlicerCameraPathLogic* logic() const;

  void init();

private:

  QTimer* Timer;

};

//-----------------------------------------------------------------------------
// qSlicerCameraPathModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCameraPathModuleWidgetPrivate
::qSlicerCameraPathModuleWidgetPrivate(qSlicerCameraPathModuleWidget &object)
  : q_ptr(&object)
{
  this->Timer = new QTimer();
}

//-----------------------------------------------------------------------------
qSlicerCameraPathModuleWidgetPrivate::~qSlicerCameraPathModuleWidgetPrivate()
{
  delete this->Timer;
}

//-----------------------------------------------------------------------------
vtkSlicerCameraPathLogic*
qSlicerCameraPathModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerCameraPathModuleWidget);

  vtkSlicerCameraPathLogic* logic=vtkSlicerCameraPathLogic::SafeDownCast(q->logic());
  if (logic==NULL)
  {
    qCritical() << "Camera path logic is invalid";
  }
  return logic;
}

//-----------------------------------------------------------------------------
// qSlicerCameraPathModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCameraPathModuleWidget::qSlicerCameraPathModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerCameraPathModuleWidgetPrivate(*this) )
{
    Q_D(qSlicerCameraPathModuleWidget);
}

//-----------------------------------------------------------------------------
qSlicerCameraPathModuleWidget::~qSlicerCameraPathModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::setup()
{
  Q_D(qSlicerCameraPathModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // MRML Nodes combobox
  connect( d->cameraPathComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
           this, SLOT(onCameraPathNodeChanged(vtkMRMLNode*)));
  connect( d->cameraPathComboBox, SIGNAL(currentNodeRenamed(QString)),
           this, SLOT(onCameraPathNodeRenamed(QString)));
  connect( d->cameraPathComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)),
           this, SLOT(onCameraPathNodeAdded(vtkMRMLNode*)));
  connect( d->cameraPathComboBox, SIGNAL(nodeAboutToBeRemoved(vtkMRMLNode*)),
           this, SLOT(onCameraPathNodeRemoved(vtkMRMLNode*)));
  connect( d->cameraPathVisibilityPushButton, SIGNAL(toggled(bool)),
           this, SLOT(onCameraPathVisibilityToggled(bool)) );

  // Slider + play buttons
  connect( d->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(onTimeSliderChanged(int)) );
  connect( d->firstFramePushButton, SIGNAL(clicked()), this, SLOT(onFirstFrameClicked()) );
  connect( d->previousFramePushButton, SIGNAL(clicked()), this, SLOT(onPreviousFrameClicked()) );
  connect( d->playPushButton, SIGNAL(toggled(bool)), this, SLOT(onPlayPauseToogled(bool)) );
  connect( d->nextFramePushButton, SIGNAL(clicked()), this, SLOT(onNextFrameClicked()) );
  connect( d->lastFramePushButton, SIGNAL(clicked()), this, SLOT(onLastFrameClicked()) );
  connect( d->fpsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onFPSChanged(int)) );

  this->setTimerInterval(d->fpsSpinBox->value());
  connect( d->Timer, SIGNAL(timeout()), this, SLOT(playToNextFrame()));

  // Keyframes buttons
  connect( d->deleteAllPushButton, SIGNAL(clicked()), this, SLOT(onDeleteAllClicked()) );
  connect( d->deleteSelectedPushButton, SIGNAL(clicked()), this, SLOT(onDeleteSelectedClicked()) );
  connect( d->goToKeyFramePushButton, SIGNAL(clicked()), this, SLOT(onGoToKeyFrameClicked()) );
  connect( d->updateKeyFramePushButton, SIGNAL(clicked()), this, SLOT(onUpdateKeyFrameClicked()) );
  connect( d->addKeyFramePushButton, SIGNAL(clicked()), this, SLOT(onAddKeyFrameClicked()) );

  // Keyframes table widget
  d->keyFramesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  d->keyFramesTableWidget->setColumnWidth(0,80);
  d->keyFramesTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  d->keyFramesTableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::Interactive);
  d->keyFramesTableWidget->setColumnWidth(2,50);
  d->keyFramesTableWidget->setColumnWidth(3,50);
  connect(d->keyFramesTableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(onCellChanged(int, int)));
  connect(d->keyFramesTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onItemClicked(QTableWidgetItem*)));

  // Export
  connect( d->viewComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
           this, SLOT(onViewNodeChanged(vtkMRMLNode*)));
  connect( d->exportCurrentSizeRadioButton, SIGNAL(clicked(bool)), this, SLOT(onCurrentSizeClicked(bool)) );
  connect( d->exportCustomSizeRadioButton, SIGNAL(clicked(bool)), this, SLOT(onCustomSizeClicked(bool)) );
  connect( d->exportPushButton, SIGNAL(clicked()), this, SLOT(onRecordClicked()) );

}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::setTimerInterval(int framerate)
{
  Q_D(qSlicerCameraPathModuleWidget);

  double msInterval = 1000.0/(double)framerate;
  d->Timer->setInterval(msInterval);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::updateSliderRange()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
      vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  if (!cameraPathNode)
    {
    return;
    }

  if (cameraPathNode->GetNumberOfKeyFrames() < 2)
    {
    d->timeSlider->setMaximum(0);
    return;
    }

  double tmax = cameraPathNode->GetMaximumT();
  double tmin = cameraPathNode->GetMinimumT();
  int framerate = d->fpsSpinBox->value();
  int numberOfFrames = framerate * int(tmax - tmin);
  d->timeSlider->setMaximum(numberOfFrames);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCameraPathNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode = vtkMRMLCameraPathNode::SafeDownCast(node);

  if (!cameraPathNode)
    {
    return;
    }

  // Update Slider Range
  this->updateSliderRange();

  // Empty Table
  this->emptyKeyFramesTableWidget();

  // Update visibility button
  d->cameraPathVisibilityPushButton->blockSignals(true);
  if (cameraPathNode->GetNumberOfKeyFrames() < 2)
    {
    d->cameraPathVisibilityPushButton->setChecked(false);
    d->cameraPathVisibilityPushButton->setEnabled(false);
    return;
    }
  else
    {
    int visibility = cameraPathNode->GetPositionSplines()->GetDisplayVisibility();
    d->cameraPathVisibilityPushButton->setChecked(visibility);
    d->cameraPathVisibilityPushButton->setEnabled(true);
    }
  d->cameraPathVisibilityPushButton->blockSignals(false);

  // Stop here if node empty
  if (cameraPathNode->GetNumberOfKeyFrames() == 0)
    {
    return;
    }

  // Block signals from table
  QTableWidget* table = d->keyFramesTableWidget;
  table->blockSignals(true);

  // Populate Table
  KeyFrameVector keyFrames = cameraPathNode->GetKeyFrames();
  for ( vtkIdType i = 0; i < cameraPathNode->GetNumberOfKeyFrames(); ++i )
    {
    // Get Key frame info
    double t = keyFrames.at(i).Time;
    char* cameraID = keyFrames.at(i).Camera->GetID();

    // Add Key frame in table
    table->insertRow(table->rowCount());

    QTableWidgetItem* timeItem = new QTableWidgetItem();
    timeItem->setData(Qt::DisplayRole,t);
    table->setItem(table->rowCount()-1, 0, timeItem );

    QTableWidgetItem* cameraItem = new QTableWidgetItem(QString(cameraID));
    cameraItem->setFlags(cameraItem->flags() ^ Qt::ItemIsEditable);
    table->setItem(table->rowCount()-1, 1, cameraItem);
    }

  // Unblock signals from table
  table->blockSignals(false);

}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCameraPathNodeRenamed(QString nodeName)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  if (!cameraPathNode)
    {
    return;
    }

  QString positionSplineName(nodeName+"_PositionSpline");
  QString focalPointSplineName(nodeName+"_FocalPointSpline");
  QString viewUpSplineName(nodeName+"_ViewUpSpline");

  cameraPathNode->GetPositionSplines()->SetName(positionSplineName.toStdString().c_str());
  cameraPathNode->GetFocalPointSplines()->SetName(focalPointSplineName.toStdString().c_str());
  cameraPathNode->GetViewUpSplines()->SetName(viewUpSplineName.toStdString().c_str());
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCameraPathNodeAdded(vtkMRMLNode* node)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode = vtkMRMLCameraPathNode::SafeDownCast(node);

  if (!cameraPathNode)
    {
    return;
    }

  // Add PointSplines to Scene
  if (cameraPathNode->GetScene()
      && !cameraPathNode->GetPositionSplines()->GetScene()
      && !cameraPathNode->GetFocalPointSplines()->GetScene()
      && !cameraPathNode->GetViewUpSplines()->GetScene()
      )
    {
    cameraPathNode->GetScene()->AddNode(cameraPathNode->GetPositionSplines());
    cameraPathNode->GetScene()->AddNode(cameraPathNode->GetFocalPointSplines());
    cameraPathNode->GetScene()->AddNode(cameraPathNode->GetViewUpSplines());
    }

  // Update PointSplines Name
  this->onCameraPathNodeRenamed(QString(cameraPathNode->GetName()));

  // Show keyframes section
  d->keyFramesSection->setEnabled(true);
  d->exportSection->setEnabled(true);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCameraPathNodeRemoved(vtkMRMLNode* node)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode = vtkMRMLCameraPathNode::SafeDownCast(node);

  if (!cameraPathNode)
    {
    return;
    }

  // Remove PointSplines from Scene
  if (cameraPathNode->GetScene())
  {
    cameraPathNode->GetScene()->RemoveNode(cameraPathNode->GetPositionSplines());
    cameraPathNode->GetScene()->RemoveNode(cameraPathNode->GetFocalPointSplines());
    cameraPathNode->GetScene()->RemoveNode(cameraPathNode->GetViewUpSplines());
  }

  // Hide keyframes section if last node
  if (d->cameraPathComboBox->nodeCount() == 1)
  {
    d->keyFramesSection->setEnabled(false);
    d->exportSection->setEnabled(false);
  }

  // Empty Tables
  this->emptyKeyFramesTableWidget();
  this->emptyCameraTableWidget();
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCameraPathVisibilityToggled(bool visibility)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  if (!cameraPathNode)
    {
    return;
    }

  cameraPathNode->GetPositionSplines()->SetDisplayVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onTimeSliderChanged(int frameNbr)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  vtkMRMLCameraNode* cameraNode =
          vtkMRMLCameraNode::SafeDownCast(d->defaultCameraComboBox->currentNode());

  if (!cameraPathNode || !cameraNode)
    {
    return;
    }

  // Get Time
  int framerate = d->fpsSpinBox->value();
  double tmin = cameraPathNode->GetMinimumT();
  double t = (frameNbr/(double)framerate) + tmin;

  // Update default camera
  if (cameraPathNode->GetNumberOfKeyFrames() != 0)
    {
    cameraPathNode->GetCameraAt(t, cameraNode);
    cameraNode->GetCamera()->SetClippingRange(0.1,cameraNode->GetCamera()->GetDistance()*6);
    }

  // Update time label
  std::stringstream stream;
  stream << std::fixed << std::setprecision(1) << t;
  std::string s = stream.str();
  d->timeValueLabel->setText(QString::fromStdString(s));

  // Check if time associated with a keyframe
  vtkIdType index = cameraPathNode->KeyFrameIndexAt(t);
  if (index != -1)
    {
    // Select row
    d->keyFramesTableWidget->selectRow(index);
    this->onItemClicked(d->keyFramesTableWidget->item(index,1));
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onFirstFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);
  d->timeSlider->setValue(d->timeSlider->minimum());
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onPreviousFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);
  d->timeSlider->setValue(d->timeSlider->value()-1);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onPlayPauseToogled(bool play)
{
  Q_D(qSlicerCameraPathModuleWidget);

  if (play)
    {
    if(d->timeSlider->value() == d->timeSlider->maximum())
      {
      d->timeSlider->setValue(0);
      }
    d->Timer->start();
    }
  else
    {
    d->Timer->stop();
    }
  d->playPushButton->setChecked(play);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onNextFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);
  d->timeSlider->setValue(d->timeSlider->value()+1);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onLastFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);
  d->timeSlider->setValue(d->timeSlider->maximum());
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onFPSChanged(int framerate)
{
  Q_D(qSlicerCameraPathModuleWidget);
  this->setTimerInterval(framerate);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  if (!cameraPathNode)
    {
    return;
    }

  double tmax = cameraPathNode->GetMaximumT();
  double tmin = cameraPathNode->GetMinimumT();
  int numberOfFrames = framerate * int(tmax - tmin);

  d->timeSlider->setMaximum(numberOfFrames);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::playToNextFrame()
{
  Q_D(qSlicerCameraPathModuleWidget);
  d->timeSlider->setValue(d->timeSlider->value()+1);

  if(d->timeSlider->value() == d->timeSlider->maximum())
    {
    d->Timer->stop();
    d->playPushButton->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onDeleteAllClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  if (!cameraPathNode)
    {
    return;
    }

  // Popup window to ask the delete confirmation
  ctkMessageBox deleteMsgBox;
  deleteMsgBox.setWindowTitle("Delete all Keyframes?");
  QString labelText = QString("Delete all Keyframes?");
  deleteMsgBox.setText(labelText);
  QPushButton *deleteButton =
    deleteMsgBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
  deleteMsgBox.addButton(QMessageBox::Cancel);
  deleteMsgBox.setDefaultButton(deleteButton);
  deleteMsgBox.setIcon(QMessageBox::Question);
  deleteMsgBox.setDontShowAgainVisible(true);
  deleteMsgBox.setDontShowAgainSettingsKey("Keyframes/AlwaysDeleteKeyframes");
  deleteMsgBox.exec();
  if (deleteMsgBox.clickedButton() == deleteButton)
    {
    // Update visibility button
    d->cameraPathVisibilityPushButton->blockSignals(true);
    d->cameraPathVisibilityPushButton->setChecked(false);
    d->cameraPathVisibilityPushButton->setEnabled(false);
    d->cameraPathVisibilityPushButton->blockSignals(false);

    // Remove All Keyframes
    cameraPathNode->RemoveKeyFrames();

    // Empty Keyframe Table
    this->emptyKeyFramesTableWidget();

    // Empty Camera Table
    this->emptyCameraTableWidget();

    // Update Slider Range
    this->updateSliderRange();
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onDeleteSelectedClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  vtkMRMLCameraNode* defaultCameraNode =
          vtkMRMLCameraNode::SafeDownCast(d->defaultCameraComboBox->currentNode());

  if (!cameraPathNode || !defaultCameraNode)
    {
    return;
    }

  // Get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->keyFramesTableWidget->selectedItems();

  // Make sure something is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += 2)
    {
    // get the row
    int row = selectedItems.at(i)->row();
    rows << row;
    }
  // sort the list
  qSort(rows);

  // Popup window to ask the delete confirmation
  ctkMessageBox deleteMsgBox;
  deleteMsgBox.setWindowTitle("Delete selected Keyframes?");
  QString labelText = QString("Delete ")
    + QString::number(rows.size())
    + QString(" Keyframes?");
  deleteMsgBox.setText(labelText);
  QPushButton *deleteButton =
    deleteMsgBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
  deleteMsgBox.addButton(QMessageBox::Cancel);
  deleteMsgBox.setDefaultButton(deleteButton);
  deleteMsgBox.setIcon(QMessageBox::Question);
  deleteMsgBox.setDontShowAgainVisible(true);
  deleteMsgBox.setDontShowAgainSettingsKey("Keyframes/AlwaysDeleteKeyframes");
  deleteMsgBox.exec();
  if (deleteMsgBox.clickedButton() == deleteButton)
    {
    // delete from the end
    for (int i = rows.size() - 1; i >= 0; --i)
      {
      int index = rows.at(i);

      // Remove Keyframe
      cameraPathNode->RemoveKeyFrame(index);

      // Remove Table row
      d->keyFramesTableWidget->removeRow(index);
      }

    // Update visibility button if less than 2 keyframes
    if(cameraPathNode->GetNumberOfKeyFrames() < 2)
      {
      d->cameraPathVisibilityPushButton->blockSignals(true);
      d->cameraPathVisibilityPushButton->setChecked(false);
      d->cameraPathVisibilityPushButton->setEnabled(false);
      d->cameraPathVisibilityPushButton->blockSignals(false);
      }

    // Empty Camera Table
    this->emptyCameraTableWidget();

    // Update Slider Range
    this->updateSliderRange();
    }

  // clear the selection on the table
  d->keyFramesTableWidget->clearSelection();

}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onGoToKeyFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  vtkMRMLCameraNode* defaultCameraNode =
          vtkMRMLCameraNode::SafeDownCast(d->defaultCameraComboBox->currentNode());

  if (!cameraPathNode || !defaultCameraNode)
    {
    return;
    }

  // Get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->keyFramesTableWidget->selectedItems();

  // Make sure that only one is selected
  if (selectedItems.size() != 2)
    {
    return;
    }

  // Get keyframe time
  int row = selectedItems.at(0)->row();
  double t = d->keyFramesTableWidget->item(row, 0)->text().toDouble();

  // Get frame number
  int framerate = d->fpsSpinBox->value();
  double tmin = cameraPathNode->GetMinimumT();
  int frameNbr = framerate * int(t - tmin);

  // Set slider to value
  d->timeSlider->setValue(frameNbr);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onUpdateKeyFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  vtkMRMLCameraNode* defaultCameraNode =
          vtkMRMLCameraNode::SafeDownCast(d->defaultCameraComboBox->currentNode());

  if (!cameraPathNode || !defaultCameraNode)
    {
    return;
    }

  // Get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->keyFramesTableWidget->selectedItems();

  // Make sure that only one is selected
  if (selectedItems.size() != 2)
    {
    return;
    }

  // Popup window to ask the update confirmation
  ctkMessageBox updateMsgBox;
  updateMsgBox.setWindowTitle("Update selected Keyframe?");
  QString labelText = QString("Do you wish to update the selected keyframe to your current camera? ");
  updateMsgBox.setText(labelText);
  QPushButton *updateButton =
    updateMsgBox.addButton(tr("Update"), QMessageBox::AcceptRole);
  updateMsgBox.addButton(QMessageBox::Cancel);
  updateMsgBox.setDefaultButton(updateButton);
  updateMsgBox.setIcon(QMessageBox::Question);
  updateMsgBox.setDontShowAgainVisible(true);
  updateMsgBox.setDontShowAgainSettingsKey("Keyframes/AlwaysUpdateKeyframes");
  updateMsgBox.exec();
  if (updateMsgBox.clickedButton() == updateButton)
    {
    // Get keyframe index
    int index = selectedItems.at(0)->row();

    // Get keyframe camera
    vtkSmartPointer<vtkMRMLCameraNode> keyframeCameraNode =
        cameraPathNode->GetKeyFrameCamera(index);

    // Update keyframe camera
    keyframeCameraNode->DisableModifiedEventOn();
    keyframeCameraNode->CopyWithoutModifiedEvent(defaultCameraNode);
    keyframeCameraNode->SetHideFromEditors(1);
    QString cameraPathName(cameraPathNode->GetName());
    QString cameraName(cameraPathName+"_Camera");
    keyframeCameraNode->SetName(cameraName.toStdString().c_str());
    keyframeCameraNode->DisableModifiedEventOff();
    keyframeCameraNode->Modified();

    // XXX Update splines
    cameraPathNode->SetKeyFrameCamera(index,keyframeCameraNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onAddKeyFrameClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  vtkMRMLCameraNode* defaultCameraNode =
          vtkMRMLCameraNode::SafeDownCast(d->defaultCameraComboBox->currentNode());

  if (!cameraPathNode || !defaultCameraNode)
    {
    return;
    }

  // Default time value
  double t = 0.0;
  if (cameraPathNode->GetNumberOfKeyFrames())
    {
    t = cameraPathNode->GetMaximumT() + 2.0;
    }

  // Ask time to user
  bool addButtonPressed;
  t = QInputDialog::getDouble(this,
                              tr("Add Keyframe"),
                              tr("Add Keyframe to time below (s) :"),
                              t, -1000000, 1000000, 1,
                              &addButtonPressed);
  if (!addButtonPressed)
    {
    return;
    }
  vtkIdType index = cameraPathNode->KeyFrameIndexAt(t);
  if(index != -1)
    {
    // Popup window to inform that time already associated
    // with other keyframe
    this->showErrorTimeMsgBox(t,index);
    return;
    }

  // Create new camera
  vtkNew<vtkMRMLCameraNode> newCameraNode;
  newCameraNode->Copy(defaultCameraNode);
  newCameraNode->SetHideFromEditors(1);
  QString cameraPathName(cameraPathNode->GetName());
  QString newCameraName(cameraPathName+"_Camera");
  newCameraNode->SetName(newCameraName.toStdString().c_str());
  cameraPathNode->GetScene()->AddNode(newCameraNode.GetPointer());

  // Listen to new camera
  this->qvtkConnect(newCameraNode.GetPointer(), vtkCommand::ModifiedEvent,
                    this, SLOT(onKeyFrameCameraModified(vtkObject*)));

  // Add key frame
  cameraPathNode->AddKeyFrame(t, newCameraNode.GetPointer());

  // Update Slider Range
  this->updateSliderRange();

  // Update visibility button
  if(cameraPathNode->GetNumberOfKeyFrames() == 2)
    {
    d->cameraPathVisibilityPushButton->blockSignals(true);
    d->cameraPathVisibilityPushButton->setChecked(true);
    d->cameraPathVisibilityPushButton->setEnabled(true);
    d->cameraPathVisibilityPushButton->blockSignals(false);
    }

  // Block signals from table
  d->keyFramesTableWidget->blockSignals(true);

  // Add Key frame in table
  QTableWidget* table = d->keyFramesTableWidget;
  table->insertRow(table->rowCount());
  QTableWidgetItem* timeItem = new QTableWidgetItem();
  timeItem->setData(Qt::DisplayRole,t);
  table->setItem(table->rowCount()-1, 0, timeItem );
  QTableWidgetItem* cameraItem = new QTableWidgetItem(QString(newCameraNode->GetID()));
  cameraItem->setFlags(cameraItem->flags() ^ Qt::ItemIsEditable);
  table->setItem(table->rowCount()-1, 1, cameraItem);

  // Select new row
  table->selectRow(table->rowCount()-1);
  this->onItemClicked(cameraItem);

  // Sort Table
  table->sortByColumn(0,Qt::AscendingOrder);

  // Unblock signals from table
  table->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCellChanged(int row, int col)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  if (!cameraPathNode || col != 0)
    {
    return;
    }

  // Get new time
  double time = d->keyFramesTableWidget->item(row, col)->text().toDouble();

  // Check if time already used
  vtkIdType index = cameraPathNode->KeyFrameIndexAt(time);
  if(index != -1)
    {
    this->showErrorTimeMsgBox(time,index);
    // XXX Need to set back the former value!!
    return;
    }

  // Set Key Frame Time
  cameraPathNode->SetKeyFrameTime(row,time);

  // Sort Table
  d->keyFramesTableWidget->sortByColumn(0,Qt::AscendingOrder);

  // Update Slider Range
  this->updateSliderRange();
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onItemClicked(QTableWidgetItem* item)
{
  if(!item)
    {
    return;
    }

  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());
  if (!cameraPathNode)
    {
    return;
    }

  // Get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->keyFramesTableWidget->selectedItems();

  // Make sure that only one is selected
  if (selectedItems.size() != 2)
    {
    this->emptyCameraTableWidget();
    return;
    }

  // Get Keyframe index
  int index = item->row();

  // Set selected camera
  d->selectedCameraIDLineEdit->setText(d->keyFramesTableWidget->item(index, 1)->text());

  // Update camera table
  d->cameraTableWidget->setEnabled(true);
  this->updateCameraTable(index);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onKeyFrameCameraModified(vtkObject *caller)
{
  if (!caller)
    {
    return;
    }

  vtkMRMLCameraNode* camera = vtkMRMLCameraNode::SafeDownCast(caller);
  if(!camera)
    {
    return;
    }

  Q_D(qSlicerCameraPathModuleWidget);

  // Get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->keyFramesTableWidget->selectedItems();

  // Make sure that only one is selected
  if (selectedItems.size() != 2)
    {
    return;
    }

  // Get keyframe index
  int index = selectedItems.at(0)->row();

  // Update camera table
  this->updateCameraTable(index);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onViewNodeChanged(vtkMRMLNode* node)
{
  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
  if (!viewNode)
    {
    return;
    }

  vtkRenderWindow* renderWindow = this->getMRMLViewRenderWindow(viewNode);

  // Listen to renderWindow
  // XXX TODO : remove connection when viewnode changed
  // (store the current renderwindow to do this?)
  this->qvtkConnect(renderWindow, vtkCommand::ModifiedEvent,
                    this, SLOT(onRenderWindowModified(vtkObject*)));
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onRenderWindowModified(vtkObject *caller)
{
  if (!caller)
    {
    return;
    }

  vtkRenderWindow* renderWindow = vtkRenderWindow::SafeDownCast(caller);
  if(!renderWindow)
    {
    return;
    }

  Q_D(qSlicerCameraPathModuleWidget);

  // Update size boxes
  if(d->exportCurrentSizeRadioButton->isChecked())
    {
    int* windowSize = renderWindow->GetSize();
    d->exportWidthSpinBox->setValue(windowSize[0]);
    d->exportHeightSpinBox->setValue(windowSize[1]);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCurrentSizeClicked(bool checked)
{
  Q_D(qSlicerCameraPathModuleWidget);

  if (checked)
    {
    // Disable size boxes
    d->exportWidthSpinBox->setEnabled(false);
    d->exportHeightSpinBox->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onCustomSizeClicked(bool checked)
{
  Q_D(qSlicerCameraPathModuleWidget);

  if (checked)
    {
    d->exportWidthSpinBox->setEnabled(true);
    d->exportHeightSpinBox->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onRecordClicked()
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());

  vtkMRMLViewNode* viewNode =
          vtkMRMLViewNode::SafeDownCast(d->viewComboBox->currentNode());

  if (!cameraPathNode || !viewNode)
    {
    return;
    }

  // Get Render Window
  vtkRenderWindow* renderWindow = this->getMRMLViewRenderWindow(viewNode);
  if (!renderWindow)
    {
    return;
    }

  // Get Export type and Export quality
  const int exportType = d->exportTypeComboBox->currentIndex();
  const int exportQuality = d->exportQualityComboBox->currentIndex();

  // Get File name
  QString fileName;
  QString path;
  QString baseName;
  QString suffix;
  if (exportType == VIDEOCLIP)
    {
    fileName = ctkFileDialog::getSaveFileName(this, tr("Save Video Clip"),".",tr("Videos (*.mkv *.avi)"));
    QFileInfo fileInfo(fileName);
    path = fileInfo.path();
    suffix = fileInfo.suffix();
    if(!fileName.isEmpty() && suffix != "mkv" && suffix != "avi")
      {
      qWarning() << "Extension incorrect, using .mkv instead";
      fileName += ".mkv";
      }
    }
  else if (exportType == SCREENSHOTS)
    {
    fileName = ctkFileDialog::getSaveFileName(this, tr("Save Screenshot Files"),".",tr("Images (*.png)"));
    QFileInfo fileInfo(fileName);
    path = fileInfo.path();
    baseName = fileInfo.baseName();
    suffix = fileInfo.suffix();
    if(!fileName.isEmpty() && suffix != "png" && suffix != "PNG")
      {
      qWarning() << "Extension incorrect, using .png instead";
      suffix = "png";
      }
    }

  if (fileName.isEmpty())
    {
    return;
    }

  // Enable OffScreen Rendering
  renderWindow->OffScreenRenderingOn();

  // Set renderWindow size
  int* windowSize = renderWindow->GetSize();
  int W = windowSize[0];
  int H = windowSize[1];
  if(d->exportCustomSizeRadioButton->isChecked())
    {
    renderWindow->SetSize(d->exportWidthSpinBox->value(),d->exportHeightSpinBox->value());
    }

  // Get Image
  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(renderWindow);

  // Create Writers
  vtkNew<vtkPNGWriter> imageWriter;
  if(exportType == SCREENSHOTS)
    {
    imageWriter->SetInputConnection(w2i->GetOutputPort());
    switch(exportQuality){
    case LOW:
      imageWriter->SetCompressionLevel(9);
      break;
    case MEDIUM:
      imageWriter->SetCompressionLevel(5);
      break;
    case HIGH:
      imageWriter->SetCompressionLevel(1);
      break;
      }
    }

  vtkNew<vtkFFMPEGWriter> FFMPEGWriter;
  if(exportType == VIDEOCLIP)
    {
    FFMPEGWriter->SetInputConnection(w2i->GetOutputPort());
    FFMPEGWriter->SetQuality(exportQuality);
    FFMPEGWriter->SetFileName(fileName.toStdString().c_str());
    FFMPEGWriter->SetRate(d->fpsSpinBox->value());
    FFMPEGWriter->Start();
    }

  // Create progress dialog
  d->flyThroughSection->setEnabled(false);
  d->keyFramesSection->setEnabled(false);
  d->exportSection->setEnabled(false);
  QProgressDialog progressDialog("Export to "+path, "Cancel",
                           d->timeSlider->minimum(), d->timeSlider->maximum(),
                           this);
  progressDialog.show();
  progressDialog.setWindowModality(Qt::WindowModal);

  // Write frame by frame
  for(int i = d->timeSlider->minimum();
      i <= d->timeSlider->maximum();
      ++i)
  {
    // Render at next frame value
    d->timeSlider->setValue(i);
    renderWindow->Render();

    // Update filter
    w2i->Modified();
    w2i->Update();

    // Write screenshot
    if(exportType == SCREENSHOTS)
      {
      std::stringstream ss;
      ss << path.toStdString() << "/"
         << baseName.toStdString() << "_" << i
         << "." << suffix.toStdString();
      const std::string s = ss.str();
      const char* screenshotFileName = s.c_str();
      qDebug() <<"Writing at "<< screenshotFileName;
      imageWriter->SetFileName(screenshotFileName);
      imageWriter->Write();
      }

    // Write video frame
    if(exportType == VIDEOCLIP)
      {
      qDebug() <<"Writing frame "<< i << "/" << d->timeSlider->maximum();
      FFMPEGWriter->Write();
      }

    // Update progress dialog
    progressDialog.setValue(i);
    if (progressDialog.wasCanceled())
      {
      qWarning() <<"Export canceled";
      d->flyThroughSection->setEnabled(true);
      d->keyFramesSection->setEnabled(true);
      d->exportSection->setEnabled(true);
      break;
      }
  }

  // End video
  if(exportType == VIDEOCLIP)
    {
    FFMPEGWriter->End();
    }

  // Reset renderwindow size
  renderWindow->SetSize(W, H);

  // Back to screen rendering
  renderWindow->OffScreenRenderingOff();

  // Enable module widget
  d->flyThroughSection->setEnabled(true);
  d->keyFramesSection->setEnabled(true);
  d->exportSection->setEnabled(true);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::emptyKeyFramesTableWidget()
{
  Q_D(qSlicerCameraPathModuleWidget);

  while (d->keyFramesTableWidget->rowCount() > 0)
    {
    d->keyFramesTableWidget->removeRow(0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::emptyCameraTableWidget()
{
  Q_D(qSlicerCameraPathModuleWidget);

  d->selectedCameraIDLineEdit->setText("Please select one keyframe");
  d->cameraTableWidget->clearContents();
  d->cameraTableWidget->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::updateCameraTable(int index)
{
  Q_D(qSlicerCameraPathModuleWidget);

  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(d->cameraPathComboBox->currentNode());
  if (!cameraPathNode)
    {
    return;
    }

  // Get keyframe camera info
  vtkSmartPointer<vtkMRMLCameraNode> keyframeCameraNode =
      cameraPathNode->GetKeyFrameCamera(index);

  double** cameraInfo = new double*[3];
  for(int i = 0; i < 3; ++i)
    {
    cameraInfo[i] = new double[3];
    }
  keyframeCameraNode->GetPosition(cameraInfo[0]);
  keyframeCameraNode->GetFocalPoint(cameraInfo[1]);
  keyframeCameraNode->GetViewUp(cameraInfo[2]);

  // Block signals from table
  d->cameraTableWidget->blockSignals(true);

  // Update info in table
  for(unsigned int i = 0; i < 3; ++i)
    {
    for(unsigned int j = 0; j < 3; ++j)
      {
      QTableWidgetItem* item = new QTableWidgetItem();
      item->setData(Qt::DisplayRole,cameraInfo[i][j]);
      item->setFlags(item->flags() ^ Qt::ItemIsEditable);
      d->cameraTableWidget->setItem(i, j, item );
      }
   }

  // Unblock signals from table
  d->cameraTableWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::showErrorTimeMsgBox(double time, vtkIdType index)
{
  // Popup window to inform that time already associated
  // with other keyframe
  ctkMessageBox errorTimeMsgBox;
  errorTimeMsgBox.setWindowTitle("Error Time selected");
  QString labelText =
      QString("A keyframe already exists for t = ")
      + QString::number(time)
      + QString(" at row # ")
      + QString::number(index);
  errorTimeMsgBox.setText(labelText);
  QPushButton *okButton =
    errorTimeMsgBox.addButton(tr("Ok"), QMessageBox::AcceptRole);
  errorTimeMsgBox.setDefaultButton(okButton);
  errorTimeMsgBox.setIcon(QMessageBox::Warning);
  errorTimeMsgBox.exec();
}

//-----------------------------------------------------------------------------
vtkRenderWindow* qSlicerCameraPathModuleWidget::getMRMLViewRenderWindow(vtkMRMLViewNode* viewNode)
{
//  const char* viewNodeID = cameraNode->GetActiveTag();
//  vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(cameraNode->GetScene()->GetNodeByID(viewNodeID));

  qSlicerLayoutManager *layoutManager = qSlicerApplication::application()->layoutManager();
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(layoutManager->mrmlViewFactory("vtkMRMLViewNode")->viewWidget(viewNode));
  vtkRenderWindow *renderWindow = threeDWidget->threeDView()->renderWindow();

  return renderWindow;
}
