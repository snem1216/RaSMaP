# RaSMaP
Wake-on-Lan console menu for Linux systems.
This is optimal for SSH servers being used as gateways for low-use systems such as homelabs that have multiple machines that need to be woken up on-demand.
This command must be run with root privileges.

## Installing, configuring, and running from .deb package (recommended)
```shell
$ wget https://github.com/snem1216/RaSMaP/releases/download/0.91-alpha/rasmap.deb
$ sudo dpkg -i rasmap.deb
$ sudo rasmap-config
$ sudo rasmap
```
## Manual building
How to build & run:
```shell
$ cmake -G "Unix Makefiles" .
$ make
$ sudo ./rasmap
```
To install:
```shell
$ make install
```
