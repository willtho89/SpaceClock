# SpaceClock

SpaceClock is an ESPHome port of the original SpaceUhr workshop clock. This repository only contains the ESPHome firmware and the supporting files needed to build, flash, and integrate it with Home Assistant.

## Supported Configurations

- `spaceclock-s2.yaml`: local ESP32-S2 build that reads Wi-Fi credentials from `secrets.yaml` and uses the local package and component sources from this checkout
- `spaceclock-example.yaml`: editable local template that reads Wi-Fi credentials from `secrets.yaml` and uses the local package and component sources from this checkout
- `spaceclock-s2-import.yaml`: tested ESP32-S2 GitHub import wrapper for a 9x5 / 45 LED matrix on `GPIO16` with the button on `GPIO0`
- `spaceclock-example-import.yaml`: editable GitHub import template for your own ESP32 board, matrix layout, and pinout

The wrappers are intentionally small. Shared behavior lives in [`packages/spaceclock-base.yaml`](packages/spaceclock-base.yaml), and the renderer implementation lives in the bundled external component under [`components/spaceclock_support/spaceclock_renderer.cpp`](components/spaceclock_support/spaceclock_renderer.cpp), [`components/spaceclock_support/spaceclock_renderer_clock_impl.h`](components/spaceclock_support/spaceclock_renderer_clock_impl.h), and [`components/spaceclock_support/spaceclock_renderer_animations_impl.h`](components/spaceclock_support/spaceclock_renderer_animations_impl.h).

## Install In ESPHome

The wrapper YAMLs are installable directly from this Git repository through ESPHome Remote/Git Packages:

- `spaceclock-s2-import.yaml`
- `spaceclock-example-import.yaml`

These files stay small and pull the shared configuration from [`packages/spaceclock-base.yaml`](packages/spaceclock-base.yaml) over Git. The renderer ships as part of the same external component, so users do not need to clone the full repository into `/config/esphome`.

### Home Assistant / ESPHome Add-on

1. Install the ESPHome Device Builder add-on in Home Assistant.
2. In the ESPHome dashboard, choose `NEW DEVICE` and use the GitHub import URL for the wrapper you want:
   - `github://willtho89/SpaceClock/spaceclock-s2-import.yaml@main`
   - `github://willtho89/SpaceClock/spaceclock-example-import.yaml@main`
3. Flash the device over USB the first time.
4. Provision Wi-Fi using either:
   - the fallback access point plus captive portal, or
   - `improv_serial` over USB
5. In Home Assistant, finish the ESPHome integration from discovery, or add the device manually by hostname or IP if needed.

The published `*-import.yaml` wrappers intentionally do not embed Wi-Fi credentials, API encryption keys, or OTA passwords. That keeps the GitHub import configuration compatible with ESPHome remote package and dashboard import workflows.

## Local Development

1. Clone the repository:
   `git clone https://github.com/willtho89/SpaceClock`
2. Enter the repo directory.
3. Install the toolchain:
   `uv sync`
4. Copy `secrets.example.yaml` to `secrets.yaml` and set your local Wi-Fi credentials.
5. If you use `spaceclock-example.yaml`, adjust the board, LED pin, button pin, LED count, matrix height, active offset, and serpentine phase to match your hardware.

The local wrapper YAMLs intentionally use `!include packages/spaceclock-base.yaml` plus a local `external_components` path, so local edits are picked up immediately without pushing to GitHub. The `*-import.yaml` wrappers intentionally keep the GitHub package/component sources so ESPHome dashboard imports continue to work from Home Assistant.

## Validate, Build, And Flash

Validate the YAML before building:

```bash
uv run esphome config spaceclock-s2.yaml
uv run esphome config spaceclock-example.yaml
```

Compile the tested configurations:

```bash
uv run esphome compile spaceclock-s2.yaml
```

Flash over USB the first time:

```bash
uv run esphome run spaceclock-s2.yaml
```

After the first wired flash, ESPHome OTA updates can be used for future changes.

### Exposed Home Assistant Entities

The firmware exposes:

- one primary matrix brightness light
- one active mode diagnostic text sensor
- one speed configuration slider
- one 24h clock configuration switch
- one binary clock configuration switch
- one continuous animation configuration switch
- one palette configuration selector
- animation buttons for Bars, Equalizer, Fireworks, HAXKO, Meteor, Noise, Radar, Snake, and Starfield
- one disabled-by-default debug clock button

## Hardware Notes

- `spaceclock-s2.yaml` uses `GPIO0` for the button. `GPIO0` is a strapping pin, so external pullups or pulldowns on that line can affect boot behavior.
- The custom SpaceClock face uses a 9-column active area. The optional `24h Clock` mode renders compact `HH:MM` digits across the full 9x5 matrix.
- `matrix_height`, `active_y_offset`, and `serpentine_phase` let the wrappers adapt the physical wiring and row placement.

## Credits

`SpaceUhr/SpaceClock`

- Haxko: [https://haxko.space](https://haxko.space)
- Magic Mystery Muesli: [https://woof.tech/@dr_muesli](https://woof.tech/@dr_muesli)

Presented at Easterhegg 23:

- [https://pretalx.eh23.easterhegg.eu/eh23/talk/JXAYCN/](https://pretalx.eh23.easterhegg.eu/eh23/talk/JXAYCN/)
