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
 * @file acquisition6000.cpp
 * @brief Definition of Acquisition6000 class.
 * Acquisition6000 methods using libps6000 to drive Picoscope 6000 series HW
 * @version 0.1
 * @date 2013, january 4
 * @author Vincent HERVIEUX    -   01.04.2013   -   initial creation
 */

#include "acquisition6000.h"

#ifdef HAVE_LIBPS6000

#ifndef WIN32
#define Sleep(x) usleep(1000*(x))
enum BOOL {FALSE,TRUE};
#endif

#define QUAD_SCOPE 4
#define DUAL_SCOPE 2

/* static members initialization */
Acquisition6000 *Acquisition6000::singleton_m = NULL;
const short Acquisition6000::input_ranges [] = {10, 20, 50, 100, 200, 500, 1000, 3000, 5000, 10000, 30000, 50000};

/****************************************************************************
 *
 * constructor
 *
 ****************************************************************************/
Acquisition6000::Acquisition6000() :
    scale_to_mv(1),
    timebase(8)
{
    DEBUG( "Opening the device...\n");

    //open unit and show splash screen
    unitOpened_m.handle = ps6000_open_unit ();
    DEBUG ( "Handle: %d\n", unitOpened_m.handle );
    if ( unitOpened_m.handle < 1 )
    {
        DEBUG ( "Unable to open device\n" );
        unitOpened_m.model = MODEL_NONE;
        return;
    }


    if ( unitOpened_m.handle >= 1 )
    {
        DEBUG ( "Device opened successfully\n\n" );
        get_info ();

    }

}

/****************************************************************************
 *
 * get_instance
 *
 ****************************************************************************/
Acquisition6000* Acquisition6000::get_instance()
{
    if(NULL == Acquisition6000::singleton_m)
    {
        Acquisition6000::singleton_m = new Acquisition6000();
    }

    return Acquisition6000::singleton_m;
}

/****************************************************************************
 *
 * destructor
 *
 ****************************************************************************/
Acquisition6000::~Acquisition6000()
{
    DEBUG ( "Device destroyed\n" );
    ps6000_close_unit ( unitOpened_m.handle ); 
    Acquisition6000::singleton_m = NULL;
}

/****************************************************************************
 *
 * ps6000FastStreamingReady
 *
 ****************************************************************************/
void  __stdcall Acquisition6000::ps6000FastStreamingReady( short **overviewBuffers,
                                                              short overflow,
                                                                unsigned long triggeredAt,
                                                                short triggered,
                                                                short auto_stop,
                                                              unsigned long nValues)
{
    (void)overviewBuffers;
    (void)overflow;
    (void)triggeredAt;
    (void)triggered;
    Acquisition6000* instance = Acquisition6000::get_instance();
    if(NULL != instance)
    {
        instance->unitOpened_m.trigger.advanced.totalSamples += nValues;
        instance->unitOpened_m.trigger.advanced.autoStop = auto_stop;
    }
}

/****************************************************************************
 *
 * get_device_info
 *
 ****************************************************************************/
void Acquisition6000::get_device_info(device_info_t* info)
{
    if(NULL == info)
    {
        ERROR("%s : invalid pointer given!\n", __FUNCTION__);
        return;
    }

    memset(info, 0, sizeof(device_info_t));

    switch(unitOpened_m.model)
    {
        case MODEL_PS6402:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6402");
            break;
        case MODEL_PS6402A:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6402A");
            break;
        case MODEL_PS6402B:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6402B");
            break;
        case MODEL_PS6402C:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6402C");
            break;
        case MODEL_PS6402D:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6402D");
            break;
        case MODEL_PS6403:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6403");
            break;
        case MODEL_PS6403A:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6403A");
            break;
        case MODEL_PS6403B:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6403B");
            break;
        case MODEL_PS6403C:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6403C");
            break;
        case MODEL_PS6403D:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6403D");
            break;
        case MODEL_PS6404:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6404");
            break;
        case MODEL_PS6404A:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6404A");
            break;
        case MODEL_PS6404B:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6404B");
            break;
        case MODEL_PS6404C:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6404C");
            break;
        case MODEL_PS6404D:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6404D");
            break;
        case MODEL_PS6407:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6407");
            break;
        case MODEL_PS6408:
            snprintf(info->device_name, DEVICE_NAME_MAX, "PS6408");
            break;
        case MODEL_NONE:
            snprintf(info->device_name, DEVICE_NAME_MAX, "No device or device not supported"); 
            break;
    }
    info->nb_channels = unitOpened_m.noOfChannels;
#ifdef TEST_WITHOUT_HW
    snprintf(info->device_name, DEVICE_NAME_MAX, "Tests without HW");
    info->nb_channels = 2;
#endif
}

/****************************************************************************
 * set_defaults - restore default settings
 ****************************************************************************/
void Acquisition6000::set_defaults (void)
{
    short ch = 0;
  ps6000_set_ets ( unitOpened_m.handle, PS6000_ETS_OFF, 0, 0 );

    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        ps6000_set_channel ( unitOpened_m.handle,
                               ch,
                                                 unitOpened_m.channelSettings[ch].enabled ,
                                                 unitOpened_m.channelSettings[ch].DCcoupled ,
                                                 unitOpened_m.channelSettings[ch].range);
    }
}

/****************************************************************************
 * set_trigger_advanced - set advance trigger parameters
 ****************************************************************************/
void Acquisition6000::set_trigger_advanced(void)
{
    short ok = 0;
    short auto_trigger_ms = 0;

    // to trigger of more than one channel set this parameter to 2 or more
    // each condition can only have on parameter set to PS6000_CONDITION_TRUE or PS6000_CONDITION_FALSE
    // if more than on condition is set then it will trigger off condition one, or condition two etc.
    unitOpened_m.trigger.advanced.nProperties = 1;
    // set the trigger channel to channel A by using PS6000_CONDITION_TRUE
    unitOpened_m.trigger.advanced.conditions = (TRIGGER_CONDITIONS*)malloc (sizeof (TRIGGER_CONDITIONS) * unitOpened_m.trigger.advanced.nProperties);
    unitOpened_m.trigger.advanced.conditions->channelA = CONDITION_TRUE;
    unitOpened_m.trigger.advanced.conditions->channelB = CONDITION_DONT_CARE;
    unitOpened_m.trigger.advanced.conditions->channelC = CONDITION_DONT_CARE;
    unitOpened_m.trigger.advanced.conditions->channelD = CONDITION_DONT_CARE;
    unitOpened_m.trigger.advanced.conditions->external = CONDITION_DONT_CARE;
    unitOpened_m.trigger.advanced.conditions->pulseWidthQualifier = CONDITION_DONT_CARE;

    // set channel A to rising
    // the remainder will be ignored as only a condition is set for channel A
    unitOpened_m.trigger.advanced.directions.channelA = RISING;
    unitOpened_m.trigger.advanced.directions.channelB = RISING;
    unitOpened_m.trigger.advanced.directions.channelC = RISING;
    unitOpened_m.trigger.advanced.directions.channelD = RISING;
    unitOpened_m.trigger.advanced.directions.ext = RISING;


    unitOpened_m.trigger.advanced.channelProperties = (TRIGGER_CHANNEL_PROPERTIES*)malloc (sizeof (TRIGGER_CHANNEL_PROPERTIES) * unitOpened_m.trigger.advanced.nProperties);
    // there is one property for each condition
    // set channel A
    // trigger level 1500 adc counts the trigger point will vary depending on the voltage range
    // hysterisis 4096 adc counts
    unitOpened_m.trigger.advanced.channelProperties->channel = (short) PS6000_CHANNEL_A;
    unitOpened_m.trigger.advanced.channelProperties->thresholdMajor = 1500;
    // not used in level triggering, should be set when in window mode
    unitOpened_m.trigger.advanced.channelProperties->thresholdMinor = 0;
    // used in level triggering, not used when in window mode
    unitOpened_m.trigger.advanced.channelProperties->hysteresis = (short) 4096;
    unitOpened_m.trigger.advanced.channelProperties->thresholdMode = LEVEL;

    ok = ps6000SetAdvTriggerChannelConditions (unitOpened_m.handle, unitOpened_m.trigger.advanced.conditions, unitOpened_m.trigger.advanced.nProperties);
    if ( !ok )
        WARNING("ps6000SetAdvTriggerChannelConditions returned value is not 0.");
    ok = ps6000SetAdvTriggerChannelDirections (unitOpened_m.handle,
                                                                                unitOpened_m.trigger.advanced.directions.channelA,
                                                                                unitOpened_m.trigger.advanced.directions.channelB,
                                                                                unitOpened_m.trigger.advanced.directions.channelC,
                                                                                unitOpened_m.trigger.advanced.directions.channelD,
                                                                                unitOpened_m.trigger.advanced.directions.ext);
    if ( !ok )
        WARNING("ps6000SetAdvTriggerChannelDirections returned value is not 0.");
    ok = ps6000SetAdvTriggerChannelProperties (unitOpened_m.handle,
                                                                                unitOpened_m.trigger.advanced.channelProperties,
                                                                                unitOpened_m.trigger.advanced.nProperties,
                                                                                auto_trigger_ms);
    if ( !ok )
        WARNING("ps6000SetAdvTriggerChannelProperties returned value is not 0.");


    // remove comments to try triggering with a pulse width qualifier
    // add a condition for the pulse width eg. in addition to the channel A or as a replacement
    //unitOpened_m.trigger.advanced.pwq.conditions = malloc (sizeof (PS6000_PWQ_CONDITIONS));
    //unitOpened_m.trigger.advanced.pwq.conditions->channelA = PS6000_CONDITION_TRUE;
    //unitOpened_m.trigger.advanced.pwq.conditions->channelB = PS6000_CONDITION_DONT_CARE;
    //unitOpened_m.trigger.advanced.pwq.conditions->channelC = PS6000_CONDITION_DONT_CARE;
    //unitOpened_m.trigger.advanced.pwq.conditions->channelD = PS6000_CONDITION_DONT_CARE;
    //unitOpened_m.trigger.advanced.pwq.conditions->external = PS6000_CONDITION_DONT_CARE;
    //unitOpened_m.trigger.advanced.pwq.nConditions = 1;

    //unitOpened_m.trigger.advanced.pwq.direction = PS6000_RISING;
    //unitOpened_m.trigger.advanced.pwq.type = PS6000_PW_TYPE_LESS_THAN;
    //// used when type    PS6000_PW_TYPE_IN_RANGE,    PS6000_PW_TYPE_OUT_OF_RANGE
    //unitOpened_m.trigger.advanced.pwq.lower = 0;
    //unitOpened_m.trigger.advanced.pwq.upper = 10000;
    //ps6000SetPulseWidthQualifier (unitOpened_m.handle,
    //                                                            unitOpened_m.trigger.advanced.pwq.conditions,
    //                                                            unitOpened_m.trigger.advanced.pwq.nConditions,
    //                                                            unitOpened_m.trigger.advanced.pwq.direction,
    //                                                            unitOpened_m.trigger.advanced.pwq.lower,
    //                                                            unitOpened_m.trigger.advanced.pwq.upper,
    //                                                            unitOpened_m.trigger.advanced.pwq.type);

    ok = ps6000SetAdvTriggerDelay (unitOpened_m.handle, 0, -10);
}

/****************************************************************************
 * Collect_block_immediate
 *  this function demonstrates how to collect a single block of data
 *  from the unit (start collecting immediately)
 ****************************************************************************/
void Acquisition6000::collect_block_immediate (void)
{
    int   i = 0;
    long  time_interval;
    short time_units;
    short oversample;
    int   no_of_samples = BUFFER_SIZE;
    int nb_of_samples_in_screen = 0;
    short auto_trigger_ms = 0;
    long  time_indisposed_ms;
    short overflow;
    long  max_samples;
    short ch = 0;
    double* values_V[CHANNEL_MAX] = {NULL};
    double* time[CHANNEL_MAX] = {NULL};
    double time_multiplier = 0.;
    double time_offset[CHANNEL_MAX] = {0.};
    int index[CHANNEL_MAX] = {0};

    DEBUG ( "Collect block immediate...\n" );

    set_defaults ();

    /* Trigger disabled
    */
    ps6000_set_trigger ( unitOpened_m.handle, PS6000_NONE, 0, PS6000_RISING, 0, auto_trigger_ms );

    /*  find the maximum number of samples, the time interval (in time_units),
    *         the most suitable time units, and the maximum oversample at the current timebase
    */
    oversample = 1;
    while (!ps6000_get_timebase ( unitOpened_m.handle,
                                timebase,
                                no_of_samples,
                                &time_interval,
                                &time_units,
                                oversample,
                                &max_samples))
    timebase++;                                        ;

    time_multiplier = adc_multipliers(time_units);
    nb_of_samples_in_screen = (int)(5 * time_per_division_m / (time_interval * time_multiplier)) + 1;
    nb_of_samples_in_screen = ( nb_of_samples_in_screen < BUFFER_SIZE ? BUFFER_SIZE : nb_of_samples_in_screen);
    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        if (unitOpened_m.channelSettings[ch].enabled)
        {
            values_V[ch] = (double*)malloc(nb_of_samples_in_screen * sizeof(double));
            time[ch] = (double*)malloc(nb_of_samples_in_screen * sizeof(double));
        }
    }
    DEBUG ( "timebase: %hd\tnb_of_samples:%d\toversample:%hd\ttime_units:%hd\ttime_interval:%lu\ttime_multiplier:%e\tnb_of_samples_in_screen:%d\n", 
             timebase, no_of_samples, oversample, time_units, time_interval, time_multiplier, nb_of_samples_in_screen );


    while ( sem_trywait(&thread_stop) )
    {
        /* Start it collecting,
        *  then wait for completion
        */
        ps6000_run_block ( unitOpened_m.handle, no_of_samples, timebase, oversample, &time_indisposed_ms );
        while ( !ps6000_ready ( unitOpened_m.handle ) )
        {
            if( !sem_trywait(&thread_stop) )
            {
                /* re-post semaphore to exit the main loop */
                sem_post(&thread_stop);
                break;
            }
            Sleep ( 100 );
        }

        ps6000_stop ( unitOpened_m.handle );

        /* Should be done now...
        *  get the times (in nanoseconds)
        *   and the values (in ADC counts)
        */
        ps6000_get_times_and_values ( unitOpened_m.handle, times,
                                    unitOpened_m.channelSettings[PS6000_CHANNEL_A].values,
                                    unitOpened_m.channelSettings[PS6000_CHANNEL_B].values,
                                    unitOpened_m.channelSettings[PS6000_CHANNEL_C].values,
                                    unitOpened_m.channelSettings[PS6000_CHANNEL_D].values,
                                    &overflow, time_units, no_of_samples );

        DEBUG ( "%d values, overflow %d\n", no_of_samples, overflow );

        for (ch = 0; (ch < unitOpened_m.noOfChannels) && (no_of_samples > 0); ch++)
        {
            if (unitOpened_m.channelSettings[ch].enabled)
            {
                for (  i = 0; (i < no_of_samples) && (index[ch] < nb_of_samples_in_screen) ; i++, index[ch]++ )
                {
                    values_V[ch][index[ch]] = 0.001 * adc_to_mv(unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range);
#if 0
                    DEBUG("(times[%d] * time_multiplier): %lf;\t(%d * time_multiplier * time_interval): %lf\n", 
                          i, 
                          (times[i] * time_multiplier), 
                          i, 
                          (i * time_multiplier * time_interval)
                         );
#endif
                    time[ch][index[ch]] = (times[i] * time_multiplier) + time_offset[ch];
                    //DEBUG("V: %lf (range %d) T: %lf\n", values_V[index[ch]], unitOpened_m.channelSettings[ch].range, time[index[ch]]);
                }
                // resetting all available data as long as the screen is not filled.
                draw->setData(ch+1, time[ch], values_V[ch], index[ch]);
                DEBUG("set %d data\n", index[ch]);
                if( (index[ch] >= nb_of_samples_in_screen) || (time[ch][index[ch] - 1] > 5 * time_per_division_m) )
                {
                    time_offset[ch] = 0.;
                    memset(time[ch], 0, nb_of_samples_in_screen * sizeof(double));
                    memset(values_V[ch], 0, nb_of_samples_in_screen * sizeof(double));
                    index[ch] = 0;
                }
                else
                {
                    time_offset[ch] = time[ch][index[ch] - 1];
                }
            }
        }
        Sleep(100);
    }
    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        if (unitOpened_m.channelSettings[ch].enabled)
        {
            free(values_V[ch]);
            free(time[ch]);
        }
    }
}

    /****************************************************************************
     * Collect_block_triggered
     *  this function demonstrates how to collect a single block of data from the
     *  unit, when a trigger event occurs.
     ****************************************************************************/

void Acquisition6000::collect_block_triggered (trigger_e trigger_slope, double trigger_level)
{
    int i = 0;
    long time_interval;
    short time_units;
    short oversample;
    int no_of_samples = BUFFER_SIZE;
    int nb_of_samples_in_screen = 0;
    short auto_trigger_ms = 0;
    long time_indisposed_ms;
    short overflow;
    int     threshold_mv = (int)(trigger_level * 1000);
    long max_samples;
    short ch = 0;
    double* values_V[CHANNEL_MAX] = {NULL};
    double* time[CHANNEL_MAX] = {NULL};
    double time_multiplier = 0.;
    double time_offset[CHANNEL_MAX] = {0.};
    int index[CHANNEL_MAX] = {0};
    DEBUG ( "Collect block triggered...\n" );
    DEBUG ( "Collects when value rises past %dmV\n", threshold_mv );

    set_defaults ();

    /* Trigger enabled
     * ChannelA - to trigger unsing this channel it needs to be enabled using ps6000_set_channel
     * Rising edge
     * Threshold = 100mV
     * 10% pre-trigger  (negative is pre-, positive is post-)
     */
    unitOpened_m.trigger.simple.channel = PS6000_CHANNEL_A;
    switch(trigger_slope)
    {
        case E_TRIGGER_FALLING:
            unitOpened_m.trigger.simple.direction = (short) PS6000_FALLING;
        break;
        case E_TRIGGER_RISING:
        default:
            unitOpened_m.trigger.simple.direction = (short) PS6000_RISING;
        break;
    }
    unitOpened_m.trigger.simple.threshold = 100.f;
    unitOpened_m.trigger.simple.delay = -10;

    ps6000_set_trigger ( unitOpened_m.handle,
                         (short) unitOpened_m.trigger.simple.channel,
                         mv_to_adc (threshold_mv, unitOpened_m.channelSettings[(short) unitOpened_m.trigger.simple.channel].range),
                         unitOpened_m.trigger.simple.direction,
                         (short)unitOpened_m.trigger.simple.delay,
                         auto_trigger_ms );


    /* find the maximum number of samples, the time interval (in time_units),
     * the most suitable time units, and the maximum oversample at the current timebase
     */
    oversample = 1;
    while (!ps6000_get_timebase ( unitOpened_m.handle,
                                    timebase,
                                    no_of_samples,
                                    &time_interval,
                                    &time_units,
                                    oversample,
                                    &max_samples))
    timebase++;

    time_multiplier = adc_multipliers(time_units);
    nb_of_samples_in_screen = (int)(5 * time_per_division_m / (time_interval * time_multiplier)) + 1;
    nb_of_samples_in_screen = ( nb_of_samples_in_screen < BUFFER_SIZE ? BUFFER_SIZE : nb_of_samples_in_screen);
    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        if (unitOpened_m.channelSettings[ch].enabled)
        {
            values_V[ch] = (double*)malloc(nb_of_samples_in_screen * sizeof(double));
            time[ch] = (double*)malloc(nb_of_samples_in_screen * sizeof(double));
        }
    }
    DEBUG ( "timebase: %hd\tnb_of_samples:%d\toversample:%hd\ttime_units:%hd\ttime_interval:%lu\ttime_multiplier:%e\tnb_of_samples_in_screen:%d\n", 
             timebase, no_of_samples, oversample, time_units, time_interval, time_multiplier, nb_of_samples_in_screen );

    while ( sem_trywait(&thread_stop) )
    {
        /* Start it collecting,
         *  then wait for completion
         */
        ps6000_run_block ( unitOpened_m.handle, BUFFER_SIZE, timebase, oversample, &time_indisposed_ms );

        while ( !ps6000_ready ( unitOpened_m.handle ) )
        {
            if( !sem_trywait(&thread_stop) )
            {
                /* re-post semaphore to exit the main loop */
                sem_post(&thread_stop);
                break;
            }
            Sleep ( 100 );
        }

        ps6000_stop ( unitOpened_m.handle );

        /* Get the times (in units specified by time_units)
         *  and the values (in ADC counts)
         */
        ps6000_get_times_and_values ( unitOpened_m.handle, times,
                                  unitOpened_m.channelSettings[PS6000_CHANNEL_A].values,
                                  unitOpened_m.channelSettings[PS6000_CHANNEL_B].values,
                                  unitOpened_m.channelSettings[PS6000_CHANNEL_C].values,
                                  unitOpened_m.channelSettings[PS6000_CHANNEL_D].values,
                                  &overflow, time_units, BUFFER_SIZE );

        DEBUG ( "%d values, overflow %d\n", no_of_samples, overflow );

        for (ch = 0; (ch < unitOpened_m.noOfChannels) && (no_of_samples > 0); ch++)
        {
            if (unitOpened_m.channelSettings[ch].enabled)
            {
                for (  i = 0; (i < no_of_samples) && (index[ch] < nb_of_samples_in_screen) ; i++, index[ch]++ )
                {
                    values_V[ch][index[ch]] = 0.001 * adc_to_mv(unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range);
                    time[ch][index[ch]] = (times[i] * time_multiplier) + time_offset[ch];
                    //DEBUG("V: %lf (range %d) T: %lf\n", values_V[index[ch]], unitOpened_m.channelSettings[ch].range, time[index[ch]]);
                }
                // resetting all available data as long as the screen is not filled.
                draw->setData(ch+1, time[ch], values_V[ch], index[ch]);
                DEBUG("set %d data\n", index[ch]);
                if( (index[ch] >= nb_of_samples_in_screen) || (time[ch][index[ch] - 1] > 5 * time_per_division_m) )
                {
                    time_offset[ch] = 0.;
                    memset(time[ch], 0, nb_of_samples_in_screen * sizeof(double));
                    memset(values_V[ch], 0, nb_of_samples_in_screen * sizeof(double));
                    index[ch] = 0;
                }
                else
                {
                    time_offset[ch] = time[ch][index[ch] - 1];
                }
            }

        }
        Sleep(100);
    }

    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        if (unitOpened_m.channelSettings[ch].enabled)
        {
            free(values_V[ch]);
            free(time[ch]);
        }
    }
}

void Acquisition6000::collect_block_advanced_triggered ()
{
int        i;
  int        trigger_sample;
  long     time_interval;
  short     time_units;
  short     oversample;
  long     no_of_samples = BUFFER_SIZE;
  FILE     *fp;
  long     time_indisposed_ms;
  short     overflow;
  int     threshold_mv =1500;
  long     max_samples;
    short ch;

  DEBUG ( "Collect block triggered...\n" );
  DEBUG ( "Collects when value rises past %dmV\n", threshold_mv );
  DEBUG ( "Press a key to start...\n" );
  //getch ();

  set_defaults ();

  set_trigger_advanced ();


  /*  find the maximum number of samples, the time interval (in time_units),
   *         the most suitable time units, and the maximum oversample at the current timebase
   */
  oversample = 1;
  while (!ps6000_get_timebase ( unitOpened_m.handle,
                        timebase,
                                no_of_samples,
                        &time_interval,
                        &time_units,
                        oversample,
                        &max_samples))
      timebase++;

  /* Start it collecting,
   *  then wait for completion
   */
  ps6000_run_block ( unitOpened_m.handle, BUFFER_SIZE, timebase, oversample, &time_indisposed_ms );

  DEBUG ( "Waiting for trigger..." );
  DEBUG ( "Press a key to abort\n" );

  while (( !ps6000_ready ( unitOpened_m.handle )) /*&& ( !kbhit () )*/)
  {
    Sleep ( 100 );
  }

//  if (kbhit ())
//  {
//    //getch ();

//    DEBUG ( "data collection aborted\n" );
//  }
//  else
//  {
    ps6000_stop ( unitOpened_m.handle );

    /* Get the times (in units specified by time_units)
     *  and the values (in ADC counts)
     */
    ps6000_get_times_and_values ( unitOpened_m.handle,
                                  times,
                                  unitOpened_m.channelSettings[PS6000_CHANNEL_A].values,
                                  unitOpened_m.channelSettings[PS6000_CHANNEL_B].values,
                                  NULL,
                                  NULL,
                                  &overflow, time_units, BUFFER_SIZE );

    /* Print out the first 10 readings,
     *  converting the readings to mV if required
     */
    DEBUG ("Ten readings around trigger\n");
    DEBUG ("Time\tValue\n");
    DEBUG ("(ns)\t(%s)\n", adc_units (time_units));

    /* This calculation is correct for 10% pre-trigger
     */
    trigger_sample = BUFFER_SIZE / 10;

    for (i = trigger_sample - 5; i < trigger_sample + 5; i++)
    {
            for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
            {
                if(unitOpened_m.channelSettings[ch].enabled)
                {
                    DEBUG ( "%d\t", adc_to_mv ( unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range) );
                }
            }
            DEBUG("\n");
    }

    fp = fopen ( "data.txt","w" );

    if (fp != NULL)
    {
      for ( i = 0; i < BUFFER_SIZE; i++ )
    {
          fprintf ( fp,"%ld ", times[i]);
            for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
            {
                if(unitOpened_m.channelSettings[ch].enabled)
                {
                    fprintf ( fp, ",%d, %d,", unitOpened_m.channelSettings[ch].values[i],
                                                                        adc_to_mv ( unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range) );
                }
            }
          fprintf(fp, "\n");
      }
    fclose(fp);
    }
    else
        ERROR("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
//  }
}


/****************************************************************************
 * Collect_block_ets
 *  this function demonstrates how to collect a block of
 *  data using equivalent time sampling (ETS).
 ****************************************************************************/

void Acquisition6000::collect_block_ets (void)
{
    int       i;
    int       trigger_sample;
    FILE      *fp;
    short     auto_trigger_ms = 0;
    long      time_indisposed_ms;
    short     overflow;
    long      ets_sampletime;
    short     ok = 0;
    short     ch;

    DEBUG ( "Collect ETS block...\n" );
    DEBUG ( "Collects when value rises past 1500mV\n" );
    DEBUG ( "Press a key to start...\n" );
    //getch ();

    set_defaults ();

    /* Trigger enabled
    * Channel A - to trigger unsing this channel it needs to be enabled using ps6000_set_channel
    * Rising edge
    * Threshold = 1500mV
    * 10% pre-trigger  (negative is pre-, positive is post-)
    */
    unitOpened_m.trigger.simple.channel = PS6000_CHANNEL_A;
    unitOpened_m.trigger.simple.delay = -10.f;
    unitOpened_m.trigger.simple.direction = PS6000_RISING;
    unitOpened_m.trigger.simple.threshold = 1500.f;


    ps6000_set_trigger ( unitOpened_m.handle,
        (short) unitOpened_m.trigger.simple.channel,
        mv_to_adc (1500, unitOpened_m.channelSettings[(short) unitOpened_m.trigger.simple.channel].range),
        unitOpened_m.trigger.simple.direction ,
        (short) unitOpened_m.trigger.simple.delay,
        auto_trigger_ms );

    /* Enable ETS in fast mode,
    * the computer will store 60 cycles
    *  but interleave only 4
    */
    ets_sampletime = ps6000_set_ets ( unitOpened_m.handle, PS6000_ETS_FAST, 60, 4 );
    DEBUG ( "ETS Sample Time is: %ld\n", ets_sampletime );
    /* Start it collecting,
    *  then wait for completion
    */
    ok = ps6000_run_block ( unitOpened_m.handle, BUFFER_SIZE, timebase, 1, &time_indisposed_ms );
    if ( !ok )
        WARNING ( "ps6000_run_block return value is not 0." );

    DEBUG ( "Waiting for trigger..." );
    DEBUG ( "Press a key to abort\n" );

    while ( (!ps6000_ready (unitOpened_m.handle)) /*&& (!kbhit ())*/ )
    {
        Sleep (100);
    }

//    if ( kbhit () )
//    {
//        //getch ();
//        DEBUG ( "data collection aborted\n" );
//    }
//    else
//    {
        ps6000_stop ( unitOpened_m.handle );
        /* Get the times (in microseconds)
        *  and the values (in ADC counts)
        */
        ok = (short)ps6000_get_times_and_values ( unitOpened_m.handle,
                                                  times,
                                                  unitOpened_m.channelSettings[PS6000_CHANNEL_A].values,
                                                  unitOpened_m.channelSettings[PS6000_CHANNEL_B].values,
                                                  NULL,
                                                  NULL,
                                                  &overflow,
                                                  PS6000_PS,
                                                  BUFFER_SIZE);

        /* Print out the first 10 readings,
        *  converting the readings to mV if required
        */

        DEBUG ( "Ten readings around trigger\n" );
        DEBUG ( "(ps)\t(mv)\n");

        /* This calculation is correct for 10% pre-trigger
        */
        trigger_sample = BUFFER_SIZE / 10;

        for ( i = trigger_sample - 5; i < trigger_sample + 5; i++ )
        {
            DEBUG ( "%ld\t", times [i]);
            for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
            {
                if (unitOpened_m.channelSettings[ch].enabled)
                {
                    DEBUG ( "%d\t\n", adc_to_mv (unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range));
                }
            }
            DEBUG ("\n");
        }

        fp = fopen ( "data.txt","w" );
        if (fp != NULL)
        {
            for ( i = 0; i < BUFFER_SIZE; i++ )
            {
                fprintf ( fp, "%ld,", times[i] );
                for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
                {
                    if (unitOpened_m.channelSettings[ch].enabled)
                    {
                        fprintf ( fp, "%ld, %d, %d", times[i],  unitOpened_m.channelSettings[ch].values[i], adc_to_mv (unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range) );
                    }
                }
                fprintf (fp, "\n");
            }
            fclose( fp );
        }
        else
            ERROR("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
//    }
}

/****************************************************************************
 *
 * Collect_streaming
 *  this function demonstrates how to use streaming.
 *
 * In this mode, you can collect data continuously.
 *
 * This example writes data to disk...
 * don't leave it running too long or it will fill your disk up!
 *
 * Each call to ps6000_get_times_and_values returns the readings since the
 * last call
 *
 * The time is in microseconds: it will wrap around at 2^32 (approx 2,000 seconds)
 * if you don't need the time, you can just call ps6000_get_values
 *
 ****************************************************************************/

void Acquisition6000::collect_streaming (void)
{
    int    i = 0;
    int count = 0;
    int    no_of_values;
    short  overflow;
    int    ok;
    short  ch;
    double values_V[BUFFER_SIZE] = {0};
    double time[BUFFER_SIZE] = {0};
    DEBUG ( "Collect streaming...\n" );

    set_defaults ();

    /* You cannot use triggering for the start of the data...
    */
    ps6000_set_trigger ( unitOpened_m.handle, PS6000_NONE, 0, 0, 0, 0 );

    /* Collect data at time_per_division_m / 100  intervals
    * Max BUFFER_SIZE points on each call
    *  (buffer must be big enough for max time between calls
    *
    *  Start it collecting,
    *  then wait for trigger event
    */
    ok = ps6000_run_streaming ( unitOpened_m.handle, (time_per_division_m * 1000. / 100.) , 1000, 0 );
    DEBUG ( "OK: %d\n", ok );

    
    while ( sem_trywait(&thread_stop) )
    {
        no_of_values = ps6000_get_values ( unitOpened_m.handle,
            unitOpened_m.channelSettings[PS6000_CHANNEL_A].values,
            unitOpened_m.channelSettings[PS6000_CHANNEL_B].values,
            unitOpened_m.channelSettings[PS6000_CHANNEL_C].values,
            unitOpened_m.channelSettings[PS6000_CHANNEL_D].values,
            &overflow,
            BUFFER_SIZE );
        DEBUG ( "%d values, overflow %d\n", no_of_values, overflow );

        for (ch = 0; (ch < unitOpened_m.noOfChannels) && (no_of_values > 0); ch++)
        {
            if (unitOpened_m.channelSettings[ch].enabled)
            {

                for (  i = 0; i < no_of_values; i++, count++ )
                {
                    values_V[count] = 0.001 * adc_to_mv(unitOpened_m.channelSettings[ch].values[i], unitOpened_m.channelSettings[ch].range);
                    // TODO time will be probably wrong here, need to guess how to convert time range to time step...
                    //time[i] = ( i ? time[i-1] : 0) + unitOpened_m.channelSettings[ch].range
                    time[count] = count * 0.01 * time_per_division_m;
                    DEBUG("V: %lf (range %d) T: %lf\n", values_V[count], unitOpened_m.channelSettings[ch].range, time[count]);
                    // 500 points are making a screen:
                    if(count == 500)
                    {
                        count = 0;
                        memset(time, 0, BUFFER_SIZE * sizeof(double));
                        memset(values_V, 0, BUFFER_SIZE * sizeof(double));
                    }
                }
                
                draw->setData(ch+1, time, values_V, count);

            }

        }
        Sleep(100);
    }

    ps6000_stop ( unitOpened_m.handle );

}

void Acquisition6000::collect_fast_streaming (void)
{
    unsigned long    i;
    short  overflow;
    int     ok;
    short ch;
    unsigned long nPreviousValues = 0;
    short values_a[BUFFER_SIZE_STREAMING];
    short values_b[BUFFER_SIZE_STREAMING];
    short *values = NULL;
    double values_mV[BUFFER_SIZE_STREAMING];
    double time[BUFFER_SIZE_STREAMING];
    unsigned long triggerAt;
    short triggered;
    unsigned long no_of_samples;
    double startTime = 0;


    DEBUG ( "Collect fast streaming...\n" );

    set_defaults ();

    /* You cannot use triggering for the start of the data...
    */
    ps6000_set_trigger ( unitOpened_m.handle, PS6000_NONE, 0, 0, 0, 0 );

    unitOpened_m.trigger.advanced.autoStop = 0;
    unitOpened_m.trigger.advanced.totalSamples = 0;
    unitOpened_m.trigger.advanced.triggered = 0;

    /* Collect data at 10us intervals
    * 100000 points with an agregation of 100 : 1
    *    Auto stop after the 100000 samples
    *  Start it collecting,
    */
    ok = ps6000_run_streaming_ns ( unitOpened_m.handle, 10, PS6000_US, BUFFER_SIZE_STREAMING, 1, 100, 30000 );
    DEBUG ( "OK: %d\n", ok );

    /* From here on, we can get data whenever we want...
    */

    while ( !unitOpened_m.trigger.advanced.autoStop && sem_trywait(&thread_stop))
    {

        ps6000_get_streaming_last_values (unitOpened_m.handle, &Acquisition6000::ps6000FastStreamingReady);
        if (nPreviousValues != unitOpened_m.trigger.advanced.totalSamples)
        {

            DEBUG ("Values collected: %ld\n", unitOpened_m.trigger.advanced.totalSamples - nPreviousValues);
            nPreviousValues =     unitOpened_m.trigger.advanced.totalSamples;

        }
        Sleep (0);

    }

    ps6000_stop (unitOpened_m.handle);

    no_of_samples = ps6000_get_streaming_values_no_aggregation (unitOpened_m.handle,
                                                                &startTime, // get samples from the beginning
                                                                values_a, // set buffer for channel A
                                                                values_b, // set buffer for channel B
                                                                NULL,
                                                                NULL,
                                                                &overflow,
                                                                &triggerAt,
                                                                &triggered,
                                                                BUFFER_SIZE_STREAMING);


    // print out the first 20 readings
    for ( i = 0; i < 20; i++ )
    {
        for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
        {
            if (unitOpened_m.channelSettings[ch].enabled)
            {
                printf("%d, ", adc_to_mv ((!ch ? values_a[i] : values_b[i]), unitOpened_m.channelSettings[ch].range) );
            }
        }
            printf("\n");
    }

    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        if (unitOpened_m.channelSettings[ch].enabled)
        {
            switch(ch)
            {
                case 0:
                    values = values_a;
                break;
                case 1:
                    values = values_b;
                break;
                default:
                    return;
                break;
            }

            for (  i = 0; i < no_of_samples; i++ )
            {
                values_mV[i] = adc_to_mv(values[i], unitOpened_m.channelSettings[ch].range);
                // TODO time will be probably wrong here, need to guess how to convert time range to time step...
                time[i] = ( i ? time[i-1] : 0) + unitOpened_m.channelSettings[ch].range;
            }

            draw->setData(ch+1, values_mV, time, no_of_samples);
        }
               
    }


/*    DEBUG ( "Data is written to disk file (data.txt)\n" );
    fp = fopen ( "data.txt", "w+" );
    if (fp != NULL)
    {
        for ( i = 0; i < no_of_samples; i++ )
        {
            for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
            {
                if (unitOpened_m.channelSettings[ch].enabled)
                {
                    fprintf ( fp, "%d, ", adc_to_mv ((!ch ? values_a[i] : values_b[i]), unitOpened_m.channelSettings[ch].range) );
                }
            }
            fprintf (fp, "\n");
        }
        fclose ( fp );
    }
    else
        ERROR("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
*/

    //getch ();
}

void Acquisition6000::collect_fast_streaming_triggered (void)
{
    unsigned long    i;
    FILE     *fp;
    short  overflow;
    int     ok;
    short ch;
    unsigned long nPreviousValues = 0;
    short values_a[BUFFER_SIZE_STREAMING];
    short values_b[BUFFER_SIZE_STREAMING];
    unsigned long    triggerAt;
    short triggered;
    unsigned long no_of_samples;
    double startTime = 0;



    DEBUG ( "Collect fast streaming triggered...\n" );
    DEBUG ( "Data is written to disk file (data.txt)\n" );
    DEBUG ( "Press a key to start\n" );
    //getch ();

    set_defaults ();

    set_trigger_advanced ();

    unitOpened_m.trigger.advanced.autoStop = 0;
    unitOpened_m.trigger.advanced.totalSamples = 0;
    unitOpened_m.trigger.advanced.triggered = 0;

    /* Collect data at 10us intervals
    * 100000 points with an agregation of 100 : 1
    *    Auto stop after the 100000 samples
    *  Start it collecting,
    */
    ok = ps6000_run_streaming_ns ( unitOpened_m.handle, 10, PS6000_US, BUFFER_SIZE_STREAMING, 1, 100, 30000 );
    DEBUG ( "OK: %d\n", ok );

    /* From here on, we can get data whenever we want...
    */

    while (!unitOpened_m.trigger.advanced.autoStop)
    {
        ps6000_get_streaming_last_values (unitOpened_m.handle, ps6000FastStreamingReady);
        if (nPreviousValues != unitOpened_m.trigger.advanced.totalSamples)
        {
            DEBUG ("Values collected: %ld\n", unitOpened_m.trigger.advanced.totalSamples - nPreviousValues);
            nPreviousValues =     unitOpened_m.trigger.advanced.totalSamples;
        }
        Sleep (0);
    }

    ps6000_stop (unitOpened_m.handle);

    no_of_samples = ps6000_get_streaming_values_no_aggregation (unitOpened_m.handle,
                                                                &startTime, // get samples from the beginning
                                                                values_a, // set buffer for channel A
                                                                values_b,    // set buffer for channel B
                                                                NULL,
                                                                NULL,
                                                                &overflow,
                                                                &triggerAt,
                                                                &triggered,
                                                                BUFFER_SIZE_STREAMING);


    // if the unit triggered print out ten samples either side of the trigger point
    // otherwise print the first 20 readings
    for ( i = (triggered ? triggerAt - 10 : 0) ; i < ((triggered ? triggerAt - 10 : 0) + 20); i++)
    {
        for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
        {
            if (unitOpened_m.channelSettings[ch].enabled)
            {
                DEBUG ("%d, ", adc_to_mv ((!ch ? values_a[i] : values_b[i]), unitOpened_m.channelSettings[ch].range) );
            }
        }
        DEBUG ("\n");
    }

    fp = fopen ( "data.txt", "w" );
    if (fp != NULL)
    {
        for ( i = 0; i < no_of_samples; i++ )
        {
            for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
            {
                if (unitOpened_m.channelSettings[ch].enabled)
                {
                    fprintf ( fp, "%d, ", adc_to_mv ((!ch ? values_a[i] : values_b[i]), unitOpened_m.channelSettings[ch].range) );
                }
            }
            fprintf (fp, "\n");
        }
        fclose ( fp );
    }
    else
        DEBUG("Cannot open the file data.txt for writing. \nPlease ensure that you have permission to access. \n");
    //getch ();
}


/****************************************************************************
 *
 *
 ****************************************************************************/
void Acquisition6000::get_info (void)
  {
        int16_t i, r = 0;
        char line [7];
        int32_t variant;

        if (unitOpened_m.handle) 
        {
                // info = 3 - PICO_VARIANT_INFO
                ps6000GetUnitInfo(unitOpened_m.handle, line, sizeof (line), &r, 3);
                variant = atoi(line);
                memcpy(&(unitOpened_m.modelString),line,sizeof(unitOpened_m.modelString)==7?7:sizeof(unitOpened_m.modelString));
                //To identify A or B model variants.....
                if (strlen(line) == 4)                                                      // standard, not A, B, C or D, convert model number into hex i.e 6402 -> 0x6402
                        variant += 0x4B00;
                else
                        if (strlen(line) == 5)                                              // A or B variant unit 
                        {
                                line[4] = toupper(line[4]);

                                switch(line[4])
                                {
                                case 65: // i.e 6402A -> 0xA402
                                        variant += 0x8B00;
                                        break;
                                case 66: // i.e 6402B -> 0xB402
                                        variant += 0x9B00;
                                        break;
                                case 67: // i.e 6402C -> 0xC402
                                        variant += 0xAB00;
                                        break;
                                case 68: // i.e 6402D -> 0xD402
                                        variant += 0xBB00;
                                        break;
                                default:
                                        break;
                                }
                        }

                switch (variant)
                {
                case MODEL_PS6402:
                        unitOpened_m.model             = MODEL_PS6402;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6402A:
                        unitOpened_m.model             = MODEL_PS6402A;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = FALSE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6402B:
                        unitOpened_m.model             = MODEL_PS6402B;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6402C:
                        unitOpened_m.model             = MODEL_PS6402C;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6402D:
                        unitOpened_m.model             = MODEL_PS6402D;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6403:
                        unitOpened_m.model             = MODEL_PS6403;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6403A:
                        unitOpened_m.model             = MODEL_PS6403;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = FALSE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6403B:
                        unitOpened_m.model             = MODEL_PS6403B;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6403C:
                        unitOpened_m.model             = MODEL_PS6403C;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6403D:
                        unitOpened_m.model             = MODEL_PS6403D;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6404:
                        unitOpened_m.model             = MODEL_PS6404;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6404A:
                        unitOpened_m.model             = MODEL_PS6404;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = FALSE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6404B:
                        unitOpened_m.model             = MODEL_PS6404B;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6404C:
                        unitOpened_m.model             = MODEL_PS6404C;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6404D:
                        unitOpened_m.model             = MODEL_PS6404D;
                        unitOpened_m.firstRange = PS6000_50MV;
                        unitOpened_m.lastRange = PS6000_20V;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_5V;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_1M;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6407:
                        unitOpened_m.model             = MODEL_PS6407;
                        unitOpened_m.firstRange = PS6000_100MV;
                        unitOpened_m.lastRange = PS6000_100MV;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++) 
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_100MV;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_50R;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                        break;

                case MODEL_PS6408:
                        unitOpened_m.model             = MODEL_PS6408;
                        unitOpened_m.firstRange = PS6000_100MV;
                        unitOpened_m.lastRange = PS6000_100MV;
                        unitOpened_m.channelCount = 4;
                        unitOpened_m.AWG = TRUE;

                        for (i = 0; i < PS6000_MAX_CHANNELS; i++)
                        {
                                unitOpened_m.channelSettings[i].range = PS6000_100MV;
                                unitOpened_m.channelSettings[i].DCcoupled = PS6000_DC_50R;
                                unitOpened_m.channelSettings[i].enabled = TRUE;
                        }
                break;

                default:
                        break;
                }

                // info = 4 - PICO_BATCH_AND_SERIAL
                ps6000GetUnitInfo(unitOpened_m.handle, unitOpened_m.serial, sizeof (unitOpened_m.serial), &r, 4);
        }
}
/*void Acquisition6000::get_info (void)
  {

  char description [6][25]=  { "Driver Version","USB Version","Hardware Version",
                              "Variant Info","Serial", "Error Code" };
  short         i;
  char      line [80];
  int    variant = MODEL_NONE;


  if( unitOpened_m.handle )
  {
    for ( i = 0; i < 5; i++ )
    {
      ps6000_get_unit_info ( unitOpened_m.handle, line, sizeof (line), i );
            if (i == 3)
            {
              variant = atoi(line);
            }
      printf ( "%s: %s\n", description[i], line );
    }

    switch (variant)
        {
        case MODEL_PS3206:
            unitOpened_m.model = MODEL_PS3206;
            unitOpened_m.external = TRUE;
            unitOpened_m.signalGenerator = TRUE;
            unitOpened_m.firstRange = PS6000_100MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3206_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = DUAL_SCOPE;
            unitOpened_m.hasAdvancedTriggering = FALSE;
            unitOpened_m.hasEts = TRUE;
            unitOpened_m.hasFastStreaming = FALSE;
        break;

        case MODEL_PS3205:
            unitOpened_m.model = MODEL_PS3205;
            unitOpened_m.external = TRUE;
            unitOpened_m.signalGenerator = TRUE;
            unitOpened_m.firstRange = PS6000_100MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3205_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = DUAL_SCOPE; 
            unitOpened_m.hasAdvancedTriggering = FALSE;
            unitOpened_m.hasEts = TRUE;
            unitOpened_m.hasFastStreaming = FALSE;
        break;
                
        case MODEL_PS3204:
            unitOpened_m.model = MODEL_PS3204;
            unitOpened_m.external = TRUE;
            unitOpened_m.signalGenerator = TRUE;
            unitOpened_m.firstRange = PS6000_100MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3204_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = DUAL_SCOPE;
            unitOpened_m.hasAdvancedTriggering = FALSE;
            unitOpened_m.hasEts = TRUE;
            unitOpened_m.hasFastStreaming = FALSE;
        break;
                
        case MODEL_PS3223:
            unitOpened_m.model = MODEL_PS3223;
            unitOpened_m.external = FALSE;
            unitOpened_m.signalGenerator = FALSE;
            unitOpened_m.firstRange = PS6000_20MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3224_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = DUAL_SCOPE;
            unitOpened_m.hasAdvancedTriggering = TRUE;
            unitOpened_m.hasEts = FALSE;
            unitOpened_m.hasFastStreaming = TRUE;
        break;

        case MODEL_PS3423:
            unitOpened_m.model = MODEL_PS3423;
            unitOpened_m.external = FALSE;
            unitOpened_m.signalGenerator = FALSE;
            unitOpened_m.firstRange = PS6000_20MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3424_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = QUAD_SCOPE;                   
            unitOpened_m.hasAdvancedTriggering = TRUE;
            unitOpened_m.hasEts = FALSE;
            unitOpened_m.hasFastStreaming = TRUE;
        break;

        case MODEL_PS3224:
            unitOpened_m.model = MODEL_PS3224;
            unitOpened_m.external = FALSE;
            unitOpened_m.signalGenerator = FALSE;
            unitOpened_m.firstRange = PS6000_20MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3224_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = DUAL_SCOPE;
            unitOpened_m.hasAdvancedTriggering = TRUE;
            unitOpened_m.hasEts = FALSE;
            unitOpened_m.hasFastStreaming = TRUE;
        break;

        case MODEL_PS3424:
            unitOpened_m.model = MODEL_PS3424;
            unitOpened_m.external = FALSE;
            unitOpened_m.signalGenerator = FALSE;
            unitOpened_m.firstRange = PS6000_20MV;
            unitOpened_m.lastRange = PS6000_20V;
            unitOpened_m.maxTimebases = PS3424_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = QUAD_SCOPE;       
            unitOpened_m.hasAdvancedTriggering = TRUE;
            unitOpened_m.hasEts = FALSE;
            unitOpened_m.hasFastStreaming = TRUE;
        break;

        case MODEL_PS3225:
            unitOpened_m.model = MODEL_PS3225;
            unitOpened_m.external = FALSE;
            unitOpened_m.signalGenerator = FALSE;
            unitOpened_m.firstRange = PS6000_100MV;
            unitOpened_m.lastRange = PS6000_400V;
            unitOpened_m.maxTimebases = PS3225_MAX_TIMEBASE;
            unitOpened_m.timebases = unitOpened_m.maxTimebases;
            unitOpened_m.noOfChannels = DUAL_SCOPE;
            unitOpened_m.hasAdvancedTriggering = TRUE;
            unitOpened_m.hasEts = FALSE;
            unitOpened_m.hasFastStreaming = TRUE;
        break;

        case MODEL_PS3425:
            unitOpened_m.model = MODEL_PS3425;
            unitOpened_m.external = FALSE;
            unitOpened_m.signalGenerator = FALSE;
            unitOpened_m.firstRange = PS6000_100MV;
            unitOpened_m.lastRange = PS6000_400V;
            unitOpened_m.timebases = PS3425_MAX_TIMEBASE;
            unitOpened_m.noOfChannels = QUAD_SCOPE;                   
            unitOpened_m.hasAdvancedTriggering = TRUE;
            unitOpened_m.hasEts = FALSE;
            unitOpened_m.hasFastStreaming = TRUE;
        break;

        default:
            printf("Unit not supported");
        }

        unitOpened_m.channelSettings [PS6000_CHANNEL_A].enabled = 1;
        unitOpened_m.channelSettings [PS6000_CHANNEL_A].DCcoupled = 1;
        unitOpened_m.channelSettings [PS6000_CHANNEL_A].range = unitOpened_m.lastRange;

        unitOpened_m.channelSettings [PS6000_CHANNEL_B].enabled = 0;
        unitOpened_m.channelSettings [PS6000_CHANNEL_B].DCcoupled = 1;
        unitOpened_m.channelSettings [PS6000_CHANNEL_B].range = unitOpened_m.lastRange;


        unitOpened_m.channelSettings [PS6000_CHANNEL_C].enabled = 0;
        unitOpened_m.channelSettings [PS6000_CHANNEL_C].DCcoupled = 1;
        unitOpened_m.channelSettings [PS6000_CHANNEL_C].range = unitOpened_m.lastRange;

        unitOpened_m.channelSettings [PS6000_CHANNEL_D].enabled = 0;
        unitOpened_m.channelSettings [PS6000_CHANNEL_D].DCcoupled = 1;
        unitOpened_m.channelSettings [PS6000_CHANNEL_D].range = unitOpened_m.lastRange;
  }
  else
  {
    printf ( "Unit Not Opened\n" );
    ps6000_get_unit_info ( unitOpened_m.handle, line, sizeof (line), PS6000_ERROR_CODE );
    printf ( "%s: %s\n", description[5], line );
        unitOpened_m.model = MODEL_NONE;
        unitOpened_m.external = TRUE;
        unitOpened_m.signalGenerator = TRUE;
        unitOpened_m.firstRange = PS6000_100MV;
        unitOpened_m.lastRange = PS6000_20V;
        unitOpened_m.timebases = PS6206_MAX_TIMEBASE;
        unitOpened_m.noOfChannels = QUAD_SCOPE;   
    }
}
*/


void Acquisition3000::set_sig_gen (e_wave_type waveform, long frequency)
{

    PS3000_WAVE_TYPES waveform_ps3000 = PS3000_MAX_WAVE_TYPES;
    if (frequency < 1000 || frequency > PS3000_MAX_SIGGEN_FREQ)
    {
        ERROR("%s: Invalid frequency setted!\n",__FUNCTION__);
        return;
    }

    if (waveform < 0 || waveform >= E_WAVE_TYPE_TRIANGLE)
    {
        ERROR("%s: Invalid waveform setted!\n",__FUNCTION__);
        return;
    }
   
    switch(waveform)
    {
        case E_WAVE_TYPE_SQUARE:
            waveform_ps3000 = PS3000_SQUARE;
            break;
        case E_WAVE_TYPE_TRIANGLE:
            waveform_ps3000 = PS3000_TRIANGLE;
            break;
        case E_WAVE_TYPE_SINE:
        default:
            waveform_ps3000 = PS3000_SINE;
            break;
    }

    ps3000_set_siggen (unitOpened_m.handle,
                       waveform_ps3000,
                       (float)frequency,
                       (float)frequency,
                       0,
                       0, 
                       0,
                       0);
}

void Acquisition3000::set_sig_gen_arb (long int frequency)
{
//    char fileName [128];
//    FILE * fp;
    unsigned char arbitraryWaveform [4096];
    short waveformSize = 0;
    double delta;

    memset(&arbitraryWaveform, 0, 4096);

    if (frequency < 1000 || frequency > 10000000)
    {
        ERROR("invalid frequency %ld\n", frequency);
        return;
    }


    waveformSize = 0;

//    DEBUG("Select a waveform file to load: ");

//    scanf("%s", fileName);
//    if ((fp = fopen(fileName, "r")))
//    { // Having opened file, read in data - one number per line (at most 4096 lines), with values in (0..255)
//        while (EOF != fscanf(fp, "%c", (arbitraryWaveform + waveformSize))&& waveformSize++ < 4096)
//            ;
//        fclose(fp);
//    }
//    else
//    {
//        WARNING("Invalid filename\n");
//        return;
//    }


    delta = ((frequency * waveformSize) / 4096) * 4294967296.0 * 20e-9;
    //TODO handle generator
    (void)delta; // avoid warning while generator si not implemented
    //ps3000_set_siggen(unitOpened_m.handle, 0, 3000000, (unsigned long)delta, (unsigned long)delta, 0, 0, arbitraryWaveform, waveformSize, PS3000_UP, 0);
}

/****************************************************************************
 *
 * Select timebase, set oversample to on and time units as nano seconds
 *
 ****************************************************************************/
void Acquisition3000::set_timebase (double time_per_division)
  {
  short  i = 0;
  long   time_interval = 0;
  short  time_units = 0;
  short  oversample = 1;
  long   max_samples = 0;

  DEBUG ( "Specified timebase : %f\n", time_per_division );
  time_per_division_m = time_per_division;
  
  /* See what ranges are available...
   */
  for (i = 0; i < unitOpened_m.timebases; i++)
  {
      ps3000_get_timebase ( unitOpened_m.handle, i, BUFFER_SIZE, &time_interval, &time_units, oversample, &max_samples );
#ifdef TEST_WITHOUT_HW
      switch(i)
      {
      case 0:
          time_interval = 10;
          time_units = 0;
          break;
      case 1:
          time_interval = 10;
          time_units = 1;
          break;
      case 2:
          time_interval = 10;
          time_units = 2;
          break;
      case 3:
          time_interval = 10;
          time_units = 3;
          break;
      case 4:
          time_interval = 10;
          time_units = 4;
          break;
      case 6:
          time_interval = 100;
          time_units = 4;
          break;
      case 8:
          time_interval = 1000;
          time_units = 4;
          break;
      case 10:
          time_interval = 10000;
          time_units = 4;
          break;
      case 11:
          time_interval = 1000000;
          time_units = 4;
          break;
      default:
          time_interval = 0;
          time_units = 4;
          break;
      }


#endif
      if ( time_interval > 0 )
      {
          DEBUG ( "%d -> %ld %s  %hd\n", i, time_interval, adc_units(time_units), time_units );
          /**
           * we want 100 points per division.
           * Screen has 5 time divisions.
           * So we want 500 points
           */
          if(((double)time_interval * adc_multipliers(time_units)) <= (time_per_division * 0.010)){
              timebase = i;
          }
          else if(((double)time_interval * adc_multipliers(time_units)) > (time_per_division * 0.010)){
              break;
          }
      }
  }

#ifndef TEST_WITHOUT_HW
  ps3000_get_timebase ( unitOpened_m.handle, timebase, BUFFER_SIZE, &time_interval, &time_units, oversample, &max_samples );
#endif
  DEBUG ( "Timebase %d - %ld %s\n", timebase, time_interval, adc_units(time_units) );
  }

/****************************************************************************
 * Select coupling for all channels
 ****************************************************************************/
void Acquisition3000::set_DC_coupled(current_e coupling)
{
    short ch = 0;
    for (ch = 0; ch < unitOpened_m.noOfChannels; ch++)
    {
        unitOpened_m.channelSettings[ch].DCcoupled = coupling;
    }

}
/****************************************************************************
 * Select input voltage ranges for channels A and B
 ****************************************************************************/
void Acquisition3000::set_voltages (channel_e channel_index, double volts_per_division)
{
    uint8_t i = 0;

    DEBUG("channel index %d, volts/div %lf\n", channel_index, volts_per_division);

    if (channel_index >= unitOpened_m.noOfChannels || channel_index >=CHANNEL_MAX)
    {
        ERROR ( "%s : invalid channel index!\n", __FUNCTION__ );
        return;
    }

    if((5. * volts_per_division) > ((double)input_ranges[unitOpened_m.lastRange] / 1000.)){
        ERROR ( "%s : invalid voltage index!\n", __FUNCTION__ );
        return;
    }

    /* find the first range that includes the voltage caliber */
    for ( i = unitOpened_m.firstRange; i <= unitOpened_m.lastRange; i++ )
    {
        DEBUG ( "trying range %d -> %d mV\n", i, input_ranges[i] );
        if(((double)input_ranges[i] / 1000.) >= (5. * volts_per_division))
        {
            unitOpened_m.channelSettings[channel_index].range = i;
            break;
        }
    }

    if(unitOpened_m.channelSettings[channel_index].range != CHANNEL_OFF)
    {
        DEBUG ( "Channel %c has now range %d mV\n", 'A' + channel_index, input_ranges[unitOpened_m.channelSettings[channel_index].range]);
        unitOpened_m.channelSettings[channel_index].enabled = TRUE;
    }
    else
    {
        DEBUG ( "Channel %c Switched off\n", 'A' + channel_index);
        unitOpened_m.channelSettings[channel_index].enabled = FALSE;
    }

}
/****************************************************************************
 * adc_to_mv
 *
 * If the user selects scaling to millivolts,
 * Convert an 12-bit ADC count into millivolts
 ****************************************************************************/
int Acquisition3000::adc_to_mv (long raw, int ch)
{
      return ( scale_to_mv ) ? ( raw * input_ranges[ch] ) / 32767 : raw;
}

/****************************************************************************
 * mv_to_adc
 *
 * Convert a millivolt value into a 12-bit ADC count
 *
 *  (useful for setting trigger thresholds)
 ****************************************************************************/
short Acquisition3000::mv_to_adc (short mv, short ch)
{
  return ( ( mv * 32767 ) / input_ranges[ch] );
}

#endif // HAVE_LIBPS3000
