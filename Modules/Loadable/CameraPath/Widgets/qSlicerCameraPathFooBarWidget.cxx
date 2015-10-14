/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerCameraPathFooBarWidget.h"
#include "ui_qSlicerCameraPathFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CameraPath
class qSlicerCameraPathFooBarWidgetPrivate
  : public Ui_qSlicerCameraPathFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerCameraPathFooBarWidget);
protected:
  qSlicerCameraPathFooBarWidget* const q_ptr;

public:
  qSlicerCameraPathFooBarWidgetPrivate(
    qSlicerCameraPathFooBarWidget& object);
  virtual void setupUi(qSlicerCameraPathFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerCameraPathFooBarWidgetPrivate
::qSlicerCameraPathFooBarWidgetPrivate(
  qSlicerCameraPathFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerCameraPathFooBarWidgetPrivate
::setupUi(qSlicerCameraPathFooBarWidget* widget)
{
  this->Ui_qSlicerCameraPathFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerCameraPathFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerCameraPathFooBarWidget
::qSlicerCameraPathFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerCameraPathFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerCameraPathFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerCameraPathFooBarWidget
::~qSlicerCameraPathFooBarWidget()
{
}
