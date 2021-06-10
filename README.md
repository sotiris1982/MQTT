# MQTT
# Programming of embedded IoT board and communication of data through cloud

The purpose of this PROJECT is to get familiar with programming in the field of embedded processors and IoT systems. 
Part of the exercise is to collect sample data and communicate it to cloud server through MQTT protocol
Design Description
The board of interest is an ESP32, a dual core system which when connected with the DHT22 sensor it can receive temperature
and humidity measures and exchange this data with the MQTT broker.
> * Communicating with MQTT is happening through the pub/sub model. In this model clients are decoupled from each other 
and the mediator is the broker that handles the distribution of messages to all subscribers. This exercise deals only 
with the client side and how to connect with the MQTT broker through specific steps.
> * Important part of the pub/sub model are the topics in which every client can publish and subscribe. Clients can 
subscribe to any topic as long as they are aware of its existence.
## Code Description

After publishing to the topic/control, the broker sends back a six digit number which includes all the details
of the thresholds that the sensor needs. This message is a string that needs to be converted into integer in order 
to use it with the CRC algorithm and send it back to the broker for acceptance, the broker acceptance allows start measuring and collecting data.

For this purpose the creation of a function was necessary (sixDigit()). This function takes as an argument the 
string message from the broker and is using three arrays to store the pairs of characters from the incoming
string for further manipulation. The atoi() function converts string into an integer and the lowByte() extracts 
the right most byte witch is the one we need. We take for granted that for this exercise the room temperature will 
not go into extreme of minus so only positive (unsigned) numbers are required.

Inside the sixDigit function the crcCalc() function is called where by using the CRC algorithm provided by
[1] (http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html) it calculates the CRC for every pair
of data that has to be send back to the broker for comparison. For this case every pair of the integers is 
calculated and then is converted back to string of nine characters this time, by applying zero padding. 

The nine digit code now will be send back to the broker for comparison and if agreed, grant for start collecting data.
The code for getting the temperature and humidity measurement was taken by [2] and only a few changes were required 
for new and threshold values comparisons and saving the data in a FIFO alike array.
Last part of the exercise was the bonus in which the requirement was to use the least square method to the five most 
updated temperature values. For this part, the code from [3] was taken as reference and modified to serve the purpose o
f the exercise. The X axis was a pre-defined array of all the five sampling data rate and Y axis was data from the FIFO designed in the previous step.

## Conclusion
The tricky part of this exercise and the most important was the understanding of the MQTT protocol and what is required
in order to establish a connection and be able to publish and subscribe to topics of interest.
