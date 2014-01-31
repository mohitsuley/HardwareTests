int led = 9;
int fadeAmount = 5;
int brightness = 0;


void setup()
{

  /* add setup code here */
	pinMode(led,OUTPUT);
}

void loop()
{

  /* add main program code here */

	analogWrite(led, brightness);
	
	brightness = brightness + fadeAmount;
	if (brightness == 0 || brightness == 255)
		fadeAmount = -fadeAmount;


	delay(30);

	/*digitalWrite(led, HIGH);
	delay(1000);
	digitalWrite(led, LOW);
	delay(1000);*/
}
