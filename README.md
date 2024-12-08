# esp32amradiofromsdcard

bitluni's esp32 AM radio transmitter extended to play back from sd card

Original is at: https://github.com/bitluni/ESP32AMRadioTransmitter

You just need an esp32 with an sd card, plug a wire in to pin 25 (or is it 26? maybe you can experiment).

**Instructions are in the source code for preparing files** https://github.com/crowdwave/esp32amradiofromsdcard/blob/main/main.ino - essentially format the card as fat32 and use ffmpeg to convert your audio files to pcm.

Be aware its illegal to transmit depending on where you live, though some countries have carve out laws for low power hobby transmissions.

**For best results, connect/solder the wire from the esp32 directly to the radio antenna.  Keep this wire as sort as you can.**

Put the esp32 in a shielded box. Put ferrite beads on the wire, Wrap it all in rf shielding tape.

License

CC0. Do whatever you like with the code but I will be thankfull if you attribute bitluni. Keep the spirit alive :-)

High five!

BUT THIS IS ILLEGAL! I hear you yell.

Here's what ChatGPT says about it - I recommend you ask your lawyer.

In the United States, the Federal Communications Commission (FCC) permits low-power, unlicensed AM radio transmissions under specific conditions outlined in Part 15 of its regulations. These provisions are designed to allow hobbyists and other users to operate without an individual license, provided they adhere to certain technical limitations to minimize potential interference with licensed services.

**Key Provisions for Low-Power AM Transmissions:**

1. **Transmitter Power and Antenna Restrictions:**
   - Under **Section 15.219**, the input power to the final radio frequency stage (exclusive of filament or heater power) must not exceed 100 milliwatts (0.1 watts).
   - The combined length of the transmission line, antenna, and ground lead (if used) must not exceed 3 meters (approximately 10 feet).

2. **Field Strength Limits:**
   - Alternatively, **Section 15.209** specifies general radiated emission limits, where the field strength of the transmitted signal must not exceed 2400/𝑓 (kHz) microvolts per meter at 300 meters.
   - For example, at 1,000 kHz (1 MHz), the maximum field strength would be 2,400 microvolts per meter.

3. **Campus and Carrier Current Systems:**
   - **Section 15.221** allows for AM transmissions on educational campuses, provided the field strength does not exceed 15.209 limits at the campus perimeter.
   - Carrier current systems, which transmit signals through electrical wiring, must also comply with specific radiated emission limits.

**Important Considerations:**

- **Interference:** Operators must ensure their transmissions do not cause harmful interference to licensed services and must accept any interference received.
- **Equipment Compliance:** Transmitters should be designed to comply with Part 15 standards to minimize the risk of interference.
- **Operational Range:** Due to power and antenna limitations, the effective range of such low-power AM transmissions is typically limited to a few hundred feet.

For detailed information, refer to the FCC's [Part 15 regulations](https://www.ecfr.gov/current/title-47/chapter-I/subchapter-A/part-15) and the [OET Bulletin No. 63](https://transition.fcc.gov/Bureaus/Engineering_Technology/Documents/bulletins/oet63/oet63rev.pdf), which provides guidance on low-power, non-licensed transmitters.

By adhering to these guidelines, hobbyists can legally experiment with AM radio transmissions within the specified parameters. 
