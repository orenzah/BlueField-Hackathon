# Welcome to my project at the Bluefield Hackathon of Mellanox Technologies
Compression on-the-fly using LZ4 method

## Description
We've implemented a concept of compression (encryption as well) independent of the host, when the host is sending packets over the transport layer (UDP).
The compression is done by the SmartNIC (Bluefield chip which is ARM based), that is the sender side.
The inflating (decompress) may be done by another SmartNIC, or simply as we've done by the host.

### Sender
The sender part main challenge was to compress the payload of the packet (UDP payload), so the host won't know nothing about that process.
The SmartNIC has to parse the whole packet and divide it to { MAC Header, IP Header, UDP  Header, Payload }.
The SmartNIC will let any other packet types to be forwarded to the interface (and host as well) without being elevated to the kernel stack, only UDP packets will be elevated and then will be parsed inside the application.


