# IsoGame #

The beginnings of an isometric video game using SDL,
the Simple DirectMedia Layer library.

Doesn't do very much just yet,
but does display a window containing an isometric grid,
some walls and doors,
and a few blocks.
There's no collision detection between the fixed blocks and the
moveable block.
The files 'cube.bmp' and 'tile.bmp' are used to draw an
animated cube and an animation cycle counter.

## Building the Program ##

Install the C compiler, linker and libraries.

`sudo apt-get install build-essential`

Install SDL libraries and the development package.

`sudo apt-get install libsdl2-2.0` <br />
`sudo apt-get install libsdl2-dev` <br />

Run 'make':

`make`
