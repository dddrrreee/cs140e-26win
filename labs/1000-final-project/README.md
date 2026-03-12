
## Useful links

- [IP Numbers](https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml)
- [FTP RFC 114](https://www.rfc-editor.org/rfc/rfc114.txt)
- [UDP RFC 768](https://www.rfc-editor.org/rfc/rfc768)
- [ICMP RFC 792](https://datatracker.ietf.org/doc/html/rfc792)
## Plan
-  [ ] Frames
   -  [ ] More filtering
      -  [ ] EtherType in higher level handler
      -  [ ] Length
-  [ ] ARP [RFC 826](https://www.rfc-editor.org/rfc/rfc826)
   -  already have FRAME_ARP
   -  Put into frame handler
-  [ ] IPv4
   -  [ ] More filtering
-  [ ] UDP
-  [ ] 
-  [ ] TCP
-  [ ] FTP
  

1. Ethernet frame filtering
2. Implement ARP table + ARP reply
3. Refactor IPv4 parsing into its own module
4. IPv4 checksum
5. UDP with ports
6. TCP
7. Build application protocols 
   1. FTP on TCP

## Working
- ARP
- IPv4
- UDP / ICMP



## Adding new protocol
- Change lower level protocol handler
- Add verbosity
- Add init to inet_init