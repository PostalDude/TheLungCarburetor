#include "PressureProfile.h"
#include <iostream>


PressureProfile::PressureProfile(float time, float ratio, float pMax, float pMin)
{
    m_bIsValid = true;
    SetCycleTime(time);
    SetInhaleRatio(ratio);
    SetMinimumPressure(pMin);
    SetMaximumPressure(pMax);
    CalcTimeStamps();
    CalcSlopes();
}

PressureProfile::PressureProfile(const float time, const float partInhale, const float partExhale, const float pMax, const float pMin)
{
    m_bIsValid = true;
    SetCycleTime(time);
    SetInhaleExhaleProporitions(partInhale, partExhale);
    SetMinimumPressure(pMin);
    SetMaximumPressure(pMax);
    CalcTimeStamps();
    CalcSlopes();
}

void PressureProfile::SetCycleTime(const float time)
{
    if (time >0)
    {
        m_fCycleTime = time;
        
    }
    else
    {
        std::cout << "Cycle time must be larger than 0" << std::endl;
        m_bIsValid = false;
    }
    
}

void PressureProfile::SetInhaleRatio(const float ratio)
{
    if ((ratio>0) && (ratio<1))
    {
        m_fInhaleRatio = ratio;
    }
    else
    {
        std::cout << "Inhale ratio must be strictly between 0 and 1" << std::endl;
        m_bIsValid = false; 
    }   
}

void PressureProfile::SetInhaleExhaleProporitions(const float inhalePart, const float exhalePart)
{
    if ((inhalePart>0) && (exhalePart>0))
    {
        float total = inhalePart + exhalePart;
        m_fInhaleRatio = inhalePart/total;
    }
    else
    {
        m_bIsValid = false;
    }
    
}

void PressureProfile::SetMinimumPressure(const float pMin)
{
    if (pMin >0)
    {
        m_fPmin = pMin;
    }
    else
    {
       std::cout << "Minimum pressure must be strictly positive" << std::endl;
       m_bIsValid = false;
    }
    
}

void PressureProfile::SetMaximumPressure(const float pMax)
{
    if (pMax > m_fPmin)
    {
        m_fPmin = pMax;
    }
    else
    {
        std::cout << "Maximum pressure must be larger than minimum pressure" << std::endl;
        m_bIsValid = false; 
    }
      
}

void PressureProfile::CalcTimeStamps()
{
    m_fTimeStamps[3] = m_fCycleTime;
    m_fTimeStamps[1] = m_fInhaleRatio*m_fCycleTime;
    m_fTimeStamps[0] = m_fTimeStamps[1]/2;
    m_fTimeStamps[2] = m_fTimeStamps[1] + (m_fTimeStamps[3] - m_fTimeStamps[1])/2;
}

void PressureProfile::CalcSlopes()
{
    m_fPositiveSlope = (m_fPmax - m_fPmin)/m_fTimeStamps[0];
    m_fNegativeSlope = (m_fPmax - m_fPmin)/(m_fTimeStamps[2] - m_fTimeStamps[1]);
    //TODO: Validate values vs hardware limits
}

const float PressureProfile::EvalProfile(const float t)
{
    if (t<0)
    {
        float newTime = fmod(t, m_fCycleTime) + m_fCycleTime;
        return EvalProfile(newTime);
    }
    else if((t>=0) && (t< m_fTimeStamps[0]))
    {
        return m_fPmin + m_fPositiveSlope*t;
    }
    else if (( t>= m_fTimeStamps[0]) && (t < m_fTimeStamps[1]))
    {
        return m_fPmax;
    }
    else if ((t >= m_fTimeStamps[1]) && (t < m_fTimeStamps[2]))
    {
        return m_fPmax - m_fPositiveSlope*(t - m_fTimeStamps[1]);
    }
    else if ((t >= m_fTimeStamps[2]) && (t < m_fTimeStamps[3]))
    {
        return m_fPmin;
    }
    else
    {
        float newTime = fmod(t, m_fCycleTime);
        return EvalProfile(newTime);
    }
    
}

void PressureProfile::Revalidate()
{
    m_bIsValid = m_fCycleTime > 0;
    m_bIsValid = m_bIsValid && ((m_fInhaleRatio>0) && (m_fInhaleRatio <1));
    m_bIsValid = m_bIsValid && (m_fPmin > 0);
    m_bIsValid = m_bIsValid && (m_fPmax > m_fPmin);
}
