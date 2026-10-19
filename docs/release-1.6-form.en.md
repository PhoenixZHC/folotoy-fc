# FC 1.6 instructions and form text

[中文](release-1.6-form.md) · [Delivery and verification](release-1.6-orientation.en.md)

## Instructions

### 1. Getting started

Use device Up/Down to navigate and OK to confirm. Preinstalled games need no internet. Controllers, compatible BLE keyboards and remotes all pair through Connect Controller: put the input device into pairing mode and select it from the list.

### 2. Using a controller

If a controller is not detected or cannot connect in Xbox mode, try switching to Android mode and pairing again. Q36 uses D mode and the name Q36 for Android. Refer to the controller manual for mode switching. Compatible BLE is required; Classic Bluetooth is not supported and not every Android mode is compatible.

Open Controller Test to check directions, A/B, Start, Select, combinations and release. Use directions to select a game, A to launch and Start to begin. In-game, use the D-pad or left stick to move, A/B for actions and Select for selection. Hold Select about one second to pause, or to go back on menu pages that support returning.

### 3. Using a Bluetooth keyboard

Put a compatible BLE keyboard into pairing mode and select it in Connect Controller. Follow any pairing-code prompt.

W=Up, S=Down, A=Left, D=Right; J=game A, K=game B, I=Select, L=Start. You can select a game using device buttons and launch with OK, then press L to begin. Use WASD to move, J/K for actions and I for selection. Hold I about one second to pause or return on supported menu pages.

### 4. Using a Bluetooth remote

Single-button operation is verified on Xiaomi RC001 firmware2671. Other models may have different mappings or may not work. Follow the remote manual to enter pairing mode and select it in Connect Controller.

For horizontal holding, directions refer to the arrows printed on the remote: Right→game Up, Up→game Left, Left→game Down, Down→game Right. TV=game A/menu confirmation; three-line Menu=game B; Home=Start; Back=Select. Center OK is not mapped; use TV to confirm.

Select with mapped directions, launch with TV and begin with Home. Hold Back about one second to pause; a short press retains Select. This remote cannot reliably combine movement and firing. Use a controller for games requiring simultaneous buttons.

### 5. Pause and game orientation

Holding device OK also pauses. The pause menu offers resume, battery-save synchronization, settings, screen off, exit and orientation. Cycle Portrait→Landscape→Flipped Landscape→Portrait, then resume. Hold the device counterclockwise for Landscape or clockwise for Flipped Landscape. Only game pixels rotate; menus stay portrait and button mappings do not change.

### 6. Default game orientation

Home Settings saves the default across reboots for subsequent game launches. The initial default is Portrait. Temporary pause-menu changes do not change the saved default.

### 7. Games and saves

Open Game Management, join folotoy-fc without a password, and visit http://192.168.4.1/. Stay connected despite a no-internet warning. Manage your own .nes files, up to16 games. For battery-backed games, use the game's save feature and synchronize from the pause menu. Arbitrary save states are not supported.

Full installation at **0x0** replaces games, saves, settings and bonds; it is not a data-preserving update.

## 1.6 changelog

1. Added compatibility with Android mode on selected controllers. Fixed controllers being discovered but stalling or timing out during connection. If Xbox mode is not detected or cannot connect, try Android mode and pair again.
2. Added Portrait, Landscape and Flipped Landscape game views. Change orientation in the pause menu and resume to apply it.
3. Added a persistent default game orientation in Settings. Portrait remains the initial default. Temporary pause-menu changes do not affect the saved default.
4. User testing passed for Q36 Android mode and for Android/Xbox modes on IINE Pocket Controller Mini and Pocket Controller Pro. All three game orientations and saved defaults also passed testing.

Compatibility depends on the model, firmware and BLE protocol. This does not imply support for every Android-mode or Classic Bluetooth controller.
