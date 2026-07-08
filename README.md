# Demonstration of Sampling Theorem and Successive Approximation ADC

## Overview

This project experimentally verifies the **Nyquist–Shannon Sampling Theorem** using an analog Sample-and-Hold (S/H) circuit, and demonstrates analog-to-digital conversion using a discrete, low-resolution **8-bit Successive Approximation Register (SAR) ADC** built from a comparator, an external DAC, and an Arduino. The digitized signal is reconstructed with a second DAC followed by an RC low-pass (reconstruction) filter, and the effects of under-sampling, Nyquist-rate sampling, and over-sampling are observed on an oscilloscope.

## Objectives

- Demonstrate the Sampling Theorem and observe aliasing due to under-sampling.
- Design and implement a Sample-and-Hold circuit.
- Implement a low-resolution SAR ADC using a comparator, a DAC, and an Arduino.
- Reconstruct the digitized signal using a DAC and a low-pass filter.
- Measure ADC performance parameters: resolution/LSB, offset error, gain error, DNL, and INL.

## Hardware Used

| Component | Purpose |
|---|---|
| Function Generator | Generates the analog sinusoidal input and the sampling control pulse |
| Oscilloscope (CRO) | Observes S/H output, quantized staircase, and reconstructed signal |
| BS170A N-channel MOSFET | Analog switch for the Sample-and-Hold circuit |
| Hold Capacitor (220 nF) | Stores the sampled voltage during the hold phase |
| LM393 Comparator | Compares sampled input voltage against the SAR DAC trial voltage |
| MCP4921 DAC (×2) | One for SAR trial voltages, one for final signal reconstruction |
| Arduino | Implements the SAR conversion logic and drives both DACs over SPI |
| RC Low-Pass Filter (R = 22 kΩ, C = 1 µF) | Reconstructs the smooth analog signal from the DAC staircase output |

## How It Works

1. **Sample-and-Hold:** A 2 Vpp, 1 Hz sinusoid is applied to the drain of the BS170A MOSFET. A 9 Vpp pulse train (configurable frequency, ~22% duty cycle) drives the gate. While the pulse is high, the MOSFET conducts in the triode region and the hold capacitor charges to the instantaneous input voltage. While the pulse is low, the MOSFET is off and the capacitor holds that voltage steady for the ADC to digitize.
2. **SAR ADC (8-bit):** Starting from the MSB, the Arduino tentatively sets each bit, converts the trial code to a voltage via the MCP4921 DAC, waits for the DAC to settle, and reads the LM393 comparator output. If the held input voltage is less than the DAC trial voltage, the bit is cleared; otherwise it is kept. This binary search runs for all 8 bits, requiring 8 comparison cycles per conversion.
3. **Reconstruction:** The final 8-bit code is written to a second MCP4921 DAC, producing a staircase waveform, which is then smoothed by a first-order RC low-pass filter (cutoff ≈ 7.32 Hz) to recover an analog approximation of the original signal.
4. **Sampling rate sweep:** By varying the sampling frequency relative to twice the input signal frequency (Nyquist rate), aliasing (under-sampling), critical sampling (Nyquist rate), and faithful reconstruction (over-sampling) are all observed.

## Firmware

The Arduino sketch (`sar_adc.ino`) implements:

- `writeDAC(csPin, value)` — sends a 12-bit value to an MCP4921 DAC over SPI using the `0011 D11...D0` command format.
- `sarADC()` — performs the 8-bit successive-approximation binary search using the comparator on pin `A0`.
- `setup()` — configures SPI, chip-select pins, and serial output.
- `loop()` — runs a conversion, writes the result to the reconstruction DAC, and logs the ADC code, quantized voltage, and debug analog reading (`A1`) over serial at a ~15 Hz sampling rate.

### Pin Mapping

| Signal | Pin |
|---|---|
| SAR DAC chip select | D10 |
| Reconstruction DAC chip select | D9 |
| Comparator output | A0 |
| Sample-and-hold output (debug) | A1 |

## Theory Summary

- **Sampling Theorem:** A signal band-limited to `f_m` can be perfectly reconstructed if sampled at `f_s ≥ 2·f_m` (the Nyquist rate). Sampling below this rate causes **aliasing**, where distinct frequency components become indistinguishable.
- **Reconstruction:** An ideally sampled, band-limited signal can be recovered exactly via the Whittaker–Shannon interpolation formula using an ideal low-pass filter with cutoff `f_m`. In practice, a first-order RC filter approximates this.
- **SAR ADC:** Resolves an analog voltage to an N-bit digital code via binary search against a DAC reference, requiring one comparison cycle per bit.

## Measured ADC Performance

| Parameter | Theoretical | Measured |
|---|---|---|
| Resolution (LSB) | 5 V / 2⁸ ≈ 19.53 mV | ≈ 20 mV |
| Offset Error | — | Negligible |
| Gain Error | Full-scale = 5 V | ≈ 3.98 V (limited by MCP4921 output swing) |
| DNL | — | ≈ 0.024 LSB (good linearity) |
| INL | — | Approximately linear; minor deviation from DAC/comparator non-idealities |

## Results

- At the Nyquist rate (`f_s = 2·f_signal`), successive samples land on identical phase points of the sinusoid, producing a nearly constant, phase-sensitive output.
- For `f_s < 2·f_signal`, aliasing appears at `f_alias = |f_signal − k·f_s|`.
- For `f_s > 2·f_signal`, the reconstructed waveform closely matches the original sinusoid apart from quantization effects.

## Repository Contents

- `Report.pdf` — Full lab report with theory, circuit diagrams, oscilloscope captures, and derivations.
- `sar_adc.ino` — Arduino firmware implementing the SAR ADC and DAC reconstruction.

## Conclusion

This project demonstrates, end-to-end, how an analog signal is sampled, held, digitized via successive approximation, and reconstructed — validating the Nyquist–Shannon Sampling Theorem in hardware and characterizing the practical non-idealities (offset, gain error, DNL, INL) of a discrete SAR ADC implementation.
