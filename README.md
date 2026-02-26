# FairQ2S: Joint Fairness and QoS-Aware Queue Scheduling for Next-Generation Networks

## Prerequisites

- NS-3 (version 3.45 or later)

## Installation

1. Download and build NS-3 from the official website: [https://www.nsnam.org/](https://www.nsnam.org/)
2. Copy the `contrib` and `scratch` directories from this repository into the root directory of your NS-3 installation.
3. Configure and build NS-3 with the following command:

   ```bash
   ./ns3 configure
   ./ns3 build
   ```
4. Run the simulation using the following command:

   ```bash
   ./ns3 run fairq2s-simulation
   ```

## Citation
If you use this code in your research, please cite the following paper:

```
@inproceedings{fairq2s,
  title={FairQ2S: Joint Fairness and QoS-Aware Queue Scheduling for Next-Generation Networks},
  author={},
  year={2026}
}
```