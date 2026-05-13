gdblast
by Riordan Callil 2025

Middleware between Godot and Nvidia Blast

Platforms tested:
. windows

Platforms supported:
. In theory: All. There is not platform specific code in this software.

todo:
. as soon as we fracture. integrate forces stops being applied correctly to new pieces

. good and useful error codes
. documentation
. tutorial
. debug for rendering bonds

Documentation
=============

Concepts
* TkActor - TkBlastActor - TkAsset - TkBlastAsset
* TkFamily
    Created when any blast actor is created. Contains listeners used to monitor events.
    Stress simulation uses the family.
* TkGroup
    Created to organize actors into work groups. Useful for multithreading.