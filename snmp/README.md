# dstransducer-snmp

Allows to make values read by dsreadout available via SNMP.

Can be installed in `/etc/snmp/snmpd.conf` using

  `pass_persist .1.3.6.1.4.1.21695.1.5 dstransducer-snmp dstransducer-snmp.conf`

Important: make sure snmpd has access to the device listed in the configuration
file. This can be accomplished by starting snmpd with the `-g dialout` option in
`/etc/default/snmpd`:

  `SNMPDOPTS='<option> -g dialout <options>'`

