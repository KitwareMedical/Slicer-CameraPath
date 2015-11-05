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

#include "vtkMRMLCameraPathStorageNode.h"
#include "vtkMRMLCameraPathNode.h"

#include "vtkMRMLScene.h"
#include "vtkSlicerVersionConfigure.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <vtksys/SystemTools.hxx>

#include <sstream>
#include <iostream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCameraPathStorageNode);

//----------------------------------------------------------------------------
vtkMRMLCameraPathStorageNode::vtkMRMLCameraPathStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLCameraPathStorageNode::~vtkMRMLCameraPathStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathStorageNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathStorageNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathStorageNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
bool vtkMRMLCameraPathStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLCameraPathNode");
}

//----------------------------------------------------------------------------
int vtkMRMLCameraPathStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  if (!refNode)
    {
    vtkErrorMacro("ReadDataInternal: null reference node!");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLCameraPathStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLCameraPathNode *cameraPathNode =
    vtkMRMLCameraPathNode::SafeDownCast(refNode);
  if (!cameraPathNode)
    {
    return 0;
    }

  // open the file for reading input
  fstream fstr;
  fstr.open(fullName.c_str(), fstream::in);
  if (!fstr.is_open())
    {
    vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for reading");
    return 0;
    }

  // clear out the list
  if (cameraPathNode->GetNumberOfKeyFrames() > 0)
    {
    cameraPathNode->RemoveKeyFrames();
    }

  char line[1024];

  // read file
  while (fstr.good())
    {
    fstr.getline(line, 1024);

    // does it start with a #?
    if (line[0] == '#')
      {
      vtkDebugMacro("Comment line, checking:\n\"" << line << "\"");
      }
    // is it empty?
    else if (line[0] == '\0')
      {
      vtkDebugMacro("Empty line, skipping:\n\"" << line << "\"");
      }
    // keyframe line
    else
      {
      std::stringstream ss(line);

      double time;
      double position[3];
      double focalPoint[3];
      double viewUp[3];

      time = ReadNextLineComponentAsDouble(&ss);
      position[0] = ReadNextLineComponentAsDouble(&ss);
      position[1] = ReadNextLineComponentAsDouble(&ss);
      position[2] = ReadNextLineComponentAsDouble(&ss);
      focalPoint[0] = ReadNextLineComponentAsDouble(&ss);
      focalPoint[1] = ReadNextLineComponentAsDouble(&ss);
      focalPoint[2] = ReadNextLineComponentAsDouble(&ss);
      viewUp[0] = ReadNextLineComponentAsDouble(&ss);
      viewUp[1] = ReadNextLineComponentAsDouble(&ss);
      viewUp[2] = ReadNextLineComponentAsDouble(&ss);

      cameraPathNode->AddKeyFrame(time, position, focalPoint, viewUp);
      }
    }
  fstr.close();
  return 1;
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathStorageNode::ReadNextLineComponentAsDouble(std::stringstream *ss)
{
  if(!ss)
  {
    vtkErrorMacro("ReadNextLineComponentAsDouble : Empty line");
    return 0.0;
  }

  std::string component;
  getline(*ss, component, ',');

  if (component.empty())
    {
    vtkErrorMacro("ReadNextLineComponentAsDouble : Empty component");
    return 0.0;
    }

  return atof(component.c_str());
}

//----------------------------------------------------------------------------
int vtkMRMLCameraPathStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  // check the filename
  std::string fullName = this->GetFullNameFromFileName();
  if (fullName.empty())
    {
    vtkErrorMacro("vtkMRMLCameraPathStorageNode: File name not specified");
    return 0;
    }
  vtkDebugMacro("WriteDataInternal: have file name " << fullName.c_str());

  // cast the input node
  vtkMRMLCameraPathNode* cameraPathNode =
          vtkMRMLCameraPathNode::SafeDownCast(refNode);
  if (!cameraPathNode)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known CameraPath node");
    return 0;
    }

  // open the file for writing
  fstream of;
  of.open(fullName.c_str(), fstream::out);
  if (!of.is_open())
    {
    vtkErrorMacro("WriteData: unable to open file " << fullName.c_str() << " for writing");
    return 0;
    }

  // put down a header
  of << "# CameraPath  file version = " << Slicer_VERSION << endl;

  // label the columns
  of << "# columns = time,posX,posY,posZ,focX,focY,focZ,viewX,viewY,viewZ" << endl;

  // add the keyframes
  for (vtkIdType i = 0; i < cameraPathNode->GetNumberOfKeyFrames(); i++)
    {
    double time = cameraPathNode->GetKeyFrameTime(i);
    of << time;

    double position[3];
    cameraPathNode->GetKeyFramePosition(i,position);
    of << "," << position[0] << "," << position[1] << "," << position[2];

    double focalPoint[3];
    cameraPathNode->GetKeyFrameFocalPoint(i,focalPoint);
    of << "," << focalPoint[0] << "," << focalPoint[1] << "," << focalPoint[2];

    double viewUp[3];
    cameraPathNode->GetKeyFrameViewUp(i,viewUp);
    of << "," << viewUp[0] << "," << viewUp[1] << "," << viewUp[2];

    of << endl;
    }

  of.close();
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Keyframes  CSV (.kcsv)");
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Keyframes  CSV (.kcsv)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLCameraPathStorageNode::GetDefaultWriteFileExtension()
{
  return "kcsv";
}
