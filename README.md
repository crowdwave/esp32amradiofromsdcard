# esp32amradiofromsdcard

bitluni's esp32 AM radio transmitter extended to play back from sd card

Original is at: https://github.com/bitluni/ESP32AMRadioTransmitter

You just need an esp32 with an sd card, plug a wire in to pin 25 (or is it 26? maybe you can experiment).

**For best results, connect/solder the wire from the esp32 directly to the radio antenna.  Keep this wire as sort as you can.**

To minimise transmitting interference:
- Put the esp32 in a shielded box.
- Put ferrite beads on the wire.
- Wrap it all in rf shielding tape.
- Make the wire from the esp32 as short as you can, and connect it directly into the antenna port of the radio you want to play on.

## ------ File Preparation Instructions ------

This guide will help you convert your `.wav` audio files to `.pcm` format.

### Supported Sample Rates (must be in filename):
- **8000 Hz**  
  Examples: `music_8000hz.pcm`, `voice_8k.pcm`
- **11025 Hz**  
  Example: `music_11025hz.pcm`
- **22050 Hz**  
  Examples: `music_22050hz.pcm`, `voice_22k.pcm`
- **44100 Hz**  
  Examples: `music_44100hz.pcm`, `voice_44k.pcm`
- **48000 Hz**  
  Examples: `music_48000hz.pcm`, `voice_48k.pcm`

---

### Batch Conversion (Convert Multiple Files):
1. Open a terminal or command prompt.  
2. Navigate to the folder containing your `.wav` files.  
3. Run the following command:

   ```bash
   for f in *.wav; do \
       rate=$(ffprobe -v error -select_streams a:0 -show_entries stream=sample_rate -of default=nw=1:nk=1 "$f"); \
       ffmpeg -i "$f" -f s8 -acodec pcm_s8 -ac 1 "${f%.*}_${rate}hz.pcm"; \
   done
   ```

#### This will:
- Convert each `.wav` file in the folder to `.pcm` format.
- Save the new file with the sample rate in the filename.  
- Examples: `music_44100hz.pcm`, `voice_22k.pcm`.

---

### Single File Conversion:
If you want to convert a single file, use:

```bash
ffmpeg -i input.wav -f s8 -acodec pcm_s8 -ac 1 output_44100hz.pcm
```

---

### Important Notes:
- Install `ffmpeg` and `ffprobe` before running commands.
- **Sample rate MUST be in the filename** (e.g., `_44100hz` or `_44k`).
- Files without a sample rate in the name will default to **22050 Hz**.
- Only `.pcm` files will be played.
- **Hidden files** (starting with `.`) are ignored.



License

CC0. Do whatever you like with the code but I will be thankfull if you attribute bitluni. Keep the spirit alive :-)

High five!

BUT THIS IS ILLEGAL! I hear you yell.

Here's what ChatGPT says about it - I recommend you ask your lawyer about the local laws where you live and then comply because there can be substantial fines.

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
