3d-kinect-modelling
===================

##Goal
Modelling and animation of complex 3D objects is difficult.  
It is time consuming and requires expensive tools. The goal of this project is to create an easier way to model 
complex 3D objects and capture their motion using a Kinect sensor. For example, modelling of the human body is 
already possible, but it requires many cameras and expensive software.   
The project will investigate doing this using Kinect and existing open source drivers and API’s.

##Current State
Currently the application does no real modelling. It shows the rgb feed from the Kinect, a color map of the depth
data from the Kinect, and a point cloud of the scene, with a texture mapped on top. 
The user can select various objects in the scene to be shown individually.  
A crude mesh can be created, but only of what the Kinect can see at any one time. Occlusion is not handled, this is the
current focus. The point cloud/mesh can be exported in various formats.

##History
This started as a class project, and is now being extended.
Original contributors included myself, Kyle Milz (https://github.com/krwm) and Romain Clement.  
This repo was created from an svn repo at: http://code.google.com/p/3d-kinect-modelling/
Some of the revision history was lost for this reason, the code is still at the above site, and history can be viewed there.
The project is moving in a different direction, so this should not be an issue.  
