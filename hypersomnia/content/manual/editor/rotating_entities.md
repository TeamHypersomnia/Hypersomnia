You are now rotating the selected entities with your mouse.
To confirm the operation, press Enter or Left Mouse Button.
To cancel the transformation without making any changes, press ESC.

The entities are always rotated around the center of their bounding box.
This allows you to sensibly rotate many entities at once.
If you rotate a single entity, this means that it will be rotated around its own center.

By default, the rotation DELTA is snapped to multiples of 15 degrees.
This means that if your object is initially rotated arbitrarily, say at an angle of 1.14,
we won't be snapping the object's rotation to values of 0, 15, 30 etc.,
but to values of 1.14, 16.14, 31.14 and so on.

If you wish to actually set the object's rotation to a snapped value, you have to:
- first reset its rotation by pressing W whenever it is selected,
- and only then start rotating it with snapping enabled.

You can see if you are currently snapping to the multiples of 15 by looking at the indicator
at the bottom of the screen.
