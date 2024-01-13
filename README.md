<!DOCTYPE HTML>
<html>
<head>
</head>

<h1 align="center">SNMP Lablights Project</h1>
<p>This Project is intended for my personal use and was made solely by me, others may use it for their own purposes.</p>
<p></p>

<h2>Some Important Functions</h2>
<div class="box">
  <pre><h2>snmp.h</h2><b>void SNMPsetup(int Array[])</b> -- Sets up a list of ports that you will be pulling data from
<b>int snmpInLoop(int Array[])</b> -- Returns the Average difference in data since the last poll coming IN to the switch.
<b>int snmpOutLoop(int Array[])</b> -- Returns the Average difference in data since the last poll coming OUT of the switch.
<b>void printVariableHeader()</b> -- Prints switch name & uptime to serial monitor
<b>void printVariableFooter()</b> -- Prints elapsed time between polls and summary of data

<h2>lablights.h</h2><b>void initFastLED()</b> -- will add your strip to fastled index (required)
<b>void litArray()</b> -- Must be called in every iteration of your main loop() function (runs as a frame)
<b>void forwardEvent(CRGB fColor, int strip)</b> -- Sends a pulse down the strip from index 0
<b>void reverseEvent(CRGB rColor, int strip)</b> -- Sends a pulse towards the beginning from the end of the strip

<h2>Other Important Info</h2><b>YOU MUST EDIT YOUR SETTINGS IN globals.h</b>
<b>Change your WIFI in secrets.h</b>
<b>Make sure platform.io is building for your device</b>
<p></p>
<b>Currently Supported Devices:</b> LilyGo T-Display S3, M5Stick-C
</pre>
</div>
</html>
