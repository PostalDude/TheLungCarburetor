
#include "common.h"
#include "safeties.h"
#include "communications.h"
#include "control.h"
#include "datamodel.h"
#include "sensors.h"
#include "gpio.h"
#include "configuration.h"

static uint32_t gStartTick = 0;

void setup() 
{
    wdt_enable(WDTO_4S);
    
    GPIO_Init();    

    DataModel_Init();
    gDataModel.nState = kState_Init;
    
    Communications_Init();

    Sensors_Init();

    Control_Init();

    Safeties_Init();
        
    bool cfgSuccess = Configuration_Init();
    
    // If loading fails, we are probably using an uninitialized device, or we are faced with eeprom corruption, force safety error after warmup.
    if (!cfgSuccess)
    {
        gSafeties.bEnabled              = true;
        gSafeties.bConfigurationInvalid = true; 
    }
        
    gDataModel.nState   = kState_Warmup;
    gStartTick          = millis();
    
    tone(PIN_OUT_PWM_BUZZER, 500); 
    delay(100);
    tone(PIN_OUT_PWM_BUZZER, 300); 
    delay(100);
}

// Main processing loop
void loop() 
{   
    wdt_reset();

    // Process state machine
    switch (gDataModel.nState)
    {
    case kState_Idle:       
    case kState_Init:
        // Shouldn't happen, reboot (watchdog not refreshed, forces reboot)
        while(1);   
        break;
        
    // Warmup system and sensors to have valid readings when going in process state
    case kState_Warmup:
        tone(PIN_OUT_PWM_BUZZER, 0); 
        if ((millis() - gStartTick) >= kPeriodWarmup)
        {
            gDataModel.nState = kState_Process;
        }
        break;
        
    // Normal processing
    case kState_Process:
        break;
        
    // Error state in case of safeties issues
    case kState_Error:
        tone(PIN_OUT_PWM_BUZZER, 2000); 
        
        // Stay in error until recovery
        if (gSafeties.bEnabled)
        {
            if (!gSafeties.bCritical)
            {
                Safeties_Clear();
                gDataModel.nState   = kState_Warmup;
                gStartTick          = millis();
            }
        }
        break;
        
    default:
        break;
    };
    
    // Peripheral processing
    GPIO_Process();
                
    DataModel_Process();
    
    if ((millis() - gDataModel.nTickCommunications) >= kPeriodCommunications)
    {
        Communications_Process();
        gDataModel.nTickCommunications = millis();
    }
        
    if ((millis() - gDataModel.nTickSensors) >= kPeriodSensors)
    {
        Sensors_Process();
        gDataModel.nTickSensors = millis();
    }
    
    if ((millis() - gDataModel.nTickControl) >= kPeriodControl)
    {
        Control_Process();
        gDataModel.nTickControl = millis();
    }
    
    Safeties_Process();
}
