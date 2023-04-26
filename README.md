#ntptpmon#

Small program to monitor Napatech interface PTP status on Linux hosts.

## Usage ##

`ntptpmon [-e]`

With no options, prints status at 1 second incrments.

With the -e option, writes a Prometheus `node_exporter` textfile at `/var/lib/node_exporter/textfiles/ntptp.prom`.` 

## Dependencies ##
Requires Napatech libs and includes. Default path is `/opt/napatech3`, change in Makefile if different.

## Installation ##
`make`, and copy executable to desired location.

`ntptpmon -e` is intended to be run as a service from systemd.

