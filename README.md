<!DOCTYPE HTML>
<html>
<head>
</head>

<h1 align="center">SNMP Lablights Project</h1>
<p>This Project is intended for my personal use and was made solely by me, others may use it for their own purposes.</p>
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
</pre>
</div>

<h2>Other Important Info</h2>

- <b>YOU MUST EDIT YOUR SETTINGS IN globals.h</b>
- <b>Change your WIFI in secrets.h</b>
- <b>Make sure platform.io is building for your device</b>

<p></p>
<b>Currently Supported Devices:</b> LilyGo T-Display S3, M5Stick-C
</html>
