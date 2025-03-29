Changelog
All notable changes to this project will be documented in this file.

# Type Of Chnage
Types of change is show as following:
### Add
for new features.
### Changed
for changes in existing functionality.
### Deprecated
for soon-to-be removed features.
### Removed
for now removed features.
### Fixed
for any bug fixes.
### Security
in case of vulnerabilities.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),

# Versioning Rule
-> MAJOR.MINOR.PATCH.PRE-RELEASE
- MAJOR: API Change that do NOT have downwards compatibility.
- MINOR: Function add that have downwards compatibility.
- PATCH: Bug fix that have downwards compatibility.
- PRE-RELEASE: Pre-release tag.(after 2022-06-09)
    * 0 = rc (release candidate)
    * 1 = b (beta)
    * 2 = a (alpha)
    * 3 = pa (pre-alpha)

Version rule follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


# Chnage Log - CalibrationV20.dll
## [3.1.0.0] - 2025 - 02 - 10
### Add
- [ALSC_SPLIT_MODE] Add ConcatFileMode for spilt alsc table number. 0 is 2 file, 1 is 1 file

## [3.0.0.0] - 2025 - 01 - 17
### Changed
- [AIBNR] improve AIBNR calibration algo by selecting ROI of color checker patches

## [2.0.2.0] - 2025 - 01 - 16
### Changed
- [AIBNR] Add dummy buffer in aibnr_cali.data to support total gain - pregain map calibration in the future.

## [2.0.1.0] - 2024 - 11 - 11
### Add
- [ALL] Add support chip JAGUAR1. 
- [ALSC_REL_EX] Add new function to calibrate ALSC relationship. 

## [2.0.0.0] - 2024 - 08 - 28
### Changed
- [ALL] Make raw reading support max 14 nodes decompress.

## [1.0.52.0] - 2024 - 08 - 15
### Changed
- [AIBNR] Remove restrict to IC except Souffle and Iford

### Fixed
- [AIBNR] Fix bug of regenerate

## [1.0.51.0] - 2024 - 07 - 03
### Add
- [AIBNR] Add AIBNR Plugin API

## [1.0.50.0] - 2024 - 07 - 01
### Add
- [NE] Support Ikayaki, Opera, Pcupid do NE calibration.

### Fixed
- [NE] Fix iFackel NE calibration.

## [1.0.49.0] - 2024 - 06 - 25
### Changed
- [AINR] Quantilize float output to fixed output.

## [1.0.48.0] - 2024 - 05 - 27
### Fixed
- [NE] Fixed iFackel calibration parameter setting.

## [1.0.47.0] - 2024 - 04 - 17
### Add
- [ALL] Support iFackel to do all calibration.
- [AINR] Add AINR Calibration.

## [1.0.46.0] - 2024 - 03 - 15 (AWB_EX Fixed in this version)
### Fixed
- [AWB] Fixed AWB didn't allocate correctly in ini mode.

## [1.0.45.0] - 2024 - 02 - 27
### Add
- [NE] Implement g channel gray mode and set gray mode as default for iFado.

## [1.0.44.0] - 2024 - 02 - 15
### Add
- [NE] make NE can consider ISP gain.

## [1.0.43.0] - 2023 - 12 - 21
### Add
- [ALL] Support iFado to do all calibration include NE calibration.

## [1.0.42.0] - 2023 - 12 - 15
### Add
- [ALL] Support iFord to do all calibration include NE calibration.

## [1.0.41.0] - 2023 - 11 - 15
### Add
- [NE] Support Tiramitsu, Pudding and Ispahan to do NE calibration.

## [1.0.40.0] - 2023 - 11 - 10
### Add
- ALSC calibration add hardware limit mode.
### Changed
- SDC Seperate debug message flag from CaliFlag to DebugFlag

## [1.0.39.0] - 2023 - 11 - 03 (AWB_EX have bug from this version)
### Fixed
- AWB set wrong OB while HighLowCTMode with Plugin Mode.

## [1.0.38.0] - 2023 - 10 - 27
### Changed
- refect code no function changed

## [1.0.37.0] - 2023 - 10 - 19
### Fixed
- SDC ClusterAmount and TotalAmount will be accumulate between calls.

### Changed
- SDC dump data will changed serial ID while CMD mode and data is loaded.

## [1.0.36.0] - 2023 - 10 - 16
### Fixed
- HighLowCTMode not set.in AWB Plugin Mode.

## [1.0.35.1] - 2023 - 09 - 28
### Changed
- refect code no function changed

## [1.0.35.0] - 2023 - 09 - 26
### Add
- SDC Calibration Support Command Line Mode.

### Fixed 
- ALSC Delta lutX/Y Error while Grid X/Y is even.

## [1.0.34.0] - 2023 - 08 - 30
### Changed 
- calibration source type which support 12 bits MIPI bayer need to decide out_data_precision.

## [1.0.33.0] - 2023 - 08 - 10
### Fixed 
- NE calibration low luma HDR process error.

### Add
- Add Function to check qualified raw for SDC Calibration.

## [1.0.32.0] - 2023 - 08 - 09
### Fixed 
- NE calibration OOM while large image resolution and frame number.

### Add
- Return Error code of NE calibration.

## [1.0.31.0] - 2023 - 08 - 08
### Fixed
- Fixed NE calibration parse from folder mode crash error.

## [1.0.30.0] - 2023 - 08 - 07
### Add
- Add new calibration source type to support 12 bits MIPI bayer.

### Fixed
- Fix NE Calibration plugin mode not generate .data file and wrong node number.

## [1.0.28.0] - 2023 - 08 - 01
### Add
- Add DoCalibrationSDC_OutputParams API for user to get output params of SDC calibration.

## [1.0.27.0] - 2023 - 07 - 11
### Add
- Implement Plugin mode for Noise Estimation Calibration. 
- Implement HDR mode for NE Calibration.

## [1.0.26.0] - 2023 - 06 - 13
### Fixed
- Fix ALSC V4 not General .data file.
- Set Maruko, Souffle as ALSC V5.

## [1.0.25.0] - 2023 - 06 - 01
### Add
- Make NE Calibration support Souffle.

## [1.0.24.0] - 2023 - 05 - 04
### Add
- Add Noise Estimation Calibration.
    * Plugin Not Supported yet.
- Add Souffle.

## [1.0.23.0] - 2023 - 03 - 16
### Add
- Add MARUKO.
- Add document from public API.

### Changed
- Create Calibration_Private_API.h and move private API from Calibration_API.h to it.

## [1.0.22.0] - 2023 - 02 - 22
### Fixed
- Fix FPN calibration overflow issue.

### Changed
- Substantially accelerate SDC calibration.

## [1.0.21.0] - 2023 - 02 - 09
### Add 
- Make Tiramitsu Support ALSC Split Mode.

## [1.0.20.0] - 2022 - 12 - 29
### Add
- Implement FPN Calibration
    * Plugin Not Supported yet.
- Add Chip id for MOCHI(0x13) and OPERA(0x14)

## [1.0.19.0] - 2022 - 12 - 27
### Changed
- [LSC] Move applying CenterToCorner ratio to the last step

## [1.0.18.0] - 2022 - 12 - 27
### Fixed
- [AWB] Fixed HighLowCTMode AWB checksum error.
- [AWB] Fixed HighLowCTMode AWB CALIB_GOLDEN_H gain calculate error.

## [1.0.17.0] - 2022 - 12 - 09
### Fixed
- Fix SDC search not in crop image 

## [1.0.16.0] - 2022 - 12 - 07
### Changed
- [ALSC_REL] Remove non-need param in ini file 
- [ALSC_REL] Save Interval Shift to file.
- [ALSC_REL] Reverse change in 1.0.15.0
- [ALSC_REL] Set table size to 128 
- [General] Add debug flag to CALI_INFO

## [1.0.15.0] - 2022 - 12 - 06
### Changed
- No flare detection limit for ALSC REL

## [1.0.14.0] - 2022 - 11 - 28
### Add
- New ALSC relationship Calibration is implemented.
    * Plugin Not Supported yet.

## [1.0.13.0] - 2022 - 09 - 23
### Changed
- Complete Plugin mode of DoAWBCalibration().

## [1.0.12.0] - 2022 - 09 - 22
### Fixed
- Move uMode and uDumpALSC from ALSC_CTRL(will be save to .data, which make api changed) to ALSC_Buffer

## [1.0.11.0] - 2022 - 09 - 20
### Add
- Revise SDC calibration White Point Algo

## [1.0.10.0] - 2022 - 09 - 13
### Add
- Add Wrapped API RGBIR_to_Bayer

## [1.0.9.0] - 2022 - 08 - 29
### Add
- Restore SDC_CALI_CTRL_V4 structure and remove CaliFlag in ini

## [1.0.8.2] - 2022 - 08 - 19
### Add
- New ini api to control if ALSC limit dark corner gain to 10x(No Plugin API - Customer pre-release version)

## [1.0.7.0] - 2022 - 08 - 18
### Fixed
- SDC Add API Variable
    * Add Variable BedPixelTh to tune SDC


## [1.0.6.0] - 2022 - 06 - 20
### Fixed
- Fix ALSC for ALSC plugin mode
    * Set SplitOverlap as -1 which means split mode not support for plugin mode.


## [1.0.5.0] - 2022 - 06 - 17
### Fixed
- Fix crop image bug while clip range equal to input image boundary(Ex: clip_image.x+clip_image.width==full_image.width).

## [1.0.4.0] - 2022 - 06 - 09
### Fixed
- Fix Bug while input_precision != 16 and output_precision != 16 on calibration that need to deduct OB such as ALSC/LSC/AWB
    * Limit OB deduction value by output_precision and do not shift OB deduction value
    * Which made user more intuitive to set OB value with output_precision.

## [1.0.4.4][Unreleased]
### Changed
- Width limitation of Pudding change from 3840 to 4208

### Fixed
- Add MONTH and DAY information to header of ALSC V5.

## [1.0.3.0] - 2022 - 02 - 21
### Changed
- Make users to decide overlap in split mode by continuous integer instead of 3 options(0/256/512).

## [1.0.2.0] - 2022 - 02 - 17
### Fixed
- Fix wrong CheckSum of AlscCali.data of left half screen while split mode.
- Fix that element of ALSC table among 3 color channels should sync to 1x gain while dark cornar case.

## [1.0.1.0] - 2022 - 02 - 15
### Add
- Support Split Mode for Muffin chip
    * New chip_info_id 8 represents Muffin chip whose width limatation is 4680 and is 9360 when ALSC_SPLIT_MODE.
    * New parameter "[ALSC_SPLIT_MODE] SplitOverlap" for alcs to support split mode(Only Muffin supported).
    * Split Mode only support Segment_Delta_Str_Mode = 0

- Support RGBIR Sensor Calibration for ALSC.
    * New cfa_type 4 - 11 represents R0, G0, B0, G1, G2, I0, G3, I1, respectively.

- Keep old AWB calibration for old FW verisoin.
    * New HighLowCTMode in [AWB] to switch old and new awb calibration method.

### Changed
- Width limitation of Tiramitsu change from 4208 to 4608

### Fixed
- Fix memory increase while continuously doing AWB.
- Fix bug of doing AWB with in_data_precision != out_data_precision.
- Fix Decompress only apply on last one of Image Stream by applying on all.

## [1.0.0.1] - 2022 - 01 - 15
### Changed
- Set full image width limitation by chip(Pudding - 3840, Tiramitsu - 4208, Other - 4096)

# Chnage Log - CalibrationRelease.exe
## [1.0.2.0] - 2022 - 11 - 28
### Add
- Support New ALSC relationship Calibration in CalibrationV20 v1.0.14.0.

## [1.0.1.0] - 2022 - 09 - 13
### Add
- Add an optional argument --nonblock or -nb to make process non-block while calibration fail.
