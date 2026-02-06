# mountatray

A simple mounting/unmounting tray applet inspired by [udiskie](https://github.com/coldfix/udiskie). This project is somewhat vibe-coded, but without any AI extentions on my text editor, only ChatGPT on my browser. To prevent that, I used Neovim with no distributed configuration (like LazyVim, etc.) and plugins, just a few `init.lua` configurations.


## Manual building

Prerequisites:
- gcc
- pkg-config
- gtk+3-devel
- libudev-devel 
- udisks2 (runtime)

Then, use this compile command:

`gcc -std=gnu99 -Wall main.c drives.c -o mountatray `pkg-config --cflags --libs gtk+-3.0 libudev``

Lastly, run it: 

`./mountatray`


