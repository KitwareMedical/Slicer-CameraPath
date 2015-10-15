// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLCameraPathNode.h"
#include "vtkMRMLPointSplineNode.h"
//#include "vtkMRMLCameraPathStorageNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>
#include <utility>
#include <vector>
#include <algorithm>


//------------------------------------------------------------------------------
// timeComp

//----------------------------------------------------------------------------


//------------------------------------------------------------------------------
// vtkMRMLCameraPathNode::vtkInternal

//------------------------------------------------------------------------------
class vtkMRMLCameraPathNode::vtkInternal
{
public:
  vtkInternal(vtkMRMLCameraPathNode* external);
  ~vtkInternal();

  int PathStatus;
  KeyFrameVector KeyFrames;
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
  this->Positions = vtkSmartPointer<vtkMRMLPointSplineNode>::New();
  this->FocalPoints = vtkSmartPointer<vtkMRMLPointSplineNode>::New();
  this->ViewUps = vtkSmartPointer<vtkMRMLPointSplineNode>::New();
}

//------------------------------------------------------------------------------
vtkMRMLCameraPathNode::vtkInternal::~vtkInternal()
{
  this->PathStatus = PATH_NOT_CREATED;
    // TODO : delete cameras in list ?
}

//------------------------------------------------------------------------------
// vtkMRMLCameraPathNode

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCameraPathNode)

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
  KeyFrameVector KeyFrames;
  vtkNew<vtkMRMLPointSplineNode> Positions;
  vtkNew<vtkMRMLPointSplineNode> FocalPoints;
  vtkNew<vtkMRMLPointSplineNode> ViewUps;

  PathStatus = node->GetPathStatus();
  KeyFrames = node->GetKeyFrames();
  Positions->Copy(node->GetPositionSplines());
  FocalPoints->Copy(node->GetFocalPointSplines());
  ViewUps->Copy(node->GetViewUpSplines());
  
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
  return this->GetKeyFrames().size();
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathNode::GetMinimumT()
{
  if(this->GetKeyFrames().empty())
    {
    return -VTK_FLOAT_MAX;
    }
  return this->GetKeyFrames().front().Time;
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathNode::GetMaximumT()
{
  if(this->GetKeyFrames().empty())
    {
    return VTK_FLOAT_MAX;
    }
  return this->GetKeyFrames().back().Time;
}

//----------------------------------------------------------------------------
KeyFrameVector vtkMRMLCameraPathNode::GetKeyFrames()
{
  return this->Internal->KeyFrames;
}

//----------------------------------------------------------------------------
KeyFrame
vtkMRMLCameraPathNode::GetKeyFrame(vtkIdType index)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    vtkErrorMacro("No keyframes.");
    return KeyFrame();
    }
  return this->GetKeyFrames().at(index);
}

//----------------------------------------------------------------------------
double vtkMRMLCameraPathNode::GetKeyFrameTime(vtkIdType index)
{
  return this->GetKeyFrame(index).Time;
}

//---------------------------------------------------------------------------
vtkMRMLCameraNode *vtkMRMLCameraPathNode::GetKeyFrameCamera(vtkIdType index)
{
  return this->GetKeyFrame(index).Camera;
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetKeyFramePosition(vtkIdType index,
                                                double position[3])
{
  if(position)
    {
    this->GetKeyFrameCamera(index)->GetPosition(position);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetKeyFrameFocalPoint(vtkIdType index,
                                                  double focalPoint[3])
{
  if(focalPoint)
    {
    this->GetKeyFrameCamera(index)->GetFocalPoint(focalPoint);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetKeyFrameViewUp(vtkIdType index,
                                              double viewUp[3])
{
  if(viewUp)
    {
    this->GetKeyFrameCamera(index)->GetViewUp(viewUp);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrames(KeyFrameVector keyFrames)
{
  this->Internal->KeyFrames = keyFrames;
  this->SortKeyFrames();
  this->SetPathChanged();
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::SetKeyFrame(vtkIdType index, KeyFrame keyFrame)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }

  if( this->GetKeyFrame(index) == keyFrame)
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

  if( this->GetKeyFrameTime(index) == time)
    {
    return;
    }

  this->Internal->KeyFrames.at(index).Time = time;
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

  if( this->GetKeyFrameCamera(index) == camera)
    {
    return;
    }

  this->Internal->KeyFrames.at(index).Camera = camera;
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
          testPos[2] == position[2])
    {
    return;
    }
  this->Internal->KeyFrames.at(index).Camera->SetPosition(position);
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
          testFoc[2] == focalPoint[2])
    {
    return;
    }
  this->Internal->KeyFrames.at(index).Camera->SetFocalPoint(focalPoint);
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
          testView[2] == viewUp[2])
    {
    return;
    }
  this->Internal->KeyFrames.at(index).Camera->SetPosition(viewUp);
  this->SetPathChanged();
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::AddKeyFrame(KeyFrame keyFrame)
{
  double t = keyFrame.Time;
  KeyFrameVector::iterator it = find_if(this->Internal->KeyFrames.begin(),
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
  KeyFrame keyFrame(camera, t);
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
  this->AddKeyFrame(t, camera.GetPointer());
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::RemoveKeyFrame(vtkIdType index)
{
  if( index < 0 || index >= this->GetNumberOfKeyFrames() )
    {
    return;
    }
  this->Internal->KeyFrames.erase(
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
            this->Internal->KeyFrames.end());
}

//----------------------------------------------------------------------------
void vtkMRMLCameraPathNode::CreatePath()
{
  if(this->GetKeyFrames().empty())
    {
    return;
    }

  double min = this->GetMinimumT();
  double max = this->GetMaximumT();

  this->GetPositionSplines()->Initialize(min, max);
  this->GetFocalPointSplines()->Initialize(min, max);
  this->GetViewUpSplines()->Initialize(min, max);

  vtkIdType numPts = this->GetNumberOfKeyFrames();
  for( vtkIdType i = 0; i < numPts; ++i)
    {
    double position[3], focalPoint[3], viewUp[3];
    double t = this->GetKeyFrameTime(i);
    this->GetKeyFramePosition(i, position);
    this->GetKeyFrameFocalPoint(i, focalPoint);
    this->GetKeyFrameViewUp(i, viewUp);

    this->GetPositionSplines()->AddPoint(t, position);
    this->GetFocalPointSplines()->AddPoint(t, focalPoint);
    this->GetViewUpSplines()->AddPoint(t, viewUp);
    }

  this->GetPositionSplines()->UpdatePolyData();
  this->GetFocalPointSplines()->UpdatePolyData();

  this->SetPathStatus(PATH_UP_TO_DATE);
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
void vtkMRMLCameraPathNode::GetCameraAt(double t,
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
  camera->SetPosition(position);
  camera->SetFocalPoint(focalPoint);
  camera->SetViewUp(viewUp);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetPositionAt(double t, double position[3])
{
  if ( this->GetPathStatus() == PATH_NOT_CREATED )
    {
    return;
    }
  t = this->ClampTime(t);
  this->GetPositionSplines()->Evaluate(t, position);
}

//---------------------------------------------------------------------------
void vtkMRMLCameraPathNode::GetFocalPointAt(double t, double focalPoint[3])
{
  if ( this->GetPathStatus() == PATH_NOT_CREATED )
    {
    return;
    }
  t = this->ClampTime(t);
  this->GetFocalPointSplines()->Evaluate(t, focalPoint);
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
