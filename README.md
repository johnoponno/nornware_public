# changelog
* [x] 4:30 PM 4/19/2023 - upgrade solution to Microsoft Visual Studio Community 2019 Version 16.11.24
* [x] 4:51 PM 4/20/2023 - replace namespaces with prefixes
* [x] 4:51 PM 4/20/2023 - dx9 -> win32
* [x] 5:38 PM 4/20/2023 - fix mouse cursor position to work with canvas scaling

# todo
* [ ] implement mouse wheel example in tpmn
* [ ] re-implement editor

# background
This public codebase stems from nornware's internal codebase, and is intended mostly as an example of what Johannes 'johno' Norneby thinks is a decent coding style ca late 2019; the most contentious point is probably the shunning of / minimal use of Object Oriented Programming. It's also a small example of a 2d platform game rendered in software, transferred to a texture, and finally rendered as a single quad using DirectX9.0c.

# project overview
* dx9: minimal Windows /  DirectX9 bootstrapping and software to hardware rendering interface, win32 mouse and keyboard input, and DirectX3 sound.

* softdraw: 16 bit per pixel software rendering, support for .tga image loading.

* tpmn: backport from Flash/AS3 of the game Twisted Pair Man Nightmare, renders in software (via softdraw) and then transfers the canvas to a texture which is rendered via DirectX9.0c.

* dx9 lib depends on: https://www.microsoft.com/en-us/download/details.aspx?id=6812

* Tested using MSVC 2017 Community Version.
  * Make sure to set tpmn as the startup project, and your working directory to tpmn/assets (tpmn Properties/Configuration Properties/Debugging/Working Directory = ..\assets)

  * Use ALT+TAB at any time to switch between windowed and fullscreen mode.

# dx9 (C++ static library)
The dx9 library is a mashup of the minimal stuff required to get a program window up with DirectX9.0c, support for basic sound (DirectX3) via .wave files with some ogg/vorbis streaming support tacked on.

There are no examples of 3d rendering here, but rather all the graphics stuff is centered around doing rendering "in software" (using the CPU, softdraw lib) in order to be able to "touch the pixels"; DirectX9.0c and similar 3d apis generally don't let you touch the framebuffer directly, and the primitives are triangles (and textures) rather than pixels. It certainly isn't "free" to update the texture each tick (bus traffic from CPU to GPU), but it's a good trade-off to allow for old-school pixel-art games (where the pixels are CRISP!) in a modern world.

Input is queryable ("is this key down now?", flanks, etc) rather than treating input as "events" which I think is deplorable. In order to support "flank events" (key pressed, key released) it uses double buffering internally (retains last tick's state).

I've always done sound via DirectX3, but I suppose it might be time to look into XAudio and the like in order to maybe be able to do things like filtering. Spatialization (for a 3d game) can be done quite well using just volume and pan, but I'm sure more modern APIs have layers to do that. It's always a trade-off in terms of complexity.

There are some residual OOP-ey things in this library, some constructors and destructors where it makes sense to have memory be conceptually owned by a struct (for example sound_file_wave_t). A bigger example is the app_i baseclass; that's there because DirectX9.0c is kind of piggy when it comes to GPU resources when the user tabs amongst programs, and it turns into a messy kind of callback system. That's also what the d3d_resource_t / traversable_t crap is all about.

In general don't worry about all of this, as the point of contact with the operating system apis tend to be messy and out of your control, and all of this stuff originally comes from a heavily refactored DirectX SDK example ca 2006. I honestly don't completely know how the Directx9.0c stuff works, I just copied a minimal example and have been evolving it ever since.

The big point is that you can create more programs based on this "engine" if you want to, just by subclassing app_i. See the tpmn project for an example, basically you could copy that project and hack from there.

Oh yeah, you can configure your app_i subclass to run in either fixed tick mode (which I almost always do, at 60Hz) or as fast as your computer will run. If you aren't running fixed tick you can use the timing variables passed to frame_move() and frame_render().

# softdraw (C++ static library)
As mentioned, softdraw was created out of a desire to do old-style "blitting" (BLock Image Transfer). The basic primitive is bitmap_t, with functions for drawing to and from them, color keying, various blend modes, etc. All of this is of course far inferior performance-wise as compared to what a GPU can do nowadays, but the whole point of making games with software is for them to be relatively low resolution and "chunky". Also note that all 3d apis (DX / OpenGL / Vulkan) generally specify screen coordinates as floating point values, so it's generally more tricky to get things to line up nicely (especially when building worlds from tiles) when programming triangles on the GPU than doing things the way software does.

There is some flexibility in how the "canvas" (as I usually call the framebuffer) is actually shown on the screen; see how tpmn calculates the size and position of the final quad in app_t::frame_render() as well as the options available in dx9::softdraw_adapter_t. That's really the only contact between the whole game and DirectX; except for app_t the whole game is basically platform independent (the softdraw library is as well).

# tpmn (C++ win32 application)
This was a project I did with some students when I was working as a high school teacher, originally done in Flash / AS3 and also originally very object oriented. I recently did a port to C/C++, and because I knew I would be touching basically every line of code in the port from AS3 to C++ I thought it would be fun to see what my current coding style would do to the project.

First of all some architectural philosophy; Model-View-Controller. It's an idea that has been thrown around a lot, and there are many interpretations of it (I actually met Trygve Reenskaug who coined the term in the 1970s at one point, but he wouldn't tell me if my interpretation of MVC aligned with his original ideas!). My version is like this:

## model
This is (as far as possible) the purely "logical" part of the game, the simulation if you will. It has nothing to do with any kind of presentation, neither rendering visuals nor audio. It's basically "raw data" (structs) and the algorithms (functions) that mutate that data.

Coming from a client-server background at Massive Entertainment, "the model is the server"; we wanted to be able to run dedicated servers without any dependencies on rendering or audio, so we had an equivalent split since the early days.

## view
This isn't a "stateful" component of the architecture beyond the fact that a game often has a bunch of "assets" (bitmaps, audio samples, 3d meshes, etc) that it can use to COMPOSE a "view". As we are doing real-time games that are basically a flip-book of animation frames (we generate a new framebuffer each tick at 60Hz or whatever), a "view" is truly transient.

It's also EXTREMELY important to realize that any given "model" can have any number of different "views". Ponder for example the Ground Control strategy games that both had a "3d view" and a "map view"; we weren't smart enough to do MVC as I'm describing it back then but rather had really messy dependencies, but my modern thoughts we certainly heavily influenced by a lot of bad decisions we made on those games.

Another good example is a bar chart vs a pie chart vs just numbers, all "different views" of the same data / "model".

Note also that most of the time the "assets" in a game are completely immutable (don't change state) after they have been loaded from whatever on-disk representation they might have. This is important in general, because figuring out what parts of your program are immutable vs mutable SIGNIFICANTLY simplifies understanding and keeps bugs out.

## controller
I understand the original intent of the Controller concept to be a thing that routes user input to cause changes in Models and to update Views. Basically I agree except for my contention that a View is never stateful and most often (in games) purely transient (exists for 1/60th of a second). I like to say that an important part of what a Controller does is to "compose a view" using "assets".

A typical game loop is often cited as being:
1. Input
1. Update
1. Draw
1. repeat from 1

Based on that, my style is:
1. Input: Controller will sample user input, use that to potentially change Model state.
1. Update: Update the Model (run the simulation).
1. Draw: Controller uses the Assets to "compose a view" for this tick.
1. repeat from 1

Controller will often be stateful (and mutable) in that it often needs some variables to track things related to rendering / audio output (particle systems come to mind), and sometimes also to do things like menus that are logically not really part of the simulation itself.

# tpmn application flow
Looking at app_t::frame_move() (called back each tick from the dx9 system):

1) Input is sampled from the operating system and cached.

2) The model / simulation is updated, resulting in a structure (model_update_t) that can be examined after the fact. This is inspired by Rich Hickey's "queues" in that it completely decouples model from anything external that might be interested in the internal workings of model.

First a check is done to see if a new level needs to be loaded (the player went through a portal). This requires some once-off loading and resetting of systems.

If no level load happened the model_update_t is examined to see if any interesting "events" happened that we want to visualize, like sounds or particles. This is packed as much as possible in order to make it as simple as possible for the simulation to note that "events that might be of interest to external parties have happened and MAY be consumed externally". Logically it should be a queue of events, but in this case the combination of bits (for events) and some other fields for specific event data suffices.

Note that aside from the level loading stuff it is completely "safe" to NOT examine / consume the information about "events"; all of that stuff is just "fancy visualization stuff".

3) The controller is called to both sample user input, mutate the model (this ONLY changes the input bits for the hero), potentially mutate some controller state, and to compose the view (change the canvas).

This is interleaved input-output in order to simplify all the user interface stuff, with a single pass controlling both what individual keypresses "mean" as well as drawing to the canvas. Doing separate input and output passes would result in some mirroring of control flow, and in this case everything is simple enough to warrant this approach.


Looking at app_t::frame_render() (called back each tick from the dx9 system):

1) The canvas is copied to a texture and rendered via the GPU.

Typically (when doing more GPU rendering) this is the place where all of that would happen ("composition of the view"). In this game the controller input and view composition is interleaved as mentioned earlier in order to simplify the code.

Worth mentioning in this setup is that a lot of GPU-accelerated "post effects" could be done at this point. In other similar 2d games I have used the GPU for drawing glows and such on top of the "software canvas", utilizing GPU blending, filtering, and superior fill-rate (because the actual resolution we are running is much higher than that of the sotware canvas, we're just scaling the pixels up).

# tpmn details
On this port I explicitly wanted to isolate things according to my version of MVC. Looking more closely at the 3 main parts:

**assets_t**
This stuff is all "immutable post load", including bitmaps for backgrounds, tiles, monsters and players. It also includes a container for all the sound effects; really the "container" design is kind of old but it has stuck around. Some data (the animation mappings) come from the Flash version; I never ported the editor.

Note how minimal the interface is; init the assets, reload them (very specifically being able to reload certain assets without shutting down the game, good for fast iteration between Photoshop and the game...), cleanup, and finally to be able to play a sound via an id.

**controller_t**
The state of controller_t has the following purposes:

	struct controller_t
	{
	/*
	If non-zero, the in-game menu is up.
	*/
		uint32_t play_menu;

	/*
	Track the last checkpoint the player touched, so we can play a sound when this changes.
	*/
		uint32_t last_checkpoint;

	/*
	Because of the design of sound::stream_t, we need to be able to setup a new .ogg stream when we get to a screen with a new song; "track" is the cache.
	It would be nicer if sound::stream_t had an immediate mode interface that took the desired filename as a parameter and did all the caching internally.
	*/
		uint32_t track;
		sound::stream_t* music;

	/*
	Based on the animation mapping in assets_t, this array is updated each tick to reflect the current frames. A nifty way to do looping animations; the tile drawing code looks up the current frame here per index, so all tiles of the same index are animated together.
	*/
		uint32_t current_tiles[MAX_TILE];

	/*
	Various particle systems.
	*/
		snowflake_t flakes[32];
		cloud_t clouds[8];
		death_t deaths[8];
		uint32_t num_deaths;

	/*
	Has something to do with tile animations, could probably be done statelessly.
	*/
		float tile_anim_tick;

	/*
	For animating the end-game credits.
	*/
		float credits_start_time;

	/*
	The "canvas" to draw each frame on (softdraw). Even though the contents are transient, we want to keep the memory around so we're not allocating and releasing a new canvas each tick (memory fragmentation).
	*/
		softdraw::bitmap_t canvas;
	};

Paraphrasing a famous programming quote, along the lines of "show me your algorithms and I won't understand anything, but show me your state and I'll understand everything". This is really one of the big wins with going procedural; it's possible to look at state / data in isolation from functions / algorithms, and it really gets you thinking about what state you actually need in your program. Conversely OOP programs tend to both hide state away as well as fragment it all over the place, and you often end up with lots of state (and caches) that you aren't even aware of.

model_t:
Twisted Pair Man Nightmare (tpmn) is based on a really old idea of how to build worlds using tiles, kind of backwards from what I would usually do but really quite productive: Starting with a big bitmap with tiles in it, an artist / designer could build worlds in an editor that looked however they might like, and then afterwards the coder could mark certain tiles by index to have special meanings (look at the LOGIC_INDEX_ constants in model.h)

What this does (which I think is bad) is to introduce coupling from the "model" onto the "assets", in that if the graphics are moved around the LOGIC_INDEX_ indices will be wrong. However this is very loose coupling, because the model doesn't really care what the values of these indices are, it just needs SOME value.

With those in place, designers can place checkpoints, servers, spawns for enemies, keys and locks, and coders can write code to react to it all (including which tiles are collidable, dangers, slippery, etc).

As model_t is "the entire simulation" it is a bit complicated at first glance. 

	struct model_t
	{
	/*
	This is the contents of the files created in the original editor, all immutable after level load.
	*/
		level_t level;

	/*
	Beyond the LOGIC_INDEX_ constants, and in a similar manner to how global tile animation is done, this array contains further information per tile index / type. It is possible to tag tiles in different manners for collision detection, as well as special effects like being lethal to the player or slippery.
	*/
		tile_info_t tile_info[MAX_TILE];	//FIXME: immutable after program startup, move out!

	/*
	This might be contentious; in order to avoid polymorphism all enemies "are the same type". The code switches on enemy_t.type in order to run different "behaviours" for each "type of enemy". Yes, that's a hard-coded limit to the maximum number of enemies; if it crashes, make it bigger!
	Also in terms of memory usage; the members of enemy_t are the union of all the variables that all "types" of enemies require, some more than others. In my experience it's better to spend a little bit more memory in order to both avoid dynamic allocation and polymorphism, as well as improving cache coherency. I could go deeper into reasons against polymorphism, but I'm trying to be brief...
	*/
		enemy_t enemy[64];

	/*
	The hero character, different enough from enemy_t in order to be a custom struct. Also it's convenient to explicitly know which "entity" is the hero in code.
	*/
		hero_t hero;

	/*
	These are for supporting portals between different level files. Yes, strings are fixed length.
	*/
		char last_world[32];
		char world[32];

	/*
	The number of enemies currently running.
	*/
		uint32_t num_enemy;

	/*
	The current simulation tick, used for timing.
	*/
		uint32_t tick;

	/*
	If set, the game is running, otherwise it's in the menu.
	*/
		uint32_t play_bit;
	};

Yes model.cpp is a bit of a beast as it contains everything that the simulation does, but look especially at model_update() and you will find that the control flow is as straight-line as possible.

Most of the other functions are just utilities both for model_update() as well as for controller_t to use in "composing a view".
