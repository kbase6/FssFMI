# FssFMI

This repository contains an implementation of **Secure Full-Text Search using Function Secret Sharing**.

## Prerequisites

Before you can build and run the project, you need to install the SDSL library.

- [SDSL Library](https://github.com/simongog/sdsl-lite)

## Installation

### Downloading and Installing Steps

1. Clone this repository

    ```bash
    $ git clone https://github.com/u-tmk/FssFMI
    ```

2. Navigate to the repository directory

    ```bash
    $ cd FssFMI
    ```

3. Build the project

    ```bash
    $ make all
    ```

4. Create the data directory

    ```bash
    $ make dir
    ```

## Tested Environment

This implementation has been tested on the following environment:

- Intel 64-bit CPUs
- 64-bit Linux (tested on Ubuntu 20.04 LTS)
- GNU Make 4.2.1, gcc 9.0.4, OpenSSL 1.1.1f

## Usage

To execute the program, use the following command:

```bash
$ ./bin/fssmain <party_id> <mode> [options]

<party_id> : Party id (0 or 1) is required

<mode> : 'test' or 'bench' to specify execution mode

options:
    -p, --port <port_number> : Specify port number (default: 55555)
    -s, --server <server_address> : Specify server address (default: 127.0.0.1)
    -n, --name <function_name> : Specify function to run
    -m, --mode <mode> : Specify function mode
    -o, --output <output_file> : Specify output file name
    -i, --iteration <iteration> : Specify iteration number
    -h, --help : Display help message
```

## Running Tests

To run the tests, use the following command:

```bash
$ python3 ./src/experiments/run.py -u
```
