README_MicroLoMania.txt

Hey jammers!

Here's a very rough version for reference (win32_mlm.exe). It should run if you have any kind of DirectX installed, but if not you need to look for an end-user runtime for DirectX9.0c. I can help with that when we meet if you need it.

Everything is driven by hotkeys and should be reasonably documented on-screen (press ALT+ENTER at any time to toggle fullscreen). All editors that I showed on friday are in there as well, but there are numerous challenges that I discovered along the way that need some tweaking.

Again, everything took a lot longer than I expected (for various reasons) but this version is pretty well isolated from external dependencies and shouldn't be terribly difficult to port to another graphics API (which is what was intended). If we don't have time for that we can always use this version sans editors for our submission.

You can look at the various editors by pressing G to toggle dev-mode once you have started a game. As you will see the main challenge that I didn't really anticipate up front was the documentation requirements and subsequent challenges that arise from running the game at such a low resolution - there is simply not enough space to print out anything of substance without completely covering the screen. I could have used text without a solid background, but the main problem still remains.

To somewhat mitigate this you can press I to toggle all on-screen info, but then things get confusing very quickly unless you intuitively know what mode you happen to be in (and also because hotkeys are mode-dependent). However I anticipate that a power-user who knows the entire system by heart could become very proficient and productive, but that might take a little while.

For these reasons I mentioned today (Philip) that we might want to make a version that shows the game canvas at the intended low-res-but-upscaled version, and then have another "documentation-canvas" at another magnification that would have more pixels for text output. Isn't it fun building engines and tools from scratch?

Artists - we need to look into file formats that actually include a palette. 8-bit .tga is what I'm currently using (with the Windows system palette, many apologies...), but .pcx is also an option. The files you exported as transparent .pngs exemplify exactly the challenges that arise from trying to feed an 8-bit system with 24-bit source data - as mentioned I have written an automated pipeline for this but it is not currently in use as I expect that you want very specific control over the colors we end up using. Not that we need a color for transparency / color-keying as well but aagain we have all 256 colors to play with.

Isabel - I encoded one of your ambience tracks to .ogg (not sure of the settings, I used the default export from Audacity) but I like the file size much better (around 6mb down from over 90mb). Also I chose some of your sound effects that matched with what was already in the game that I was porting from. No problem to add more stuff.

Philip - I anticipate some challenges with mapping keys in some kind of platform-agnostic way. I have abstracted some of the win32 virtual keycodes that I'm used to using, but in some cases I'm still using keycodes that map to uppercase ASCII. As you know GLFW has its own custom abstraction, and we simply need to figure out what makes the most sense for micron.

In terms of gameplay things took quite a while to get even to the rough state that they are currently at simply because I underestimated the challenges involved in halving the scale of everything in the game. This posed challenges pretty much everywhere, but I tweaked all involved constants so that it feels semi-reasonable to me. For example things like jump heights, air acceleration, water physics all need to be tuned.

After initially dispairing that everything looked like crap (which my use of the Windows system palette DOES NOT help) I managed to get something that I imagine feels sort of like the refs that are on the Miro-board. I included some examples of 2-corner Wang tile systems as well as some tile-based animation - some the of animated art that is in there is from my friend Mats Persson who worked on a 24x24 pixel tile version of this stuff many many years ago, and the art has been hanging around forever.

Note that there are no enemies in the current build (simply because they aren't being spawned) but we can get going with that ASAP today. However there is some physics based stuff going on, with basic spiky-trap stuff and a related proxy death animation that uses some legacy assets (no, I don't intend for the player character to turn into a chubby knight upon death...)

I hope what I've done can inspire you all to make a final push with this. Again I apologize and freely admit that this was an extremely optimistic and aggressive technical push on my part, but I learned a ton about dependency-management that feeds directly into what I'm trying to do with micron. Perhaps selfish on my part, and again for that I apologize, but also again I hope that we manage to hammer out SOMETHING cool with this - if not within the deadline of this jam then perhaps in some future extension project. :)

/johno at 05:46 AM
