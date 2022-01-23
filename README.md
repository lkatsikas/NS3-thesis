# NS3 - thesis

NS-3 3.21 version bundled with v2v module for v2v communication.


## Description

Implementation of v2v communication algorithm. 
Nodes communicate locally using 802.11p protocol.
One or more nodes have access to the lte network. The elected cluster head sends the information collected
to the network. 

Integration with SUMO simulator to visualize demo.

## Getting Started

### Dependencies

* 3.21 version of NS-3 simulator


### Building program

```
./waf configure --enable-examples`
./waf
```

### Executing program

```
./waf --run v2v-clustering-example
```


## Authors

Contributors names and contact info

* [lkatsikas](https://github.com/lkatsikas)

## License

This project is licensed under the GNU License - see the LICENSE.md file for details



