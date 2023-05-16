
1. The directory structure has been changed so newer version of the IDE can access files that can be modified.
2. Additional comments have been added to make it clear how to integrate additional sensors.
3. Frequency calibration is done in 10 sec instead of 40 sec.  No loss in accuracy, but callbration
    is less likely to cause a missed time slot.
4. Config.h option is added to suppress transmission on a second WSPR band
5. FrequencyCorrection.h was corrected.  The band center frequency was off by 55 Hz.
6. The default band is now 20m and there is no second transmission.

