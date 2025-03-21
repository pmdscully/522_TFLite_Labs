/* Edge Impulse Arduino examples
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// These sketches are tested with 2.0.4 ESP32 Arduino Core
// https://github.com/espressif/arduino-esp32/releases/tag/2.0.4

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

/* Includes ---------------------------------------------------------------- */
#include <gesture_class_ESP32_dataForwarder_inferencing.h>

#ifndef WIRE_H
#define WIRE_H
#include <Wire.h>
#endif // WIRE_H

#include "adxl_read.h"


#define FREQUENCY_HZ        60
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))


float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;

static unsigned long last_interval_ms = 0;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize ADXL345
  initADXL345();

  Serial.println("ADXL345 Accelerometer Test");

  Serial.print("Features: ");
  Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  Serial.print("Label count: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);

}

void loop() {

  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();
  
    // Read raw accelerometer data
    readAccelData_float(features[feature_ix], features[feature_ix+1], features[feature_ix+2]);

//    Serial.print("X: ");
//    Serial.print(features[feature_ix]);
//    Serial.print("\tY: ");
//    Serial.print(features[feature_ix+1]);
//    Serial.print("\tZ: ");
//    Serial.println(features[feature_ix+2]);
    feature_ix += 3;


    if (feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
      Serial.println("Running the inference...");
      signal_t signal;
      ei_impulse_result_t result;
      int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
      if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
      }

      EI_IMPULSE_ERROR res = run_classifier(&signal, &result, true);

      if (res != 0) return;

      ei_printf("Predictions ");
      ei_printf("(DSP: %d ms., Classification: %d ms.)",
                result.timing.dsp, result.timing.classification);
      ei_printf(": \n");

      for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
      }
      feature_ix = 0;
    }

  }
}

void ei_printf(const char *format, ...) {
  static char print_buf[1024] = { 0 };

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0) {
    Serial.write(print_buf);
  }
}
