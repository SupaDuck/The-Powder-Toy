------------------------------------------------------------------------------
If you're missing any DLL's, download them here: http://min.us/lYzg3yW4VN2ce

This is a mod that basically does the same thing as the Lua script I made. 
It has a favorites menu under the Tools menu, that has all of the elements 
you selected recently, including walls and tools. Ctrl+Shift+left clicking 
an element in any menu will pin it permanantely to the menu, even after you 
exit the Powder Toy. Ctrl+Shift+ right clicking an element will unpin an 
element from the menu.

If you click the more button, a new menu will appear with 4 options. The HUD 
button activates a better HUD that fixes some problems, has less useless 
info, and has more decimal places in debug mode. The FIND button colors 
whatever element you have selected red, and dims everything else. The INFO 
button displays statistics about how long and how many times you have played 
powder toy, and your average and highest FPS.

I will take any suggestions for new elements or other features, no matter 
how hard they are to code, and if I like it, I will try to add it in.

Things in my mod suggested by other people:
Atrayin - Powered Portals, RAZR
OmegaSupreme - Animated LCRY
Videogamer555 - New heat displays
disturbed666 - A way to turn PHOT into ELEC (but I used PINV instead of TESC)
Dynotec - new statistics
therocketeer - smudge tool
cip - INWR with tmp of 1 doesn't conduct to SWCH




Explanation of other new features that need a long explanation:

Moving Solids: No lag, and it actually works. They are found in the special
menu, and you can have a maximum of 256 at once. They are drawn the same 
shape as your brush, no matter what shape or size it is. They are very 
bouncy, and fall with gravity. Pressure over 4.0 will destroy them, and if 
the center particle is destroyed, it will fall apart, making really tiny 
bouncing pixels. Go into the fav2 menu and click SPIN to make them be able 
to rotate. Don't save moving solids if you allowed them to rotate, reloading 
them after that is glitchy and won't work.

Animated LCRY: Found in the powered elements menu. In the decoration editor, 
press left or right to change frames, and paint each frame normally. Press 
Del to delete a frame or press Ctrl+right when going to a new frame to copy 
the old one over to the new one. PSCN activates it, and the speed that it 
changes depends on its temperature. NSCN deactivates it. METL pauses/unpauses
it. If you set the tmp2 of the PSCN or NSCN that (de)activates it, you can 
set which frame ANIM starts or freezes at. Also, you can use the tmp of the 
PSCN to set the delay for the first frame. You can have a maximum of 25 
frames, unless you use the lua command tpt.maxframes(num), where num is the 
new maximum, between 1 and 256. Using that command will reset all existing 
ANIM. If you delete a lot of it at once, the game will freeze for a while, 
just wait until it keeps working.

New heat displays: Found in the Fav2 menu. Click the HEAT button once to go 
to automatic display mode, where the hottest temp is always pink, the lowest 
is dark blue, and everything else in between is based off of that. Click it 
again to go into manual heat display. The min and max temps will stop 
changing when you go into this, so it will help if automatic mode flickers 
because of something like fire clone constantly changing the max temp. If 
you right click the  button, it will ask you to enter the new maximum and 
minimum temp. Enter the temperatures in Celcius. It will change the display 
mode into manual automatically after this.

Virus: Found in liquids. Turns everything it touches into VIRS. It has 3
states: Solid, liquid, and gas; depending on the temperature. If you infect
something room temp or less, it will all be solid virus, and you will be able
to cure it with CURE (in liquids) without noticing any difference in the
save. If you don't cure it in time, it will start to die slowly and randomly.
It is also cured randomly, and not instantly. Note:VIRS can burn and be 
destroyed, even if it infected something like DMND.

Powered elements: Most powered elements are now activated instantly. The 
elements that don't are LCRY, ANIM, and PBCN. When you draw a powered element,
it's tmp is set to 1, which causes it to be instantly activated. Use PROP to 
change it's tmp back to 0 if you want to keep the old way. Also, PCLN uses 
tmp2, not tmp, to do this.

Lua code: Put a lua script in luacode.txt and click the LUA button in the 
FAV2 menu. It will change the script into INDI and put it in the top left 
corner. It will then run the code once, so if you want it to run every frame, 
use tpt.register_step. When you save it in a stamp, and will allow it to be 
run one more time when you open it. After you save a stamp/save or open one, 
you can delete the INDI. The code will already be in newluacode.txt, and you 
can look at it to see what it does. I also added in virus checks. You can't 
add any code that reads or writes files, renames or deletes things, uses the 
os.execute command, and a few other things. It checks for these things when 
you try to save and try to run it. There is a bug that may cause it to crash 
when it finds one of these things.




Changes:

Version 1.0: http://min.us/lbuOtsPdEMibD2
Original release, with most of the stuff from my Lua Mod

Version 2.0: http://min.us/lzFWVBYhl5Xfi
Now with MOVING SOLIDS!

Version 2.1: http://min.us/lbvyL8rkNJlcni
Latest source + source now on github

Version 3.0: http://min.us/lbjHzEoxmXAFzP
Added Animated LCRY. Moving solids and ANIM save when you make a stamp or 
save.

Version 4.0: http://min.us/l2FzyIz4AYBS9
Moving solids can rotate now, disabled by default, 
go into the Fav2 menu and click SPIN to allow them to rotate. 
Also added Indestructible insulator and One time wire, 
and did some fixes to FIND and BALL.

Version 5.0: http://min.us/lbh6DDTJzdNFU3
Added automatic and manual heat displays. In automatic heat display, 
the highest temp is pink and the lowest is dark blue. In Manual, 
you can decide which temp is displayed as the hottest color and which is 
the lowest, and everything inbetween will be based off that. 
I also added powdered portals and virus/cure, and updated to the latest 
source to get rid of the update box.

Version 5.1: http://min.us/lDCKmb9BdFazn
Updated to latest version, added activator, which can be used in creations 
to prevent people from sparking buttons until specific times. Also, re-added 
in buttons to switch to the different modes, except blob and heat gradient.

Version 6.0: http://min.us/lUgHNsGndzdbh
Added powered invisible, blob and heat gradient display, portal effects, 
and wifi/portal lines. Also added the old lua console commands: 
sound(must be enable with a shortcut), load, bubble, reset_temp, and 
reset_pressure(works with the same parameters reset_velocity does). 
There are three new lua commands: get_pressure, 
get_gravity, 
and maxframes(sets the number of frames ANIM can have when created, 1-256). 
Also, render_ui uses the old cmode pictures in color.
Note: the button keys to change display modes, blob & heat gradient 
display, portal effects (Simon forgot the wifi/portal lines), and new 
icons all got into the official version. Not much of this code was 
actually written by me, a lot of it was taken from github after Simon 
deleted it. He said he might add in more stuff that I wrote later.

Version 7.0: http://min.us/lK604TH6bFNV4
Added readme. Faster drawing and deleting of elements. Added RAZR, an element
heavier than all other elements. Added new button in FAV2 menu that can make
saves compatible with the latest beta or release version and able to be
published. It makes sure that you don't add in any new elements not in those
versions. PINV+PHOT = ELEC. SPNG drags along other particles with it, even 
solids that normally can't move.

Version 8.0: http://min.us/muN92vQxC
Wall names display in the hud. New lua functions: get_wall and create_wall. A 
New Wall: ERASEALL. It can erase walls, signs, and particles all at once. 
Most powered elements drawn in this version are now activated instantly. Set 
the tmp to zero to keep the old way of activating. New properties that can be 
set using the lua console: CLONE (0x40000), BREAKABLECLONE (0x80000), 
POWERED (0x100000), and INDESTRUCTIBLE (0x20000). Use the first 3 together to 
make different kinds of clone using elements. The last one makes things 
invincible to every element, and even prevents it from being destroyed in 
reactions. You can make things like indestructible WATR or METL. Lua code can 
now be saved. Put it in luacode.txt, and it will be saved and run in 
stamps/saves. Press up in the console and then enter to load an example.

Version 9 http://min.us/muN92vQxC
Fix loading some saves and fixed crash with HUD. Removed update button 
permanetely and updated to newer source. Also, decoration color is now drawn 
on stamps in the stamp browser. 2 new quickoptions: Newtonian gravity and 
ambient heat. Custom HUDs. To change one, click the HUD2 button in the second 
favorites menu and different options will come up. You can change all 4 of 
the original HUD's and it will save and be loaded the next time you start. 
Also added a few more options not in any normal HUD.

Version 10
Added display mode toggles, press Ctrl+ a number key besides 4 and 8 to 
toggle the main feature of that display temporarily. Added more statistics. 
Fixed some moving solid bugs and made rotation save. Updated to newest 
source, which includes the new save format, but you can choose to save in the 
old format instead. You can delete corrupted stamps now. Fix three powered 
electronic related bugs, all electronics should work like they're supposed 
to now. Added a decoration smudge (blend) tool

Version 11 - 2/21/12
Added PROP2 tool, you can draw properties on with the normal brush now. 
Improved decoration drawing speed and added a hex value display and floodfill 
to it. Changed menus of some elements. Added a new test icon for the favorites 
menu. Added an update check, if there is a new version, you can use the link 
in the message of the day to download a zip file containing the next version 
of this mod. Added COND, only conducts to other COND with same tmp within a 
tmp2 radius. Added PWHT, flood fill heats the element above it to it's temp. 
Use PROP on it to make it create other properties. Added rechargeable battery, 
set the tmp of battery to the maximum charge it can hold, METL to charge, 
PSCN/NSCN to turn on/off. Some other stuff not listed here.




Bugs:
None that I know of




Future Ideas/TODO: > means currently being worked on/added next
draw something in empty space around ball to make it look solid
multiple tabs open (it will probably just save them, not actually simulate multiple tabs at the same time)
more lua commands, in next version (12)
choose what to save in saves/stamps
sensor? - http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=12361
new lua elements - Videogamer555 - http://powdertoy.co.uk/Discussions/Thread/View.html?Thread=12127
More deco tools
brush rotation
Maybe convert this mod to c++ when Simon finishes converting The Powder Toy to c++