# Arduino SD Music Player
## Introduction
This function of this project is to read wav files from a sd card and play it out with a speaker. This is how it basically works - </br>
1. Arduino reads wav file from sd card.
2. Sends output to a R-2R Dac, interfaced with 8 digital pins to further give out an analog music signal.
3. Signal is first stabilized by a Buffer circuit then after passing through a RC Low Pass, is stabilized by a Voltage Following Buffer circuit again. 
4. Then by controlling it's amplitude with a potentiometer(volume control), it is amplified with a Op-Amp and sent to the speaker to play the sound.
## Components used
* R-2R Ladder (as a DAC)
We used R-2R Ladder because it is easy to use and cheap as well. It will not give the highest audio quality but it is good enough for this project.
* TL072 (Dual OP-Amp)
We were given LM386 as a reference for the project. We used TL072 because it has significantly less noise, less power consumption, low offset voltage compared to other op amps,  and also it is easily available in the market. It amplifies weak signal voltage very efficiently. Two TL072 are used in the circuit - 
  * Both amplifiers of one TL072 are used in voltage follower buffer circuits to stabilize the signal coming from the R-2R DAC(as it has noise)
  * The other TL072 is used to amplify the audio signal to drive the speaker. Both the amps are connected as parallel voltage followers to maximise the current output.
* Arduino Mega 
Arduino Mega is used as the brain for the whole project. It was used instead of Arduino Uno because the code required more memory than available in arduino uno. We could modify the code a bit to fit in Uno but that could reduce stability.
