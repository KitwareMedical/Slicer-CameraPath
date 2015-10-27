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

#ifndef __qSlicerCameraPathModuleWidget_h
#define __qSlicerCameraPathModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerCameraPathModuleExport.h"

class qSlicerCameraPathModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_CAMERAPATH_EXPORT qSlicerCameraPathModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerCameraPathModuleWidget(QWidget *parent=0);
  virtual ~qSlicerCameraPathModuleWidget();

public slots:


  void onDefaultCameraNodeChanged(vtkMRMLNode* node);
  void onCameraPathNodeChanged(vtkMRMLNode* inputNode);
  void onCameraPathVisibilityToggled(bool visibility);
  void onTimeSliderChanged(int frameNbr);
  void onFirstFrameClicked();
  void onPreviousFrameClicked();
  void onPlayPauseToogled(bool play);
  void onNextFrameClicked();
  void onLastFrameClicked();
  void onFPSChanged(int framerate);
  void playToNextFrame();

protected:
  QScopedPointer<qSlicerCameraPathModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerCameraPathModuleWidget);
  Q_DISABLE_COPY(qSlicerCameraPathModuleWidget);

  void setTimerInterval(int framerate);
  double getFrameTime(int frameNbr);
};

#endif
