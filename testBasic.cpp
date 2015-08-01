
#define HEART_BEAT_LED 8


void setup(void)
{
    pinMode(HEART_BEAT_LED, OUTPUT);
}

void loop(void)
{
	digitalWrite(HEART_BEAT_LED, HIGH);
        delay(1000);
        digitalWrite(HEART_BEAT_LED, LOW);
        delay(1000);
}