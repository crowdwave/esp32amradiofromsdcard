# esp32amradiofromsdcard

bitluni's esp32 AM radio transmitter extended to play back from sd card

Original is at: https://github.com/bitluni/ESP32AMRadioTransmitter

You just need an esp32 with an sd card, plug a wire in to pin 25 (or is it 26? maybe you can experiment).

**Instructions are in the source code for preparing files** - essentially format the card as fat32 and use ffmpeg to convert your audio files to pcm.

Be aware its illegal to transmit depending on where you live, though some countries have carve out laws for low power hobby transmissions.

**For best results, connect/solder the wire from the esp32 directly to the radio antenna.  Keep this wire as sort as you can.**

Put the esp32 in a shielded box. Put ferrite beads on the wire, Wrap it all in rf shielding tape.

License

CC0. Do whatever you like with the code but I will be thankfull if you attribute bitluni. Keep the spirit alive :-)

High five!
