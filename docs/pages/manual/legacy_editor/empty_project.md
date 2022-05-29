You are currently inside an empty, untitled project.
All untitled projects reside within the ``user/editor/untitled`` folder.

This empty project is useless as it is,  
so you should begin by filling it with a test scene built-in during compilation.

Press Shift+F5 to fill the project with the standard test scene.
Press Ctrl+Shift+F5 to fill the project with a minimal test scene designed for troubleshooting.

## Details

The two test scenes: testbed and minimal_scene are simple scenes generated purely with C++.  
Their code is maintained by developers so that any newly introduced game mechanic  
can be quickly tested in a reasonably populated game world. 

If we didn't add these test scenes into the executable at the compilation stage,
but instead stored them in separate files, they would require porting to a newer version of the game, 
possibly with different data layouts, which would needlessly complicate development.
