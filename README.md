# IoT Semaphore
Electronics and Software for a semaphore controlled by Raspberry Pi 3

![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/img/class.png)
![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/img/prystine.png)

### Getting started
Download the latest version of raspbian from
`https://www.raspberrypi.org/downloads/raspbian/` and flashing it in an SD card.

#### Hardware connection

![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/hw/connections.jpg)


- Connect the power distribution board to a car battery using the supplied terminals.
- Connect the Raspberry Pi 3 to the 5V output of the power distribution board.
- Connect the COM connector of the semaphore (white wire) to the 12V output of the power distribution board.
- Connect the COM connector of each Relay of the control module to the GND of the power distribution board.
- Connect the Normally Open (NO) output of each Relay of the control module to the respective led of the semaphore (black, brown and grey wires).
- Power up the control module via the 5V output provided by the Raspberry Pi 3.
- Connect each GPIOs specified in the configuration file to the control module.


![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/hw/semaphore.jpg)


#### Build the project
Required libraries:

- RapidJson
- cURL
- cURLpp
- libmodbus

install the RapidJson library from the git repository:

`git clone https://github.com/Tencent/rapidjson`

Copy the content of `rapidjson/include` in `/usr/include`.

Install cURLpp from the Raspbian repository:

`sudo apt-get install libcurl-openssl-dev libcurlpp-dev`

If you need to use the Modbus protocol, install libmodbus from repository:

`sudo apt-get install libmodbus5 libmodbus-dev`

or alternatively clone and compile this git project:

`git clone https://github.com/stephane/libmodbus`

* Note that Modbus is not mandatory for using this traffic light.

```bash
cd libmodbus
./configure && make install
sudo ldconfig
```

Compile the project with:

```bash
mkdir build
cd build
cmake ..
make -j4
```

#### Connectivity

##### Modbus Ethernet

If you need to use the Modbus protocol, you have to specify in /etc/network/interface, the network interface for handling the modbus connection, for example:

```bash
auto eth0
allow-hotplug eth0
iface eth0 inet static
	address 169.254.53.170
```

##### WIFI connection through Mikrotik router

Configurare una connessione wireless con i parametri seguenti:

```bash
auto wlan0
iface wlan0 inet dhcp
wpa-ssid Semaphore1
wpa-psk 
```

##### WIFI connection through a Huawei Dongle

Connect the Huawei dongle to a computer and wait for the new connection to be created.
Then connect to http://192.168.8.1 and see that the APN settings of your sim are correct.
Disconnect the dongle and insert it into a free USB port of the traffic light controller.
Note: The NetworkManager package is required.

#### Semaphore configuration

An example of configuration file for the traffic light controller is the following:

```json
{
	"semname" : "sem1",
	"semaphores": [
		{
			"//": "Traffic Light TLB2",
			"r-pin": 18,
			"g-pin": 24,
			"y-pin": 25
		},{
			"//": "Traffic Light TLB1",
			"r-pin": 5,
			"g-pin": 6,
			"y-pin": 13
		},{
			"//": "Traffic Light TLA2",
			"r-pin": 17,
			"g-pin": 27,
			"y-pin": 4
		},{
			"//": "Traffic Light TLA1",
			"r-pin": 16,
			"g-pin": 20,
			"y-pin": 21
		}
	],
	"delays": {
		"//": "Time for each traffic light phase expressed in seconds",
		"R": 10,
		"G": 8,
		"Y": 2
	},
	"modbus-master": {
		"ip-addr": "169.254.53.170",
		"port"   : 1502,
		"debug"  : 1
	},
	"//": "1: The traffic light is controlled by the IoT Server",
	"//": "2: The traffic light is controlled by a Modbus Master", 
	"//": "3: The traffic light phases are time-controlled",
	"ctrl_type" : 3,
	"http_server.enable" : true,
	"//": "URL or IP address of the IoT Server",
	"http_server.url": "http://your-server.com"
}

```

Through this configuration file is possibile to configure:

- the GPIOs used by the Raspberry to manage every traffic light, via r-pin, y-pin and g-pin. Within the list from top to bottom we have TLA1, TLA2, TLB1 and TLB2.
Finally, we note that the first two semaphores are respectively the TL0 and the TL1 reported on the case of the semaphore controller.
- time for each traffic light phase, expressed in seconds, in case of internal control (ctrl_type = 3) by means of the delays object.
- If needed, the Modbus connection configuration parameters, in particular it is necessary to specify the IP address of the Modbus SLAVE (Raspberry pi), the port used and whether it is necessary to print debugging information.
- type of traffic light control (ctrl_type), in particular it can be controlled by a UNIMORE demo server (ctrl_type = 1), controlled by Modbus for Magneti Marelli demos (ctrl_type = 2) or timed internal control (ctrl_type = 3)
- with the use_server flag it is possible to transmit the traffic light status to the url specified in the server. In particular use an internal IP address.

It is possible to stop and start the controller with `sudo ./semaphore conf.JSON` and stopping it with ctrl+C.
### Controller schematic

If you want to build your own relay board, this is the schematic we used in our demo

![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/hw/schematics/controller_schematic.png)
