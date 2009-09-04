/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000 - 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "NMEA/Info.h"
#include "NMEA/Derived.hpp"

class BaseBlackboard {
  // all blackboards can be read as const
public:
  const NMEA_INFO& Basic() const { return gps_info; }
  const DERIVED_INFO& Calculated() const { return calculated_info; }
protected:
  NMEA_INFO     gps_info;
  DERIVED_INFO  calculated_info;
};


class GlideComputerBlackboard: public BaseBlackboard {
public:
  void ReadBlackboard(const NMEA_INFO &nmea_info);
protected:
  void ResetFlight(const bool full=true);
  void StartTask();
  void Initialise();
  void SaveFinish();
  void RestoreFinish();

  virtual const double GetAverageThermal() const;
  virtual void OnClimbBase(double StartAlt) = 0;
  virtual void OnClimbCeiling() = 0;
  virtual void OnDepartedThermal() = 0;

  // only the glide computer can write to calculated
  DERIVED_INFO& SetCalculated() { return calculated_info; }
private:
  DERIVED_INFO Finish_Derived_Info;
};


class DeviceBlackboard: public BaseBlackboard {
public:
  void Initialise();
  void ReadBlackboard(const DERIVED_INFO &derived_info);

  // only the device blackboard can write to gps
  friend class ComPort;
protected:
  NMEA_INFO& SetBasic() { return gps_info; }
public:
  void SetStartupLocation(double lon, double lat, double alt);
  // used by replay logger
  void SetLocation(double lon, double lat, double speed, double bearing,
		   double alt, double baroalt, double t);
  void ProcessSimulation();
  void StopReplay();
  void FLARM_RefreshSlots();
  void SetBaroAlt(double x) {
    SetBasic().BaroAltitude = x;
  }
  void SetNAVWarning(bool val);
  void SetTrackBearing(double val);
  void SetSpeed(double val);
};


class MapWindowBlackboard: public BaseBlackboard {
protected:
  virtual void ReadBlackboard(const NMEA_INFO &nmea_info,
			      const DERIVED_INFO &derived_info);
};


class InterfaceBlackboard: public BaseBlackboard {
public:
  void ReadBlackboardBasic(const NMEA_INFO &nmea_info);
  void ReadBlackboardCalculated(const DERIVED_INFO &derived_info);
};


class InstrumentBlackboard {
protected:
  static const NMEA_INFO& Basic() { return blackboard.Basic(); }
  static const DERIVED_INFO& Calculated() { return blackboard.Calculated(); }
public:
  static void ReadBlackboardBasic(const NMEA_INFO& nmea_info) {
    blackboard.ReadBlackboardBasic(nmea_info);
  }
  static void ReadBlackboardCalculated(const DERIVED_INFO& derived_info) {
    blackboard.ReadBlackboardCalculated(derived_info);
  }
private:
  static InterfaceBlackboard blackboard;
};

// master flight data
//extern NMEA_INFO GPS_INFO;
//extern DERIVED_INFO CALCULATED_INFO;

// gps detection
extern BOOL GPSCONNECT;
extern BOOL VARIOCONNECT;

extern BOOL extGPSCONNECT;

extern DeviceBlackboard device_blackboard;

#endif
