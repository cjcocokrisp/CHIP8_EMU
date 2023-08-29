# CHIP8_EMU 
My first stab at writing an emulator by emulating the CHIP8 interpreted programming language.

![C8_Logo](https://cdn.discordapp.com/attachments/519260031158321189/1146135357742665920/repo_img.PNG)  

This [article](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) by Laurence Muller acted as a guide for this project and helped me learn what goes into writing an emulator as well as provided good examples for some of the opcodes.

This emulator was built for use on Windows. 

## Usage

To run a rom input the following command.

```
> CHIP8.exe <ROM_PATH>
```

Controls:
│ ─ ┌ ┬ ┐ ├ ┼ ┤ └ ┴ ┘
```
               ┌─┬─┬─┬─┐                                  ┌─┬─┬─┬─┐  
               │1│2│3│C│                                  │1│2│3│4│  
               ├─┼─┼─┼─┤                                  ├─┼─┼─┼─┤ 
               │4│5│6|D│                                  │Q│W│E|R│
Chip8 Keypad   ├─┼─┼─┼─┤   --------------------------->   ├─┼─┼─┼─┤  Keyboard
               │7│8│9│E│                                  │A│S│D│F│
               ├─┼─┼─┼─┤                                  ├─┼─┼─┼─┤
               │A│0│B│F│                                  │Z│X│C│V│
               └─┴─┴─┴─┘                                  └─┴─┴─┴─┘
```
 
There is an optional debug mode as well that can be launched by added this to the launch command.

```
> CHIP8.exe <ROM_PATH> --debug
```
The debugger can then be opened by pressing P.

[Link to video with preview footage.](https://www.youtube.com/watch?v=kGFa-tu4tKs&feature=youtu.be)
