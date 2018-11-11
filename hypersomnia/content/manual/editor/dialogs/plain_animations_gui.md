An animation is an unpathed asset.
It can be explicitly created or duplicated.

## Creating a new animation

When you create an animation by pressing "Create" button, 
it starts as being Orphaned, since it is not yet used anywhere.

By default, a Blank image will be used for the first frame.
First, select the proper image for the first frame of your animation.
Then, press the "From file sequence" button in order to import all files
whose names are properly indexed with subsequent numbers:

```
my_frame_1.png
my_frame_2.png
my_frame_3.png
...
my_frame_n.png
```

These files must also be in the same folder.
The first frame does not necessarily have to begin with 1, although it is a nice convention.
Alternatively, you can create animation frames manually if your filenames don't follow this pattern.

An image chosen for an animation frame is automatically imported to the project.
You will be able to edit its properties right away, in Images GUI (Alt+I).

A single image on the disk can perfectly be a part of many animations.

## Animation meta

Apart from the frames themselves, an animation also stores some metadata.
For example, ``stop_movement_at_frame`` specifies when should a bubble particle stop moving
due to an impending burst.
