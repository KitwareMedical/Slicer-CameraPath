// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLPointSplineNode.h"
//#include "vtkMRMLPointSplineStorageNode.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#if (VTK_MAJOR_VERSION > 5)
#include <vtkAlgorithmOutput.h>
#include <vtkEventForwarderCommand.h>
#include <vtkTrivialProducer.h>
#endif


//------------------------------------------------------------------------------
// vtkMRMLPointSplineNode::vtkInternal

//------------------------------------------------------------------------------
class vtkMRMLPointSplineNode::vtkInternal
{
public:
  vtkInternal(vtkMRMLPointSplineNode* external);
  ~vtkInternal();

  vtkSmartPointer<splineType> XSpline;
  vtkSmartPointer<splineType> YSpline;
  vtkSmartPointer<splineType> ZSpline;

  vtkMRMLPointSplineNode* External;
};

//------------------------------------------------------------------------------
vtkMRMLPointSplineNode::vtkInternal::vtkInternal(
    vtkMRMLPointSplineNode* external):External(external)
{
  this->XSpline = vtkSmartPointer<splineType>::New();
  this->YSpline = vtkSmartPointer<splineType>::New();
  this->ZSpline = vtkSmartPointer<splineType>::New();
}

//------------------------------------------------------------------------------
vtkMRMLPointSplineNode::vtkInternal::~vtkInternal()
{
}

//------------------------------------------------------------------------------
// vtkMRMLPointSplineNode

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPointSplineNode)

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode::vtkMRMLPointSplineNode()
{
  this->Internal = new vtkInternal(this);
  this->HideFromEditors = 1;
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode::~vtkMRMLPointSplineNode()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLPointSplineNode *node =
      vtkMRMLPointSplineNode::SafeDownCast(anode);
  if (!node)
    {
    vtkErrorMacro("Node empty");
    return;
    }

  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  vtkNew<splineType> XSpline;
  vtkNew<splineType> YSpline;
  vtkNew<splineType> ZSpline;

  XSpline->DeepCopy(node->GetXSpline());
  YSpline->DeepCopy(node->GetYSpline());
  ZSpline->DeepCopy(node->GetZSpline());

  this->SetSplines(XSpline.GetPointer(),
                   YSpline.GetPointer(),
                   ZSpline.GetPointer());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);


  os << indent << "NumberOfPoints: "
     << this->GetXSpline()->GetNumberOfPoints() << "\n";
  os << indent << "ParametricRange: [ "
     << this->GetMinimumT() << ", "
     << this->GetMaximumT() << "] \n";
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::CreateDefaultDisplayNodes()
{
  if (vtkMRMLModelDisplayNode::SafeDownCast(this->GetDisplayNode())!=NULL)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==NULL)
    {
    vtkErrorMacro("vtkMRMLPointSplineNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkNew<vtkMRMLModelDisplayNode> dispNode;
  this->GetScene()->AddNode(dispNode.GetPointer());
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
  dispNode->SetScalarVisibility(1);
  dispNode->SetActiveScalarName("Time");
  dispNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeInvertedGrey");
}

//----------------------------------------------------------------------------
double vtkMRMLPointSplineNode::GetMinimumT()
{
  double range[2];
  this->GetXSpline()->GetParametricRange(range);

  return range[0];
}

//----------------------------------------------------------------------------
double vtkMRMLPointSplineNode::GetMaximumT()
{
  double range[2];
  this->GetXSpline()->GetParametricRange(range);

  return range[1];
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode::splineType* vtkMRMLPointSplineNode::GetXSpline()
{
  return this->Internal->XSpline;
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode::splineType* vtkMRMLPointSplineNode::GetYSpline()
{
  return this->Internal->YSpline;
}

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode::splineType* vtkMRMLPointSplineNode::GetZSpline()
{
  return this->Internal->ZSpline;
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::SetSplines(splineType* xSpline,
                                        splineType* ySpline,
                                        splineType* zSpline)

{
  this->Internal->XSpline = xSpline;
  this->Internal->YSpline = ySpline;
  this->Internal->ZSpline = zSpline;

  this->UpdatePolyData(30);
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::UpdatePolyData(int framerate)
{
  double tmin = this->GetMinimumT();
  double tmax = this->GetMaximumT();
  int numSplinePoints = framerate * int(tmax - tmin);

  vtkSmartPointer<vtkPoints> splinePoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkFloatArray> timeArray = vtkSmartPointer<vtkFloatArray>::New();
  timeArray->SetName("Time");
  for (int i = 0; i < numSplinePoints; ++i)
    {
    double t = (i/(double)framerate) + tmin;
    double pt[3];
    this->Evaluate(t,pt);
    splinePoints->InsertNextPoint(pt);
    timeArray->InsertNextValue(t);
    }

  // Set up spline points
  vtkSmartPointer<vtkPolyData> splinePolyData = vtkSmartPointer<vtkPolyData>::New();
  splinePolyData->SetPoints( splinePoints );
  splinePolyData->GetPointData()->AddArray(timeArray);
  splinePolyData->GetPointData()->SetActiveScalars("Time");

  // Set up curves between adjacent points
  splinePolyData->Allocate( numSplinePoints-1 );
  for (vtkIdType i = 0; i <  numSplinePoints-1; ++i) {
    vtkIdType ii[2];
    ii[0] = i;
    ii[1] = i+1;
    splinePolyData->InsertNextCell( VTK_LINE, 2, ii );
  }

  this->SetAndObservePolyData(splinePolyData);
  this->CreateDefaultDisplayNodes();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::RemoveAllPoints()
{
  if ( !this->GetXSpline() || !this->GetYSpline() || !this->GetZSpline() )
    {
    vtkErrorMacro("Please specify splines");
    return;
    }

  this->GetXSpline()->RemoveAllPoints();
  this->GetYSpline()->RemoveAllPoints();
  this->GetZSpline()->RemoveAllPoints();
}
//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::RemovePoint(double t)
{
  if ( !this->GetXSpline() || !this->GetYSpline() || !this->GetZSpline() )
    {
    vtkErrorMacro("Please specify splines");
    return;
    }
  if ( t < this->GetMinimumT() || t > this->GetMaximumT() )
    {
    vtkErrorMacro("Time outside of parametric range");
    return;
    }

  this->GetXSpline()->RemovePoint(t);
  this->GetYSpline()->RemovePoint(t);
  this->GetZSpline()->RemovePoint(t);
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::AddPoint(double t, double point[3])
{
  if ( !this->GetXSpline() || !this->GetYSpline() || !this->GetZSpline() )
    {
    vtkErrorMacro("Please specify splines");
    return;
    }
  if ( !point )
    {
    vtkErrorMacro("No point given");
    return;
    }

  this->GetXSpline()->AddPoint(t, point[0]);
  this->GetYSpline()->AddPoint(t, point[1]);
  this->GetZSpline()->AddPoint(t, point[2]);
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::Evaluate(double t, double point[3])
{
  if ( !this->GetXSpline() || !this->GetYSpline() || !this->GetZSpline() )
    {
    vtkErrorMacro("Please specify splines");
    return;
    }
  if ( t < this->GetMinimumT() || t > this->GetMaximumT() )
    {
    vtkErrorMacro("Parameter outside of parameter range");
    return;
    }

  point[0] = this->GetXSpline()->Evaluate(t);
  point[1] = this->GetYSpline()->Evaluate(t);
  point[2] = this->GetZSpline()->Evaluate(t);
}
