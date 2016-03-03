/*****************************************************************************
*   Copyright 2012 Vincent HERVIEUX
*
*   This file is part of QPicoscope.
*
*   QPicoscope is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   any later version.
*
*   QPicoscope is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with QPicoscope in files COPYING.LESSER and COPYING.
*   If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
/**
 * @file Acquisition6000
 * @brief Declaration of Acquisition6000 class.
 * Acquisition methods using libps6000 to drive Picoscope 6000 series HW
 * @version 0.1
 * @date 2013, january 4
 * @author Vincent HERVIEUX    -   01.04.2014   -   initial creation
 */
#ifndef ACQUISITION6000_H
#define ACQUISITION6000_H

#include "../qpicoscope-config.h" 

#ifdef HAVE_LIBPS6000

#include <string>
#include <vector>

#include "oscilloscope.h"
#include "drawdata.h"
#include "acquisition.h"

#ifdef WIN32
/* Headers for Windows */
#include "windows.h"
#include <conio.h>

/* Definitions of PS6000 driver routines on Windows*/
#include "PS6000.h"

#else
/* Headers for Linux */
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* Definition of PS6000 driver routines on Linux */
//#define DYNLINK
#include <libps6000-1.4/ps6000Api.h>
#define __stdcall
/* End of Linux-specific definitions */
#endif


class Acquisition6000 : public Acquisition{
public:
    /**
     * @brief public typedef declarations
     */

    typedef struct
    {
        double value;
        std::string name;
    }volt_item_t;

    /** @brief get singleton instance */
    static Acquisition6000* get_instance();
    /** @brief destructor */
    virtual ~Acquisition6000();
    /**
     * @brief set input voltage range
     * @param[in] : the channel index (0 for channel A, 1 for channel B, etc)
     * @param[in] : volts per division caliber
     */
    void set_voltages (channel_e channel_index, double volts_per_division);
    /**
     * @brief set input time base
     * @param[in] : time per division valiber
     */
    void set_timebase (double time_per_division);
    /**
     * @brief set AC/DC
     * @param[in] : a current_e value (0 = AC, 1 = DC)
     */
    void set_DC_coupled(current_e coupling);
    /**
     * @brief set signal generator (2200 series only) 
     * @param[in] : waveform type 
     * @param[in] : frequency in Hertz
     */
    void set_sig_gen (e_wave_type waveform, long frequency);
    /**
     * @brief set signal generator arbitrary (2200 series only) 
     * @param[in] : frequency in Hertz
     */
    void set_sig_gen_arb (long int frequency);
    /**
     * @brief get device informations 
     */
    void get_device_info(device_info_t* info);
private:
    /**
     * @brief private typedef declarations
     */
    typedef enum {
        MODEL_NONE = 0,
        MODEL_PS6402  = 0x6402, //Bandwidth: 350MHz, Memory: 32MS, AWG
        MODEL_PS6402A = 0xA402, //Bandwidth: 250MHz, Memory: 128MS, FG
        MODEL_PS6402B = 0xB402, //Bandwidth: 250MHz, Memory: 256MS, AWG
        MODEL_PS6402C = 0xC402, //Bandwidth: 350MHz, Memory: 256MS, AWG
        MODEL_PS6402D = 0xD402, //Bandwidth: 350MHz, Memory: 512MS, AWG
        MODEL_PS6403  = 0x6403, //Bandwidth: 350MHz, Memory: 1GS, AWG
        MODEL_PS6403A = 0xA403, //Bandwidth: 350MHz, Memory: 256MS, FG
        MODEL_PS6403B = 0xB403, //Bandwidth: 350MHz, Memory: 512MS, AWG
        MODEL_PS6403C = 0xC403, //Bandwidth: 350MHz, Memory: 512MS, AWG
        MODEL_PS6403D = 0xD403, //Bandwidth: 350MHz, Memory: 1GS, AWG
        MODEL_PS6404  = 0x6404, //Bandwidth: 500MHz, Memory: 1GS, AWG
        MODEL_PS6404A = 0xA404, //Bandwidth: 500MHz, Memory: 512MS, FG
        MODEL_PS6404B = 0xB404, //Bandwidth: 500MHz, Memory: 1GS, AWG
        MODEL_PS6404C = 0xC404, //Bandwidth: 350MHz, Memory: 1GS, AWG
        MODEL_PS6404D = 0xD404, //Bandwidth: 350MHz, Memory: 2GS, AWG
        MODEL_PS6407  = 0x6407, //Bandwidth: 1GHz, Memory 2GS, AWG
        MODEL_PS6408  = 0x6408  //Bandwidth: 2GHz, Memory 2GS, AWG
    } MODEL_TYPE;

    typedef struct tTriggerDirections {       
        enum enPS6000ThresholdDirection channelA;
        enum enPS6000ThresholdDirection channelB;
        enum enPS6000ThresholdDirection channelC;
        enum enPS6000ThresholdDirection channelD;
        enum enPS6000ThresholdDirection ext;
        enum enPS6000ThresholdDirection aux;
    } TRIGGER_DIRECTIONS;
    /*typedef struct
    {
        THRESHOLD_DIRECTION    channelA;
        THRESHOLD_DIRECTION    channelB;
        THRESHOLD_DIRECTION    channelC;
        THRESHOLD_DIRECTION    channelD;
        THRESHOLD_DIRECTION    ext;
    } DIRECTIONS;*/

    typedef struct tPwq {
        struct tPS6000PwqConditions * conditions;
        int16_t nConditions;
        enum enPS6000ThresholdDirection direction;
        uint32_t lower;
        uint32_t upper;
        PS6000_PULSE_WIDTH_TYPE type;
    } PWQ;
    /*typedef struct
    {
        PWQ_CONDITIONS                    *    conditions;
        short                                                        nConditions;
        THRESHOLD_DIRECTION          direction;
        unsigned long                                        lower;
        unsigned long                                        upper;
        PULSE_WIDTH_TYPE                    type;
    } PULSE_WIDTH_QUALIFIER;*/


    /*typedef struct
    {
        PS6000_CHANNEL channel;
        float threshold;
        short direction;
        float delay;
    } SIMPLE;*/

    /*typedef struct
    {
        short hysterisis;
        DIRECTIONS directions;
        short nProperties;
        TRIGGER_CONDITIONS * conditions;
        TRIGGER_CHANNEL_PROPERTIES * channelProperties;
        PULSE_WIDTH_QUALIFIER pwq;
        unsigned long totalSamples;
        short autoStop;
        short triggered;
    } ADVANCED;*/


    /*typedef struct
    {
        SIMPLE simple;
        ADVANCED advanced;
    } TRIGGER_CHANNEL;*/

    typedef struct {
        short DCcoupled;
        short range;
        short enabled;
        /*short values [BUFFER_SIZE];*/
    } CHANNEL_SETTINGS;

    typedef struct {
        int16_t handle;
        MODEL_TYPE                              model;
        char                                    modelString[8];
        char                                    serial[10];
        int16_t                                 complete;
        int16_t                                 openStatus;
        int16_t                                 openProgress;
        PS6000_RANGE                    firstRange;
        PS6000_RANGE                    lastRange;
        int16_t                                 channelCount;
        BOOL                                    AWG;
        CHANNEL_SETTINGS        channelSettings [PS6000_MAX_CHANNELS];
    } UNIT;
    /*typedef struct  {
        short handle;
        MODEL_TYPE model;
        PS6000_RANGE firstRange;
        PS6000_RANGE lastRange;
        char signalGenerator;
        char external; 
        short timebases;
        short maxTimebases;
        short noOfChannels;
        CHANNEL_SETTINGS channelSettings[MAX_CHANNELS];
        TRIGGER_CHANNEL trigger;
        short                hasAdvancedTriggering;
        short                hasFastStreaming;
        short                hasEts;
    } UNIT_MODEL;*/
    /**
     * @brief private methods declarations
     */
    Acquisition6000();
    int adc_to_mv (long raw, int ch);    
    short mv_to_adc (short mv, short ch);
    void get_info (void);
    void set_defaults (void);
    void set_trigger_advanced(void);
    void collect_block_immediate (void);
    void collect_block_triggered (trigger_e trigger_slope, double trigger_level);
    void collect_block_advanced_triggered ();
    void collect_block_ets (void);
    void collect_streaming (void);
    void collect_fast_streaming (void);
    void collect_fast_streaming_triggered (void);
    static void  __stdcall ps6000FastStreamingReady( short **overviewBuffers,
                                                     short overflow,
                                                     unsigned long triggeredAt,
                                                     short triggered,
                                                     short auto_stop,
                                                     unsigned long nValues);
    /**
     * @brief private instances declarations
     */
    UNIT_MODEL unitOpened_m;
    static Acquisition6000 *singleton_m;
    int scale_to_mv;
    short timebase;
    double time_per_division_m;
    long times[BUFFER_SIZE];
    static const short input_ranges [PS6000_MAX_RANGES] /*= {10, 20, 50, 100, 200, 500, 1000, 3000, 5000, 10000, 30000, 50000}*/;
};

#endif // HAVE_LIBPS6000
#endif // ACQUISITION6000_H
