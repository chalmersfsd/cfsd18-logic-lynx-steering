/*
 * Copyright (C) 2018  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PWM
#define PWM

#include "opendlv-standard-message-set.hpp"

#include <memory>
#include <string>
#include <vector>
#include <utility>

class Steering {
   private:
    //Steering(const Steering &) = delete;
    //Steering(Steering &&)      = delete;
    //Steering &operator=(const Steering &) = delete;
    //Steering &operator=(Steering &&) = delete;

   public:
    Steering(float pconst, float iconst, float tolerance);
    ~Steering();

   public:
    float decode(const std::string &data) noexcept;
    void callOnReceive(cluon::data::Envelope data);
    void body(cluon::OD4Session &od4);

   private:
    void setUp();
    void tearDown();

    bool m_debug;
    bool m_initialised;
    float m_groundSteeringRequest;
    uint32_t m_steeringCurrentDuty;
    //bool m_steerLeft;
    bool m_steerRight;
    //bool m_steerSelect;
    float m_steerCurrent;
    float m_steerPosition;
    float m_steerVoltage;
    float m_iControlOld;
    float m_pConst;
    float m_iConstTI;
    float m_tolerance;

    //const uint16_t m_gpioPinSteerLeft = 47;
    const uint16_t m_gpioPinSteerRight = 46;
    //const uint16_t m_gpioPinSteerSelect = 26;
    const uint16_t m_pwmPinSteer = 40;
    const uint16_t m_analogPinSteerCurrent = 4;
    const uint16_t m_analogPinSteerPosition = 0;
    const uint16_t m_analogPinSteerVoltage = 6;
    const double m_analogConvSteerCurrent = 1;
    const double m_analogConvSteerPosition = 82.362;
    const double m_analogConvSteerVoltage = 1;
    const double m_analogOffsetSteerPosition = 25;
    const double m_iConstTS = 0.03;
    
};

#endif

