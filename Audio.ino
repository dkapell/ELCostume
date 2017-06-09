float incomingAverage = 128.f;

float getAudioValue(void){
  int incomingAudio = analogRead(A2);
  incomingAverage = rollingAverage(incomingAudio);
  if (averageReady){
    float value = (float)incomingAudio - incomingAverage;
    return(value);
  } else {
    return 0;
  }
}

boolean readAudio(void) { 
  unsigned long time = micros();
  float sample, value, envelope, beat, thresh;
  unsigned char i;
  for (i = 0; ; i++){
    sample = getAudioValue();

    value = bassFilter(sample);

    if (value < 0) value = -value;
    envelope = envelopeFilter(value);
    
    if (i = 200){
      beat = beatFilter(envelope);
      //Serial.println(beat);
      thresh = 0.10f;

      return (beat > thresh);
    }
     // Consume excess clock cycles, to keep at 5000 hz
    for(unsigned long up = time+200; time > 20 && time < up; time = micros());
  }
}

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 9.1f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { //10hz low pass
    static float xv[2] = {0,0}, yv[2] = {0,0};
    xv[0] = xv[1]; 
    xv[1] = sample / 160.f;
    yv[0] = yv[1]; 
    yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
    return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 7.015f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
    return yv[2];
}
