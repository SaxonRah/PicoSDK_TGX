# VSCode + PicoSDK + TGX + ILI9341
I've been tasked with making the TGX library usable in VSCode + Offical Pico SDK extension with a ILI9341 screen.

Everything should be included and setup to compile and run, as is.

## Directions
* Open in VSCode with Pico SDK extension
* Compile.
* Flash.
* Done.
  
## Pin Mapping for Pico/ILI9341
| TFT Pin  | Screen Pin |
| -------- | ---------- |
| TFT_MISO | GPIO_4     |
| TFT_CS   | GPIO_5     |
| TFT_SCK  | GPIO_6     |
| TFT_MOSI | GPIO_7     |
| TFT_RST  | GPIO_8     |
| TFT_DC   | GPIO_9     |

![Screenshot.png](/Screenshot.png)

## Resources Used
* Offical Pico SDK extension for VSCode
* * https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico
* ILI9341 Library
* * https://github.com/RPiks/pico-touchscr-sdk
* TGX Library
* * https://github.com/vindar/tgx
