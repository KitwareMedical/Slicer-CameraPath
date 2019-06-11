# CameraPath: Slicer module for Intelligent Virtual Surgical Planning

## Background

This project was created as a sandbox to design surgical planning tools leveraging 3D Slicer and the Slicer Custom App Template.

 **This project is not currently maintained. See the 3D Slicer [discourse forum] to learn how we plan to reuse some of IVSPlan's features in 3D Slicer**

## Modules

It provides two modules:

* [CameraPath](./Modules/Loadable/CameraPath)
* [testCameraPath](./Modules/Scripted/testCameraPath)

It requires a custom version of Slicer and corresponding changes can be found here: [KitwareMedical/Slicer:ivsplan-v4.4.0-2015-11-06-05f8365ee](https://github.com/KitwareMedical/Slicer/compare/05f8365ee...KitwareMedical:ivsplan-v4.4.0-2015-11-06-05f8365ee)


## Branches

* [master](../../tree/master): Filtered history containing only `CameraPath` and `testCameraPath` modules.
* [ivsplan-custom-app](../../tree/ivsplan-custom-app): Original IVSPlan application project based on an older version of [`Slicer Custom App Template`].

## Acknowledgments

[`3D Slicer`] is an open-source and extensible application for visualization and medical image analysis. 3D Slicer works with optical imaging, MRI, CT, and ultrasound data.

The [`Slicer Custom App Template`] is a template developed by Kitware to be used as a starting point for creating a custom 3D Slicer application. IVSPlan uses an older/deprecated version of the Slicer Custom App Template, which was 

At [Kitware], we have applied 3D Slicer to research and commercial applications ranging from pre-clinical animal studies, to surgical planning and guidance, to ultrasound image analysis, to medical robot control, to population studies.

## License

This project template is distributed under the Apache 2.0 license. Please see
the *LICENSE* file for details.


[discourse forum]: https://discourse.slicer.org/t/support-for-keyframe-based-animation
[`3D Slicer`]: https://slicer.org
[`Slicer Custom App Template`]: https://github.com/KitwareMedical/SlicerCustomAppTemplate
[Kitware]: https://www.kitware.com
