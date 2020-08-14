# Custom Protocol Stack

Our team protocol stack design comprises of 5-layers, application, transport, network, datalink and physical. 
The implementation is a full, working, network architecture capable of supporting a smart lighting application 
for a Ill Matto and RFM12B hardware (shown at the end of the page). 3 Ill Mattos are implemented with 2 switches 
and 2 LEDs and has wireless control each other's LEDs. The architecture supports a distributed application, 
rather than requiring a centralised ‘hub’. Each layer has its own defined byte ‘struct’ format, comprised of 
‘uint8_t’ and ‘uint16_t’ variables, and a separate transmit and receive function. At the end of each layer’s 
function, it will either call the layer’s above or below, transmit or receive function, depending on the current
operation of the program. 
