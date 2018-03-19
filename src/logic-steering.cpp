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

#include "cluon-complete.hpp"
#include "logic-steering.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>
//#include <stdlib.h>

float Steering::decode(const std::string &data) noexcept {
    std::cout << "Got data:" << data << std::endl;
    float temp = std::stof(data);
    return temp;
}

Steering::Steering(float pconst, float iconst, float tolerance)
    : m_debug(1)
    , m_initialised()
    , m_groundSteeringRequest()
    , m_steeringCurrentDuty()
    //, m_steerLeft(1)
    , m_steerRight(1)
    //, m_steerSelect(0)
    , m_steerCurrent()
    , m_steerPosition()
    , m_steerVoltage()
    , m_iControlOld()
    , m_pConst(pconst)
    , m_iConstTI(iconst)
    , m_tolerance(tolerance)

{
	Steering::setUp();
}

Steering::~Steering() 
{
  Steering::tearDown();
}

void Steering::callOnReceive(cluon::data::Envelope data){
    if (!m_initialised) {
        return;
    }
    if (data.dataType() == static_cast<int32_t>(opendlv::proxy::GroundSteeringRequest::ID())) {
        opendlv::proxy::GroundSteeringRequest steeringReq = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(data));
        if (steeringReq.groundSteering() > -21 && steeringReq.groundSteering() < 21)
            m_groundSteeringRequest = steeringReq.groundSteering();

        std::cout << "[LOGIC-STEERING] Steering Request:" << m_groundSteeringRequest << std::endl;
    }else if (data.dataType() == static_cast<int32_t>(opendlv::proxy::VoltageReading::ID())) {

        if (data.senderStamp() == m_analogPinSteerCurrent){
          opendlv::proxy::VoltageReading analogInput = cluon::extractMessage<opendlv::proxy::VoltageReading>(std::move(data));
          m_steerCurrent = analogInput.torque()/((float) m_analogConvSteerCurrent);
          // std::cout << "[LOGIC-STEERING-CURRENT] Current reading:" << m_steerCurrent << std::endl;
        }else if (data.senderStamp() == m_analogPinSteerPosition){
          opendlv::proxy::VoltageReading analogInput = cluon::extractMessage<opendlv::proxy::VoltageReading>(std::move(data));
          m_steerPosition = analogInput.torque()/((float) m_analogConvSteerPosition)-((float) m_analogOffsetSteerPosition);
          // std::cout << "[LOGIC-STEERING-POSITION] Position reading:" << m_steerPosition << std::endl;
        }else if (data.senderStamp() == m_analogPinSteerVoltage){
          opendlv::proxy::VoltageReading analogInput = cluon::extractMessage<opendlv::proxy::VoltageReading>(std::move(data));
          m_steerVoltage = analogInput.torque()/((float) m_analogConvSteerVoltage);
         // std::cout << "[LOGIC-STEERING-VOLTAGE] Voltage reading:" << m_steerVoltage << std::endl;
        }
    }

    

}

void Steering::body(cluon::OD4Session &od4)
{
        
    float steerError = m_groundSteeringRequest-m_steerPosition;
    float pControl = m_pConst * steerError;
    float iControl = 0;

    if (abs((int) pControl) < 10000){
          iControl = (float) (pControl*m_iConstTS/m_iConstTI+m_iControlOld);
    } 

    if (iControl > 25000){
        iControl = 25000;
    } else if(iControl < -25000){
        iControl = -25000;
    }

    m_iControlOld = iControl;

    float controlSignal = pControl + iControl;

    m_steeringCurrentDuty = (uint32_t) abs((int)round(controlSignal));

    if (m_steeringCurrentDuty > 50000){
        m_steeringCurrentDuty = 50000;
    }

    m_steerRight = controlSignal < 0;
     
    if(steerError > -m_tolerance && steerError < m_tolerance) {
      m_steeringCurrentDuty = 0;
      iControl = 0;
    }

     std::cout << "[LOGIC-STEERING] Error: " << steerError 
				<< "\t Duty: " << m_steeringCurrentDuty 
                << "\t pControl: " << pControl 
                << "\t iControl: " << iControl 
				<< "\t Direction: " << m_steerRight
				<< "\t Request: " << m_groundSteeringRequest
				<< "\t Measure: " << m_steerPosition 
				<< std::endl;

   
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    cluon::data::TimeStamp sampleTime = cluon::time::convert(tp);
    int16_t senderStamp = 0;
    
    opendlv::proxy::SwitchStateRequest msgGpio;

    senderStamp = m_gpioPinSteerRight;
    msgGpio.state(m_steerRight);
    od4.send(msgGpio, sampleTime, senderStamp);


    opendlv::proxy::PulseWidthModulationRequest msgPwm;
 
    senderStamp = m_pwmPinSteer;
    msgPwm.dutyCycleNs(m_steeringCurrentDuty);
    od4.send(msgPwm, sampleTime, senderStamp);


}

void Steering::setUp()
{
  m_initialised = true;
}

void Steering::tearDown()
{
}
