#ifndef __vtkMRMLCameraPathNode_h
#define __vtkMRMLCameraPathNode_h

// MRML includes
#include "vtkSlicerCameraPathModuleMRMLExport.h"
#include "vtkMRMLPointSplineNode.h"
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLStorableNode.h>
class vtkMRMLStorageNode;

// STD includes
#include <utility>
#include <vector>

struct KeyFrame
{
  vtkSmartPointer<vtkMRMLCameraNode> Camera;
  double Time;
  //std::string ID;
  //std::string Label;
  //bool Locked;
  //bool Visibility;

  KeyFrame(vtkMRMLCameraNode* camera = NULL,
           double time = 0.0):
      Camera(camera),
      Time(time)
  {}

  KeyFrame& operator = (const KeyFrame& a)
    {
    Camera = a.Camera;
    Time = a.Time;
    return *this;
    }
  bool operator < (const KeyFrame& a) const
    {
    return Time < a.Time;
    }
  bool operator == (const KeyFrame& a) const
    {
    return (Camera == a.Camera && Time == a.Time); //TODO : camera, check members not pointer ?
    }
};
typedef std::vector<KeyFrame> KeyFrameVector;

struct timeEqual
{
  timeEqual(double const& time) : Time(time) { }
  double Time;

  bool operator ()(const KeyFrame& a)
    {
    return Time == a.Time;
    }
};

/// \brief MRML node to hold the information about a camera path.
///
class VTK_SLICER_CAMERAPATH_MODULE_MRML_EXPORT vtkMRMLCameraPathNode:
        public vtkMRMLStorableNode
{
public:
  typedef vtkMRMLCameraPathNode Self;

  enum {PATH_NOT_CREATED=0,
        PATH_NOT_UP_TO_DATE,
        PATH_UP_TO_DATE};

  static vtkMRMLCameraPathNode *New();
  vtkTypeMacro(vtkMRMLCameraPathNode,vtkMRMLStorableNode)
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "CameraPath";}

  /// Create default camera path storage node
  /// \sa vtkMRMLCameraPathStorageNode
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  //--------------------------------------------------------------------------
  /// CameraPath methods
  //--------------------------------------------------------------------------

  vtkIdType GetNumberOfKeyFrames();
  double GetMinimumT();
  double GetMaximumT();

  KeyFrameVector GetKeyFrames();
  KeyFrame GetKeyFrame(vtkIdType index);
  double GetKeyFrameTime(vtkIdType index);
  vtkMRMLCameraNode* GetKeyFrameCamera(vtkIdType index);
  void GetKeyFramePosition(vtkIdType index, double position[3] = 0);
  void GetKeyFrameFocalPoint(vtkIdType index, double focalPoint[3] = 0);
  void GetKeyFrameViewUp(vtkIdType index, double viewUp[3] = 0);

  void SetKeyFrames(KeyFrameVector keyFrames);
  void SetKeyFrame(vtkIdType index, KeyFrame keyFrame);
  void SetKeyFrameTime(vtkIdType index, double time);
  void SetKeyFrameCamera(vtkIdType index, vtkMRMLCameraNode* camera);
  void SetKeyFramePosition(vtkIdType index, double position[3]);
  void SetKeyFrameFocalPoint(vtkIdType index, double focalPoint[3]);
  void SetKeyFrameViewUp(vtkIdType index, double viewUp[3]);

  void AddKeyFrame(KeyFrame keyFrame);
  void AddKeyFrame(double t, vtkMRMLCameraNode* camera);
  void AddKeyFrame(double t,
                   double position[3],
                   double focalPoint[3],
                   double viewUp[3]);
  vtkIdType KeyFrameIndexAt(double t);

  void RemoveKeyFrames();
  void RemoveKeyFrame(vtkIdType index);
  void RemoveCameraFromScene(vtkIdType index);

  void SortKeyFrames();

  void CreatePath();

  vtkMRMLPointSplineNode* GetPositionSplines();
  vtkMRMLPointSplineNode *GetFocalPointSplines();
  vtkMRMLPointSplineNode* GetViewUpSplines();
  void SetPointSplines(vtkMRMLPointSplineNode* positions,
                       vtkMRMLPointSplineNode* focalPoints,
                       vtkMRMLPointSplineNode* viewUps);

  void GetCameraAt(double t, vtkMRMLCameraNode* camera);
  void GetPositionAt(double t, double position[3] = 0);
  void GetFocalPointAt(double t, double focalPoint[3] = 0);
  void GetViewUpAt(double t, double viewUp[3] = 0);
  double ClampTime(double t);

protected:
  vtkMRMLCameraPathNode();
  virtual ~vtkMRMLCameraPathNode();
  vtkMRMLCameraPathNode(const vtkMRMLCameraPathNode&);
  void operator=(const vtkMRMLCameraPathNode&);

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
