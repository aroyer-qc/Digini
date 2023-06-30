// not if missing bracket
#define ESC_CUU         "\33A"              //  Cursor Up by 1
#define ESC_CUD         "\33B"              //  Cursor Down by 1
#define ESC_CUF         "\33C"              //  Cursor Forward (Right) by 1
#define ESC_CUB         "\33D"	            //  Cursor Backward (Left) by 1
#define ESC_RI          "\33M"              //  Reverse Index – Performs the reverse operation of \n, moves cursor up one line, maintains horizontal position, scrolls buffer if necessary*
#define ESC_DECSC       "\337"	            //  Save Cursor Position in Memory
#define ESC_DECSR       "\338"	            //  Restore Cursor Position from Memory

#define ESC_CUU_BY      "\33[%dA"           //	Cursor Up Cursor up by <n>
#define ESC_CUD_BY      "\33[%dB"	        //	Cursor Down	Cursor down by <n>
#define ESC_CUF_BY      "\33[%dC"	        //	Cursor Forward Cursor forward (Right) by <n>
#define ESC_CUB_BY      "\33[%dD"	        //  Cursor Backward	Cursor backward (Left) by <n>
#define ESC_CNL_BY      "\33[%dE"	        //  Cursor Next Line Cursor down to beginning of <n>th line in the viewport
#define ESC_CPL_BY      "\33[%dF"	        //  Cursor Previous Line Cursor up to beginning of <n>th line in the viewport
#define ESC_CHA_BY      "\33[%dG"	        //  Cursor Horizontal Absolute Cursor moves to <n>th position horizontally in the current line
#define ESC_VPA_BY      "\33[%dd"	        //	Vertical Line Position Absolute	Cursor moves to the <n>th position vertically in the current column
// *<x> and <y> parameters have the same limitations as <n> above. If <x> and <y> are omitted, they will be set to 1;1.
#define ESC_CUP         "\33[%d;%dH"        //  y;x	Cursor Position	                *Cursor moves to <x>; <y> coordinate within the viewport, where <x> is the column of the <y> line
#define ESC_HVP	        "\33[%d;%df"        //	y;x	Horizontal Vertical Position	*Cursor moves to <x>; <y> coordinate within the viewport, where <x> is the column of the <y> line

#define ESC_CEB         "\33[ [ ? 12 h	ATT160	Text Cursor Enable Blinking	Start the cursor blinking
#define ESC_CDB         "\33[ [ ? 12 l	ATT160	Text Cursor Enable Blinking	Stop blinking the cursor
#define ESC_            "\33[ [ ? 25 h	DECTCEM	Text Cursor Enable Mode Show	Show the cursor
#define ESC_            "\33[ [ ? 25 l	DECTCEM	Text Cursor Enable Mode Hide	Hide the cursor
Viewport Positioning
All commands in this section are generally equivalent to calling ScrollConsoleScreenBuffer console API to move the contents of the console buffer.

Caution The command names are misleading. Scroll refers to which direction the text moves during the operation, not which way the viewport would seem to move.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ [ <n> S	SU	Scroll Up	Scroll text up by <n>. Also known as pan down, new lines fill in from the bottom of the screen
#define ESC_            "\33[ [ <n> T	SD	Scroll Down	Scroll down by <n>. Also known as pan up, new lines fill in from the top of the screen
The text is moved starting with the line the cursor is on. If the cursor is on the middle row of the viewport, then scroll up would move the bottom half of the viewport, and insert blank lines at the bottom. Scroll down would move the top half of the viewport’s rows, and insert new lines at the top.

Also important to note is scroll up and down are also affected by the scrolling margins. Scroll up and down won’t affect any lines outside the scrolling margins.

The default value for <n> is 1, and the value can be optionally omitted.

Text Modification
All commands in this section are generally equivalent to calling FillConsoleOutputCharacter, FillConsoleOutputAttribute, and ScrollConsoleScreenBuffer console APIs to modify the text buffer contents.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ [ <n> @	ICH	Insert Character	Insert <n> spaces at the current cursor position, shifting all existing text to the right. Text exiting the screen to the right is removed.
#define ESC_            "\33[ [ <n> P	DCH	Delete Character	Delete <n> characters at the current cursor position, shifting in space characters from the right edge of the screen.
#define ESC_            "\33[ [ <n> X	ECH	Erase Character	Erase <n> characters from the current cursor position by overwriting them with a space character.
#define ESC_            "\33[ [ <n> L	IL	Insert Line	Inserts <n> lines into the buffer at the cursor position. The line the cursor is on, and lines below it, will be shifted downwards.
#define ESC_            "\33[ [ <n> M	DL	Delete Line	Deletes <n> lines from the buffer, starting with the row the cursor is on.
Note For IL and DL, only the lines in the scrolling margins (see Scrolling Margins) are affected. If no margins are set, the default margin borders are the current viewport. If lines would be shifted below the margins, they are discarded. When lines are deleted, blank lines are inserted at the bottom of the margins, lines from outside the viewport are never affected.

For each of the sequences, the default value for <n> if it is omitted is 1.

For the following commands, the parameter <n> has 3 valid values:

0 erases from the beginning of the line/display up to and including the current cursor position
1 erases from the current cursor position (inclusive) to the end of the line/display
2 erases the entire line/display
Sequence	Code	Description	Behavior
#define ESC_            "\33[ [ <n> J	ED	Erase in Display	Replace all text in the current viewport/screen specified by <n> with space characters
#define ESC_            "\33[ [ <n> K	EL	Erase in Line	Replace all text on the line with the cursor specified by <n> with space characters
Text Formatting
All commands in this section are generally equivalent to calling SetConsoleTextAttribute console APIs to adjust the formatting of all future writes to the console output text buffer.

This command is special in that the <n> position below can accept between 0 and 16 parameters separated by semicolons.

When no parameters are specified, it is treated the same as a single 0 parameter.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ [ <n> m	SGR	Set Graphics Rendition	Set the format of the screen and text as specified by <n>
The following table of values can be used in <n> to represent different formatting modes.

Formatting modes are applied from left to right. Applying competing formatting options will result in the right-most option taking precedence.

For options that specify colors, the colors will be used as defined in the console color table which can be modified using the SetConsoleScreenBufferInfoEx API. If the table is modified to make the “blue” position in the table display an RGB shade of red, then all calls to Foreground Blue will display that red color until otherwise changed.

 ::: Foreground Colours :::
#define ESC_FORE_Black      "\033[30m"
#define ESC_FORE_Red        "\033[31m"
#define ESC_FORE_Green      "\033[32m"
#define ESC_FORE_Yellow     "\033[33m"
#define ESC_FORE_Blue       "\033[34m"
#define ESC_FORE_Magenta    "\033[35m"
#define ESC_FORE_Cyan       "\033[36m"
#define ESC_FORE_White      "\033[37m"
#define ESC_FORE_Default    "\033[39m"
 ::: Background Colours :::
#define ESC_BACK_Black      "\033[40m"      
#define ESC_BACK_Red        "\033[41m"      
#define ESC_BACK_Green      "\033[42m"      
#define ESC_BACK_Yellow     "\033[43m"      
#define ESC_BACK_Blue       "\033[44m"      
#define ESC_BACK_Magenta    "\033[45m"      
#define ESC_BACK_Cyan       "\033[46m"      
#define ESC_BACK_White      "\033[47m"      
#define ESC_BACK_Default    "\033[49m"   

Value	Description	Behavior
0	Default	Returns all attributes to the default state prior to modification
1	Bold/Bright	Applies brightness/intensity flag to foreground color
4	Underline	Adds underline
24	No underline	Removes underline
7	Negative	Swaps foreground and background colors
27	Positive (No negative)	Returns foreground/background to normal
38	Foreground Extended	Applies extended color value to the foreground (see details below)
48	Background Extended	Applies extended color value to the background (see details below)
90	Bright Foreground Black	Applies bold/bright black to foreground
91	Bright Foreground Red	Applies bold/bright red to foreground
92	Bright Foreground Green	Applies bold/bright green to foreground
93	Bright Foreground Yellow	Applies bold/bright yellow to foreground
94	Bright Foreground Blue	Applies bold/bright blue to foreground
95	Bright Foreground Magenta	Applies bold/bright magenta to foreground
96	Bright Foreground Cyan	Applies bold/bright cyan to foreground
97	Bright Foreground White	Applies bold/bright white to foreground
100	Bright Background Black	Applies bold/bright black to background
101	Bright Background Red	Applies bold/bright red to background
102	Bright Background Green	Applies bold/bright green to background
103	Bright Background Yellow	Applies bold/bright yellow to background
104	Bright Background Blue	Applies bold/bright blue to background
105	Bright Background Magenta	Applies bold/bright magenta to background
106	Bright Background Cyan	Applies bold/bright cyan to background
107	Bright Background White	Applies bold/bright white to background
Extended Colors
Some virtual terminal emulators support a palette of colors greater than the 16 colors provided by the Windows Console. For these extended colors, the Windows Console will choose the nearest appropriate color from the existing 16 color table for display. Unlike typical SGR values above, the extended values will consume additional parameters after the initial indicator according to the table below.

SGR Subsequence	Description
38 ; 2 ; <r> ; <g> ; <b>	Set foreground color to RGB value specified in <r>, <g>, <b> parameters*
48 ; 2 ; <r> ; <g> ; <b>	Set background color to RGB value specified in <r>, <g>, <b> parameters*
38 ; 5 ; <s>	Set foreground color to <s> index in 88 or 256 color table*
48 ; 5 ; <s>	Set background color to <s> index in 88 or 256 color table*
*The 88 and 256 color palettes maintained internally for comparison are based from the xterm terminal emulator. The comparison/rounding tables cannot be modified at this time.

Mode Changes
These are sequences that control the input modes. There are two different sets of input modes, the Cursor Keys Mode and the Keypad Keys Mode. The Cursor Keys Mode controls the sequences that are emitted by the [arrow] keys as well as [Home] and [End], while the Keypad Keys Mode controls the sequences emitted by the keys on the [numpad] primarily, as well as the [function] keys.

Each of these modes are simple boolean settings – the Cursor Keys Mode is either Normal (default) or Application, and the Keypad Keys Mode is either Numeric (default) or Application.

See the Cursor Keys and Numpad & Function Keys sections for the sequences emitted in these modes.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ =	DECKPAM	Enable Keypad Application Mode	Keypad keys will emit their Application Mode sequences.
#define ESC_            "\33[ >	DECKPNM	Enable Keypad Numeric Mode	Keypad keys will emit their Numeric Mode sequences.
#define ESC_            "\33[ [ ? 1 h	DECCKM	Enable Cursor Keys Application Mode	Cursor keys will emit their Application Mode sequences.
#define ESC_            "\33[ [ ? 1 l	DECCKM	Disable Cursor Keys Application Mode (use Normal Mode)	Cursor keys will emit their Numeric Mode sequences.
Query State
All commands in this section are generally equivalent to calling Get* console APIs to retrieve status information about the current console buffer state.

Note These queries will emit their responses into the console input stream immediately after being recognized on the output stream while ENABLE_VIRTUAL_TERMINAL_PROCESSING is set. The ENABLE_VIRTUAL_TERMINAL_INPUT flag does not apply to query commands as it is assumed that an application making the query will always want to receive the reply.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ [ 6 n	DECXCPR	Report Cursor Position	Emit the cursor position as: "\33[ [ <r> ; <c> R Where <r> = cursor row and <c> = cursor column
#define ESC_            "\33[ [ 0 c	DA	Device Attributes	Report the terminal identity. Will emit \x1b[?1;0c, indicating "VT101 with No Options".
Tabs
While the windows console traditionally expects tabs to be exclusively eight characters wide, *nix applications utilizing certain sequences can manipulate where the tab stops are within the console windows to optimize cursor movement by the application.

The following sequences allow an application to set the tab stop locations within the console window, remove them, and navigate between them.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ H	HTS	Horizontal Tab Set	Sets a tab stop in the current column the cursor is in.
#define ESC_            "\33[ [ <n> I	CHT	Cursor Horizontal (Forward) Tab	Advance the cursor to the next column (in the same row) with a tab stop. If there are no more tab stops, move to the last column in the row. If the cursor is in the last column, move to the first column of the next row.
#define ESC_            "\33[ [ <n> Z	CBT	Cursor Backwards Tab	Move the cursor to the previous column (in the same row) with a tab stop. If there are no more tab stops, moves the cursor to the first column. If the cursor is in the first column, doesn’t move the cursor.
#define ESC_            "\33[ [ 0 g	TBC	Tab Clear (current column)	Clears the tab stop in the current column, if there is one. Otherwise does nothing.
#define ESC_            "\33[ [ 3 g	TBC	Tab Clear (all columns)	Clears all currently set tab stops.
For both CHT and CBT, <n> is an optional parameter that (default=1) indicating how many times to advance the cursor in the specified direction.
If there are no tab stops set via HTS, CHT and CBT will treat the first and last columns of the window as the only two tab stops.
Using HTS to set a tab stop will also cause the console to navigate to the next tab stop on the output of a TAB (0x09, \t) character, in the same manner as CHT.
Designate Character Set
The following sequences allow a program to change the active character set mapping. This allows a program to emit 7-bit ASCII characters, but have them displayed as other glyphs on the terminal screen itself. Currently, the only two supported character sets are ASCII (default) and the DEC Special Graphics Character Set. See the DEC Caracter Set for a listing of all of the characters represented by the DEC Special Graphics Character Set.

Sequence	Description	Behavior
#define ESC_            "\33[ ( 0	Designate Character Set – DEC Line Drawing	Enables DEC Line Drawing Mode
#define ESC_            "\33[ ( B	Designate Character Set – US ASCII	Enables ASCII Mode (Default)
Notably, the DEC Line Drawing mode is used for drawing borders in console applications. The following table shows what ASCII character maps to which line drawing character.

Hex	ASCII	DEC Line Drawing
0x6a	j	┘
0x6b	k	┐
0x6c	l	┌
0x6d	m	└
0x6e	n	┼
0x71	q	─
0x74	t	├
0x75	u	┤
0x76	v	┴
0x77	w	┬
0x78	x	│
Scrolling Margins
The following sequences allow a program to configure the “scrolling region” of the screen that is affected by scrolling operations. This is a subset of the rows that are adjusted when the screen would otherwise scroll, for example, on a \n or RI. These margins also affect the rows modified by Insert Line ( IL ) and Delete Line ( DL ), Scroll Up ( SU ) and Scroll Down ( SD ).

The scrolling margins can be especially useful for having a portion of the screen that doesn’t scroll when the rest of the screen is filled, such as having a title bar at the top or a status bar at the bottom of your application.

For DECSTBM, there are two optional parameters, <t> and <b>, which are used to specify the rows that represent the top and bottom lines of the scroll region, inclusive. If the parameters are omitted, <t> defaults to 1 and <b> defaults to the current viewport height.

Scrolling margins are per-buffer, so importantly, the Alternate Buffer and Main Buffer maintain separate scrolling margins settings (so a full screen application in the alternate buffer will not poison the main buffer’s margins).

Sequence	Code	Description	Behavior
"\33[ [ <t> ; <b> r	DECSTBM	Set Scrolling Region	Sets the VT scrolling margins of the viewport.
Window Title
The following commands allows the application to set the title of the console window to the given <string> parameter. The string must be less than 255 characters to be accepted. This is equivalent to calling SetConsoleTitle with the given string.

Note that these sequences are OSC “Operating system command” sequences, and not a CSI like many of the other sequences listed, and as such starts with \x1b], not \x1b[.

Sequence	Description	Behavior
#define ESC_            "\33[ ] 0 ; <string> BEL	Set Icon and Window Title	Sets the console window’s title to <string>.
#define ESC_            "\33[ ] 2 ; <string> BEL	Set Window Title	Sets the console window’s title to <string>.
The terminating character here is the Bell character, \x07.

Alternate Screen Buffer
*Nix style applications often utilize an alternate screen buffer, so that they can modify the entire contents of the buffer, without affecting the application that started them. The alternate buffer is exactly the dimensions of the window, without any scrollback region.

For an example of this behavior, consider when vim is launched from bash. Vim uses the entirety of the screen to edit the file, then returning to bash leaves the original buffer unchanged.

Sequence	Description	Behavior
#define ESC_            "\33[ [ ? 1 0 4 9 h	Use the Alternate Screen Buffer	Switches to a new alternate screen buffer.
#define ESC_            "\33[ [ ? 1 0 4 9 l	Use the Main Screen Buffer	Switches to the main buffer.
Window Width
The following sequences can be used to control the width of the console window. They are roughly equivalent to the calling the SetConsoleScreenBufferInfoEx console API to set the window width.

Sequence	Code	Description	Behavior
#define ESC_            "\33[ [ ? 3 h	DECCOLM	Set Number of Columns to 132	Sets the console width to 132 columns wide.
#define ESC_            "\33[ [ ? 3 l	DECCOLM	Set Number of Columns to 80	Sets the console width to 80 columns wide.
Soft Reset
The following sequence can be used to reset certain properties to their default values. The following properties are reset to the following default values (also listed are the sequences that control those properties):

Property	Default values
Cursor visibility	visible ( DECTEM )
Numeric Keypad	numeric mode ( DECNKM )
Cursor Keys	normal mode ( DECCKM )
Top and Bottom Margins	Top=1, Bottom=console height ( DECSTBM )
Character Set	US *ASCII*
Graphics Rendition	default/off ( SGR )
Save cursor state	home position (0,0) ( DECSC )
Sequence	Code	Description	Behavior
"\33[ [ ! p	DECSTR	Soft Reset	Reset certain terminal settings to their defaults.
Input Sequences
The following terminal sequences are emitted by the console host on the input stream if the ENABLE_VIRTUAL_TERMINAL_INPUT flag is set on the input buffer handle using the SetConsoleMode flag.

There are two internal modes that control which sequences are emitted for the given input keys, the Cursor Keys Mode and the Keypad Keys Mode. These are described in the Mode Changes section.

Cursor Keys
Key	Normal Mode	Application Mode
Up Arrow	"\33[ [ A	"\33[ O A
Down Arrow	"\33[ [ B	"\33[ O B
Right Arrow	"\33[ [ C	"\33[ O C
Left Arrow	"\33[ [ D	"\33[ O D
Home	"\33[ [ H	"\33[ O H
End	"\33[ [ F	"\33[ O F
Additionally, if [Ctrl] is pressed with any of these keys, the following sequences are emitted instead, regardless of the Cursor Keys Mode:

Key	Any Mode
Ctrl + Up Arrow	"\33[ [ 1 ; 5 A
Ctrl + Down Arrow	"\33[ [ 1 ; 5 B
Ctrl + Right Arrow	"\33[ [ 1 ; 5 C
Ctrl + Left Arrow	"\33[ [ 1 ; 5 D
Numpad & Function Keys
Key	Sequence
Backspace	0x7f, [DEL]
Pause	0x1a, [SUB]
Escape	0x1b, ["\33[]
Insert	"\33[ [ 2 ~
Delete	"\33[ [ 3 ~
Page Up	"\33[ [ 5 ~
Page Down	"\33[ [ 6 ~
F1	"\33[ O P
F2	"\33[ O Q
F3	"\33[ O R
F4	"\33[ O S
F5	"\33[ [ 1 5 ~
F6	"\33[ [ 1 7 ~
F7	"\33[ [ 1 8 ~
F8	"\33[ [ 1 9 ~
F9	"\33[ [ 2 0 ~
F10	"\33[ [ 2 1 ~
F11	"\33[ [ 2 3 ~
F12	"\33[ [ 2 4 ~
Modifiers
[Alt] is treated by prefixing the sequence with an escape: "\33[ <c> where <c> is the character passed by the operating system. [Alt+Ctrl] is handled the same way except that the operating system will have pre-shifted the <c> key to the appropriate control character which will be relayed to the application.

[Ctrl] is generally passed through exactly as received from the system. This is typically a single character shifted down into the control character reserved space (0x0-0x1f). For example, [Ctrl+@] (0x40) becomes NUL (0x00), [Ctrl+[] (0x5b) becomes "\33[ (0x1b), etc. A few [Ctrl] key combinations are treated specially according to the following table:

Key	Sequence
Ctrl + Space	0x00 (NUL)
Ctrl + Up Arrow	"\33[ [ 1 ; 5 A
Ctrl + Down Arrow	"\33[ [ 1 ; 5 B
Ctrl + Right Arrow	"\33[ [ 1 ; 5 C
Ctrl + Left Arrow	"\33[ [ 1 ; 5 D
Note [Left Ctrl + Right Alt] is treated as [AltGr]. When both are seen together, they will be stripped and the Unicode value of the character presented by the system will be passed into the target. The system will pre-translate [AltGr] values according to the current system input settings.

