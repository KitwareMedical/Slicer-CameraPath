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

// CameraPath Logic includes
#include "vtkSlicerCameraPathLogic.h"
#include "vtkMRMLCameraPathNode.h"
#include "vtkMRMLCameraPathStorageNode.h"
#include "vtkMRMLPointSplineNode.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCameraPathLogic);

//----------------------------------------------------------------------------
vtkSlicerCameraPathLogic::vtkSlicerCameraPathLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerCameraPathLogic::~vtkSlicerCameraPathLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerCameraPathLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerCameraPathLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerCameraPathLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  vtkMRMLScene *scene = this->GetMRMLScene();

  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCameraPathNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCameraPathStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLPointSplineNode>::New());
}

//---------------------------------------------------------------------------
void vtkSlicerCameraPathLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerCameraPathLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (node==NULL)
  {
    vtkErrorMacro("An invalid node is attempted to be added");
    return;
  }
  if (node->IsA("vtkMRMLCameraPathNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Have a vtkMRMLCameraPathNode node");
    vtkUnObserveMRMLNodeMacro(node); // remove any previous observation that might have been added
    vtkObserveMRMLNodeMacro(node);
  }
  else if (node->IsA("vtkMRMLPointSplineNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeAdded: Have a vtkMRMLPointSplineNode node");
    vtkUnObserveMRMLNodeMacro(node); // remove any previous observation that might have been added
    vtkObserveMRMLNodeMacro(node);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerCameraPathLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  if (node==NULL)
  {
    vtkErrorMacro("An invalid node is attempted to be removed");
    return;
  }
  if (node->IsA("vtkMRMLCameraPathNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeRemoved: Have a vtkMRMLCameraPathNode node");
    vtkUnObserveMRMLNodeMacro(node);
  }
  else if (node->IsA("vtkMRMLPointSplineNode"))
  {
    vtkDebugMacro("OnMRMLSceneNodeRemoved: Have a vtkMRMLPointSplineNode node");
    vtkUnObserveMRMLNodeMacro(node);
  }
}

//---------------------------------------------------------------------------
char* vtkSlicerCameraPathLogic::LoadCameraPath(const char *fileName, const char *nodeName)
{
  char *nodeIDs = NULL;
  std::string idList;

  if (!fileName)
    {
    vtkErrorMacro("LoadCameraPath: null file name, cannot load");
    return NULL;
    }
  if (fileName[0] == '\0')
    {
    vtkErrorMacro("LoadCameraPath: empty file name, cannot load");
    return NULL;
    }
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("LoadCameraPath: no MRML scene, cannot load");
    return NULL;
    }

  // turn on batch processing
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  // storage node
  vtkNew<vtkMRMLCameraPathStorageNode> storageNode;
  storageNode->SetFileName(fileName);

  // camera path node
  vtkNew<vtkMRMLCameraPathNode> cameraPathNode;
  cameraPathNode->SetName(nodeName);

  // Disable modified event
  cameraPathNode->DisableModifiedEventOn();

  // adding nodes to scene
  this->GetMRMLScene()->AddNode(storageNode.GetPointer());
  this->GetMRMLScene()->AddNode(cameraPathNode.GetPointer());
  idList += std::string(cameraPathNode->GetID());

  this->GetMRMLScene()->AddNode(cameraPathNode->GetPositionSplines());
  idList += std::string(",");
  idList += std::string(cameraPathNode->GetPositionSplines()->GetID());

  this->GetMRMLScene()->AddNode(cameraPathNode->GetFocalPointSplines());
  idList += std::string(",");
  idList += std::string(cameraPathNode->GetFocalPointSplines()->GetID());

  this->GetMRMLScene()->AddNode(cameraPathNode->GetViewUpSplines());
  idList += std::string(",");
  idList += std::string(cameraPathNode->GetViewUpSplines()->GetID());

  // read the file
  if (!storageNode->ReadData(cameraPathNode.GetPointer()))
    {
    vtkErrorMacro("LoadCameraPath: coud not read data");
    this->GetMRMLScene()->RemoveNode(cameraPathNode.GetPointer());
    this->GetMRMLScene()->RemoveNode(storageNode.GetPointer());
    return NULL;
    }

  // adding camera nodes to scene
  for (vtkIdType i = 0; i < cameraPathNode->GetNumberOfKeyFrames(); ++i)
    {
    vtkMRMLCameraNode* cameraNode = cameraPathNode->GetKeyFrameCamera(i);
    this->GetMRMLScene()->AddNode(cameraNode);
    cameraNode->SetHideFromEditors(1);
    idList += std::string(",");
    idList += std::string(cameraNode->GetID());
    }

  // Enable modified event
  cameraPathNode->DisableModifiedEventOff();
  cameraPathNode->Modified();

  // removing storage node
  this->GetMRMLScene()->RemoveNode(storageNode.GetPointer());

  // turn off batch processing
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);


  // return IDs
  if (idList.length())
    {
    nodeIDs = (char *)malloc(sizeof(char) * (idList.length() + 1));
    strcpy(nodeIDs, idList.c_str());
    }

  return nodeIDs;
}
