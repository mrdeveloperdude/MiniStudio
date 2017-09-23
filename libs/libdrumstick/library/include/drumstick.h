/*
    MIDI Sequencer C++ library 
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; if not, write to the Free Software Foundation, Inc., 
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.    
*/

#ifndef DRUMSTICK_H
#define DRUMSTICK_H
#include <QtCore>
/**
 * @file drumstick.h
 * The main header that a program can include to use all drumstick features.
 */

#if defined(Q_OS_LINUX)
// ALSA library interface
#include <drumstick/alsaclient.h>
#include <drumstick/alsaevent.h>
#include <drumstick/alsaport.h>
#include <drumstick/alsaqueue.h>
#include <drumstick/alsatimer.h>
#include <drumstick/drumstickcommon.h>
#include <drumstick/playthread.h>
#include <drumstick/subscription.h>
#endif

// File formats
#include <drumstick/qsmf.h>
#include <drumstick/qwrk.h>
#include <drumstick/qove.h>

// RealTime interfaces
#include <drumstick/rtmidiinput.h>
#include <drumstick/rtmidioutput.h>
#include <drumstick/backendmanager.h>

#endif /*DRUMSTICK_H*/
