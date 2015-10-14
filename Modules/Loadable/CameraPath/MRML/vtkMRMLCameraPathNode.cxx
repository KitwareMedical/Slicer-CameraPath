// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLCameraPathNode.h"
//#include "vtkMRMLCameraPathStorageNode.h"

// VTK includes
#include <vtkMRMLPointSplineNode.h>
#include <vtkNew.h>

// STD includes
#include <sstream>
#include <pair>
#include <vector>


//------------------------------------------------------------------------------
// timeComp

//----------------------------------------------------------------------------
struct timeSort
{
    timeSort() : { }
    bool operator ()(const vtkMRMLCameraPathNode::KeyFrameType& firstElem,
                     const vtkMRMLCameraPathNode::KeyFrameType& secondElem)
      {
      return firstElem.second < secondElem.second;
      }
};
//----------------------------------------------------------------------------
struct timeEqual
{
    timeEqual(double const& time) : Time(time) { }
    double Time;

    bool operator ()(const vtkMRMLCameraPathNode::KeyFrameType& elem)
      {
      return elem.second == Time;
      }
};

//------------------------------------------------------------------------------
// vtkMRMLCameraPathNode::vtkInternal

//------------------------------------------------------------------------------
class vtkMRMLCameraPathNode::vtkInternal
{
public:
  vtkInternal(vtkMRMLCameraPathNode* external);
  ~vtkInternal();

  int PathStatus;
  KeyFramesType KeyFrames;
  vtkSmartPointer<vtkMRMLPointSplineNode> Positions;
  vtkSmartPointer<vtkMRMLPointSplineNode> FocalPoints;
  vtkSmartPointer<vtkMRMLPointSplineNode> ViewUps;

  vtkMRMLCameraPathNode* External;
};

//------------------------------------------------------------------------------
vtkMRMLCameraPathNode::vtkInternal::vtkInternal(
    vtkMRMLCameraPathNode* external):External(external)
{
  this->PathStatus = PATH_NOT_CREATED;
//  this->KeyFrames = new KeyFramesType;
  this->Positions = vtkSmartPointer<vtkMRMLPointSplineNode>::New();
  this->FocalPoints = vtkSmartPointer<vtkMRMLPointSplineNode>::New();
  this->ViewUps = vtkSmartPointer<vtkMRMLPointSplineNode>::New();
}

//------------------------------------------------------------------------------
vtkMRMLCameraPathNode::vtkInternal::~vtkInternal()
{
  this->PathStatus = PATH_NOT_CREATED;
  delete this->KeyFrames;
}

//------------------------------------------------------------------------------
// vtkMRMLCameraPathNode

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCameraPathNode);

//----------------------------------------------------------------------------
vtkMRMLCameraPathNode::vtkMRMLCameraPathNode()
{
  this->Internal = new vtkInternal(this);
  this->HideFromEditors = 0;
}

//----------------------------------------------------------------------------
vtkMRMLCameraPathNode::~vtkMRMLCameraPathNode()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLCameraPathNode *node =
      vtkMRMLCameraPathNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  int disabledModify = this->StartModify();
  
  this->Superclass::Copy(anode);

  int PathStatus;
  KeyFramesType KeyFrames;
  vtkNew<vtkMRMLPointSplineNode> Positions;
  vtkNew<vtkMRMLPointSplineNode> FocalPoints;
  vtkNew<vtkMRMLPointSplineNode> ViewUps;

  PathStatus = node->GetPathStatus();
  KeyFrames = node->GetKeyFrames; // TODO : Deep Copy of cameras?
  Positions->DeepCopy(node->GetPositionSplines());
  FocalPoints->DeepCopy(node->GetFocalPointSplines());
  ViewUps->DeepCopy(node->GetViewUpSplines());
  
  this->SetKeyFrames(KeyFrames);
  this->SetPointSplines(Positions.GetPointer(),
                        FocalPoints.GetPointer(),
                        ViewUps.GetPointer());
  this->SetPathStatus(PathStatus);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "NumberOfKeyFrames: " << this->GetNumberOfKeyFrames() << "\n";
  os << indent << "MinimumT: " << this->GetMinimumT() << "\n";
  os << indent << "MaximumT: " << this->GetMaximumT() << "\n";
}

//----------------------------------------------------------------------------
//vtkMRMLStorageNode* vtkMRMLCameraPathNode::CreateDefaultStorageNode()
//{
//  return vtkMRMLCameraPathStorageNode::New();
//}

//----------------------------------------------------------------------------
int vtkMRMLCameraPathNode::GetPathStatus()
{
  return this->Internal->PathStatus;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetPathStatus(int status)
{
  this->Internal->PathStatus = status;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetPathChanged()
{
  if(this->GetPathStatus() != PATH_NOT_CREATED)
    {
    this->SetPathStatus(PATH_NOT_UP_TO_DATE);
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLCameraPathNode::GetNumberOfKeyFrames()
{
  return this->GetKeyFrames.size();
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathNode::GetMinimumT()
{
  if(this->GetKeyFrames.empty())
    {
    return -VTK_FLOAT_MAX;
    }
  return this->GetKeyFrames.front().second;
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathNode::GetMaximumT()
{
  if(this->GetKeyFrames.empty())
    {
    return VTK_FLOAT_MAX;
    }
  return this->GetKeyFrames.back().second;
}

//----------------------------------------------------------------------------
vtkMRMLCameraPathNode::KeyFramesType vtkMRMLCameraPathNode::GetKeyFrames()
{
  return this->Internal->KeyFrames;
}

//----------------------------------------------------------------------------
vtkMRMLCameraPathNode::KeyFrameType
vtkMRMLCameraPathNode::GetKeyFrame(vtkIdType index)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  return this->GetKeyFrames().at(index);
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathNode::GetKeyFrameTime(vtkIdType index)
{
  return this->GetKeyFrame(index).second;
}

//----------------------------------------------------------------------------
vtkMRMLCameraNode* vtkMRMLCameraPathNode::GetKeyFrameCamera(vtkIdType index)
{
  return this->GetKeyFrame(index).first;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetKeyFramePosition(vtkIdType index,
                                                double position[3])
{
  this->GetKeyFrameCamera(index)->GetPosition(position);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetKeyFrameFocalPoint(vtkIdType index,
                                                  double focalPoint[3])
{
  this->GetKeyFrameCamera(index)->GetFocalPoint(focalPoint);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetKeyFrameViewUp(vtkIdType index,
                                              double viewUp[3])
{
  this->GetKeyFrameCamera(index)->GetViewUp(viewUp);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrames(KeyFramesType keyFrames)
{
  this->Internal->KeyFrames = keyFrames;
  this->SortKeyFrames();
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrame(vtkIdType index, KeyFrameType keyFrame)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  if( this->GetKeyFrameTime(index) == keyFrame.second
          && this->GetKeyFrameCamera(index) == keyFrame.first)
    {
    return;
    }
  this->Internal->KeyFrames.at(index) = keyFrame;
  this->SortKeyFrames();
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrameTime(vtkIdType index, double time)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  if( this->GetKeyFrameTime(index) == keyFrame.second)
    {
    return;
    }
  this->Internal->KeyFrames.at(index).second = time;
  this->SortKeyFrames();
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrameCamera(vtkIdType index,
                                              vtkMRMLCameraNode* camera)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  if( this->GetKeyFrameCamera(index) == keyFrame.first)
    {
    return;
    }
  this->Internal->KeyFrames.at(index).first = camera;
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFramePosition(vtkIdType index,
                                                double position[3])
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  double testPos[3];
  this->GetKeyFramePosition(index, testPos);
  if( testPos[0] == position[0] &&
          testPos[1] == position[1] &&
          testPos[2] == position[2] &&)
    {
    return;
    }
  this->Internal->KeyFrames.at(index).first->SetPosition(position);
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrameFocalPoint(vtkIdType index,
                                                  double focalPoint[3])
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  double testFoc[3];
  this->GetKeyFrameFocalPoint(index, testFoc);
  if( testFoc[0] == focalPoint[0] &&
          testFoc[1] == focalPoint[1] &&
          testFoc[2] == focalPoint[2] &&)
    {
    return;
    }
  this->Internal->KeyFrames.at(index).first->SetFocalPoint(focalPoint);
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrameViewUp(vtkIdType index,
                                              double viewUp[3])
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  double testView[3];
  this->GetKeyFrameViewUp(index, testView);
  if( testView[0] == viewUp[0] &&
          testView[1] == viewUp[1] &&
          testView[2] == viewUp[2] &&)
    {
    return;
    }
  this->Internal->KeyFrames.at(index).first->SetPosition(viewUp);
  this->SetPathChanged();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::AddKeyFrame(KeyFrameType keyFrame)
{
  t = keyFrame.second;
  KeyFramesType::iterator it = find_if(this->Internal->KeyFrames.begin(),
                                       this->Internal->KeyFrames.end(),
                                       timeEqual(t));
  if (it != this->Internal->KeyFrames.end())
    {
    vtkIdType index = it - this->Internal->KeyFrames.begin();
    std::cout << "A keyframe already exists at t = " << t << std::endl
              << "Keyframe ID : " << index << std::endl
              << "To update it, use the methods 'SetKeyFrame(index)' instead."
              << std::endl;
    return;
    }
  this->Internal->KeyFrames.push_back(keyFrame);
  this->SortKeyFrames();
  this->SetPathChanged();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::AddKeyFrame(double t,
                                      vtkMRMLCameraNode* camera)
{
  KeyFrameType keyFrame(camera, t);
  this->AddKeyFrame(keyFrame);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::AddKeyFrame(double t,
                                        double position[3],
                                        double focalPoint[3],
                                        double viewUp[3])
{
  vtkNew<vtkMRMLCameraNode> camera;
  camera->SetPosition(position);
  camera->SetFocalPoint(focalPoint);
  camera->SetViewUp(viewUp);
  this->AddKeyFrame(t, camera);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::RemoveKeyFrame(vtkIdType index)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  this->Internal->KeyFrames.vec.erase(
              this->Internal->KeyFrames.begin() + index);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SortKeyFrames()
{
  if( this->GetNumberOfKeyFrames() < 2)
  {
    return;
  }
  std::sort(this->Internal->KeyFrames.begin(),
            this->Internal->KeyFrames.end(), timeSort());
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::CreatePath()
{
  if(this->GetKeyFrames.empty())
    {
    return;
    }

  double min = this->GetMinimumT();
  double max = this->GetMaximumT();

  this->GetPositionSplines()->Initialize(min, max);
  this->GetFocalPointSplines()->Initialize(min, max);
  this->GetViewUpSplines()->Initialize(min, max);

  for( vtkIdType i; i < numPts; ++i)
    {
    double position[3], focalPoint[3], viewUp[3];
    this->GetKeyFramePosition(i, position);
    this->GetKeyFrameFocalPoint(i, focalPoint);
    this->GetKeyFrameViewUp(i, viewUp);

    this->GetPositionSplines()->AddPoint(i, position);
    this->GetFocalPointSplines()->AddPoint(i, focalPoint);
    this->GetViewUpSplines()->AddPoint(i, viewUp);
    }
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode* vtkMRMLCameraPathNode::GetPositionSplines()
{
  return this->Internal->Positions;
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode* vtkMRMLCameraPathNode::GetFocalPointSplines()
{
  return this->Internal->FocalPoints;
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode* vtkMRMLCameraPathNode::GetViewUpSplines()
{
  return this->Internal->ViewUps;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetPointSplines(vtkMRMLPointSplineNode* positions,
                                            vtkMRMLPointSplineNode* focalPoints,
                                            vtkMRMLPointSplineNode* viewUps)
{
  this->Internal->Positions = positions;
  this->Internal->FocalPoints = focalPoints;
  this->Internal->ViewUps = viewUps;
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetCameraInfo(double t,
					  vtkMRMLCameraNode* camera)
{
  if ( this->GetPathStatus() == PATH_NOT_CREATED )
    {
    return;
    }
  double position[3], focalPoint[3], viewUp[3];
  this->GetPositionAt(t, position);
  this->GetFocalPointAt(t, focalPoint);
  this->GetViewUpAt(t, viewUp);
  camera->GetPosition(position);
  camera->GetFocalPoint(focalPoint);
  camera->GetViewUp(viewUp);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetPositionAt(double t, double position[3])
{
  if ( this->GetPathStatus() == PATH_NOT_CREATED )
    {
    return;
    }
  t = this->ClampTime(t);
  this->GetPositionSplines->Evaluate(t, position);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetFocalPointAt(double t, double focalPoint[3])
{
  if ( this->GetPathStatus() == PATH_NOT_CREATED )
    {
    return;
    }
  t = this->ClampTime(t);
  this->GetFocalPointSplines->Evaluate(t, focalPoint);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetViewUpAt(double t, double viewUp[3])
{
  if ( this->GetPathStatus() == PATH_NOT_CREATED )
    {
    return;
    }
  this->ClampTime(t);
  this->GetViewUpSplines()->Evaluate(t, viewUp);
}

//---------------------------------------------------------------------------
double vtkMRMLCameraPathNode::ClampTime(double t)
{
  if ( t < this->GetMinimumT() )
    {
    t = this->GetMinimumT();
    }
  else if ( t > this->GetMaximumT() )
    {
    t = this->GetMaximumT();
    }
  return t;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  if (std::string(reference->GetReferenceRole()) == this->DisplayNodeReferenceRole)
    {
    this->Internal->UpdateDisplayNode(
          vtkMRMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
    }
  this->Superclass::OnNodeReferenceAdded(reference);
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  this->Internal->UpdateDisplayNode(
        vtkMRMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
  this->Superclass::OnNodeReferenceModified(reference);
}
