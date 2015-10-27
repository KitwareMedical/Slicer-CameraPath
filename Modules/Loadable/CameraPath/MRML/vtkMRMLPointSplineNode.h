#ifndef __vtkMRMLPointSplineNode_h
#define __vtkMRMLPointSplineNode_h

// MRML includes
#include "vtkSlicerCameraPathModuleMRMLExport.h"
#include <vtkMRMLModelNode.h>
class vtkMRMLStorageNode;

// VTK includes
#include <vtkKochanekSpline.h>
class vtkAlgorithmOutput;

/// \brief MRML node to hold the information about a 3D spline.
///
class VTK_SLICER_CAMERAPATH_MODULE_MRML_EXPORT vtkMRMLPointSplineNode :
    public vtkMRMLModelNode
{
public:
  typedef vtkMRMLPointSplineNode Self;
  typedef vtkKochanekSpline splineType;

  static vtkMRMLPointSplineNode *New();
  vtkTypeMacro(vtkMRMLPointSplineNode,vtkMRMLModelNode)
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "PointSpline";}

  /// Create and observe default camera path display node
  /// \sa vtkMRMLPointSplineDisplayNode
  void CreateDefaultDisplayNodes();

  //--------------------------------------------------------------------------
  /// PointSpline methods
  //--------------------------------------------------------------------------

  double GetMinimumT();
  double GetMaximumT();

  splineType* GetXSpline();
  splineType* GetYSpline();
  splineType* GetZSpline();
  void SetSplines(splineType* xSpline,
                  splineType* ySpline,
                  splineType* zSpline);

  void Initialize(double min, double max); // RemoveAllPoints and SetParametricRange
  void AddPoint(double t, double point[3]);
  void UpdatePolyData(int framerate);
  void Evaluate(double t, double point[3]=0);

protected:
  vtkMRMLPointSplineNode();
  virtual ~vtkMRMLPointSplineNode();
  vtkMRMLPointSplineNode(const vtkMRMLPointSplineNode&);
  void operator=(const vtkMRMLPointSplineNode&);

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
