# Arduino SD Music Player
## Introduction
This function of this project is to read wav files from a sd card and play it out with a speaker. This is how it basically works - </br>
1. Arduino reads wav file from sd card.
2. Sends output to a R2R Dac, interfaced with 8 digital pins to further give out an analog music signal.
3. Signal is first stabilized by a Buffer circuit then after passing through a RC Low Pass, is stabilized by a Voltage Following Buffer circuit again. 
4. Then by controlling it's amplitude with a potentiometer(volume control), it is amplified with a Op-Amp and sent to the speaker to play the sound.
