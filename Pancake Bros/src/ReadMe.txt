Pancake Bros (16/12/03)
-----------------------

PROBLEMS
--------

RTTI solution does not support multiple inheritance, should use compiler solution.
	* See DlgDebugPancake, ideal solution is to cast to IController, but cannot.
	* Generally, will cause most problems for "interface" design pattern.
	* Downside: compiler does not support "Super"

NOTES
-----

* D3D is left-handed
* D3D is row-major

00 01 02 03
10 11 12 13
20 21 22 23
30 31 32 33

Row major translation:
1 0 0 0
0 1 0 0
0 0 1 0
x y z 1

Row major transformation from a to b

a->b = a * b

Row major vector * matrix transformation:

                  m11 m12 m13 m14

                  m21 m22 m23 m24
     x y z 1
                  m31 m32 m33 m34

                  m41 m42 m43 m44

Linear system of equations:

|y | = |a b c| |x^2  x1^2  x2^2| 
|y1|   	       |x    x1    x2  | 
|y2|   	       |1    1     1   |     

--------
File Notes
--------

When writing to file, "\n" is ascii 13 (CR). However, in code "\n" is 10 (LF).

--------
PTRGC Note

Efficiency is reduced: pointer access involves calling a virtual function because of the need for virtual inheritance of
PtrGCHost. If need be, a second PTRGC class can be created strictly to hold IPtrGCHost objects, with the other holding PtrGCHost
objects.

--------
TBD:
Add a UIElement that creates a list of Editable Properties
Add tooltips to Editable Properties
Add a ribbon emitter (fix emitter inheritance structure)


Weapon calls damage on man with impact point, man resolves head, chest or leg hit


Character Animation:
Character
	Sequence
		Frame
			UV
			Texture
		Looping
 
 
Texture organisation:
Step 1: maximise space in a texture for a collection of square pictures
	Related pictures should be grouped in a single texture
		Relation is defined by directory structure


Particle Optimisation Ideas:
* Store all frames in a vector in emitter, avoid frameAt
* Execute render during update, 1 loop pass only

----------------------------------------------------------------------------------------------------------------------------
16/11/04
* Create DynamicObj (name?), holds RTTI name and pointer to object, creates object given RTTI name, hook into editable properties sys.
