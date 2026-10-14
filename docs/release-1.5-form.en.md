# FC 1.5 form text

[中文](release-1.5-form.md). For the current production menu repair; exact firmware identities are in the [full delivery](release-1.5.en.md).

## English instructions

```text
1. Prepare
Install the firmware and power on the device. Use a compatible BLE controller or Bluetooth keyboard; Classic Bluetooth controllers are not supported.
Preinstalled games require no Wi-Fi or internet connection. Use the device's Up/Down buttons to navigate and OK to confirm.

2. Connect an input device
Select “连接手柄” (Connect Controller) on the home screen. Put your controller, keyboard, or remote into pairing mode, then select it using the device buttons.
For IINE L1241, use PC wireless mode and select “IINE GAMEPAD”. Follow any firmware confirmation or keyboard pairing-code instructions shown on screen.

3. Test the controls
Open “手柄测试” (Controller Test). Check that supported controls respond and return to their released state.
All devices share the same complete test layout. Sticks, shoulder buttons, or triggers absent from your device remain inactive.

4. Start a game
Once connected, the first home item becomes “游戏列表” (Game List). Open it, select a game, and press A or the device's OK button to launch. Use Start to begin playing.
Keyboard mapping: WASD=directions, J=A, K=B, I=Select, L=Start.

5. Use a Xiaomi remote horizontally
Using the original labels on the remote: Right=game Up, Up=game Left, Left=game Down, Down=game Right.
TV=A, three-line Menu=B, Home=Start, Back=Select. Use TV to confirm menu selections; the remote's center OK button is not mapped.
Single-button operation was verified on RC001 firmware 2671. This remote cannot move and shoot simultaneously; use a controller for games requiring button combinations.

6. Pause and return
Hold Select for about one second to open the pause menu, where you can resume, adjust settings, or exit. Holding the device's OK button also pauses.
On the Xiaomi remote, hold Back for this action; a short press remains game Select. Release the button before holding it again after entering or resuming a page.

7. Manage games
Open “游戏管理” (Game Management). Connect your phone or computer to the password-free “folotoy-fc” Wi-Fi hotspot, then open http://192.168.4.1/ in a browser.
Stay connected if warned that the network has no internet. Follow the page instructions to manage your own .nes files. There are 16 slots, all occupied in the preinstalled edition, so free space before replacing games.

Flashing the complete BIN at 0x0 replaces existing games, saves, settings, and Bluetooth bonds. It is not a data-preserving incremental update.
```

## English changelog

```text
1. Added horizontal Xiaomi BLE remote mapping: rotated directional controls, TV=A, Menu=B, Home=Start, and Back=Select.
2. Fixed Xiaomi RC001 remotes connecting successfully but producing no usable button input.
3. Restored one complete controller-test screen for controllers, keyboards, and remotes. Controls absent from a device remain inactive.
4. The first home item now shows “Game List” while connected and returns to “Connect Controller” after Bluetooth disconnects.
5. Fixed unintended game launches when the confirmation button remained held while entering the game list from the home screen.
6. Provided a complete firmware image retaining the original 16 preinstalled games, with input diagnostic logging disabled.

Single-button operation was verified on RC001 firmware 2671. This remote cannot reliably report simultaneous buttons and is unsuitable for games requiring movement and shooting at the same time.
```
