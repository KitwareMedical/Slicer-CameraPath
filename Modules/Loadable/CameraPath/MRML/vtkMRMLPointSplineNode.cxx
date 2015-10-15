// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLPointSplineNode.h"
//#include "vtkMRMLPointSplineStorageNode.h"

// VTK includes
#include <vtkKochanekSpline.h>
#include <vtkParametricSpline.h>
#include <vtkSplineRepresentation.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
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

  /// Called when a display node is added or modified. Propagate the polydata
  /// to the new display node.
  void UpdateDisplayNode(vtkMRMLDisplayNode *dnode);

  /// Sets the polydata to a display node
  void SetPolyDataToDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode);

  /// Sets the polydata to all the display nodes
  void SetPolyDataToDisplayNodes();

  vtkSmartPointer<splineType> XSpline;
  vtkSmartPointer<splineType> YSpline;
  vtkSmartPointer<splineType> ZSpline;

#if (VTK_MAJOR_VERSION <= 5)
  /// Hold the polydata for vtkMRMLModelDisplayNode
  vtkSmartPointer<vtkPolyData> PolyData;
#else
  vtkSmartPointer<vtkAlgorithmOutput> PolyDataConnection;
  vtkSmartPointer<vtkEventForwarderCommand> DataEventForwarder;
#endif

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

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::vtkInternal::UpdateDisplayNode(
    vtkMRMLDisplayNode *dnode)
{
  if (!vtkMRMLModelDisplayNode::SafeDownCast(dnode))
    {
    return;
    }
  vtkMRMLModelDisplayNode* modelDisplayNode =
    vtkMRMLModelDisplayNode::SafeDownCast(dnode);
  if (modelDisplayNode)
    {
    this->SetPolyDataToDisplayNode(modelDisplayNode);
    return;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLPointSplineNode::vtkInternal
::SetPolyDataToDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode)
{
#if (VTK_MAJOR_VERSION <= 5)
  modelDisplayNode->SetInputPolyData(this->PolyData);
#else
  modelDisplayNode->SetInputPolyDataConnection(this->PolyDataConnection);
#endif
}

//---------------------------------------------------------------------------
void vtkMRMLPointSplineNode::vtkInternal
::SetPolyDataToDisplayNodes()
{
  int ndisp = this->External->GetNumberOfDisplayNodes();
  for (int n=0; n<ndisp; n++)
    {
    vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::SafeDownCast(
      this->External->GetNthDisplayNode(n));
    if (dnode)
      {
      this->SetPolyDataToDisplayNode(dnode);
      }
    }
}

//------------------------------------------------------------------------------
// vtkMRMLPointSplineNode

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPointSplineNode)

//----------------------------------------------------------------------------
vtkMRMLPointSplineNode::vtkMRMLPointSplineNode()
{
  this->Internal = new vtkInternal(this);
  this->HideFromEditors = 0;
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

  this->UpdatePolyData();
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::UpdatePolyData()
{
  vtkNew<vtkParametricSpline> parametricSpline;
  parametricSpline->SetXSpline(this->GetXSpline());
  parametricSpline->SetYSpline(this->GetYSpline());
  parametricSpline->SetZSpline(this->GetZSpline());

  vtkNew<vtkSplineRepresentation> splineRepresentation;
  splineRepresentation->SetParametricSpline(parametricSpline.GetPointer());

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  splineRepresentation->GetPolyData(polyData);

#if (VTK_MAJOR_VERSION <= 5)
  vtkSmartPointer<vtkPolyData> oldPolyData = this->Internal->PolyData;

  this->Internal->PolyData = polyData;

  if (this->Internal->PolyData != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      this->Internal->PolyData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->Internal->SetPolyDataToDisplayNodes();

  if (oldPolyData != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations (
      oldPolyData, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->StorableModifiedTime.Modified();
  this->Modified();

#else
  if (polyData == 0)
    {
    this->SetPolyDataConnection(0);
    }
  else
    {
    vtkTrivialProducer* oldProducer = vtkTrivialProducer::SafeDownCast(
      this->Internal->PolyDataConnection ? this->Internal->PolyDataConnection->GetProducer() : 0);
    if (oldProducer && oldProducer->GetOutputDataObject(0) == polyData.GetPointer())
      {
      return;
      }
    else if (oldProducer && oldProducer->GetOutputDataObject(0))
      {
      oldProducer->GetOutputDataObject(0)->RemoveObservers(
        vtkCommand::ModifiedEvent, this->Internal->DataEventForwarder);
      }

    vtkNew<vtkTrivialProducer> tp;
    tp->SetOutput(polyData);
    // Propagate ModifiedEvent onto the trivial producer to make sure
    // PolyDataModifiedEvent is triggered.
    if (!this->Internal->DataEventForwarder)
      {
      this->Internal->DataEventForwarder = vtkEventForwarderCommand::New();
      }
    this->Internal->DataEventForwarder->SetTarget(tp.GetPointer());
    polyData->AddObserver(vtkCommand::ModifiedEvent, this->Internal->DataEventForwarder);
    this->SetPolyDataConnection(tp->GetOutputPort());
    }
#endif

}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::Initialize(double min, double max)
{
  if ( !this->GetXSpline() || !this->GetYSpline() || !this->GetZSpline() )
    {
    vtkErrorMacro("Please specify splines");
    return;
    }

  this->GetXSpline()->RemoveAllPoints();
  this->GetYSpline()->RemoveAllPoints();
  this->GetZSpline()->RemoveAllPoints();

  this->GetXSpline()->SetParametricRange(min, max);
  this->GetYSpline()->SetParametricRange(min, max);
  this->GetZSpline()->SetParametricRange(min, max);

  // this->UpdatePolyData();
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
  if ( t < this->GetMinimumT() || t > this->GetMaximumT() )
    {
    vtkErrorMacro("Parameter outside of parameter range");
    return;
    }

  this->GetXSpline()->AddPoint(t, point[0]);
  this->GetYSpline()->AddPoint(t, point[1]);
  this->GetZSpline()->AddPoint(t, point[2]);

  // this->UpdatePolyData();
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

#if (VTK_MAJOR_VERSION > 5)
//--------------------------------------------------------------------------
void vtkMRMLPointSplineNode
::SetPolyDataConnection(vtkAlgorithmOutput *newPolyDataConnection)
{
  if (newPolyDataConnection == this->Internal->PolyDataConnection)
    {
    return;
    }

  vtkAlgorithm* oldPolyDataAlgorithm = this->Internal->PolyDataConnection ?
    this->Internal->PolyDataConnection->GetProducer() : 0;

  this->Internal->PolyDataConnection = newPolyDataConnection;

  vtkAlgorithm* polyDataAlgorithm = this->Internal->PolyDataConnection ?
    this->Internal->PolyDataConnection->GetProducer() : 0;
  if (polyDataAlgorithm != NULL)
    {
    vtkEventBroker::GetInstance()->AddObservation(
      polyDataAlgorithm, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->Internal->SetPolyDataToDisplayNodes();

  if (oldPolyDataAlgorithm != NULL)
    {
    vtkEventBroker::GetInstance()->RemoveObservations (
      oldPolyDataAlgorithm, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
    }

  this->StorableModifiedTime.Modified();
  this->Modified();
  //this->InvokeEvent( vtkMRMLModelNode::PolyDataModifiedEvent , this);
}
#endif

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::OnNodeReferenceAdded(vtkMRMLNodeReference *reference)
{
  if (std::string(reference->GetReferenceRole()) == this->DisplayNodeReferenceRole)
    {
    this->Internal->UpdateDisplayNode(
          vtkMRMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
    }
  this->Superclass::OnNodeReferenceAdded(reference);
}

//----------------------------------------------------------------------------
void vtkMRMLPointSplineNode::OnNodeReferenceModified(vtkMRMLNodeReference *reference)
{
  this->Internal->UpdateDisplayNode(
        vtkMRMLDisplayNode::SafeDownCast(reference->GetReferencedNode()));
  this->Superclass::OnNodeReferenceModified(reference);
}
