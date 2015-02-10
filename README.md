# datastream-transducer-readout

This is a program for working with Data Stream digital transducers by [CR
Magnetics, Inc.](http://www.crmagnetics.com). It works with transducers
connected to an RS485 to RS232 converter and allows to configure transducers
and read all sorts of power and energy data from them.

The program has been developed for Linux, but shouldn't be too difficult to
port to other systems.

## Supported Transducers

At the moment, the following digital transducers are supported:

  * CRD5110-300-25 
  * CRD5170-300-5 

Adding support for other transducers should be pretty straightforward.

## Usage

After compiling the program, it offers different operations:

  * `dsreadout --identify A` Identify the transducer at address `A` (0-255). 
  * `dsreadout --read A` Read current values from the transducer at address `A`. 
  * `dsreadout --clear A` Clear the energy totalizer of transducer `A`. 
  * `dsreadout --scan` Scan all 256 addresses for transducers. This operation is very slow. 

The following two operations are not meant to be used on a bus to which
multiple transducers are connected. They are for the initial configuration of
your transducers (one at a time!). Using them on multiple transducers will
require you to reprogram all your transducers! Use with care, you have been
warned!

  * `dsreadout --reset` Reset transducers to factory defaults. 
  * `dsreadout --set-address A` Reset transducers to factory defaults and configure the transducer at address 0 to address `A`. 

## Problems

In the case of errors on the RS485 bus (e.g. due to bad wiring), transducers
can be messed up and return non-deterministic values. A power cycle of the
whole system (transducers + RS232 to RS485 converter) helped in such cases.

## Todo

  * Add support for other transducer types. 
  * Add support for scaling values when an external transformer is used. 

## Author

The code was written by [Oliver Hitz](mailto:oliver@net-track.ch). The author
is not affiliated with CR Magnetics.

## License

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA.

