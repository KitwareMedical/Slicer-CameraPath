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
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerCameraPathReader.h"

// Logic includes
#include <vtkSlicerApplicationLogic.h>
#include "vtkSlicerCameraPathLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerCameraPathReaderPrivate
{
  public:
  vtkSmartPointer<vtkSlicerCameraPathLogic> CameraPathLogic;
};

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotations
//-----------------------------------------------------------------------------
qSlicerCameraPathReader::qSlicerCameraPathReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCameraPathReaderPrivate)
{
}

qSlicerCameraPathReader::qSlicerCameraPathReader(vtkSlicerCameraPathLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCameraPathReaderPrivate)
{
  this->setCameraPathLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerCameraPathReader::~qSlicerCameraPathReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerCameraPathReader::setCameraPathLogic(vtkSlicerCameraPathLogic* logic)
{
  Q_D(qSlicerCameraPathReader);
  d->CameraPathLogic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerCameraPathLogic* qSlicerCameraPathReader::CameraPathLogic()const
{
  Q_D(const qSlicerCameraPathReader);
  return d->CameraPathLogic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerCameraPathReader::description()const
{
  return "CameraPath Keyframes";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerCameraPathReader::fileType()const
{
  return QString("CameraPath");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCameraPathReader::extensions()const
{
  return QStringList()
    << "Keyframes (*.kcsv)";
}

//-----------------------------------------------------------------------------
bool qSlicerCameraPathReader::load(const IOProperties& properties)
{
  Q_D(qSlicerCameraPathReader);

  // get the properties
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }

  if (d->CameraPathLogic.GetPointer() == nullptr)
    {
    return false;
    }

  // pass to logic to do the loading
  char* nodeIDs = d->CameraPathLogic->LoadCameraPath(
        fileName.toLatin1(), name.toLatin1());

  if (nodeIDs)
    {
    // returned a comma separated list of ids of the nodes that were loaded
    QStringList nodeIDList;
    char *ptr = strtok(nodeIDs, ",");

    while (ptr)
      {
      nodeIDList.append(ptr);
      ptr = strtok(nullptr, ",");
      }
    this->setLoadedNodes(nodeIDList);
    }
  else
    {
    this->setLoadedNodes(QStringList());
    return false;
    }

  return nodeIDs != nullptr;
}
