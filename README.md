# CmdManager Library
`CmdManager` Library provide a command manager, such as CLI, Telnet, ... for your embedded system over a HAL (Hardware Abstract Layer)

## Features
- Support multiple command manager
- Support multiple command event (Execute, Help, Set, Get, Response)
- Support custom command format
- Auto detect parameter type and convert it into data type
- Support multiple parameter for each command
- Support binary search or linear search
- Automatic sort command by name for more performance in searching
- Support customize command configuration based on hardware

## Examples
- [Basic](./Examples/Basic/) shows basic usage of `CmdManager` Library
- [AVR-CmdManager](./Examples/AVR-CmdManager/) shows basic usage of `CmdManager` Library ported for AVR microcontroller
- [STM32F429-DISCO](./Examples/STM32F429-DISCO/) shows basic usage of `CmdManager` Library ported for STM32F429-DISCO microcontroller
