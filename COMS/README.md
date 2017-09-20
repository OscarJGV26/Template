# COMS
This library sends 6 arrays of floats over UDP. Each of this arrays has a identifier e.g. "DATA", a scaler to send the data using only 2 bytes whilst preserving some digits, and a checksum. 

On the otherside, the UDP server must receive all the message and then look for the data identifiers, check that the checksum matches, cast the respective data into a float, then divide over the scaler to recover the original scale.

An example is given function Coms_Test. Calling ./Coms_Test 50 1 0.02 sends a sinusoidal signal with amplitude of 50, frequency of 1 Hz and 0.02s sampling time on identifiers "ANGL","RATE","REFA","REFR"; on identifier "MOTO" it scales it to 500 and offsets it by 800; and on identifiers "ROLP","PITP" it scales it to one.

The user can select how often in multiples of the sampling time can send with the coms.begin(limit) function.
