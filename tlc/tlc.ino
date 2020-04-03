///
/// \file 		tlc.ino
/// \brief 		The Lung Carburetor Firmware main source file
///				
/// \author 	Frederic Lauzon
/// \defgroup 	main Main program


#include "common.h"
#include "safeties.h"
#include "communications.h"
#include "control.h"
#include "datamodel.h"
#include "sensors.h"
#include "gpio.h"
#include "configuration.h"
#include "lcd_keypad.h"

static uint32_t gStartTick = 0;

void setup() 
{
    wdt_enable(WDTO_4S);
    
    GPIO_Init();    

    DataModel_Init();
        
    Communications_Init();

    Sensors_Init();

    Control_Init();

    Safeties_Init();

#ifdef ENABLE_LCD
    LcdKeypad_Init();
#endif
        
    bool cfgSuccess = Configuration_Init();
    
    // If loading fails, we are probably using an uninitialized device, or we are faced with eeprom corruption, force safety error after warmup.
    if (!cfgSuccess)
    {
        gSafeties.bEnabled              = true;
        gSafeties.bConfigurationInvalid = true; 
        Serial.println("DEBUG: loading failed.");
    }
        
    gDataModel.nState   = kState_Warmup;
    gStartTick          = millis();

	digitalWrite(PIN_OUT_BUZZER, LOW);
    delay(100);
    digitalWrite(PIN_OUT_BUZZER, HIGH);
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
        digitalWrite(PIN_OUT_BUZZER, HIGH);
		
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
        digitalWrite(PIN_OUT_BUZZER, LOW);
	       
        // Stay in error until recovery
		if (!gSafeties.bCritical)
		{			
			gDataModel.nState   = kState_Warmup;
			gStartTick          = millis();
		}
        break;
        
    default:
        break;
    };
    
    if ((millis() - gDataModel.nTickCommunications) >= kPeriodCommunications)
    {
        gDataModel.nTickCommunications = millis();
		Communications_Process();        
    }
        
    if ((millis() - gDataModel.nTickSensors) >= kPeriodSensors)
    {
        gDataModel.nTickSensors = millis();
		Sensors_Process();        
    }
    
    if ((millis() - gDataModel.nTickControl) >= kPeriodControl)
    {
        gDataModel.nTickControl = millis();
		Control_Process();        
    }

#ifdef ENABLE_LCD
    if ((millis() - gDataModel.nTickLcdKeypad) >= kPeriodLcdKeypad)
    {
        gDataModel.nTickLcdKeypad = millis();
		LcdKeypad_Process();
    }
#endif
    
    Safeties_Process();
}
