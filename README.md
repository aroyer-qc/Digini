# Digini
GUI and low level library C++

![alt text](https://github.com/aroyer-qc/Digini/blob/master/Digini.png)

# Digini

Digini is a HAL and GUI library written in embedded C++. It is highly optimized and is also a highly configurable graphic library. GUI is based on Widget/Service concept. What this mean is, your application is running on his own task loop, and is not aware the GUI even exist. the GUI on its side is running multiple task to display graphic page. Those page are a chain list of widget, and those widget are configured with information about position, graphic to use, service to call to display the proper information and link to other page. all information is processed via variables and/or call to users functions.

  ## Note(s):
  
>   * Widget are easy to add in Digini.
    
  ## Feature:

>   * Code is C++ (following embedded rules found on IAR C++ document [Inefficiencies of C++: Fact or Fiction?](https://github.com/aroyer-qc/Digini/blob/master/inefficiencies%20of%20c++fact%20or%20fiction.pdf)).
>   * Run on nOS from Jim Tremblay. See [RTOS for microcontrollers](https://github.com/jimtremblay/nOS)
>   * Digini graphics run parallel and independant of the main application, all graphics are handles via service.
>   * Data are display by hooking to variables and/or by calling function(s), input are also done via variables and/or function(s). no graphic code exist in application.
>   * Control over the application can be done via state machine and/or message at the discretion of the application user.
>   * Skin data is loaded from SD card via a skin file (.skn). to build a skin file you can used my application [SkinGUI_Builder](https://github.com/aroyer-qc/SkinGUI_Builder)
  
  ## Widget list so far:
  
>    * Background widget to display background layer.
>    * Basic box.
>    * Basic button with text (no graphic).
>    * Basic rectangle.
>    * Button with text, icon and support for multiple graphic so button can have different state. ( EX. normal, pressed, gray out)
>    * Exchange is a non grahical special widget to exchange data between graphic page.
>    * Gif to display gif as animation.
>    * Graph display XY plot live or static (WIP).
>    * Icon support for multiple graphic to display different state.
>    * Label to display static/formatted label. also support for different language.
>    * Label list, like label but with a list different label according to state.
>    * Meter, (WIP).
>    * Panel, can add touch zone with graphics under other widget.
>    * Progress bar.
>    * Spectrum analyser display (Use data from application, no processing).
>    * Terminal basic terminal screen.
>    * Virtual is a non grahical special widget to decide whart page to display according to application setting.
>    * Virtual windows (WIP).
  
  ## Standard service provided in library
  
>    * SERV_ID_BACK    Service to copy background to background layer.
>    * SERV_ID_BDEF    Service to handle button used only to jump to other page link. Ex. Setting, Back, Ok, etc...
>    * SERV_ID_DATE    Service to return the date of the system into a formatted string.
>    * SERV_ID_DAYS    Service to return the days of the system into a formatted string.
>    * SERV_ID_INPT    Service to input data via a generic page in decimal or hexa.
>    * SERV_ID_INPS    Service to input string via a generic page. (WIP).
>    * SERV_ID_NONE    Service to return default state
>    * SERV_ID_RFSH    Service to display static widget
>    * SERV_ID_SKLD    Service to return percentage of the skin loading progression
>    * SERV_ID_TIME    Service to return the timeof the system into a formatted string (can handle all format defined by user).
>    * SERV_ID_XCHG    Service to exchange data via structure between page. (title, min, max, value, edit type)


