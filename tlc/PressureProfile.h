#ifndef PRESSUREPROFILE_H
#define PRESSUREPROFILE_H



class PressureProfile
{
    /*PressureProfiles computes a trapezoidal profile of a given amplitude and duration. Profile is defined with inhaling first*/
public:
    PressureProfile(const float time, const float ratio, const float pMax, const float pMin); //ratio = timeExhale / totalCyleTime
    PressureProfile( const float time, const float partInhale, const float partExhale, const float pMax, const float pMin); // if ratio is expressed as partInhale:partExhale

    //Getters
    const float GetCycleTime()        {return m_fCycleTime;};
    const float GetInhaleRatio()      {return m_fInhaleRatio;};
    const float GetMaximumPressure()  {return m_fPmax;};
    const float GetMinimumPressure()  {return m_fPmin;};
    const bool  IsValid()             {return m_bIsValid};

    //Setters
    void SetCycleTime( const float t);
    void SetInhaleRatio( const float ratio);
    void SetInhaleExhaleProporitions( const float inhalePart, float exhalePart);
    void SetMaximumPressure( const float pMax);
    void SetMinimumPressure( const float pMin);

    
    const float EvalProfile( const float t); //Evaluate pressure at instant t
    void CalcTimeStamps(); //Recompute after setting cycle time or ratio
    void CalcSlopes(); //Recompute after setting  any value
    void Revalidate(); //Validates all members and sets m_bIsValid back to true if valid
    


protected:
    float m_fCycleTime;
    float m_fInhaleRatio;
    float m_fPmax;
    float m_fPmin;
    float m_fPositiveSlope;
    float m_fNegativeSlope;
    float m_fTimeStamps[4];
    bool  m_bIsValid; 
}

#endif //PRESSUREPROFILE_H
