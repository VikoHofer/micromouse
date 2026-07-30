# Micro Mouse Simulator

Can be used together with the debugger to visualize the mouse's movement and the labyrinth.

## Setup

> Requirements: C# .Net 8.0+ SDK

1. Download the [Simulator](https://github.com/mackorone/mms) (tested with `Nov 2024` release on Microslop Windows)
2. Open the simulator
3. On the top right there is a section called `Config`, click on the `+` next to `Mouse` to add the debugger integration
4. For `Directory` input the path to the `DebugDisplay` directory (same as where this file is)
5. `Build Command`: `dotnet build`
6. `Run Command`: `dotnet run MMSAPI`
7. `OK` to save the config. Make sure for `Maze` `:/resources/mazes/blank.num` and for `Mouse` the newly added config are selected.
8. Click `Build` in the `Controls` section to build the debugger integration for the simulator.
9. Once you click `Run`, the DebugDisplay will connect to the debugger on the mouse and visualize its movements