# GDBLAST

Riordan Callil 2026

Middleware between Godot and Nvidia Blast

https://github.com/NVIDIAGameWorks/Blast

Platforms tested:

* windows

Platforms supported:

*  In theory: All. There is not platform specific code in this software.

### TODO

* as soon as we fracture. integrate forces stops being applied correctly to new pieces
* bend factor. Allow objects to loosen on impact before breaking like in Red Faction
* Break effects like dust emitted from fracture positions
* good and useful error codes
* documentation
* tutorial
* debug for rendering bonds
* debug timers

. create the BlastFamily object. Created once on initial create_actor.
    . Manages stress

Documentation
=============

Concepts

* TkActor - TkBlastActor - TkAsset - TkBlastAsset
* TkFamily
  Created when any blast actor is created. Contains listeners used to monitor events.
  Stress simulation uses the family.
* TkGroup
  Created to organize actors into work groups. Useful for multithreading.
  Listens and produces split signals
