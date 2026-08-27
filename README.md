# Parsian SSL AI

Parsian SSL AI is the autonomous soccer software used by the Parsian Robotics
Lab for the RoboCup Small Size League (SSL). It combines the game model,
strategy and play selection, robot skills, motion planning, vision and referee
communication, logging, and the interfaces needed to control a team of robots.

This is a legacy C++/Qt codebase. It is kept here as a reproducible snapshot of
the team software and includes the source of `ssl-visual-planner` directly in
the repository.

## What is included

- `ai.pro`: qmake project for the main AI application (`ai`)
- `mainapplication.cpp` and `soccer.cpp`: application and match lifecycle
- `worldmodel.cpp`, `visionclient.cpp`, and `net/`: world state and SSL network
  communication
- `plays/`, `plans/`, `roles/`, `skills/`, and `behaviours/`: tactical and
  behavioural layers
- `simulation/`: simulation connection and simulator support
- `proto/`: checked-in Protocol Buffers sources used by SSL and robot messages
- `ssl-visual-planner/`: the vendored visual planner application
- `fedit2/`: supporting field-editor/simulation code
- `PlansConfigs.json`, profiler files, lookup tables, and training data:
  example runtime and development data

## Requirements

The main application was developed and tested on Ubuntu 14.04 and later. Its
original toolchain is:

- C++ compiler and `make`
- Qt 4.8 or later, including Network, OpenGL, XML, Script, and SQL modules
- OpenGL development libraries and a working graphics driver
- Protocol Buffers development tools and libraries
- Boost, Eigen, QJSON, ODE, and libdc1394 development packages
- VarTypes 0.6 and 0.7, including its development headers and libraries

The dependency list is historical and this project may require compatibility
work on current Linux distributions. `install.sh` installs the system packages
and builds VarTypes from its Parsian repository; review it before running it
because it uses `sudo` and installs system-wide libraries.

The bundled visual planner is a separate Qt application and uses Qt 5.4 or
later. Its own build files and documentation are in
[`ssl-visual-planner/`](ssl-visual-planner/).

## Build the main AI

Install the requirements, then build from the repository root:

```bash
qmake -r ai.pro
make -j"$(nproc)"
```

On systems where the Qt 4 executable is named `qmake-qt4`, use:

```bash
qmake-qt4 -r ai.pro
make -j"$(nproc)"
```

The resulting executable is `./ai`. Generated Protocol Buffers sources are
already present under `proto/`; a separate protobuf generation step is not
normally needed for the main project.

## Run

### Simulation

Start the application and select **Simulation** from the application menu, or
run the executable directly and use the simulation controls exposed by the
GUI. Confirm that the simulator and any required network services are running
before starting a match.

### Real robots

`start.bash` repeatedly launches the application in real mode:

```bash
./start.bash
```

Equivalent one-shot invocation:

```bash
./ai mode real ref s
```

Do not use real mode unless the robot network, SSL-Vision feed, referee
connection, radio/robot interfaces, and emergency-stop procedure have been
checked. The application installs signal handlers intended to halt robots and
save runtime state on termination, but those handlers are not a substitute for
an independent emergency stop.

The `ref` argument selects the startup referee state. The GUI can also switch
between simulation and real operation and expose coach controls after startup.

## Logs and data

Runtime logs are written to `logs/`. To initialize the separate logs working
directory and its remote, run:

```bash
./add-logs.sh
```

The application also uses configuration, plan, profiler, and training files in
the repository. Avoid committing match logs, generated build output, local
hardware settings, or credentials unless they are intentionally part of a
reproducible experiment.

## Development workflow

1. Build and test in simulation before connecting real hardware.
2. Make tactical changes in the relevant `plays/`, `plans/`, `roles/`, or
   `skills/` area and keep communication changes in `net/` or `proto/`.
3. Record bugs and proposed work in the issue tracker before larger changes;
   see [`CONTRIBUTING.md`](CONTRIBUTING.md).
4. Include the build command, runtime mode, simulator or hardware setup, and
   relevant logs when reporting a problem.

## Related tools

- [RoboCup Small Size League](https://ssl.robocup.org/): competition context
- [SSL-Vision](https://github.com/RoboCup-SSL/ssl-vision): vision system
- [grSim](https://github.com/RoboCup-SSL/grSim): simulator commonly used with
  SSL software
- [ssl-visual-planner](ssl-visual-planner/): bundled plan editor and visual
  planner

## License and provenance

This repository contains legacy team software and historical third-party code.
Check the relevant source files and bundled project documentation for the
licensing terms that apply before redistributing it.
