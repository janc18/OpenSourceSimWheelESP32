# Unit test: Frame server output

## Purpose and summary

To test that the notification interface is able to serialize notifications while running a generic frameserver in a loop.

## Hardware setup

Nothing required. Output through USB serial port at 115200 bauds. Connect before reset.

## Procedure and expected output

This test runs in a never-ending loop.

1. Reset. Ignore output from the operating system itself.
2. The following text must appear:

   ```text
   -- READY --
   (DEVICE READY)
   ```

3. The text line `(FRAME n MS)` must appear at almost timed intervals,
   where "n" is a number near to 333.
4. Between those lines, the text line `(BLE CONNECTED)` must appear.
5. A few lines after that, the text line `(BITE POINT: 0)` must appear.
6. Back to 3.
