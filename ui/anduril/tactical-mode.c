// tactical-mode.c: Tactical (ish) mode for Anduril.
// Copyright (C) 2023 Selene ToyKeeper
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "anduril/tactical-mode.h"


uint8_t tactical_state(Event event, uint16_t arg) {
    // momentary(ish) tactical mode
    uint8_t mem_lvl = memorized_level;  // save this to restore it later
    uint8_t ret = EVENT_NOT_HANDLED;
    #if NUM_CHANNEL_MODES > 1
    uint8_t prev_ch = channel_mode; // save channel mode from entering tactical mode
    #endif

    // button is being held
    if ((event & (B_CLICK | B_PRESS)) == (B_CLICK | B_PRESS)) {
        // 1H: 1st level
        // 2H: 2nd level
        // 3H: 3rd level
        // 4+: nothing
        momentary_active = 0;
        ret = EVENT_HANDLED;
        uint8_t click = event & 0x0f; // click number
        if (click <= 3) {
            momentary_active = 1;
            uint8_t lvl;
            lvl = cfg.tactical_levels[click-1];
            if ((1 <= lvl) && (lvl <= RAMP_SIZE)) {  // steady output
                memorized_level = lvl;
                momentary_mode = 0;
                #if NUM_CHANNEL_MODES > 1
                    // use configured channel. tactical mode config is Level/Channel as LLLCCC
                    // click                            1 2 3
                    // tactical_levels[] level slot     0 1 2
                    // tactical_levels[] channel slot   3 4 5
                    if (
                        (cfg.tactical_levels[click+2]) // if configured, i.e. not zero
                        && (lvl <= RAMP_SIZE) // don't mess with channel at all if it's a strobe mode
                                              // (strobes have per-channel config already, although setting a channel per
                                              // strobe in tactical mode is feasible for all but police strobe if we *wanted*
                                              // the extra complexity? (TODO: How would these two interact?)(
                        ){
                        channel_mode = (cfg.tactical_levels[click+2] - 1); // -1 is so that entering value '1' selects
                                                                           // the first channel mode (0-indexed)
                    } else {
                        // we need to set the channel mode *back* to prev_ch because by passing through earlier slots
                        // to reach slot 2 or 3, channel mode gets reset if there are modes configured for either of
                        // those slots, so reset it to be safe.
                        channel_mode = prev_ch;
                    }
                #endif
            } else {  // momentary strobe mode
                momentary_mode = 1;
                if (lvl > RAMP_SIZE) {
                    current_strobe_type = (lvl - RAMP_SIZE - 1) % strobe_mode_END;
                }
            }
        }
    }
    // button was released
    else if ((event & (B_CLICK | B_PRESS)) == (B_CLICK)) {
        momentary_active = 0;
        set_level(0);
        #if NUM_CHANNEL_MODES > 1
        channel_mode = prev_ch; // reset mode. This technically doesn't need to be set when the configured
                                // mode selection is zero (i.e. follow active) but it would waste space
                                // to add a guard condition for zero benefit, as prev_ch is always set
                                // on entering this state and active channel mode can not be changed within,
                                // so there's no way to end up potentially clobbering it in that case.
        #endif
        interrupt_nice_delays();  // stop animations in progress
    }

    // delegate to momentary mode while button is pressed
    if (momentary_active) {
        momentary_state(event, arg);
    }

    memorized_level = mem_lvl;  // restore temporarily overridden mem level

    // copy lockout mode's aux LED and sleep behaviors
    if (event == EV_enter_state) {
        lockout_state(event, arg);
    }
    else if (event == EV_tick) {
        if (! momentary_active) {
            return lockout_state(event, arg);
        }
        return EVENT_HANDLED;
    }
    else if (event == EV_sleep_tick) {
        return lockout_state(event, arg);
    }

    // handle 3C here to prevent changing channel modes unintentionally
    if (event == EV_3clicks) {
        return EVENT_HANDLED;
    }

    // 6 clicks: exit and turn off
    else if (event == EV_6clicks) {
        blink_once();
        set_state(off_state, 0);
        return EVENT_HANDLED;
    }

    ////////// Every action below here is blocked in the simple UI //////////
    // (unnecessary since this entire mode is blocked in simple UI)
    /*
    #ifdef USE_SIMPLE_UI
    if (cfg.simple_ui_active) {
        return EVENT_NOT_HANDLED;
    }
    #endif
    */

    // 7H: configure tactical mode
    else if (event == EV_click7_hold) {
        push_state(tactical_config_state, 0);
        return EVENT_HANDLED;
    }

    return ret;
}

void tactical_config_save(uint8_t step, uint8_t value) {
    // update tac mode values
    // 6 values
    // first three: each value is 1 to 150, or other:
    // - 1..150 is a ramp level
    // - other means "strobe mode"
    // second three: index of the channel mode to use
    // only use the level slots on lights with only a single channel

    #if NUM_CHANNEL_MODES > 1
    // naively accepting any value for channel mode above the real number that exist will
    // cause the light to act erratically if a higher number is selected and used.
    // in any case, we need to increment the entered value by 1 for the channel mode selected
    // so an entry of 0 has its specific function of following the active channel mode.
    if ((step < 4)||(value <= NUM_CHANNEL_MODES)){
      cfg.tactical_levels[step - 1] = value;
    }
    #else
    cfg.tactical_levels[step - 1] = value;
    #endif
}

uint8_t tactical_config_state(Event event, uint16_t arg) {
    #if NUM_CHANNEL_MODES > 1
    // 6 slots for Level/Channel as LLLCCC
    return config_state_base(event, arg, 6, tactical_config_save);
    #else
    // 3 slots only (LLL)
    return config_state_base(event, arg, 3, tactical_config_save);
    #endif
}


