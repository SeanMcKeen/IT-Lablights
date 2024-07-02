<!DOCTYPE HTML>
<html>
<head>
</head>

<h1 align="center">SNMP Lablights Project</h1>
<p>This Project is intended for my personal use and was made solely by me, others may use it for their own purposes if they find it useful.</p>

<h2>Overview</h2>
This Project is an fully configurable LED project that uses SNMP data in order to display lights that show network traffic. 
Currently pulses have a small trail effect, and can go forward and backward on the strip with Independant data. 
This means that your first strip can show network traffic on a set list of ports, and the next strip can use another list of ports. 
This is a seamless process and you will see no lag on your lights while it's polling SNMP. 
Also, it will not pollute or spam your switch as long as the delay is set at 10 or more seconds. 
(You may be able to go less, but I wouldn't recommend it).

<h2>Roadmap</h2>
Eventually I intend to add display/screen functionality for devices like the lilygo and m5stick-c, but as of right now that is a low priority.
<p></p>

<h2>Some Important Functions</h2>
<div class="box">
  <pre><h2>snmpgrab.h</h2><b>void SNMPsetup(int Array[])</b> -- <i>Sets up a list of ports that you will be pulling data from.</i>
<b>void snmpLoop(int Array[], int arrayCount, int arrayIndex)</b> -- <i>Calculates the average difference in data since the last poll coming IN and OUT of the switch.</i>
<b>void printVariableHeader()</b> -- <i>Prints switch name & uptime to serial monitor.</i>
<b>void printVariableFooter()</b> -- <i>Prints elapsed time between polls and summary of data.</i>
<b>void callLoop</b> -- <i>Should be called as often as possible according to SNMP library documentation</i>

<h2>lablights.h</h2><b>void initFastLED()</b> -- <i>Will add your strip to fastled index (required)</i>
<b>void litArray()</b> -- <i>Must be called in every iteration of your main loop() function (runs as a frame)</i>
<b>void forwardEvent(CRGB fColor, int strip)</b> -- <i>Sends a pulse down the strip from index 0.</i>
<b>void reverseEvent(CRGB rColor, int strip)</b> -- <i>Sends a pulse towards the beginning from the end of the strip.</i>

<h2>mathhandler.cpp handles the data thresholds for colors and pulse amounts, change it to your liking.</h2>
</pre>
</div>

<h2>Other Important Info</h2>

- <b>YOU MUST EDIT YOUR SETTINGS IN globals.h & platformio.ini</b>
- <b>Change your WIFI in secrets.h</b>
- <b>Make sure platformio is building for your device</b>

<p></p>
<b>Currently Supported Devices:</b> LilyGo T-Display S3, M5Stick-C

<h2>Instructions (READ CAREFULLY)</h2>
<div class="box"><pre><h3>For 4 or Fewer Strips:</h3>
  
- Change your settings in globals.h and secrets.h, making sure everything is set for your device.
- In platformio.ini, make sure there's a build for your device, if not, you may have to create it yourself through trial and error.
- After setting your arrays and strips in globals.h everything should work without changing anything up to a total of 4 strips.

<h3>To Add More Strips:</h3>

- Establish new variables in globals.h and change MAX_CHANNELS to the amount of strips you'll use.
- In lablights.cpp, fix the #IF statement on lines 14-26 to add bools for your new strips.
- Also in lablights.cpp, set more LedSplits on line 31 and repeat the process in initFastLed() for each strip.
- In lablights.h extern the bools.
- In main.cpp there will be blocks of variables from line 15 to 67, you'll need to follow the instructions in the comments.
- On line 76 add arrays, then in setup() follow the pattern and add your arrays to SNMPsetup()
- Replicate the process done inside loop() on line 150 to 162 for each strip, changing variable names to the ones you made before.
- On line 165 to 173, repeat that process with the variables you made before.
- Then call each pulse just as it is done below that on lines 178 to 188.
- Then move to snmp.cpp, on lines 47 to 54, replicate that process for each strip.
- Again, replicate the process on lines 72 to 82 for each strip.
- In handleAllOutputs() again, continue the process for the totals at the top of it.
- Then, also in handleAllOutputs(), replicate the else if() loops for each arrayIndex (arrayIndex means strip number)
- The first For loop is for INs and the second is for OUTs, you need to continue the else if()s in both.
- Remember to change every variable that has a number like lastInOctets4 to lastInOctets5
- in setTotals() again, replicate for each arrayIndex
- Optionally you can replicate the print process in printVariableFooter() for debugging.
- Finally, in snmpgrab.h, extern each arrTotals variable at the top as done already.</pre></div>
</html>
