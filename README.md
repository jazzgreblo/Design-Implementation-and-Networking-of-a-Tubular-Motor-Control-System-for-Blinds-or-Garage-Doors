# Design-Implementation-and-Networking-of-a-Tubular-Motor-Control-System-for-Blinds-or-Garage-Doors
PROJEKTIRANJE, IMPLEMENTACIJA I UMREŽAVANJE SUSTAVA ZA UPRAVLJANJE CJEVASTIM MOTORIMA ROLETA ILI GARAŽNIH VRATA
- dokumentacija diplomskog rada

PIC18F47Q10_MPLAB_code is the complete folder with all MPLAB files used to program and create files for the PIC18F47Q10 microcontroler

wavegen_uart_gui.py is the python application used to communicate with the DIGILENT Analog Descovery 3 device

CurrentDetectionCircuit is a LTSpice file that was used to test a current detection circuit that generates a square wave for 
voltages above 150mV on the load resistor. The capacitors on the input are used to create a reference point to GND because
the input is a current transformer connected to the load current of a motor. The next part is a differental amplifier used
to add a DC offset to the input voltage to put the input in the middle of the voltage supply range. Lastly a histeresis comparator
is used to define the value of the detected voltage. 
The values of the components in the histeresis comparator were calculated from Texas Instruments guide on histeresis comparators
that can be found in the file: tidu020a_Comparator with Hysteresis Reference Design.pdf
