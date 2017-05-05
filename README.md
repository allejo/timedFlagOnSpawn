# Timed Flag on Spawn

[![GitHub release](https://img.shields.io/github/release/allejo/timedFlagOnSpawn.svg)](https://github.com/allejo/timedFlagOnSpawn/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.0+-blue.svg)

A BZFlag plug-in that will allow map makers to give players flags as soon as they spawn. This is [not an original idea](https://forums.bzflag.org/viewtopic.php?f=79&t=9249) but this plug-in will allow you to limit the time the players get to keep the flag before it's automatically taken away; players are free to drop the flags they receive.

This plug-in could be used on maps where players are given the opportunity to hide for a few seconds before their stealth flags are taken away and must be ready to fight.

## Requirements

- BZFlag 2.4.0+ (latest version available on GitHub is recommended)
- C++11
- [bzToolkit](https://github.com/allejo/bztoolkit)

## Usage

**Loading the plug-in**

This plug-in takes its configuration as a command line argument.

```
-loadplugin timedFlagOnSpawn,ST=20;CL=10;US=0
```

The syntax for flag definitions is as follows: `<flag abbr>=<time in seconds>[;]`. The `;` is used to split up multiple flag definitions; no `;` is needed if only one flag type is being used. If multiple flag definitions are given, one of the definitions will randomly be chosen from the list to be given to a player when they spawn.

In the above example, players will randomly be given either an ST, CL, or US flag when they spawn.

- If the player receives an ST flag, they'll have it for 20 seconds before it's taken away automatically
- If the player receives a CL flag, they'll have it for 10 seconds before it's taken away automatically
- If the player receives a US, they'll have it indefinitely until they drop it themselves (this is a "sticky" flag)

## License

[MIT](https://github.com/allejo/timedFlagOnSpawn/blob/master/LICENSE.md)