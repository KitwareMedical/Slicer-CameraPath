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

// SlicerQt includes
#include "qSlicerCameraPathModuleWidget.h"
#include "ui_qSlicerCameraPathModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCameraPathModuleWidgetPrivate: public Ui_qSlicerCameraPathModuleWidget
{
public:
  qSlicerCameraPathModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCameraPathModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCameraPathModuleWidgetPrivate::qSlicerCameraPathModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCameraPathModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCameraPathModuleWidget::qSlicerCameraPathModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerCameraPathModuleWidgetPrivate )
{
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
}
