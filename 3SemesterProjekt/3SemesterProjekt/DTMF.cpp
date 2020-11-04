#include "DTMF.h"
DTMF::DTMF() {
    for (int i = 0; i < 4; i++) {
        goertzelH[i] = new Goertzel(tonesH[i], SAMPLING_RATE);
        goertzelL[i] = new Goertzel(tonesL[i], SAMPLING_RATE);
    }
}
void DTMF::sendSequence(vector<char>& sequence)
{
    for (char tone : sequence) {
       sendTone(tone);
    }
}
char DTMF::listenTone()
{
    return receiveDTMF();
    
}
void DTMF::sendTone(char tonevalg) {
    
    vector<sf::Int16> dtmf;
    const double incrementL = tonesL[tonevalg % 4] / 44100;
    const double incrementH= tonesH[tonevalg / 4 - 1] / 44100;
    double x = 0;
    double y = 0;
    int antalSamples = (int)(SAMPLING_RATE * (TONE_DURATION / 1000.0));
    for (unsigned i = 0; i < antalSamples; i++) {
        dtmf.push_back(AMPLITUDE * sin(x * PI) + AMPLITUDE * sin(y * PI));
        x += incrementL;
        y += incrementH;
    };
      
       
    sf::SoundBuffer buffer;

    buffer.loadFromSamples(&dtmf[0],dtmf.size(), 2, 44100);
    sf::Sound Sound;

    Sound.setBuffer(buffer);
    Sound.setLoop(true);
    Sound.play();
    sf::sleep(sf::milliseconds(TONE_DURATION));
}
char DTMF::receiveDTMF()
{
    
    vector<float> goertzelresL;
    vector<float> goertzelresH;
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds
    std::vector<std::string> availableDevices = sf::SoundRecorder::getAvailableDevices();
    std::string inputDevice = availableDevices[0];
    sf::SoundBufferRecorder recorder;
    recorder.start();
    sleep_for(milliseconds(LISTEN_DURATION));
    sleep_until(system_clock::now() + milliseconds(LISTEN_DURATION));
    recorder.stop();
    const sf::SoundBuffer& buffer = recorder.getBuffer();
    buffer.saveToFile("my_record.ogg");
    /*sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("sin1633.wav")) {
        cout << "Load failed!" << endl;
        return;
    }*/
    const sf::Int16* samples = buffer.getSamples();
    std::size_t count = buffer.getSampleCount();

    for (Goertzel* g : goertzelL) {
        cout << g->processSamples(samples, count) << " ";
        goertzelresL.push_back(g->processSamples(samples, count));
    }
    for (Goertzel* g : goertzelH) {
        cout << g->processSamples(samples, count) << " ";
        goertzelresH.push_back(g->processSamples(samples, count));
    }
    cout << endl;

    return determineDTMF(goertzelresL, goertzelresH);
    
}
 
char DTMF::determineDTMF(vector<float> goertzelresL, vector<float> goertzelresH)
{
     int pos1=0, pos2=0;
     float  largest = 0, second_largest = 0;
     float stoej[6] = {};
     int stoeji=0;
     int res = 0;
    //finding Largest second largset
    for (int i = 0; i < goertzelresH.size(); i++)
    {
        if (goertzelresL[i] > largest)
        {
            largest = goertzelresL[i];
            pos1 = i;
        }
        else
        {
            stoej[stoeji] = goertzelresL[i];
            stoeji++;
        }
        if (goertzelresH[i] > second_largest)
        {
            second_largest = goertzelresH[i];
            pos2 = i;

         }
        {
            stoej[stoeji];
            stoeji++;
        }
    }
    float P_Signal = (largest + second_largest) / 2;
    float sum=0.0;
    for (int i = 0; i < 6; i++)
    {
        sum += stoej[i];
    }
    float P_stoej = sum / 6.0;

    float SNR = 10 * log10(P_Signal/P_stoej);

    if (SNR>DBthreshhold)
    {
        res= (pos1 + pos2) * 4;
        return (pos1 + pos2) * 4;
    }
    else
    {
        res = -1;
        return -1;
    }
    /*cout << "nn Largest Number :" << largest << " at position " << (pos1 + 1);
    cout << "nn Second Largest Number :" << second_largest << " at position " << (pos2 + 4) << endl;*/
    
}




DTMF::~DTMF() {};
