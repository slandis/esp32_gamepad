# HID/L2CAP via GAP

The GAP handling portion of this was adapted from https://github.com/NURobotics/PS4-esp32

Cheat a little and use some undocumented GAP API calls to establish L2CAP
channels for an HID compliant device. Example callback and basic printing of
data. Bluetooth device should only need to be paired once (no PIN needed),
simply reconnect on subsequent attempts. There's no attempt to manage disconnects
or anything of the sort as this is merely a proof of concept.

You must define gamepad_btaddr properly in main.c to connect to your device.
