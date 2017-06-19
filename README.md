# nRF24 tools

## Travis CI status
[![Build Status](https://travis-ci.org/geomatsi/nrf24-tools.svg?branch=master)](https://travis-ci.org/geomatsi/nrf24-tools)

## Download

```bash
$ git clone https://github.com/geomatsi/nrf24-tools
$ cd nrf24-tools
```

## Build with UM232H backend

```bash
$ mkdir build && cd build
$ cmake .. -DWITH_UNIT_TESTS=ON -DNRF24_CONN='UM232H'
$ make
$ make test

```

or

```bash
$ mkdir build && cd build
$ cmake .. -DWITH_UNIT_TESTS=ON -DNRF24_CONN='UM232H' -G Ninja
$ ninja
$ ninja test
```

### SBC connector

```bash
$ mkdir build && cd build
$ cmake .. -DKERNEL_DIR=/usr -DWITH_UNIT_TESTS=ON -DNRF24_CONN='SBC'
$ make
$ make test

```

or

```bash
$ mkdir build && cd build
$ cmake .. -DKERNEL_DIR=/usr -DWITH_UNIT_TESTS=ON -DNRF24_CONN='SBC' -G Ninja
$ ninja
$ ninja test
```
