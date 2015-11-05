/*==============================================================================

  Program: 3D Slicer

  Copyright (c) BWH

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerCameraPathReader
#define __qSlicerCameraPathReader

// SlicerQt includes
#include "qSlicerFileReader.h"

class qSlicerCameraPathReaderPrivate;
class vtkSlicerCameraPathLogic;

//----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CameraPath
class qSlicerCameraPathReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerCameraPathReader(QObject* parent = 0);
  qSlicerCameraPathReader(vtkSlicerCameraPathLogic* logic, QObject* parent = 0);
  virtual ~qSlicerCameraPathReader();

  vtkSlicerCameraPathLogic* CameraPathLogic()const;
  void setCameraPathLogic(vtkSlicerCameraPathLogic* logic);

  virtual QString description()const;
  virtual IOFileType fileType()const;
  virtual QStringList extensions()const;

  virtual bool load(const IOProperties& properties);

protected:
  QScopedPointer<qSlicerCameraPathReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCameraPathReader);
  Q_DISABLE_COPY(qSlicerCameraPathReader);
};

#endif
