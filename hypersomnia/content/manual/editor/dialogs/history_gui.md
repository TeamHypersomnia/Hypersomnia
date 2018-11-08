The "History" dialog lets you easily navigate through the entire chronology of changes made to your project.

It is able to deterministically replay all modifications from the very first filling with test scene
unto the final touches when the map is already complete.

The filter box lets you browse changes to specific fields, flavours, assets and so on.

The revision that is currently saved to hard drive will be highlighted in green.

Some commands are parents and can then be unfolded by pressing an arrow next to their name.
One or more indented commands will be revealed.
This means that, for your convenience, whenever you press Ctrl+Z or Ctrl+Shift+Z,
the subsequent command will be redone or undone ALONG all its children at once.
This makes sense for actions that, under the hood, spawn multiple commands to accomplish your goal.
For example, changing a sprite will additionally adjust the sprite's size to match the image size.
It is however perfectly correct to manually navigate to a specific child command with your mouse,
for example if you wanted to change the sprite without at all adjusting the sprite's size.

Not everything is versioned, e.g. movements of camera, or view settings like grid density.
