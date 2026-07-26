# YimMenu

![](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2FNightFall-Dev%2FBigBaseV2%2Frefs%2Fheads%2Fdev%2Fmetadata.json&query=game.online&style=flat-square&label=Online&labelColor=000000&color=bf8000)![](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2FNightFall-Dev%2FBigBaseV2%2Frefs%2Fheads%2Fdev%2Fmetadata.json&query=game.build&style=flat-square&label=Build&labelColor=000000&color=bf8000)

A mod menu for Grand Theft Auto V Legacy

YimMenu is originally based of off [BigBaseV2](https://github.com/Pocakking/BigBaseV2) which was an amazing base at the time but nowadays is a bit dated.
So here I am with an up-to-date menu focusing on protecting the user from toxic modders.

## Table of contents

 * [Staying Up To Date](#staying-up-to-date)
 * [Project Structure](#project-structure)
 * [Contributing](#contributing)

## Staying Up To Date

If you are doing custom modifications to the codebase and have a fork you are on your own for staying up to date with upstream (this repository), google stuff like "merge from upstream" and learn how to use Git.

## Project Structure

- `backend/` all features that should be ran in a loop are in here sorted by category
- `gui/` includes everything related to UI elements
- `hooks/` function hooks
- `native_hooks/` hooks to natives
- `services/` service files to interact and maintain stuff
- `util/` general utility functions to keep code as compact and as much in one place as possible

## Contributing

You're free to contribute to YimMenu as long as the features are useful, not overly toxic and do not contain anything money related that might get the menu targeted by Take2.

Make sure to read the [CONTRIBUTING.md](CONTRIBUTING.md) file.
