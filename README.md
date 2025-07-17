# Mari 💫 👀

Mari is a lightweight wireless connectivity solution designed for dense IoT networks, with a focus on supporting real-time interactions and fast over-the-air (OTA) updates.

The driving use case for the design of Mari is the OpenSwarm Testbed of 1,000 [DotBots](https://github.com/DotBots/DotBot-firmware). Mari is suitable for any dense IoT deployment requiring low-latency communication.

## Key Features

- **TSCH over BLE**: Implements Time-Synchronized Channel Hopping (TSCH) over Bluetooth Low Energy (BLE) 2 Mbps PHY
- **Multi-Gateway Architecture**: Allows scaling the network by adding more gateways
- **Non-Coordinated Gateways**: Gateways are independent, so the infrastructure setup is very simple
- **Fast Handovers**: Enables quick transitions between gateways as nodes move
- **Low-Power Operation**: Designed for energy-efficient operation in battery-powered devices (BLE radio)
- **Real-Time Communication**: Achieves 100-150 ms average latency with 100 nodes per gateway
- **Reasonable Throughput for OTAP**: About 10 Kb/s downlink
- **Quick Network Join**: Best and worst -case join time of 150 ms and 6 seconds, respectively
- **Dense Network Support**: Scales for networks with hundreds to thousands of nodes

## Project Structure

```
mari/
├── app/                    # Example applications and tests
│   ├── 03app_gateway/     # Gateway implementation example
│   ├── 03app_node/        # Node implementation example
│   └── ...                # Various test applications
├── drv/                   # Hardware drivers
├── mari/                  # Core protocol implementation
└── nRF/                   # Nordic Semiconductor SDK files
```

## Example Usage

Gateway initialization:
```c
mari_init(MARI_GATEWAY, MARI_NET_ID_DEFAULT, schedule, event_callback);
```

Node initialization:
```c
mari_init(MARI_NODE, MARI_NET_ID_PATTERN_ANY, schedule, event_callback);
```

## Hardware Support

Mari has been validated with the following Nordic Semiconductor chips:
- nRF52833
- nRF52840
- nRF5340

## Development Environment

The project includes configuration files for:
- Segger Embedded Studio (`.emProject` files)
- Code formatting (`.clang-format`)
- Git hooks (`.pre-commit-config.yaml`)

## Getting Started

[Coming soon]

## License

This project is licensed under the terms included in the LICENSE file.

## Publications

- Fedrecheski et al., "Mari: Connecting Large Scale Robot Swarms over BLE using TSCH with Multiple Independent Gateways", CrystalFreeIoT Workshop 2025 [Forthcoming]

## Acknowledgments

This project has received funding from the EU's Horizon Europe Framework Programme under Grant Agreement No. 101093046.
