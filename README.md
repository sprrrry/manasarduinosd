# Arduino SD Music Player

## Introduction
This function of this project is to read wav files from a sd card and play it out with a speaker. This is how it basically works - </br>
1. Arduino reads wav file from sd card.
2. Sends output to a R-2R Dac, interfaced with 8 digital pins to further give out an analog music signal.
3. Signal is first stabilized by a Buffer circuit then after passing through a RC Low Pass, is stabilized by a Voltage Following Buffer circuit again. 
4. Then by controlling it's amplitude with a potentiometer(volume control), it is amplified with a Op-Amp and sent to the speaker to play the sound.

## Components 
* **R-2R Ladder** (as a DAC) </br>
R-2R Ladder has been used here because it is easy to use and cheap as well. It will not give the highest audio quality but it is good enough for this project.

* **Low Pass Filter** </br>
The purpose of a low pass filter is to smooth out the output of the DAC in order to reduce noise.  By using a low pass filter on the signal, you can smooth out the "steps" in your waveform while keeping the overall shape of the waveform intact. A simple RC flow pass filter is used to achieve this: with a resistor and a capacitor. This filtered signal is then sent into another buffer circuit (op amp in a voltage follower configuration) to protect the filtered signal from any loads further down in the circuit. 
<p align="center">
Cutoff Frequency = 1/ (2*pi*R*C) </br>
For a cutoff frequency of 20,000Hz and 1kOhm resistor: 20000=1/(2*3.14*1000*C) </br>
C = ~8nF  </br>
</p> 
&emsp;&emsp;Since 8nF capacitors are hard to by, a 0.01uF capacitor has been used. This gives a cutoff frequency of about 16kHz.

* **Potentiometer** </br>
A potentiometer has been used to control the amplitude of the signal. This functions as a volume control knob.

* **TL072 (Dual OP-Amp)** </br>
The amplifier is used to increase the current of the signal so that it can drive a load (like a speaker). TL072 was used because it has significantly less noise, less power consumption, low offset voltage compared to other op amps,  and also it is easily available in the market. It amplifies weak signal voltage very efficiently. Two TL072 are used in the circuit - 
  * Both amplifiers of one TL072 are used in voltage follower buffer circuits to stabilize the signal coming from the R-2R DAC. The R-2R DAC is very sensitive to any loads put on it, so trying to drive speakers directly from the DAC will distort the signal heavily.
  * The other TL072 is used to amplify the audio signal to drive the speaker. Both the amps are connected as parallel voltage followers to maximise the current output(60mA*2=120mA). 

* **Arduino Mega**  </br>
Arduino Mega is used as the brain for the whole project. It was used instead of Arduino Uno because the code required more memory than available in arduino uno. The code could be modified a bit to fit in Uno but that could reduce the stability and functionality.

## Simulation
![This is Proteus Simulation](https://www.linkpicture.com/q/Screenshot-2022-02-01-114313.png)

