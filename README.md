<h1 align="center">MBMS Middleware</h1>
<p align="center">
  <img src="https://img.shields.io/badge/Status-Under_Development-yellow" alt="Under Development">
  <img src="https://img.shields.io/github/v/tag/5G-MAG/rt-mbms-mw?label=version" alt="Version">
  <img src="https://img.shields.io/badge/License-AGPL_v3-blue.svg" alt="License">
</p>

## Introduction

The MBMS Middleware implementation provides the best available content to the (internal or external) application at any time. If available, it combines content from (mobile) broadband, WiFi with the 5G BC content from the MBMS Modem using an advanced decision logic. The content is presented to the applications in form of an intelligent edge cache ready for pick up via http(s).

![Architecture](https://github.com/5G-MAG/Getting-Started/blob/main/media/architecture/5G-MAG%20RT%20Architecture%20Current%20Architecture%205G%20Media%20Client%20v8.drawio.png)

> **Status**: The specific parts of the *MBMS Middleware*, especially outer interfaces and the surrounding environment is currently subject of investigation of the project team.  The project team targets to use as much as possible from existing specifications and concepts (3GPP, DVB,...) for the *MBMS Middleware*.

Additional information can be found at: https://5g-mag.github.io/Getting-Started/pages/lte-based-5g-broadcast/

## About the implementation
* In the current release 0.9.x the *MBMS Middleware* uses the UDP multicast IP packets from the *MBMS Modem*.
* If the payload contains FLUTE decoded content (files, i.e. Service Announcement, DASH, HLS) the *MBMS Middleware* decodes the packets with its FLUTE/ALC decoder into files.
* The *MBMS Middleware* includes a web-cache server and each service is available like an CDN publishing point including manifest and segment files.
* Information how to access the endpoints (e.g. URL to manifest.m3u8 or manifest.mpd) can be found on the corresponding Middleware tab in the *GUI* or *Webinterface*.
> **Note:** The FLUTE/ALC decoder is available as an independent repository (library) [here](https://github.com/5G-MAG/libflute) and includes encoding/decoding functionalities.

## Install dependencies

### Ubuntu 20.04 LTS

````
sudo apt update
sudo apt install ssh g++ git libboost-atomic-dev libboost-thread-dev libboost-system-dev libboost-date-time-dev libboost-regex-dev libboost-filesystem-dev libboost-random-dev libboost-chrono-dev libboost-serialization-dev libwebsocketpp-dev openssl libssl-dev ninja-build libspdlog-dev libmbedtls-dev libboost-all-dev libconfig++-dev libsctp-dev libfftw3-dev vim libcpprest-dev libusb-1.0-0-dev net-tools smcroute python-psutil python3-pip clang-tidy gpsd gpsd-clients libgps-dev libgmime-3.0-dev libtinyxml2-dev libtinyxml2-6a
sudo snap install cmake --classic
sudo pip3 install cpplint
````

### Ubuntu 22.04 LTS
````
sudo apt update
sudo apt install ssh g++ git libboost-atomic-dev libboost-thread-dev libboost-system-dev libboost-date-time-dev libboost-regex-dev libboost-filesystem-dev libboost-random-dev libboost-chrono-dev libboost-serialization-dev libwebsocketpp-dev openssl libssl-dev ninja-build libspdlog-dev libmbedtls-dev libboost-all-dev libconfig++-dev libsctp-dev libfftw3-dev vim libcpprest-dev libusb-1.0-0-dev net-tools smcroute python3-psutil python3-pip clang-tidy gpsd gpsd-clients libgps-dev libgmime-3.0-dev libtinyxml2-dev libtinyxml2-9
sudo snap install cmake --classic
sudo pip3 install cpplint
sudo pip3 install psutil
````
## Downloading

````
cd ~
git clone --recurse-submodules https://github.com/5G-MAG/rt-mbms-mw

cd rt-mbms-mw

git submodule update

mkdir build && cd build
````

## Build the MBMS Middleware

To build the MBMS Middleware from the source:

`` cmake -DCMAKE_INSTALL_PREFIX=/usr -GNinja .. ``

Alternatively, to configure a debug build:
`` cmake -DCMAKE_INSTALL_PREFIX=/usr -GNinja -DCMAKE_BUILD_TYPE=Debug .. ``

Build with:
`` ninja ``

## Installing

`` sudo ninja install `` 

The MBMS Middleware, like the MBMS Modem, also installs a systemd unit.

## Running
The configuration for the *MBMS Middleware* (file paths, max file age, api ports, ...) can be changed in the <a href="#config-file">configuration file</a>.
When starting, the *MBMS Middleware* listens to the local [tun interface](https://github.com/5G-MAG/Documentation-and-Architecture/wiki/MBMS-Modem#multicast-routing). Received multicast packets from the *Receive Process* are FLUTE decoded and the files are stored in the cache.

### Background process
The *MBMS Middleware* runs manually or as a background process (daemon).
If the process terminates due to an error, it is automatically restarted. With systemd, execution, automatic start and manual restart of the process can be configured or triggered (systemctl enable / disable / start / stop / restart).
Starting, stopping and configuring autostart for *5gmag-rt-mw*: The standard systemd mechanisms are used to control *5gmag-rt-mw*:

| Command| Result |
| ------------- |-------------|
|  `` systemctl start 5gmag-rt-mw `` | Manually start the process in background |
|  `` systemctl stop 5gmag-rt-mw `` | Manually stop the background process |
|  `` systemctl status 5gmag-rt-mw `` | Show process status |
|  `` systemctl disable 5gmag-rt-mw `` | Disable autostart, 5gmag-rt-mw will not be started after reboot |
|  `` systemctl enable 5gmag-rt-mw `` | Enable autostart, 5gmag-rt-mw will be started automatically after reboot |

### Manual start/stop
After [installing](https://github.com/5G-MAG/rt-mbms-mw#readme) the *MBMS Middleware* you can start the process manually in the terminal using the command ``mw``. This will start the *mw* with default log level (info). *MBMS Middleware* can be used with the following OPTIONs:

| Option | | Description |
| ------------- |---|-------------|
|  `` -c `` | `` --config=FILE `` | Configuration file (default: /etc/5gmag-rt.conf)) |
|  `` -i `` | `` --interface=IF `` | IP address of the interface to bind flute receivers to (default: 192.168.180.10) |
|  `` -l `` | `` --log-level=LEVEL  `` | Log verbosity: 0 = trace, 1 = debug, 2 = info, 3 = warn, 4 = error, 5 = critical, 6 = none. Default: 2. |
|  `` -? `` | `` --help `` | Give this help list |
|  `` -V `` | `` --version `` | Print program version |

## Configuration
### Config file

The config file for *MBMS Middleware* is located in ``/etc/5gmag-rt.conf)``. The file contains configuration parameters for:
* Cache
* HTTP-Server

````
mw: {
  cache: { 
    max_segments_per_stream: 30;
    max_file_age: 120;    /* seconds */
    max_total_size: 128; /* megabyte */
  }
  http_server: {
    uri: "http://0.0.0.0:3020/";
    api_path: "mw-api";
    cert: "/usr/share/5gmag-rt/cert.pem";
    key: "/usr/share/5gmag-rt/key.pem";
    api_key:
    {
      enabled: false;
      key: "106cd60-76c8-4c37-944c-df21aa690c1e";
    }
  }
  control_system: {
    enabled: false;
    interval: 20; //seconds
    endpoint: "https://5gbc.ors-aws.cloud/obeca-api";
  }
  seamless_switching: {
    enabled: false;
    truncate_cdn_playlist_segments: 3
  }
  bootstrap_format: "";
  local_service: {
    enabled: false;
    bootstrap_file: "";
    mcast_address: "238.1.1.95:40085";
    lcid: 1;
    tmgi: "00000009f165";
  }
}

````
## Docker Implementation

An easy to use docker Implentation is also available. The `middleware` folder contains all the essential files for running the process in a container. Please check into the [wiki](https://github.com/5G-MAG/Documentation-and-Architecture/wiki/5G-MAG-Reference-Tools:-Docker-Implementation-of-RT-MBMS-processes) page for a detailed description on how to run the processes in a docker container. 
