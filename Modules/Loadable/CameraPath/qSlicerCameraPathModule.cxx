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
#include <QtPlugin>

// CameraPath Logic includes
#include <vtkSlicerCameraPathLogic.h>

// QTGUI includes
#include <qSlicerApplication.h>
#include <qSlicerIOManager.h>
#include <qSlicerNodeWriter.h>

// CameraPath includes
#include "qSlicerCameraPathModule.h"
#include "qSlicerCameraPathModuleWidget.h"
#include "qSlicerCameraPathReader.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerCameraPathModule, qSlicerCameraPathModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerCameraPathModulePrivate
{
public:
  qSlicerCameraPathModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCameraPathModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCameraPathModulePrivate::qSlicerCameraPathModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCameraPathModule methods

//-----------------------------------------------------------------------------
qSlicerCameraPathModule::qSlicerCameraPathModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCameraPathModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCameraPathModule::~qSlicerCameraPathModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerCameraPathModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerCameraPathModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCameraPathModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCameraPathModule::icon() const
{
  return QIcon(":/Icons/CameraPath.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCameraPathModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCameraPathModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathModule::setup()
{
  this->Superclass::setup();

  // Reader
  qSlicerCameraPathReader* cameraPathReader =
      new qSlicerCameraPathReader(vtkSlicerCameraPathLogic::SafeDownCast(this->logic()), this);

  // Writer
  qSlicerNodeWriter* cameraPathWriter =
      new qSlicerNodeWriter("CameraPath Keyframes",
                            cameraPathReader->fileType(),
                            QStringList() << "vtkMRMLCameraPathNode",
                            false, this);

  // IO Manager
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->registerIO(cameraPathReader);
  ioManager->registerIO(cameraPathWriter);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerCameraPathModule
::createWidgetRepresentation()
{
  return new qSlicerCameraPathModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCameraPathModule::createLogic()
{
  return vtkSlicerCameraPathLogic::New();
}
