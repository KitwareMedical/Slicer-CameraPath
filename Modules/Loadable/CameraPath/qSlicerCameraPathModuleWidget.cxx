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

// SlicerQt includes
#include "qSlicerCameraPathModuleWidget.h"
#include "ui_qSlicerCameraPathModuleWidget.h"

// CameraPath includes
#include "vtkSlicerCameraPathLogic.h"
#include "vtkMRMLCameraPathNode.h"

// VTK includes
#include "vtkNew.h"
#include "vtkMRMLScene.h"

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

  connect( d->defaultCameraComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
           this, SLOT(onDefaultCameraNodeChanged(vtkMRMLNode*)));
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

  connect( d->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(onTimeSliderChanged(int)) );
  connect( d->firstFramePushButton, SIGNAL(clicked()), this, SLOT(onFirstFrameClicked()) );
  connect( d->previousFramePushButton, SIGNAL(clicked()), this, SLOT(onPreviousFrameClicked()) );
  connect( d->playPushButton, SIGNAL(toggled(bool)), this, SLOT(onPlayPauseToogled(bool)) );
  connect( d->nextFramePushButton, SIGNAL(clicked()), this, SLOT(onNextFrameClicked()) );
  connect( d->lastFramePushButton, SIGNAL(clicked()), this, SLOT(onLastFrameClicked()) );
  connect( d->fpsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onFPSChanged(int)) );

  this->setTimerInterval(d->fpsSpinBox->value());
  connect( d->Timer, SIGNAL(timeout()), this, SLOT(playToNextFrame()));

  connect( d->deleteAllPushButton, SIGNAL(clicked()), this, SLOT(onDeleteAllClicked()) );
  connect( d->deleteSelectedPushButton, SIGNAL(clicked()), this, SLOT(onDeleteSelectedClicked()) );
  connect( d->goToKeyFramePushButton, SIGNAL(clicked()), this, SLOT(onGoToKeyFrameClicked()) );
  connect( d->updateKeyFramePushButton, SIGNAL(clicked()), this, SLOT(onUpdateKeyFrameClicked()) );
  connect( d->addKeyFramePushButton, SIGNAL(clicked()), this, SLOT(onAddKeyFrameClicked()) );

  d->keyFramesTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  d->keyFramesTableWidget->setColumnWidth(0,80);
  d->keyFramesTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  d->keyFramesTableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::Interactive);
  d->keyFramesTableWidget->setColumnWidth(2,50);
  d->keyFramesTableWidget->setColumnWidth(3,50);
  connect(d->keyFramesTableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(onCellChanged(int, int)));
//  connect(d->keyFramesTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onCellClicked(QTableWidgetItem*)));
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::setTimerInterval(int framerate)
{
  Q_D(qSlicerCameraPathModuleWidget);

  double msInterval = 1000.0/(double)framerate;
  d->Timer->setInterval(msInterval);
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onDefaultCameraNodeChanged(vtkMRMLNode* node)
{
  //vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(node);
  //TODO
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

  // Slider min/max
  double tmax = cameraPathNode->GetMaximumT();
  double tmin = cameraPathNode->GetMinimumT();
  int framerate = d->fpsSpinBox->value();
  int numberOfFrames = framerate * int(tmax - tmin);
  d->timeSlider->setMaximum(numberOfFrames);

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
  }

  // Empty Table
  this->emptyKeyFramesTableWidget();
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

  int framerate = d->fpsSpinBox->value();
  double tmin = cameraPathNode->GetMinimumT();
  double t = (frameNbr/(double)framerate) + tmin;

  if (cameraPathNode->GetNumberOfKeyFrames() != 0)
    {
    cameraPathNode->GetCameraAt(t, cameraNode);
    cameraNode->ResetClippingRange();
    }

  std::stringstream stream;
  stream << std::fixed << std::setprecision(1) << t;
  std::string s = stream.str();
  d->timeValueLabel->setText(QString::fromStdString(s));
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

  // Remove All Keyframes
  cameraPathNode->RemoveKeyFrames();

  // Update Slider range
  this->onCameraPathNodeChanged(cameraPathNode);

  // Empty Table
  this->emptyKeyFramesTableWidget();
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onDeleteSelectedClicked()
{
 //TODO
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModuleWidget::onGoToKeyFrameClicked()
{
 //TODO
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

  // Get keyframe index
  int index = selectedItems.at(0)->row();

  // Get keyframe camera
  vtkSmartPointer<vtkMRMLCameraNode> keyframeCameraNode =
      cameraPathNode->GetKeyFrameCamera(index);

  // Update keyframe camera
  keyframeCameraNode->Copy(defaultCameraNode);
  keyframeCameraNode->SetHideFromEditors(1);
  QString cameraPathName(cameraPathNode->GetName());
  QString cameraName(cameraPathName+"_Camera");
  keyframeCameraNode->SetName(cameraName.toStdString().c_str());

  // XXX Update splines
  cameraPathNode->SetKeyFrameCamera(index,keyframeCameraNode);
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

  // Create new camera
  vtkNew<vtkMRMLCameraNode> newCameraNode;
  newCameraNode->Copy(defaultCameraNode);
  newCameraNode->SetHideFromEditors(1);
  QString cameraPathName(cameraPathNode->GetName());
  QString newCameraName(cameraPathName+"_Camera");
  newCameraNode->SetName(newCameraName.toStdString().c_str());
  cameraPathNode->GetScene()->AddNode(newCameraNode.GetPointer());

  // Add key frame to t = tmax + 2s
  double t = 0;
  if (cameraPathNode->GetNumberOfKeyFrames() > 0)
    {
    t = cameraPathNode->GetMaximumT() + 2.0;
    }
  cameraPathNode->AddKeyFrame(t, newCameraNode.GetPointer());

  // Update Slider range
  this->onCameraPathNodeChanged(cameraPathNode);

  // Add Key frame in table
  QTableWidget* table = d->keyFramesTableWidget;
  table->insertRow(table->rowCount());
  QTableWidgetItem* timeItem = new QTableWidgetItem();
  timeItem->setData(Qt::DisplayRole,t);
  table->setItem(table->rowCount()-1, 0, timeItem );
  QTableWidgetItem* cameraItem = new QTableWidgetItem(QString(newCameraNode->GetID()));
  cameraItem->setFlags(cameraItem->flags() ^ Qt::ItemIsEditable);
  table->setItem(table->rowCount()-1, 1, cameraItem);
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

  // Set Key Frame Time
  double time = d->keyFramesTableWidget->item(row, col)->text().toDouble();
  cameraPathNode->SetKeyFrameTime(row,time);

  // Sort Table
  d->keyFramesTableWidget->sortByColumn(0,Qt::AscendingOrder);

  // Update Slider range
  this->onCameraPathNodeChanged(cameraPathNode);
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

