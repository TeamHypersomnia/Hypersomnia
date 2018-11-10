In the Images GUI, you can:

- Edit properties of imported images.
- Export properties images to a neighboring ``.meta.lua`` file.
	- Any time that an editor imports an ``path/to/img/image.png`` for its first use, 
	  it will check if a ``path/to/img/image.meta.lua`` exists. If it does,
	  the default settings will be loaded from that file.
	- Almost all official images have a ``.meta.lua`` file. 
	  Naturally, they will be used whenever you fill your project with a test scene.
- Manually import properties of images from a neighbouring ``.meta.lua`` file. 
- Forget the orphaned images - those that are not used anywhere anymore.

## Neon maps

## Physical shapes

## Offsets

Some images require some important metrics.

For example, we can generate a neon map for each image,  
or specify its shape in detail when it is going to be used as a physical shape,
or specify some attachment positions if it'll going to be a container like a torso or a gun.

If you create an image and want to always load some sensible properties 
