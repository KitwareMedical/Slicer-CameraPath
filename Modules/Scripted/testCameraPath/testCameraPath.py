import time
import os

import ctk
import qt
import vtk

import slicer


#
# testCameraPath
#

class testCameraPath:
    def __init__(self, parent):
        parent.title = "testCameraPath"
        parent.categories = ["Examples"]
        parent.dependencies = []
        parent.contributors = ["Alexis Girault (Kitware)"]
        parent.helpText = """
    """
        parent.acknowledgementText = """
    """
        self.parent = parent


#
# qtestCameraPathWidget
#

class testCameraPathWidget:
    def __init__(self, parent=None):
        if not parent:
            self.parent = slicer.qMRMLWidget()
            self.parent.setLayout(qt.QVBoxLayout())
            self.parent.setMRMLScene(slicer.mrmlScene)
        else:
            self.parent = parent
        self.layout = self.parent.layout()
        if not parent:
            self.setup()
            self.parent.show()

    def setup(self):

        # 3) PROGRESS AND RESULTS
        self.ProgressCollapsibleButton = ctk.ctkCollapsibleButton()
        self.ProgressCollapsibleButton.text = "Progress and Results"
        self.ProgressCollapsibleButton.collapsed = 0
        progress_layout = qt.QVBoxLayout(self.ProgressCollapsibleButton)
        self.layout.addWidget(self.ProgressCollapsibleButton)

        # 4) vertical spacer
        self.layout.addStretch(1)

        # BUTTONS
        self.RemoveButton = qt.QPushButton("Remove KeyFrames")
        self.RemoveButton.toolTip = "Remove KeyFrames"
        self.RemoveButton.connect('clicked()', self.onRemoveButton)
        progress_layout.addWidget(self.RemoveButton)

        self.AddButton = qt.QPushButton("Add KeyFrame")
        self.AddButton.toolTip = "Add KeyFrame"
        self.AddButton.connect('clicked()', self.onAddButton)
        progress_layout.addWidget(self.AddButton)

        self.CreateButton = qt.QPushButton("Create Path")
        self.CreateButton.toolTip = "Create Path"
        self.CreateButton.connect('clicked()', self.onCreateButton)
        progress_layout.addWidget(self.CreateButton)

        self.FlyButton = qt.QPushButton("Fly !")
        self.FlyButton.toolTip = "Fly !"
        self.FlyButton.connect('clicked()', self.onFlyButton)
        self.FlyButton.setEnabled(False)
        progress_layout.addWidget(self.FlyButton)

        # LOGIC
        self.defaultCam = slicer.mrmlScene.GetNodeByID('vtkMRMLCameraNode1')
        self.Time = 0
        self.CameraList = []
        self.cameraPath = slicer.vtkMRMLCameraPathNode()
        posSpline = self.cameraPath.GetPositionSplines()
        focalSpline = self.cameraPath.GetFocalPointSplines()
        viewSpline = self.cameraPath.GetViewUpSplines()

        posSpline.SetName('posSpline')
        focalSpline.SetName('focalSpline')
        viewSpline.SetName('viewSpline')

        slicer.mrmlScene.AddNode(self.cameraPath)
        slicer.mrmlScene.AddNode(posSpline)
        slicer.mrmlScene.AddNode(focalSpline)
        slicer.mrmlScene.AddNode(viewSpline)

        self.Flying = False

    def onRemoveButton(self):
       print("-- Removing KeyFrames")
       self.cameraPath.RemoveKeyFrames()

       if self.CameraList:
           for camera in self.CameraList:
               print(camera.GetName())
               slicer.mrmlScene.RemoveNode(camera)
           del self.CameraList[:]

       self.Time = 0

    def onAddButton(self):
        print(" -- Adding Keyframe at t =", self.Time)
        camera = slicer.mrmlScene.CreateNodeByClass('vtkMRMLCameraNode')
        slicer.mrmlScene.AddNode(camera)

        x = vtk.mutable(0)
        y = vtk.mutable(0)
        z = vtk.mutable(0)
        value = [x,y,z]

        self.defaultCam.GetPosition(value)
        camera.SetPosition(value)
        self.defaultCam.GetFocalPoint(value)
        camera.SetFocalPoint(value)
        self.defaultCam.GetViewUp(value)
        camera.SetViewUp(value)

        cameraName = 'Camera T = '+str(self.Time)
        print(cameraName)
        camera.SetName(cameraName)

        self.cameraPath.AddKeyFrame(self.Time, camera)
        self.Time += 100
        self.CameraList.append(camera)

        self.FlyButton.setEnabled(False)

    def onCreateButton(self):
        print(" -- Creating Path")
        self.cameraPath.CreatePath();
        self.FlyButton.setEnabled(True)

    def onFlyButton(self):
        if not self.Flying:
            print(" -- FLYING !")
            self.Flying = True
            self.FlyButton.setText("Stop")
            for t in range(0,self.Time-99):
                if not self.Flying:
                    return
                self.cameraPath.GetCameraAt(t,self.defaultCam)
                self.defaultCam.ResetClippingRange()
                print(t)
                slicer.app.processEvents()
                time.sleep(0.05)
            print("End")
            self.Flying = False
            self.FlyButton.setText("Fly!")
        else:
            print(" -- Stopped")
            self.Flying = False
            self.FlyButton.setText("Fly!")
        self.FlyButton.setEnabled(True)

