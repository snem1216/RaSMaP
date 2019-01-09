# RaSMaP
Wake-on-Lan console menu for Linux systems.
This is optimal for SSH servers being used as gateways for low-use systems such as homelabs that have multiple machines that need to be woken up on-demand.

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
An official Linux package will be coming soon!
